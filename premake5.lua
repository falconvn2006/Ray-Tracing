workspace "Frame"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "App"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "VisualStudioConfig.lua"
include "App"