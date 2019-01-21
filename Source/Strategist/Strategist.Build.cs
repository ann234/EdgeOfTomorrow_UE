// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Strategist : ModuleRules
{
	public Strategist(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay","UMG" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore"});
	}
}
