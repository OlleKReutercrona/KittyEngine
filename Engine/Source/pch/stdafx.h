#pragma once

#define NOMINMAX
#include <Windows.h>

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif

// common stl includes
#include <Editor/Source/EditorInterface.h>
#include <Engine/Source/Math/Transform.h>
#include <Engine/Source/Math/Vector.h>
#include <Engine/Source/Utility/Global.h>
#include <algorithm>
#include <any>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// #include <External/Include/physx/PxPhysicsAPI.h>
