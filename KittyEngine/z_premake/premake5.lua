local output = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

local ENGINE_NAME = "KittyEngine"
local ENGINE_NAME_SHORT = "Engine"

local EDITOR_NAME = "KittyEditor"
local EDITOR_NAME_SHORT = "Editor"

local WORKSPACE_NAME = "Project_8"

local PROJECT_NAME = "Project"
local PROJECT_NAME_SHORT = "Project"

local EXTERNAL_NAME = "External"

local GAME_NAME_VALUE = "Intergalactic Ball Throwing Championship With Friends ツ"
local GAME_NAME = {
    [[GAME_NAME=L"]]..GAME_NAME_VALUE..[["]]
}

local DEBUG_BUILD_NAME = [[Debug - Moggie]]
local DEBUG_BUILD_DEFINES = {
    "KITTYENGINE_DEBUG",
    "K_FBXSDK",
    "FBXSDK_SHARED",
}

local RELEASE_BUILD_NAME = [[Release - Hybrid]]
local RELEASE_BUILD_DEFINES = {
    "KITTYENGINE_RELEASE",
    "K_FBXSDK",
    "FBXSDK_SHARED",
    "NDEBUG",
}

local FINAL_BUILD_NAME = [[Ship - Purebreed]]
local FINAL_BUILD_DEFINES = {
    "KITTYENGINE_SHIP",
    "KITTYENGINE_NO_EDITOR",
    "LOLKITTYENGINE_STANDALONE_EDITOR",
    "K_FBXSDK",
    "FBXSDK_SHARED",
    "NDEBUG",
}

local GRAPHICS_DEFINES = {
    "KE_MAX_LIGHTS=16",
    "KE_MAX_BONES=128",
}

local CONFIG_FILTERS = {
    DEBUG = "configurations:"..tostring(DEBUG_BUILD_NAME),
    RELEASE = "configurations:"..tostring(RELEASE_BUILD_NAME),
    FINAL = "configurations:"..tostring(FINAL_BUILD_NAME),
}

local DO_LOGGING = true

local LOG = DO_LOGGING and print or function() end

local USE_ABSOLUTE_PATHS = false
local basePath = USE_ABSOLUTE_PATHS and os.realpath("../") or "../"

local cppVersion = "C++20"

local PROJECT_KIND = "WindowedApp"
local EDITOR_KIND = "StaticLib"
local OTHER_KIND = "StaticLib"

local directories = {
    root            = basePath,
    bin             = basePath .. "Bin/",
    temp            = basePath .. "Temp/",
    shaders         = basePath .. "Bin/Shaders/",

    intermediateLib = basePath .. "Temp/IntermediateLib",

    pchPath         = basePath .. "Engine/Source/pch/",

    --engine section
    engine          = basePath .. ENGINE_NAME_SHORT .. "//",
    engineAssets    = basePath .. ENGINE_NAME_SHORT .. "/EngineAssets/",
    engineSource    = basePath .. ENGINE_NAME_SHORT .. "/Source/",
    engineSettings  = basePath .. ENGINE_NAME_SHORT .. "/Settings/",
    engineShaders   = basePath .. ENGINE_NAME_SHORT .. "/Source/Graphics/Shaders/",

    --editor section
    editor          = basePath .. EDITOR_NAME_SHORT .. "//",
    editorAssets    = basePath .. EDITOR_NAME_SHORT .. "/EditorAssets/",
    editorSource    = basePath .. EDITOR_NAME_SHORT .. "/Source/",
    editorSettings  = basePath .. EDITOR_NAME_SHORT .. "/Settings/",

    --project section
    project         = basePath .. PROJECT_NAME_SHORT .. "//",
    projectAssets   = basePath .. PROJECT_NAME_SHORT .. "/ProjectAssets/",
    projectSource   = basePath .. PROJECT_NAME_SHORT .. "/Source/",
    projectSettings = basePath .. PROJECT_NAME_SHORT .. "/Settings/",

    --external section
    external        = basePath .. "/External/",
    externalDLL     = basePath .. "/External/dll/",
    externalInclude = basePath .. "/External/Include/",
    externalLib     = basePath .. "/External/Lib/",
    debugLib        = basePath .. "/External/Lib/Debug/",
    releaseLib      = basePath .. "/External/Lib/Release/",


    -- other
    premake         = basePath .. "z_premake/",
}

local IGNORED_WARNINGS = {
    "4100", -- unreferenced formal parameter
    "4505", -- unreferenced local function removed
    "4514", -- same but inline function
    "4189", -- unreferenced local variable
    "26819", -- unannonotated fallthrough between switch labels (mostly json?)
    "26800", -- use of a moved object (mostly json?)
    "5056", -- stupid matrix operator thing
    "26495", -- Variable 'variable' is uninitialized. Always initialize a member variable
}

local LINKER_OPTIONS =  {
    "/ignore:4006", -- multi definition thing (mostly xaudio?)
    "/ignore:4099", -- linking object as if no debug info (like ok i dont care?)
}

local SHADER_INCLUDE_PATH = [[/I"..\\Engine\\Source\\Graphics\\Shaders"]]


local USE_PCH = true
local function UsePrecompiled()
    if not USE_PCH then return end
    pchheader ("stdafx.h")
    pchsource ("../Engine/Source/pch/stdafx.cpp")
end

-- Create the Visual Studio solution

 workspace(WORKSPACE_NAME)
    location(directories.root)
    startproject(PROJECT_NAME)
    architecture("x64")
    cppdialect(cppVersion)

    configurations {
        DEBUG_BUILD_NAME,
        RELEASE_BUILD_NAME,
        FINAL_BUILD_NAME
    }

    defines (GRAPHICS_DEFINES)
    defines (GAME_NAME)
    shaderdefines (GRAPHICS_DEFINES)

    -- pchheader ("stdafx.h")
    -- pchsource ("../Engine/Source/pch/stdafx.cpp")
    
    shaderincludedirs(directories.engineShaders)
    shadermodel("5.0")
    filter("files:**PS.hlsl")
        removeflags("ExcludeFromBuild")
        shadertype("Pixel")
        shaderobjectfileoutput(directories.shaders .. "/%%(Filename).cso")

    filter("files:**VS.hlsl")
        removeflags("ExcludeFromBuild")
        shadertype("Vertex")
        shaderobjectfileoutput(directories.shaders .. "/%%(Filename).cso")

    filter("files:**CS.hlsl")
        removeflags("ExcludeFromBuild")
        shadertype("Compute")
        shaderobjectfileoutput(directories.shaders .. "/%%(Filename).cso")
        --



--create the solution projects!

project(EXTERNAL_NAME)
    location(directories.temp)
    language("C++")
    cppdialect(cppVersion)
    kind(OTHER_KIND)

    debugdir(directories.intermediateLib)
    targetdir(directories.intermediateLib)
    targetname(EXTERNAL_NAME.."_%{cfg.buildcfg}")
    objdir(directories.temp.."/"..EXTERNAL_NAME.."/%{cfg.buildcfg}") -- add to directories?

    files {
        directories.external.."**.h",
        directories.external.."**.hpp",
        directories.external.."**.cpp",

        directories.external.."**.hlsl",
        directories.external.."**.hlsli"
    }

    includedirs {
        directories.externalInclude,
        directories.root,
        directories.externalInclude.."TGAFBXImporter/FBXSDK/include",
		directories.externalInclude.."TGAFBXImporter/source",
        directories.externalInclude.."physx",
    }

    filter (CONFIG_FILTERS.DEBUG)
        defines (DEBUG_BUILD_DEFINES)
        runtime "Debug"
        symbols "on"
    filter (CONFIG_FILTERS.RELEASE)
        defines (RELEASE_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
    filter (CONFIG_FILTERS.FINAL)
        defines (FINAL_BUILD_DEFINES)
        runtime "Release"
        optimize "on"

    filter "system:windows"
        staticruntime "off"
        symbols "On"
        systemversion "latest"
        warnings "Off"

        disablewarnings(IGNORED_WARNINGS)
        linkoptions(LINKER_OPTIONS)

        flags {
            --"FatalCompileWarnings",
            "MultiProcessorCompile"
        }

        defines {
            "WIN32",
        }

        links {
            "DXGI" -- unsure
        }

--

project(ENGINE_NAME_SHORT)
    location(directories.temp)
    language "C++"
    cppdialect(cppVersion)
    kind(OTHER_KIND)
    
    debugdir(directories.intermediateLib) -- NOT SURE!!!
    targetdir(directories.intermediateLib)
    targetname(ENGINE_NAME.."_%{cfg.buildcfg}")
    objdir(directories.temp.."/"..ENGINE_NAME_SHORT.."/%{cfg.buildcfg}") -- add to directories?

    UsePrecompiled()
    
    shaderincludedirs(engineShaders)

    files {
        directories.engineSource.."**.h",
        directories.engineSource.."**.hpp",
        directories.engineSource.."**.cpp",

        directories.pchPath.."**",
        
        directories.engineSource.."**.hlsl",
        directories.engineSource.."**.hlsli"
    }
    
    includedirs {
        directories.externalInclude,
        directories.engineSource,
        directories.root,
        directories.pchPath,
        -- not sure if physx is needed here yet but probably maybe mvh Anton
        directories.externalInclude.."physx", -- needed inside physx itself 
    }
    
    filter (CONFIG_FILTERS.DEBUG)
        defines (DEBUG_BUILD_DEFINES)
        runtime "Debug"
        symbols "on"
    filter (CONFIG_FILTERS.RELEASE)
        defines (RELEASE_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
    filter (CONFIG_FILTERS.FINAL)
        defines (FINAL_BUILD_DEFINES)
        runtime "Release"
        optimize "on"

    filter "system:windows"
        staticruntime "off"
        symbols "On"
        systemversion "latest"
        warnings "Extra"

        disablewarnings(IGNORED_WARNINGS)
        linkoptions(LINKER_OPTIONS)

        flags {
            "FatalCompileWarnings",
            "MultiProcessorCompile"
        }

        defines {
            "WIN32",
        }

        links {
            "DXGI", -- unsure
            "XAudio2",
        }
--

--[[
        SHARED SECTION FOR EDITOR AND PROJECT
--]]

local function FindLibraries()
    LOG("\nFinding Libraries to link:")

    local foundNames = {}
    local out = {}
    for _, lib in pairs(os.matchfiles(directories.externalLib.."**")) do        
        if (path.getextension(lib) == ".lib") then
            local name = path.getname(lib)
            if not foundNames[name] then
                out[#out+1] = path.getname(lib)
                foundNames[name] = true
                LOG(name)
            end
        end
    end
    
    LOG("\n")

    return out
end

local LIBRARY_LIST = FindLibraries()
LIBRARY_LIST[#LIBRARY_LIST + 1] = EXTERNAL_NAME
LIBRARY_LIST[#LIBRARY_LIST + 1] = ENGINE_NAME_SHORT

--

project(EDITOR_NAME_SHORT)
    location(directories.temp)
    language "C++"
    cppdialect(cppVersion)
    dependson{ENGINE_NAME, EXTERNAL_NAME, directories.pchPath}

    links(LIBRARY_LIST)
    
    kind(OTHER_KIND)
    
    UsePrecompiled()

    shaderincludedirs(engineShaders)

    targetdir(directories.intermediateLib)
    debugdir(directories.intermediateLib)
    objdir(directories.temp.."/"..EDITOR_NAME_SHORT.."/%{cfg.buildcfg}") -- add to directories?
    targetname(EDITOR_NAME.."_%{cfg.buildcfg}")

    files {
        directories.editorSource.."**.h",
        directories.editorSource.."**.hpp",
        directories.editorSource.."**.cpp",

        directories.pchPath.."**",


        directories.editorSource.."**.hlsl",
        directories.editorSource.."**.hlsli"
    }

    includedirs {
        directories.externalInclude,
        directories.engineSource,
        directories.root,
        directories.pchPath,
        directories.externalInclude.."physx",
    }

    
    libdirs {
        directories.externalLib,
        directories.intermediateLib -- where engine ends up!
    }

    filter (CONFIG_FILTERS.DEBUG)
        defines (DEBUG_BUILD_DEFINES)
        runtime "Debug"
        symbols "on"
        libdirs {directories.debugLib}

    filter (CONFIG_FILTERS.RELEASE)
        defines (RELEASE_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
        libdirs {directories.releaseLib}

        
    filter (CONFIG_FILTERS.FINAL)
        defines (FINAL_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
        libdirs {directories.releaseLib}



    filter "system:windows"
        staticruntime "off"
        symbols "On"
        systemversion "latest"
        warnings "Extra"

        disablewarnings(IGNORED_WARNINGS)
        linkoptions(LINKER_OPTIONS)

        flags {
            --"FatalCompileWarnings",
            "MultiProcessorCompile"
        }

        defines {
            "WIN32",
        }

        links {
            "DXGI", -- unsure
            "XAudio2"
        }



--

project(PROJECT_NAME)
    location(directories.temp)
    kind(PROJECT_KIND)
    language "C++"
    cppdialect(cppVersion)
    icon "../Bin/gameIconTESTTEST.ico"

    debugdir(directories.bin)
    targetdir(directories.bin)
    targetname(PROJECT_NAME.."_%{cfg.buildcfg}")
    objdir(directories.temp.."/"..PROJECT_NAME.."/%{cfg.buildcfg}") -- add to directories?

    dependson{ENGINE_NAME, EXTERNAL_NAME, directories.pchPath}

    shaderincludedirs(engineShaders)

    links(LIBRARY_LIST)
    links(EDITOR_NAME_SHORT)

    UsePrecompiled()

    includedirs {
        directories.externalInclude,
       -- directories.engineSource,
        --directories.editorSource, -- unsure, only add in non-Ship?
        directories.projectSource,
        directories.root,
        directories.pchPath,
        directories.externalInclude.."physx", -- needed inside physx itself
    }

    files {
        directories.projectSource.."**.h",
        directories.projectSource.."**.hpp",
        directories.projectSource.."**.cpp",

        directories.pchPath.."**",


        directories.projectSource.."**.hlsl",
        directories.projectSource.."**.hlsli"
    }

    libdirs {
        directories.externalLib,
        directories.intermediateLib -- where engine ends up!
    }

    filter (CONFIG_FILTERS.DEBUG)
        defines (DEBUG_BUILD_DEFINES)
        runtime "Debug"
        symbols "on"
        defines {[[_KITTYENGINE_BUILD=L"]]..tostring(DEBUG_BUILD_NAME)..[["]]}
        libdirs {directories.debugLib}

    filter (CONFIG_FILTERS.RELEASE)
        defines (RELEASE_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
        defines {[[_KITTYENGINE_BUILD=L"]]..tostring(RELEASE_BUILD_NAME)..[["]]}
        libdirs {directories.releaseLib}

    filter (CONFIG_FILTERS.FINAL)
        targetname(GAME_NAME_VALUE)
        defines (FINAL_BUILD_DEFINES)
        runtime "Release"
        optimize "on"
        defines {[[_KITTYENGINE_BUILD=L"]]..tostring(FINAL_BUILD_NAME)..[["]]}
        libdirs {directories.releaseLib}
 
    filter "system:windows"
        --staticruntime "off" -- need to look into this
        symbols "On"
        systemversion "latest"
        warnings "Extra" -- move to only Release and Ship?
    
        files { 'Resources/resources.rc', '**.ico' }
        vpaths { ['Resources/*'] = { '*.rc', '**.ico' } }

        disablewarnings(IGNORED_WARNINGS)
        linkoptions(LINKER_OPTIONS)

        flags {
            "FatalCompileWarnings", -- probably only want this in Release and Ship too
            "MultiProcessorCompile"
        }

        defines {
            "WIN32",
        }

--

local function MakeFolderStructure()
    for _, dir in pairs(directories) do
        if not os.isdir(dir) then
            os.mkdir(dir)
        end
    end
end

local function CopyDLLs()
    for _, dll in pairs(os.matchfiles(directories.externalDLL.."**")) do
        os.copyfile(dll, directories.bin..path.getname(dll))
    end
end


MakeFolderStructure()
CopyDLLs()
