// Copyright 1998-2011 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;
using System.Reflection;

// Platform-specific files implement this interface, returning their particular dependencies, defines, etc.
public abstract class AkUEPlatform
{
	protected ReadOnlyTargetRules Target;
	protected string ThirdPartyFolder;
	
	public AkUEPlatform(ReadOnlyTargetRules in_Target, string in_ThirdPartyFolder)
	{
		Target = in_Target;
		ThirdPartyFolder = in_ThirdPartyFolder;
	}
	
	public static AkUEPlatform GetAkUEPlatformInstance(ReadOnlyTargetRules Target, string ThirdPartyFolder)
	{
		var AkUEPlatformType = System.Type.GetType("AkUEPlatform_" + Target.Platform.ToString());
		if(AkUEPlatformType == null)
		{
			throw new BuildException("Wwise does not support platform " + Target.Platform.ToString());
		}
		
		AkUEPlatform PlatformInstance = Activator.CreateInstance(AkUEPlatformType, Target, ThirdPartyFolder) as AkUEPlatform;
		if(PlatformInstance == null)
		{
			throw new BuildException("Wwise could not instanciate platform " + Target.Platform.ToString());
		}

		return PlatformInstance;
	}
	

	protected string akConfigurationDir
	{
		get
		{
			if (Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				// change bDebugBuildsActuallyUseDebugCRT to true in BuildConfiguration.cs to actually link debug binaries
				if (!Target.bDebugBuildsActuallyUseDebugCRT)
				{
					return "Profile";
				}
				else
				{
					return "Debug";
				}
			}
			else if (Target.Configuration == UnrealTargetConfiguration.Development ||
					Target.Configuration == UnrealTargetConfiguration.Test ||
					Target.Configuration == UnrealTargetConfiguration.DebugGame)
			{
				return "Profile";
			}
			else // if (Target.Configuration == UnrealTargetConfiguration.Shipping)
			{
				return "Release";
			}
		}
	}
	
	public abstract string SanitizeLibName(string in_libName);
	public abstract string GetPluginFullPath(string PluginName, string LibPath);
	public abstract bool SupportsAkAutobahn { get; }
	public abstract bool SupportsCommunication { get; }
	
	public abstract List<string> GetPublicLibraryPaths();
	public abstract List<string> GetPublicAdditionalLibraries();
	public abstract List<string> GetPublicDefinitions();
	public abstract List<string> GetPublicAdditionalFrameworks();
}

public class AkAudio : ModuleRules
{
	private static AkUEPlatform AkUEPlatformInstance;

	public AkAudio(ReadOnlyTargetRules Target) : base(Target)
	{
		string ThirdPartyFolder = Path.Combine(ModuleDirectory, "../../ThirdParty");
		AkUEPlatformInstance = AkUEPlatform.GetAkUEPlatformInstance(Target, ThirdPartyFolder);
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateIncludePathModuleNames.AddRange(new string[] { "Settings", "UMG"});
		PublicDependencyModuleNames.Add("UMG");
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"SlateCore",
				"NetworkReplayStreaming",
				"MovieScene",
				"MovieSceneTracks",
				"Projects",
				"Json",
				"Slate",
				"InputCore",
				"Projects"
			});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"DesktopPlatform"
				});
			
			foreach (var Platform in GetAvailablePlatforms(ModuleDirectory))
			{
				PublicDefinitions.Add("AK_PLATFORM_" + Platform.ToUpper());
			}
		}

		PrivateIncludePaths.Add("AkAudio/Private");
		PublicIncludePaths.Add(Path.Combine(ThirdPartyFolder, "include"));
		
		PublicDefinitions.Add("AK_UNREAL_MAX_CONCURRENT_IO=32");
		PublicDefinitions.Add("AK_UNREAL_IO_GRANULARITY=32768");
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("AK_OPTIMIZED");
		}
		
		if (Target.Configuration != UnrealTargetConfiguration.Shipping && AkUEPlatformInstance.SupportsCommunication)
		{
			AddWwiseLib("CommunicationCentral");
			PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=1");
		}
		else
		{
			PublicDefinitions.Add("AK_ENABLE_COMMUNICATION=0");
		}
		
		// Platform-specific dependencies
		PublicLibraryPaths.AddRange(AkUEPlatformInstance.GetPublicLibraryPaths());
		PublicAdditionalLibraries.AddRange(AkUEPlatformInstance.GetPublicAdditionalLibraries());
		PublicDefinitions.AddRange(AkUEPlatformInstance.GetPublicDefinitions());
		
		var Frameworks = AkUEPlatformInstance.GetPublicAdditionalFrameworks();
		foreach(var fw in Frameworks)
		{
#if UE_4_22_OR_LATER
			PublicAdditionalFrameworks.Add(new ModuleRules.Framework(fw));
#else
			PublicAdditionalFrameworks.Add(new UEBuildFramework(fw));
#endif
		}

		AddWwiseLib("AkSoundEngine");
		AddWwiseLib("AkMemoryMgr");
		AddWwiseLib("AkStreamMgr");
		AddWwiseLib("AkMusicEngine");
		AddWwiseLib("AkSpatialAudio");
		AddWwiseLib("AkVorbisDecoder");
		AddWwiseLib("AkSilenceSource");
		AddWwiseLib("AkSineSource");
		AddWwiseLib("AkToneSource");
		AddWwiseLib("AkPeakLimiterFX");
		AddWwiseLib("AkMatrixReverbFX");
		AddWwiseLib("AkParametricEQFX");
		AddWwiseLib("AkDelayFX");
		AddWwiseLib("AkExpanderFX");
		AddWwiseLib("AkFlangerFX");
		AddWwiseLib("AkCompressorFX");
		AddWwiseLib("AkGainFX");
		AddWwiseLib("AkHarmonizerFX");
		AddWwiseLib("AkTimeStretchFX");
		AddWwiseLib("AkPitchShifterFX");
		AddWwiseLib("AkStereoDelayFX");
		AddWwiseLib("AkMeterFX");
		AddWwiseLib("AkGuitarDistortionFX");
		AddWwiseLib("AkTremoloFX");
		AddWwiseLib("AkRoomVerbFX");
		AddWwiseLib("AkAudioInputSource");
		AddWwiseLib("AkSynthOneSource");
		AddWwiseLib("AkRecorderFX");
		AddWwiseLib("AkOpusDecoder");
		
		AddWwisePlugin("AkReflectFX");
		AddWwisePlugin("AkConvolutionReverbFX");
		AddWwisePlugin("AuroHeadphoneFX");

		if (AkUEPlatformInstance.SupportsAkAutobahn)
		{
			PublicDefinitions.Add("AK_SUPPORT_WAAPI=1");
			AddWwiseLib("AkAutobahn");
		}
		else
		{
			PublicDefinitions.Add("AK_SUPPORT_WAAPI=0");
		}
	}
	
	private bool IsPluginInstalled(string PluginName)
	{
		foreach (string LibPath in PublicLibraryPaths)
		{
			if (File.Exists(AkUEPlatformInstance.GetPluginFullPath(PluginName, LibPath)))
			{
				return true;
			}
		}

		return false;
	}
	
	private void AddWwiseLib(string in_libName)
	{
		string SanitizedLibName = AkUEPlatformInstance.SanitizeLibName(in_libName);
		if(!string.IsNullOrEmpty(SanitizedLibName))
		{
			PublicAdditionalLibraries.Add(SanitizedLibName);
		}
	}

	private void AddWwisePlugin(string in_libName)
	{
		if (IsPluginInstalled(in_libName))
		{
			AddWwiseLib(in_libName);
			PublicDefinitions.Add("AK_WITH_" + in_libName.ToUpper() + "=1");
		}
		else
		{
			PublicDefinitions.Add("AK_WITH_" + in_libName.ToUpper() + "=0");
		}
	}

	private static List<string> GetAvailablePlatforms(string ModuleDir)
	{
		List<string> FoundPlatforms = new List<string>();
		const string StartPattern = "AkAudio_";
		const string EndPattern = ".Build.cs";
		foreach (var BuildCsFile in System.IO.Directory.GetFiles(ModuleDir, "*" + EndPattern))
		{
			if (BuildCsFile.Contains("AkAudio_"))
			{
				int StartIndex = BuildCsFile.IndexOf(StartPattern) + StartPattern.Length;
				int StopIndex = BuildCsFile.IndexOf(EndPattern);
				FoundPlatforms.Add(BuildCsFile.Substring(StartIndex, StopIndex - StartIndex));
			}
		}

		return FoundPlatforms;
	}
}
