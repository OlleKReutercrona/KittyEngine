#pragma once

#define NOMINMAX
#include <Windows.h>


#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif



//common stl includes
#include <vector>
#include <array>
#include <queue>
#include <map>
#include <unordered_map>
#include <any>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Engine/Source/Math/Transform.h>
#include <Engine/Source/Math/Vector.h>

#include <Engine/Source/Utility/Global.h>
#include <Editor/Source/EditorInterface.h>


//#include <External/Include/physx/PxPhysicsAPI.h>
