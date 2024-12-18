project "Frame"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "off"

    files{
        "src/**.cpp", "src/**.h"
    }

    includedirs
    {
        "src",

        "../Vendor/imgui",
        "../Vendor/GLFW/include",
        "../Vendor/stb_image",

        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
    }

    links
    {
        "ImGui",
        "GLFW",

        "%{Library.Vulkan}"
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
        defines { "FRAME_DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
