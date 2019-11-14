// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class AkUEPlatform_PS4 : AkUEPlatform
{
	public AkUEPlatform_PS4(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string SanitizeLibName(string in_libName)
	{
		return in_libName;
	}

	public override string GetPluginFullPath(string PluginName, string LibPath)
	{		
		return Path.Combine(LibPath, "lib" + PluginName + ".a");
	}

	public override bool SupportsAkAutobahn { get { return false; } }
	
	public override bool SupportsCommunication { get { return true; } }

	public override List<string> GetPublicLibraryPaths()
	{
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, "PS4", akConfigurationDir, "lib")
		};
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		return new List<string>
		{
			"SceAjm_stub_weak",
			"SceAudio3d_stub_weak",
			"SceMove_stub_weak",
			SanitizeLibName("SceAudio3dEngine")
		};
	}
	
	public override List<string> GetPublicDefinitions()
	{
		return new List<string>
		{
			"__ORBIS__"
		};
	}

	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>();
	}
}
