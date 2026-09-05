#include "Ocean/FMOcean.h"

FVector3f FMOcean::Wave(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const FFMWaveComponent& W)
{
	const float Amplitude = W.Amplitude * SeaState;
	const float Steepness = W.Steepness * SeaState;
	const float Angle = W.AngleDegrees * (Pi / 180.0f);
	const float SinA = FMath::Sin(Angle);
	const float CosA = FMath::Cos(Angle);
	const FVector2f D(Wind.X * CosA - Wind.Y * SinA, Wind.X * SinA + Wind.Y * CosA);
	const float K = 2.0f * Pi / W.Wavelength;
	const float Omega = FMath::Sqrt(Gravity * K);
	float Phase = (P.X * D.X + P.Y * D.Y) * K - Omega * Time;
	const float Turns = Phase / (2.0f * Pi);
	Phase = (Turns - FMath::FloorToFloat(Turns)) * (2.0f * Pi);
	const float S = FMath::Sin(Phase);
	const float C = FMath::Cos(Phase);
	return FVector3f(-Steepness * Amplitude * D.X * S, -Steepness * Amplitude * D.Y * S, Amplitude * C);
}

FVector3f FMOcean::Displace(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves)
{
	FVector3f Out = FVector3f::ZeroVector;
	for (const FFMWaveComponent& W : Waves)
	{
		Out += Wave(P, Time, SeaState, Wind, W);
	}
	return Out;
}

FVector2f FMOcean::SourceOf(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations)
{
	FVector2f Source = P;
	for (int32 i = 0; i < Iterations; ++i)
	{
		const FVector3f D = Displace(Source, Time, SeaState, Wind, Waves);
		Source = P - FVector2f(D.X, D.Y);
	}
	return Source;
}

float FMOcean::HeightAt(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations)
{
	return Displace(SourceOf(P, Time, SeaState, Wind, Waves, Iterations), Time, SeaState, Wind, Waves).Z;
}

float FMOcean::InversionResidual(const FVector2f& P, float Time, float SeaState, const FVector2f& Wind, const TArray<FFMWaveComponent>& Waves, int32 Iterations)
{
	const FVector2f Source = SourceOf(P, Time, SeaState, Wind, Waves, Iterations);
	const FVector3f D = Displace(Source, Time, SeaState, Wind, Waves);
	return (Source + FVector2f(D.X, D.Y) - P).Size();
}

FVector2f FMOcean::WindFromAngle(float Degrees)
{
	const float Radians = Degrees * (Pi / 180.0f);
	return FVector2f(FMath::Cos(Radians), FMath::Sin(Radians));
}
