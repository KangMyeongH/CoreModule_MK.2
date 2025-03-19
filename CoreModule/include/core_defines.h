#pragma once

#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgidebug.h>

#include "VertexTypes.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
//#include "d3dx11effect.h"

#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>
#include <wrl.h>

#include "json.hpp"

#include "core_typedef.h"
#include "core_function.h"
#include "core_macro.h"
#include "core_math.h"
#include "core_struct.h"
#include "core_enum.h"

#pragma warning(disable : 4251)

#define VK_MAX 0xff

#ifdef COREMODULE_EXPORTS
#define COREMODULE_API __declspec(dllexport)
#else
#define COREMODULE_API __declspec(dllimport)
#endif