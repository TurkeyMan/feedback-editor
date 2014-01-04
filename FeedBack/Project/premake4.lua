solution "FeedBack"
	configurations { "Debug", "DebugOpt", "Release", "Retail" }
--	platforms { "Native", "x32", "x64", "Xbox360", "PS3" }
--	platforms { "Native", "Xbox" }
	platforms { "Native", "x32", "x64" }

	-- include the fuji project...
	dofile  "../../../Fuji/Fuji/Project/fujiproj.lua"

	project "FeedBack"
		kind "WindowedApp"
		language "C++"
		files { "../Source/**.h", "../Source/**.cpp" }
		files { "../Themes/**.ini" }

		includedirs { "../Source/" }
		objdir "../Build/"
		targetdir "../"

		flags { "StaticRuntime", "NoExceptions", "NoRTTI", "WinMain" }

		links { "Fuji" }

		dofile "../../../Fuji/dist/Project/fujiconfig.lua"
