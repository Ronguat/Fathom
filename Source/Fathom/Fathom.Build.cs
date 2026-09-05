using UnrealBuildTool;

public class Fathom : ModuleRules
{
	public Fathom(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NetCore",
			"Mover",
			"NetworkPrediction",
			"Json"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Project code includes relative to the module root, e.g. #include "Core/FMGameMode.h"
		PublicIncludePaths.AddRange(new string[] {
			"Fathom"
		});
	}
}
