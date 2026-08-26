using UnrealBuildTool;
using System.Collections.Generic;

public class UEGT1Target : TargetRules
{
	public UEGT1Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("UEGT1");
	}
}

