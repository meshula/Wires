
project "LabText"
    kind "StaticLib"
    language "C++"    
    platforms { "x32", "x64" }
    
    includedirs { "src" } 
    files { "src/**.h", "src/**.cpp", "src/**.c" }        
    excludes { }

    configuration "Debug"
        targetdir "build/Debug"
        defines {  "DEBUG", "__MACOSX_CORE__", "OSX" }
        flags { "Symbols" }

    configuration "Release"
        targetdir "build/Release"
        defines { "NDEBUG", "__MACOSX_CORE__", "OSX" }
        flags { "Optimize" } 

    configuration "macosx"
        buildoptions { "-std=c++11", "-stdlib=libc++" }
