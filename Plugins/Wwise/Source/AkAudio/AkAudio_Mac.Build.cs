// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class AkUEPlatform_Mac : AkUEPlatform
{
	private string akLibPath = string.Empty;

	public AkUEPlatform_Mac(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder)
	{
		akLibPath = Path.Combine(ThirdPartyFolder, "Mac", akConfigurationDir, "lib");
	}

	public override string SanitizeLibName(string in_libName)
	{
		return Path.Combine(akLibPath, "lib" + in_libName + ".a");
	}

	public override string GetPluginFullPath(string PluginName, string LibPath)
	{
		return Path.Combine(LibPath, "lib" + PluginName + ".a");
	}

	public override bool SupportsAkAutobahn { get { return Target.Configuration != UnrealTargetConfiguration.Shipping; } }
	
	public override bool SupportsCommunication { get { return true; } }

	public override List<string> GetPublicLibraryPaths()
	{
		return new List<string>
		{
			akLibPath
		};
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		return new List<string> 
		{
			SanitizeLibName("AkAACDecoder")
		};
	}
	
	public override List<string> GetPublicDefinitions()
	{
		return new List<string>();
	}
	
	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>
		{
			"AudioUnit",
			"AudioToolbox",
			"CoreAudio"
		};
	}
}
