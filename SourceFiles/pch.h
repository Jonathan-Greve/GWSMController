//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <string>
#include <format>
#include <span>
#include <unordered_set>
#include <variant>

// Dear ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imconfig.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

// Dear ImGui File Dialog
#include "ImGuiFileDialog.h"

#include "GWUnpacker.h"
#include "AtexAsm.h"
#include "AtexReader.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

//Other
// <instance_id, party_id>
using InstancePartyId = std::pair<uint32_t, uint32_t>;
#include "get_first_integer_in_string.h"
#include "GW_skill.h"
#include "GWCA_Maps.h"

// Shared memory
#include "GWIPC.h"

namespace DX
{
// Helper class for COM exceptions
class com_exception : public std::exception
{
public:
    com_exception(HRESULT hr) noexcept
        : result(hr)
    {
    }

    const char* what() const noexcept override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}
}
