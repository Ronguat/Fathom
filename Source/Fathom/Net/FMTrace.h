#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "FMTrace.generated.h"

class FArchive;

/** Writes one trace line, `[frame] [world] TAG key=value ...`; Format opens with the tag. */
#define FM_TRACE(WorldContext, Format, ...) \
	UFMTraceSubsystem::Trace(WorldContext, FString::Printf(Format, ##__VA_ARGS__))

/**
 * One trace per game world. Every line carries the shared simulation frame and the world tag,
 * `S` or `C<n>`. The server prints COST per connection once a second; a client relays its lines
 * to the server every tenth of a second in chunks of sixteen. Each world keeps its own session bundle under
 * Saved/Fathom/Sessions/<stamp>-<tag>/: trace.log, and meta.json rewritten every ten seconds.
 */
UCLASS()
class FATHOM_API UFMTraceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFMTraceSubsystem* Get(const UObject* WorldContext);
	static void Trace(const UObject* WorldContext, const FString& Body);

	/** The prediction framework's pending frame in server numbering; 0 before it exists. */
	int32 GetFrame() const;
	const FString& GetWorldTag() const { return WorldTag; }

	void Write(const FString& Body);

	/** Lines a client relayed: into the session file, and into the log outside PIE. */
	void Ingest(const TArray<FString>& Lines);

	void WriteMeta();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	struct FConnectionStats
	{
		double InBps = 0.0, OutBps = 0.0, LagMs = 0.0;
		int32 Samples = 0;
	};

	void OnTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void OnTickEnd(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void PrintCost();
	void FlushRelay();
	void AppendToSession(const FString& Line);

	FString WorldTag;
	FString SessionDir;
	FString Commit;
	TUniquePtr<FArchive> SessionFile;
	TArray<FString> RelayBatch;
	TMap<FString, FConnectionStats> Connections;
	FDelegateHandle TickStartHandle;
	FDelegateHandle TickEndHandle;
	double TickStartSeconds = 0.0;
	double TickAccumulatedMs = 0.0;
	int32 TicksAccumulated = 0;
	double LastCostSeconds = 0.0;
	double LastMetaSeconds = 0.0;
	double LastRelaySeconds = 0.0;
	int32 FirstFrame = -1;
	int32 LastFrame = -1;
	int64 LinesWritten = 0;
};

/** The trace for scripts and the console. */
UCLASS()
class FATHOM_API UFMTraceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Fathom|Trace", meta=(WorldContext="WorldContextObject"))
	static int32 GetFrame(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Fathom|Trace", meta=(WorldContext="WorldContextObject"))
	static FString GetWorldTag(const UObject* WorldContextObject);

	/** Writes `MARK category=<Category>` at the current frame. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Trace", meta=(WorldContext="WorldContextObject"))
	static void Mark(const UObject* WorldContextObject, const FString& Category);

	/** Rewrites the world's meta.json now. */
	UFUNCTION(BlueprintCallable, Category="Fathom|Trace", meta=(WorldContext="WorldContextObject"))
	static void WriteBundle(const UObject* WorldContextObject);
};
