workspace "MapGeneration"
    configurations { "Debug", "Release" }
    platforms { "x64"}

filter { "platforms:x64" }
    system "Windows"
    architecture "x64"

filter "configurations:Debug"
    defines { "_DEBUG", "_CONSOLE" }
    runtime "Debug"
    symbols "On"

filter "configurations:Release"
    defines {"NDEBUG","_CONSOLE"}
    runtime "Release"

project "MapGenerator"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    location "mapGenerator/"
    files {
        "mapGenerator/*.cpp",
        "mapGenerator/*.h",
        "mapGenerator/*.hpp",
        "mapGenerator/vendor/SDL2_gfx-1.0.4/*.c",
        "mapGenerator/vendor/SDL2_gfx-1.0.4/*.h"
    }
    links {
        "SDL2",
        "SDL2main"
    }
    libdirs {
        "mapGenerator/vendor/SDL2-2.0.20/lib/x64/"
    }
    includedirs {
        "mapGenerator/vendor/SDL2-2.0.20/include/",
        "mapGenerator/vendor/SDL2_gfx-1.0.4/",
        "mapGenerator/vendor/voronoi/src/",
        "mapGenerator/vendor/PerlinNoise/",
    }