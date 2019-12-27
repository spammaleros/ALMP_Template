// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class AkUEPlatform_Switch : AkUEPlatform
{
	public AkUEPlatform_Switch(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string SanitizeLibName(string in_libName)
	{
		return in_libName;
	}

	public override string GetPluginFullPath(string PluginName, string LibPath)
	{
		return Path.Combine(LibPath, "lib" + PluginName + ".a");
	}

	public override bool SupportsAkAutobahn { get { return false; } }
	
	public override bool SupportsCommunication { get { return false; } }

	public override List<string> GetPublicLibraryPaths()
	{
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, "NX64", "Release", "lib")
		};
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		return new List<string>
		{
			SanitizeLibName("AkOpusNXDecoder")
		};
	}
	
	public override List<string> GetPublicDefinitions()
	{
		return new List<string>
		{
			"NN_NINTENDO_SDK"
		};
	}
	
	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>();
	}
}
