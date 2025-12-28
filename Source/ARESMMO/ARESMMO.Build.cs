using UnrealBuildTool;

public class ARESMMO : ModuleRules
{
	public ARESMMO(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd", 
		});
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"AnimGraphRuntime",
		});
	}
}
