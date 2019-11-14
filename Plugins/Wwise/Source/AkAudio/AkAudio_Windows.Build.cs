// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public abstract class AkUEPlatform_Windows : AkUEPlatform
{
	public AkUEPlatform_Windows(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}

	public abstract string PlatformPrefix { get; }

	public override string SanitizeLibName(string in_libName)
	{
		return in_libName + ".lib";
	}

	public override string GetPluginFullPath(string PluginName, string LibPath)
	{
		return Path.Combine(LibPath, PluginName + ".lib");
	}

	public override bool SupportsAkAutobahn { get { return Target.Configuration != UnrealTargetConfiguration.Shipping; } }
	
	public override bool SupportsCommunication { get { return true; } }

	public override List<string> GetPublicLibraryPaths()
	{
		string akPlatformLibDir = PlatformPrefix + "_" + GetVisualStudioVersion();
		return new List<string> { Path.Combine(ThirdPartyFolder, akPlatformLibDir, akConfigurationDir, "lib") };
	}
	
	public override List<string> GetPublicAdditionalLibraries()
	{
		var Dependencies = new List<string> 
		{
			"dsound.lib",
			"dxguid.lib",
			"Msacm32.lib",
			"XInput.lib",
			"dinput8.lib"
		};
		
		if(Target.bBuildEditor)
		{
			Dependencies.Add("SFLib.lib");
		}
		
		return Dependencies;
	}
	
	public override List<string> GetPublicDefinitions()
	{
		var Definitions = new List<string>();
		if(Target.bBuildEditor)
		{
			Definitions.Add("AK_SOUNDFRAME");
		}
		return Definitions;
	}

	public override List<string> GetPublicAdditionalFrameworks()
	{
		return new List<string>();
	}

	private string GetVisualStudioVersion()
	{
		string VSVersion = "vc140";

		try
		{
			if (Target.WindowsPlatform.Compiler == (WindowsCompiler)Enum.Parse(typeof(WindowsCompiler), "VisualStudio2013"))
			{
				VSVersion = "vc120";
			}
		}
		catch (Exception)
		{
		}

		try
		{
			if (Target.WindowsPlatform.Compiler == (WindowsCompiler)Enum.Parse(typeof(WindowsCompiler), "VisualStudio2015"))
			{
				VSVersion = "vc140";
			}
		}
		catch (Exception)
		{
		}

		try
		{
			if (Target.WindowsPlatform.Compiler == (WindowsCompiler)Enum.Parse(typeof(WindowsCompiler), "VisualStudio2017"))
			{
				VSVersion = "vc150";
			}
		}
		catch (Exception)
		{
		}

		return VSVersion;
	}
}

public class AkUEPlatform_Win32 : AkUEPlatform_Windows
{
	public AkUEPlatform_Win32(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
	public override string PlatformPrefix { get { return "Win32"; } }
}

public class AkUEPlatform_Win64 : AkUEPlatform_Windows
{
	public AkUEPlatform_Win64(ReadOnlyTargetRules in_TargetRules, string in_ThirdPartyFolder) : base(in_TargetRules, in_ThirdPartyFolder) {}
	public override string PlatformPrefix { get { return "x64"; } }
}
