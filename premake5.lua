
workspace "PlaneLab"
   filename "PlaneLab"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   defaultplatform "x64"

   filter "configurations:Debug"
      symbols "On"
      defines { "_DEBUG" }

   filter "configurations:Release"
      optimize "On"
   
   filter "platforms:x64"
      architecture "x86_64"

include "tools/embed_resource.lua"
include "lib/glfw"
include "lib/cimgui"
include "lib/cimplot"
include "lib/nfd"
include "lib/stb"

project "PlaneLab"
   kind "WindowedApp"

   language "C"
   cdialect "C17"
   location "build"
   staticruntime "On"
   objdir "build/obj/%{cfg.platform}/%{cfg.buildcfg}"
   targetdir "build/bin/%{cfg.platform}/%{cfg.buildcfg}"

   embed_resources("resources", "src/resources")
   files { "src/**.h", "src/**.c" }

   includedirs {
      "lib/glfw/src/include",
      "lib/cimgui/src",
      "lib/cimgui/src_local/include",
      "lib/cimplot/src",
      "lib/nfd/src/src/include",
      "lib/stb/src",
      "lib/debugmalloc/src_local/include"
   }

   links { "cimgui", "glfw", "cimplot", "nfd", "stb" }

   defines { "CIMGUI_USE_GLFW", "CIMGUI_USE_OPENGL3", "CIMGUI_DEFINE_ENUMS_AND_STRUCTS", "APP_NAME=\"%{wks.name}\"" }

   filter {"system:windows", "action:vs*" }
      files { "resources/**", "src/resources/win32/**.rc", "src/resources/win32/**.h" }

   filter "system:windows"
      links { "opengl32" }

   filter "system:linux"
      links { "GL", "X11", "pthread", "dl" }
