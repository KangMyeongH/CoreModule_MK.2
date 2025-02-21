#pragma once

#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

#include "json.hpp"

#include "core_typedef.h"
#include "core_function.h"
#include "core_macro.h"
#include "core_math.h"

#pragma warning(disable : 4251)

#define VK_MAX 0xff

#ifdef COREMODULE_EXPORTS
#define COREMODULE_API __declspec(dllexport)
#else
#define COREMODULE_API __declspec(dllimport)
#endif

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif