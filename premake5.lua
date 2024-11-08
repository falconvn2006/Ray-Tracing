workspace "FrameRayTracing"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }
    startproject "FrameRayTracingApp"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "VisualStudioConfig.lua"
include "FrameRayTracingApp"