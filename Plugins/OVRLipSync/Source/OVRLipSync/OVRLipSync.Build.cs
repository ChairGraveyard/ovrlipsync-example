// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OVRLipSync : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    private string BinariesPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
    }

	public OVRLipSync(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"OVRLipSync/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"OVRLipSync/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects",
                "CoreUObject",
                "Engine",
                "InputCore",
                "RHI",
                "Voice",
                "OnlineSubsystem",
                "OnlineSubsystemUtils"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
                "Core",
                "Projects",
                "CoreUObject",
                "Engine",
                "InputCore",
                "RHI",
                "Voice",
                "OnlineSubsystem",
                "OnlineSubsystemUtils"
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        LoadOVRLipSyncLib(Target); 
	}

    public bool LoadOVRLipSyncLib(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x32";

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicDelayLoadDLLs.Add("OVRLipSync_x64.dll");
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(BinariesPath, "Win64", "OVRLipSync_x64.dll")));
            }
            else
            {
                PublicDelayLoadDLLs.Add("OVRLipSync.dll");
                RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(BinariesPath, "Win32", "OVRLipSync.dll")));
            }
        }

        return isLibrarySupported;
    }
}
