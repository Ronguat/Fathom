#include "Net/FMTrace.h"

#include "Core/FMPlayerController.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Fathom.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "NetworkPredictionWorldManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Package.h"

namespace
{
	constexpr double CostPeriodSeconds = 1.0;
	constexpr double MetaPeriodSeconds = 10.0;
	constexpr int32 RelayBatchSize = 64;

	FString ComputeWorldTag(const UWorld* World)
	{
		if (World->GetNetMode() != NM_Client)
		{
			return TEXT("S");
		}
		int32 Index = World->GetOutermost()->GetPIEInstanceID();
		if (Index < 0 && !FParse::Value(FCommandLine::Get(), TEXT("FMClient="), Index))
		{
			Index = 1;
		}
		return FString::Printf(TEXT("C%d"), Index);
	}

	FString ReadCommit()
	{
		const FString GitDir = FPaths::ProjectDir() / TEXT(".git");
		FString Head;
		if (!FFileHelper::LoadFileToString(Head, *(GitDir / TEXT("HEAD"))))
		{
			return FString();
		}
		Head.TrimStartAndEndInline();
		if (!Head.StartsWith(TEXT("ref: ")))
		{
			return Head;
		}
		FString Hash;
		FFileHelper::LoadFileToString(Hash, *(GitDir / Head.Mid(5)));
		Hash.TrimStartAndEndInline();
		return Hash;
	}

	FAutoConsoleCommandWithWorldAndArgs GFMMark(
		TEXT("FM.Mark"), TEXT("Writes a MARK trace line: FM.Mark <category>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UFMTraceLibrary::Mark(World, Args.Num() ? Args[0] : TEXT("console"));
		}));

	FAutoConsoleCommandWithWorld GFMBundle(
		TEXT("FM.Bundle"), TEXT("Rewrites this world's session meta.json now"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			UFMTraceLibrary::WriteBundle(World);
		}));
}

UFMTraceSubsystem* UFMTraceSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFMTraceSubsystem>() : nullptr;
}

void UFMTraceSubsystem::Trace(const UObject* WorldContext, const FString& Body)
{
	if (UFMTraceSubsystem* Subsystem = Get(WorldContext))
	{
		Subsystem->Write(Body);
	}
}

bool UFMTraceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFMTraceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WorldTag = ComputeWorldTag(GetWorld());
	Commit = ReadCommit();
	SessionDir = FPaths::ProjectSavedDir() / TEXT("Fathom/Sessions")
		/ (GetWorld()->WorldType == EWorldType::PIE ? TEXT("PIE") : TEXT("Field"))
		/ (FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")) + TEXT("-") + WorldTag);
	TickStartHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &UFMTraceSubsystem::OnTickStart);
	TickEndHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &UFMTraceSubsystem::OnTickEnd);
	LastCostSeconds = LastMetaSeconds = FPlatformTime::Seconds();
}

void UFMTraceSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldTickStart.Remove(TickStartHandle);
	FWorldDelegates::OnWorldPostActorTick.Remove(TickEndHandle);
	FlushRelay();
	if (LinesWritten > 0)
	{
		WriteMeta();
	}
	if (SessionFile)
	{
		SessionFile->Close();
		SessionFile.Reset();
	}
	Super::Deinitialize();
}

int32 UFMTraceSubsystem::GetFrame() const
{
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	if (!Prediction)
	{
		return 0;
	}
	const FFixedTickState& Tick = Prediction->GetFixedTickState();
	return Tick.PendingFrame + Tick.Offset;
}

void UFMTraceSubsystem::Write(const FString& Body)
{
	const int32 Frame = GetFrame();
	const FString Line = FString::Printf(TEXT("[%d] [%s] %s"), Frame, *WorldTag, *Body);
	UE_LOG(LogFMTrace, Log, TEXT("%s"), *Line);
	AppendToSession(Line);
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		RelayBatch.Add(Line);
	}
	if (FirstFrame < 0)
	{
		FirstFrame = Frame;
	}
	LastFrame = Frame;
	++LinesWritten;
}

void UFMTraceSubsystem::Ingest(const TArray<FString>& Lines)
{
	const bool bToLog = GetWorld()->WorldType != EWorldType::PIE;
	for (const FString& Line : Lines)
	{
		if (bToLog)
		{
			UE_LOG(LogFMTrace, Log, TEXT("%s"), *Line);
		}
		AppendToSession(Line);
	}
}

void UFMTraceSubsystem::AppendToSession(const FString& Line)
{
	if (!SessionFile)
	{
		IFileManager::Get().MakeDirectory(*SessionDir, true);
		SessionFile.Reset(IFileManager::Get().CreateFileWriter(*(SessionDir / TEXT("trace.log")), FILEWRITE_AllowRead));
		if (!SessionFile)
		{
			return;
		}
	}
	const FTCHARToUTF8 Utf8(*(Line + TEXT("\n")));
	SessionFile->Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Utf8.Length());
}

void UFMTraceSubsystem::OnTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World == GetWorld())
	{
		TickStartSeconds = FPlatformTime::Seconds();
	}
}

void UFMTraceSubsystem::OnTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld())
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	TickAccumulatedMs += (Now - TickStartSeconds) * 1000.0;
	++TicksAccumulated;
	if (Now - LastCostSeconds >= CostPeriodSeconds)
	{
		LastCostSeconds = Now;
		if (World->GetNetMode() == NM_Client)
		{
			FlushRelay();
		}
		else
		{
			PrintCost();
		}
		if (SessionFile)
		{
			SessionFile->Flush();
		}
	}
	if (Now - LastMetaSeconds >= MetaPeriodSeconds && LinesWritten > 0)
	{
		LastMetaSeconds = Now;
		WriteMeta();
	}
}

void UFMTraceSubsystem::PrintCost()
{
	const UNetDriver* Driver = GetWorld()->GetNetDriver();
	const float TickMs = TicksAccumulated ? TickAccumulatedMs / TicksAccumulated : 0.0f;
	TickAccumulatedMs = 0.0;
	TicksAccumulated = 0;
	if (!Driver)
	{
		return;
	}
	const int32 Players = Driver->ClientConnections.Num();
	for (const UNetConnection* Connection : Driver->ClientConnections)
	{
		if (!Connection)
		{
			continue;
		}
		const AFMPlayerController* Controller = Cast<AFMPlayerController>(Connection->OwningActor);
		const FString Tag = Controller && !Controller->GetClientWorldTag().IsEmpty()
			? Controller->GetClientWorldTag() : FString(TEXT("C?"));
		const float LagMs = Connection->AvgLag * 1000.0f;
		Write(FString::Printf(TEXT("COST conn=%s in_bps=%d out_bps=%d tick_ms=%.2f lag_ms=%.0f players=%d"),
			*Tag, Connection->InBytesPerSecond, Connection->OutBytesPerSecond, TickMs, LagMs, Players));
		FConnectionStats& Stats = Connections.FindOrAdd(Tag);
		Stats.InBps += Connection->InBytesPerSecond;
		Stats.OutBps += Connection->OutBytesPerSecond;
		Stats.LagMs += LagMs;
		++Stats.Samples;
	}
}

void UFMTraceSubsystem::FlushRelay()
{
	if (RelayBatch.IsEmpty())
	{
		return;
	}
	AFMPlayerController* Controller = Cast<AFMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!Controller || !Controller->GetNetConnection())
	{
		return;
	}
	for (int32 Start = 0; Start < RelayBatch.Num(); Start += RelayBatchSize)
	{
		TArray<FString> Chunk(RelayBatch.GetData() + Start, FMath::Min(RelayBatchSize, RelayBatch.Num() - Start));
		Controller->ServerRelayTrace(Chunk);
	}
	RelayBatch.Reset();
}

void UFMTraceSubsystem::WriteMeta()
{
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("world"), WorldTag);
	Root->SetStringField(TEXT("commit"), Commit);
	Root->SetStringField(TEXT("map"), GetWorld()->GetMapName());
	Root->SetNumberField(TEXT("net_mode"), static_cast<int32>(GetWorld()->GetNetMode()));
	Root->SetNumberField(TEXT("fixed_tick_frame_rate"), Prediction ? Prediction->GetSettings().FixedTickFrameRate : 0);
	Root->SetNumberField(TEXT("first_frame"), FirstFrame);
	Root->SetNumberField(TEXT("last_frame"), LastFrame);
	Root->SetNumberField(TEXT("lines"), static_cast<double>(LinesWritten));
	Root->SetStringField(TEXT("written"), FDateTime::UtcNow().ToIso8601());

	TArray<TSharedPtr<FJsonValue>> ConnectionsJson;
	for (const TPair<FString, FConnectionStats>& Pair : Connections)
	{
		TSharedRef<FJsonObject> Item = MakeShared<FJsonObject>();
		const double N = FMath::Max(1, Pair.Value.Samples);
		Item->SetStringField(TEXT("conn"), Pair.Key);
		Item->SetNumberField(TEXT("samples"), Pair.Value.Samples);
		Item->SetNumberField(TEXT("in_bps"), Pair.Value.InBps / N);
		Item->SetNumberField(TEXT("out_bps"), Pair.Value.OutBps / N);
		Item->SetNumberField(TEXT("lag_ms"), Pair.Value.LagMs / N);
		ConnectionsJson.Add(MakeShared<FJsonValueObject>(Item));
	}
	Root->SetArrayField(TEXT("connections"), ConnectionsJson);

	FString Text;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Text);
	FJsonSerializer::Serialize(Root, Writer);
	IFileManager::Get().MakeDirectory(*SessionDir, true);
	FFileHelper::SaveStringToFile(Text, *(SessionDir / TEXT("meta.json")));
}

int32 UFMTraceLibrary::GetFrame(const UObject* WorldContextObject)
{
	const UFMTraceSubsystem* Subsystem = UFMTraceSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->GetFrame() : -1;
}

FString UFMTraceLibrary::GetWorldTag(const UObject* WorldContextObject)
{
	const UFMTraceSubsystem* Subsystem = UFMTraceSubsystem::Get(WorldContextObject);
	return Subsystem ? Subsystem->GetWorldTag() : FString();
}

void UFMTraceLibrary::Mark(const UObject* WorldContextObject, const FString& Category)
{
	FM_TRACE(WorldContextObject, TEXT("MARK category=%s"), *Category);
}

void UFMTraceLibrary::WriteBundle(const UObject* WorldContextObject)
{
	if (UFMTraceSubsystem* Subsystem = UFMTraceSubsystem::Get(WorldContextObject))
	{
		Subsystem->WriteMeta();
	}
}
