project "App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "%{cfg.buildcfg}"
    staticruntime "off"

    files{
        "src/**.cpp", "src/**.h"
    }

    includedirs
    {
        "../Vendor/imgui",
        "../Vendor/GLFW/include",

        "../Frame/src",

        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
    }

    links
    {
        "Frame"
    }

    targetdir("../bin/" .. outputdir .. "/%{prj.name}")
    objdir("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "FRAME_PLATFORM_WINDOWS" }

    filter "configurations:Debug"
        defines { "FRAME_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "FRAME_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        kind "WindowedApp"
        defines { "FRAME_DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
