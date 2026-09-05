#include "Ocean/FMOceanSubsystem.h"

#include "Core/FMGameState.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Net/FMTrace.h"
#include "NetworkPredictionWorldManager.h"
#include "Ocean/FMOceanActor.h"
#include "RHI.h"
#include "TextureResource.h"

namespace
{
	const FName GlobalsName(TEXT("Globals"));
	const FName ProbeName(TEXT("Probe"));
	const FName WaveNames[4] = { FName(TEXT("W0")), FName(TEXT("W1")), FName(TEXT("W2")), FName(TEXT("W3")) };

	FLinearColor WaveVector(const TArray<FFMWaveComponent>& Waves, int32 Index)
	{
		if (!Waves.IsValidIndex(Index))
		{
			return FLinearColor(1.0f, 0.0f, 0.0f, 0.0f);
		}
		const FFMWaveComponent& W = Waves[Index];
		return FLinearColor(W.Wavelength, W.Amplitude, W.Steepness, W.AngleDegrees);
	}
}

UFMOceanSubsystem* UFMOceanSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFMOceanSubsystem>() : nullptr;
}

bool UFMOceanSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UFMOceanSubsystem::Initialize(FSubsystemCollectionBase& InCollection)
{
	Super::Initialize(InCollection);
	const UFMOceanSettings* Settings = GetDefault<UFMOceanSettings>();
	Waves = Settings->Components;
	Waves.SetNum(4);
	Collection = Settings->Collection.LoadSynchronous();
	if (Renders())
	{
		if (UMaterialInterface* ProbeBase = Settings->ProbeMaterial.LoadSynchronous())
		{
			ProbeMaterial = UMaterialInstanceDynamic::Create(ProbeBase, this);
		}
		const int32 Cells = FMath::Max(2, Settings->ProbeCells);
		ProbeTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), Cells, Cells, RTF_RGBA32f, FLinearColor::Black, false, false);
	}
	TickHandle = FWorldDelegates::OnWorldPostActorTick.AddUObject(this, &UFMOceanSubsystem::OnTickEnd);
}

void UFMOceanSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldPostActorTick.Remove(TickHandle);
	Super::Deinitialize();
}

void UFMOceanSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (const UNetworkPredictionWorldManager* Prediction = InWorld.GetSubsystem<UNetworkPredictionWorldManager>())
	{
		FrameRate = static_cast<float>(FMath::Max(1, Prediction->GetSettings().FixedTickFrameRate));
	}
	if (!Renders())
	{
		return;
	}
	const UFMOceanSettings* Settings = GetDefault<UFMOceanSettings>();
	UMaterialInterface* Material = Settings->SurfaceMaterial.LoadSynchronous();
	Surface = InWorld.SpawnActor<AFMOceanActor>(FVector(0.0, 0.0, Settings->PlaneZ), FRotator::ZeroRotator);
	if (Surface && Material)
	{
		Surface->Build(Settings->PlaneSize, FMath::Max(1, Settings->PlaneSteps), Material);
	}
}

bool UFMOceanSubsystem::Renders() const
{
	return !GUsingNullRHI && GetWorld()->GetNetMode() != NM_DedicatedServer;
}

FVector3f UFMOceanSubsystem::Displace(const FVector2f& XY, int32 Frame) const
{
	return FMOcean::Displace(XY, TimeOfFrame(Frame), FMath::Max(SeaState, 0.0f), Wind, Waves);
}

float UFMOceanSubsystem::PresentedTime(int32 Frame) const
{
	const UNetworkPredictionWorldManager* Prediction = GetWorld()->GetSubsystem<UNetworkPredictionWorldManager>();
	if (!Prediction)
	{
		return TimeOfFrame(Frame);
	}
	const FFixedTickState& Tick = Prediction->GetFixedTickState();
	const float Fraction = FMath::Clamp(Tick.UnspentTimeMS / static_cast<float>(FMath::Max(1, Tick.FixedStepMS)), 0.0f, 1.0f);
	return TimeOfFrame(Frame - 1) + Fraction / FrameRate;
}

float UFMOceanSubsystem::HeightAt(const FVector2f& XY, int32 Frame) const
{
	return FMOcean::HeightAt(XY, TimeOfFrame(Frame), FMath::Max(SeaState, 0.0f), Wind, Waves);
}

void UFMOceanSubsystem::SyncFromGameState()
{
	const AFMGameState* State = GetWorld()->GetGameState<AFMGameState>();
	if (!State || State->SeaState < 0.0f)
	{
		return;
	}
	const FVector2f NewWind(static_cast<float>(State->Wind.X), static_cast<float>(State->Wind.Y));
	if (State->SeaState != SeaState || NewWind != Wind)
	{
		SeaState = State->SeaState;
		Wind = NewWind;
		bParamsPushed = false;
	}
}

void UFMOceanSubsystem::PushCollection(int32 Frame)
{
	if (!Collection)
	{
		return;
	}
	UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		return;
	}
	if (!bParamsPushed)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			Instance->SetVectorParameterValue(WaveNames[i], WaveVector(Waves, i));
		}
		bParamsPushed = true;
	}
	Instance->SetVectorParameterValue(GlobalsName, FLinearColor(FMath::Max(SeaState, 0.0f), Wind.X, Wind.Y, PresentedTime(Frame)));
}

void UFMOceanSubsystem::OnTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld())
	{
		return;
	}
	SyncFromGameState();
	const UFMTraceSubsystem* Trace = World->GetSubsystem<UFMTraceSubsystem>();
	const int32 Frame = Trace ? Trace->GetFrame() : 0;
	PushCollection(Frame);
	Follow();
	const int32 Every = FMath::Max(1, GetDefault<UFMOceanSettings>()->ProbeEveryFrames);
	if (IsReady() && Frame % Every == 0 && Frame != LastProbeFrame)
	{
		LastProbeFrame = Frame;
		Probe(Frame);
	}
}

void UFMOceanSubsystem::Follow()
{
	const APlayerController* PC = Surface ? GetWorld()->GetFirstPlayerController() : nullptr;
	const APawn* Viewer = PC ? PC->GetPawn() : nullptr;
	if (!Viewer)
	{
		return;
	}
	const UFMOceanSettings* Settings = GetDefault<UFMOceanSettings>();
	const double Step = Settings->PlaneSize / FMath::Max(1, Settings->PlaneSteps);
	const FVector At = Viewer->GetActorLocation();
	Surface->SetActorLocation(FVector(FMath::GridSnap(At.X, Step), FMath::GridSnap(At.Y, Step), Settings->PlaneZ));
}

void UFMOceanSubsystem::Probe(int32 Frame)
{
	const UFMOceanSettings* Settings = GetDefault<UFMOceanSettings>();
	const int32 Cells = FMath::Max(2, Settings->ProbeCells);
	const float Extent = Settings->ProbeExtent;
	const FVector2f Origin(-0.5f * Extent, -0.5f * Extent);
	const float Time = TimeOfFrame(Frame);
	const FIntPoint Points[4] = { FIntPoint(0, 0), FIntPoint(Cells - 1, Cells - 1), FIntPoint(Cells / 4, 3 * Cells / 4), FIntPoint(3 * Cells / 4, Cells / 4) };
	const auto SamplePoint = [&](int32 I, int32 J)
	{
		return Origin + FVector2f((I + 0.5f) / Cells, (J + 0.5f) / Cells) * Extent;
	};
	float H[4];
	float Residual = 0.0f;
	for (int32 k = 0; k < 4; ++k)
	{
		const FVector2f P = SamplePoint(Points[k].X, Points[k].Y);
		H[k] = FMOcean::Displace(P, Time, SeaState, Wind, Waves).Z;
		Residual = FMath::Max(Residual, FMOcean::InversionResidual(P, Time, SeaState, Wind, Waves));
	}

	if (!Renders() || !ProbeMaterial || !ProbeTarget)
	{
		FM_TRACE(this, TEXT("OCEAN sf=%d sea=%.2f h0=%.2f h1=%.2f h2=%.2f h3=%.2f inv=%.3f gpu=none"), Frame, SeaState, H[0], H[1], H[2], H[3], Residual);
		return;
	}

	ProbeMaterial->SetVectorParameterValue(GlobalsName, FLinearColor(SeaState, Wind.X, Wind.Y, Time));
	for (int32 i = 0; i < 4; ++i)
	{
		ProbeMaterial->SetVectorParameterValue(WaveNames[i], WaveVector(Waves, i));
	}
	ProbeMaterial->SetVectorParameterValue(ProbeName, FLinearColor(Origin.X, Origin.Y, Extent, 0.0f));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(GetWorld(), ProbeTarget, ProbeMaterial);

	TArray<FLinearColor> Pixels;
	FTextureRenderTargetResource* Resource = ProbeTarget->GameThread_GetRenderTargetResource();
	if (!Resource || !Resource->ReadLinearColorPixels(Pixels) || Pixels.Num() != Cells * Cells)
	{
		FM_TRACE(this, TEXT("OCEAN sf=%d sea=%.2f h0=%.2f h1=%.2f h2=%.2f h3=%.2f inv=%.3f gpu=unread"), Frame, SeaState, H[0], H[1], H[2], H[3], Residual);
		return;
	}

	float MaxError = 0.0f;
	double SumError = 0.0;
	for (int32 J = 0; J < Cells; ++J)
	{
		for (int32 I = 0; I < Cells; ++I)
		{
			const FLinearColor& Px = Pixels[J * Cells + I];
			const FVector3f Gpu(Px.R - FMOcean::ProbeOffset, Px.G - FMOcean::ProbeOffset, Px.B - FMOcean::ProbeOffset);
			const FVector3f Cpu = FMOcean::Displace(SamplePoint(I, J), Time, SeaState, Wind, Waves);
			const float Error = (Gpu - Cpu).Size();
			MaxError = FMath::Max(MaxError, Error);
			SumError += Error;
		}
	}
	float G[4];
	for (int32 k = 0; k < 4; ++k)
	{
		G[k] = Pixels[Points[k].Y * Cells + Points[k].X].B - FMOcean::ProbeOffset;
	}
	FM_TRACE(this, TEXT("OCEAN sf=%d sea=%.2f h0=%.2f h1=%.2f h2=%.2f h3=%.2f inv=%.3f g0=%.2f g1=%.2f g2=%.2f g3=%.2f gpu_max=%.3f gpu_mean=%.4f"),
		Frame, SeaState, H[0], H[1], H[2], H[3], Residual, G[0], G[1], G[2], G[3], MaxError, static_cast<float>(SumError / (Cells * Cells)));
}
