workspace "CppChrome"
	architecture "x64"
	startproject "CppChrome"

	configurations
	{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "lib.lua"

group "Dependencies"
include "vendor/Glad"
include "vendor/GLFW"
group ""

project "CppChrome"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++23"
	staticruntime "On"

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	flags { "MultiProcessorCompile" }

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
	}

	defines
	{
	}

	includedirs
	{
		"src",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.cef}",
	}

	links
	{
		"GLFW",
		"Glad",
	}

	filter "system:windows"
		systemversion   "latest" 
		editAndContinue "Off"

		defines
		{
			"WIN32_LEAN_AND_MEAN",
			"NOMINMAX"
		}

		links
		{
		}

		debugenvs 
		{
		}

	filter "configurations:Debug"

		defines 
		{
			"DEBUG"
		}

		links
		{
			"%{Library.cef_debug}",
			"%{Library.cef_wrapper_debug}",
		}

        debugenvs 
		{
            "PATH=%{LibraryDir.cef_debug}"
		}

		runtime "Debug"
		symbols "On"

	filter "configurations:Release"

		defines 
		{
			"RELEASE"
		}

		links
		{
			"%{Library.cef_release}",
			"%{Library.cef_wrapper_release}",
		}

        debugenvs 
		{
            "PATH=%{LibraryDir.cef_release}"
		}

		runtime "Release"
		optimize "On"