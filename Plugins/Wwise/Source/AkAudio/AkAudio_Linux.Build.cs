// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class AkUEPlatform_Linux : AkUEPlatform
{
	public AkUEPlatform_Linux(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string SanitizeLibName(string in_libName)
	{
		// Linux cross-compile toolchain does not support communication
		if(in_libName == "CommunicationCentral")
		{
			return string.Empty;
		}
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
		// Linux cross-compile toolchain does not support communication, force release config.
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, "Linux_x64", "Release", "lib")
		};
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		return new List<string> ();
	}
	
	public override List<string> GetPublicDefinitions()
	{
		return new List<string>();
	}

	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>();
	}
}
