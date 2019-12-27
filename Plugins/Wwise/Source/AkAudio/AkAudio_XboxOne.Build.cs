// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class AkUEPlatform_XboxOne : AkUEPlatform
{
	public AkUEPlatform_XboxOne(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public override string SanitizeLibName(string in_libName)
	{
		return in_libName + ".lib";
	}

	public override string GetPluginFullPath(string PluginName, string LibPath)
	{
		return Path.Combine(LibPath, PluginName + ".lib");
	}

	public override bool SupportsAkAutobahn { get { return false; } }
	
	public override bool SupportsCommunication { get { return true; } }

	public override List<string> GetPublicLibraryPaths()
	{
		string akPlatformLibDir = "XboxOne_" + GetVisualStudioVersion();
		
		return new List<string>
		{
			Path.Combine(ThirdPartyFolder, akPlatformLibDir, akConfigurationDir, "lib")
		};
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		return new List<string>
		{
			"AcpHal.lib",
			"MMDevApi.lib"
		};
	}
	
	public override List<string> GetPublicDefinitions()
	{
		return new List<string>
		{
			"_XBOX_ONE"
		};
	}
	
	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>();
	}

	private string GetVisualStudioVersion()
	{
		string VSVersion = "vc140";

		// Use reflection because the GitHub version of UE is missing things.
		var XboxOnePlatformType = System.Type.GetType("XboxOnePlatform", false);
		if (XboxOnePlatformType != null)
		{
			var XboxOneCompilerField = XboxOnePlatformType.GetField("Compiler");
			if (XboxOneCompilerField != null)
			{
				var XboxOneCompilerValue = XboxOneCompilerField.GetValue(null);
				if (XboxOneCompilerValue.ToString() == "VisualStudio2012")
				{
					VSVersion = "vc110";
				}
				else if(XboxOneCompilerValue.ToString() == "VisualStudio2015")
				{
					VSVersion = "vc140";
				}
				else if(XboxOneCompilerValue.ToString() == "VisualStudio2017")
				{
					VSVersion = "vc150";
				}
			}
		}
		
		return VSVersion;
	}
}
