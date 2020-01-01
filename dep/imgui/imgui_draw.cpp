// dear imgui, v1.71
// (drawing and font code)
/*
Modified default SizePixels to 18.0f at line 1651

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImDrawListSplitter
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__) || defined(__APPLE__) || defined(__SWITCH__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference is.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wstack-protector"          // warning: stack protector not protecting local variables: variable length buffer
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)     IM_ASSERT(x)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)   ((void)(u), IM_ALLOC(x))
#define STBTT_free(x,u)     ((void)(u), IM_FREE(x))
#define STBTT_assert(x)     IM_ASSERT(x)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorStd(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDarcula(ImGuiStyle *dst) 
{

	auto *style = (dst ? dst : &ImGui::GetStyle());
	style->WindowRounding = 0.0f;
	style->GrabRounding = style->FrameRounding = 2.3f;
	style->ScrollbarRounding = 5.0f;
	style->FrameBorderSize = 1.0f;
	style->ItemSpacing.y = 6.5f;

	style->Colors[ImGuiCol_Text]                  = {0.73333335f, 0.73333335f, 0.73333335f, 1.00f};
	style->Colors[ImGuiCol_TextDisabled]          = {0.34509805f, 0.34509805f, 0.34509805f, 1.00f};
	style->Colors[ImGuiCol_WindowBg]              = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_ChildBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.00f};
	style->Colors[ImGuiCol_PopupBg]               = {0.23529413f, 0.24705884f, 0.25490198f, 0.94f};
	style->Colors[ImGuiCol_Border]                = {0.33333334f, 0.33333334f, 0.33333334f, 0.50f};
	style->Colors[ImGuiCol_BorderShadow]          = {0.15686275f, 0.15686275f, 0.15686275f, 0.00f};
	style->Colors[ImGuiCol_FrameBg]               = {0.16862746f, 0.16862746f, 0.16862746f, 0.54f};
	style->Colors[ImGuiCol_FrameBgHovered]        = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_FrameBgActive]         = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_TitleBg]               = {0.04f, 0.04f, 0.04f, 1.00f};
	style->Colors[ImGuiCol_TitleBgCollapsed]      = {0.16f, 0.29f, 0.48f, 1.00f};
	style->Colors[ImGuiCol_TitleBgActive]         = {0.00f, 0.00f, 0.00f, 0.51f};
	style->Colors[ImGuiCol_MenuBarBg]             = {0.27058825f, 0.28627452f, 0.2901961f, 0.80f};
	style->Colors[ImGuiCol_ScrollbarBg]           = {0.27058825f, 0.28627452f, 0.2901961f, 0.60f};
	style->Colors[ImGuiCol_ScrollbarGrab]         = {0.21960786f, 0.30980393f, 0.41960788f, 0.51f};
	style->Colors[ImGuiCol_ScrollbarGrabHovered]  = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ScrollbarGrabActive]   = {0.13725491f, 0.19215688f, 0.2627451f, 0.91f};
	// style->Colors[ImGuiCol_ComboBg]               = {0.1f, 0.1f, 0.1f, 0.99f};
	style->Colors[ImGuiCol_CheckMark]             = {0.90f, 0.90f, 0.90f, 0.83f};
	style->Colors[ImGuiCol_SliderGrab]            = {0.70f, 0.70f, 0.70f, 0.62f};
	style->Colors[ImGuiCol_SliderGrabActive]      = {0.30f, 0.30f, 0.30f, 0.84f};
	style->Colors[ImGuiCol_Button]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.49f};
	style->Colors[ImGuiCol_ButtonHovered]         = {0.21960786f, 0.30980393f, 0.41960788f, 1.00f};
	style->Colors[ImGuiCol_ButtonActive]          = {0.13725491f, 0.19215688f, 0.2627451f, 1.00f};
	style->Colors[ImGuiCol_Header]                = {0.33333334f, 0.3529412f, 0.36078432f, 0.53f};
	style->Colors[ImGuiCol_HeaderHovered]         = {0.453125f, 0.67578125f, 0.99609375f, 0.67f};
	style->Colors[ImGuiCol_HeaderActive]          = {0.47058827f, 0.47058827f, 0.47058827f, 0.67f};
	style->Colors[ImGuiCol_Separator]             = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorHovered]      = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_SeparatorActive]       = {0.31640625f, 0.31640625f, 0.31640625f, 1.00f};
	style->Colors[ImGuiCol_ResizeGrip]            = {1.00f, 1.00f, 1.00f, 0.85f};
	style->Colors[ImGuiCol_ResizeGripHovered]     = {1.00f, 1.00f, 1.00f, 0.60f};
	style->Colors[ImGuiCol_ResizeGripActive]      = {1.00f, 1.00f, 1.00f, 0.90f};
	style->Colors[ImGuiCol_PlotLines]             = {0.61f, 0.61f, 0.61f, 1.00f};
	style->Colors[ImGuiCol_PlotLinesHovered]      = {1.00f, 0.43f, 0.35f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogram]         = {0.90f, 0.70f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_PlotHistogramHovered]  = {1.00f, 0.60f, 0.00f, 1.00f};
	style->Colors[ImGuiCol_TextSelectedBg]        = {0.18431373f, 0.39607847f, 0.79215693f, 0.90f};
}


void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.90f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);
    InitialFlags = ImDrawListFlags_None;

    // Const data
    for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = ImVec2(ImCos(a), ImSin(a));
    }
}

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = _Data->InitialFlags;
    _VtxCurrentOffset = 0;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _Splitter.Clear();
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _Splitter.ClearFreeMemory();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(NULL));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : (ImTextureID)NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();
    draw_cmd.VtxOffset = _VtxCurrentOffset;
    draw_cmd.IdxOffset = IdxBuffer.Size;

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    // Large mesh support (when enabled)
    if (sizeof(ImDrawIdx) == 2 && (_VtxCurrentIdx + vtx_count >= (1 << 16)) && (Flags & ImDrawListFlags_AllowVtxOffset))
    {
        _VtxCurrentOffset = VtxBuffer.Size;
        _VtxCurrentIdx = 0;
        AddDrawCmd();
    }

    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superflous function calls to optimize debug/non-inlined builds.
// Those macros expects l-values.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)     { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / ImSqrt(d2); VX *= inv_len; VY *= inv_len; } }
#define IM_FIXNORMAL2F(VX,VY)               { float d2 = VX*VX + VY*VY; if (d2 < 0.5f) d2 = 0.5f; float inv_lensq = 1.0f / d2; VX *= inv_lensq; VY *= inv_lensq; }

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2)); //-V630
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y)
                dm_x *= AA_SIZE;
                dm_y *= AA_SIZE;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_FIXNORMAL2F(dm_x, dm_y);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2)); //-V630
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_FIXNORMAL2F(dm_x, dm_y);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(centre.x + ImCos(a) * radius, centre.y + ImSin(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * ( ((rounding_corners & ImDrawCornerFlags_Top)  == ImDrawCornerFlags_Top)  || ((rounding_corners & ImDrawCornerFlags_Bot)   == ImDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * ( ((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + ImVec2(0.5f,0.5f));
    PathLineTo(b + ImVec2(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.50f,0.50f), rounding, rounding_corners_flags);
    else
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.49f,0.49f), rounding, rounding_corners_flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col, float rounding, int rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, a, b, uv_a, uv_b, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(a, b, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, a, b, uv_a, uv_b, true);

    if (push_texture_id)
        PopTextureID();
}


//-----------------------------------------------------------------------------
// ImDrawListSplitter
//-----------------------------------------------------------------------------
// FIXME: This may be a little confusing, trying to be a little too low-level/optimal instead of just doing vector swap..
//-----------------------------------------------------------------------------

void ImDrawListSplitter::ClearFreeMemory()
{
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == _Current) 
            memset(&_Channels[i], 0, sizeof(_Channels[i]));  // Current channel is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i]._CmdBuffer.clear();
        _Channels[i]._IdxBuffer.clear();
    }
    _Current = 0;
    _Count = 1;
    _Channels.clear();
}

void ImDrawListSplitter::Split(ImDrawList* draw_list, int channels_count)
{
    IM_ASSERT(_Current == 0 && _Count <= 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _Count = channels_count;

    // Channels[] (24/32 bytes each) hold storage that we'll swap with draw_list->_CmdBuffer/_IdxBuffer
    // The content of Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy draw_list->_CmdBuffer/_IdxBuffer into Channels[0] and then Channels[1] into draw_list->CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i]._CmdBuffer.resize(0);
            _Channels[i]._IdxBuffer.resize(0);
        }
        if (_Channels[i]._CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = draw_list->_ClipRectStack.back();
            draw_cmd.TextureId = draw_list->_TextureIdStack.back();
            _Channels[i]._CmdBuffer.push_back(draw_cmd);
        }
    }
}

static inline bool CanMergeDrawCommands(ImDrawCmd* a, ImDrawCmd* b)
{
    return memcmp(&a->ClipRect, &b->ClipRect, sizeof(a->ClipRect)) == 0 && a->TextureId == b->TextureId && a->VtxOffset == b->VtxOffset && !a->UserCallback && !b->UserCallback;
}

void ImDrawListSplitter::Merge(ImDrawList* draw_list)
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_Count <= 1)
        return;

    SetCurrentChannel(draw_list, 0);
    if (draw_list->CmdBuffer.Size != 0 && draw_list->CmdBuffer.back().ElemCount == 0)
        draw_list->CmdBuffer.pop_back();

    // Calculate our final buffer sizes. Also fix the incorrect IdxOffset values in each command.
    int new_cmd_buffer_count = 0;
    int new_idx_buffer_count = 0;
    ImDrawCmd* last_cmd = (_Count > 0 && _Channels[0]._CmdBuffer.Size > 0) ? &_Channels[0]._CmdBuffer.back() : NULL;
    int idx_offset = last_cmd ? last_cmd->IdxOffset + last_cmd->ElemCount : 0;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch._CmdBuffer.Size > 0 && ch._CmdBuffer.back().ElemCount == 0)
            ch._CmdBuffer.pop_back();
        if (ch._CmdBuffer.Size > 0 && last_cmd != NULL && CanMergeDrawCommands(last_cmd, &ch._CmdBuffer[0]))
        {
            // Merge previous channel last draw command with current channel first draw command if matching.
            last_cmd->ElemCount += ch._CmdBuffer[0].ElemCount;
            idx_offset += ch._CmdBuffer[0].ElemCount;
            ch._CmdBuffer.erase(ch._CmdBuffer.Data);
        }
        if (ch._CmdBuffer.Size > 0)
            last_cmd = &ch._CmdBuffer.back();
        new_cmd_buffer_count += ch._CmdBuffer.Size;
        new_idx_buffer_count += ch._IdxBuffer.Size;
        for (int cmd_n = 0; cmd_n < ch._CmdBuffer.Size; cmd_n++)
        {
            ch._CmdBuffer.Data[cmd_n].IdxOffset = idx_offset;
            idx_offset += ch._CmdBuffer.Data[cmd_n].ElemCount;
        }
    }
    draw_list->CmdBuffer.resize(draw_list->CmdBuffer.Size + new_cmd_buffer_count);
    draw_list->IdxBuffer.resize(draw_list->IdxBuffer.Size + new_idx_buffer_count);

    // Write commands and indices in order (they are fairly small structures, we don't copy vertices only indices)
    ImDrawCmd* cmd_write = draw_list->CmdBuffer.Data + draw_list->CmdBuffer.Size - new_cmd_buffer_count;
    ImDrawIdx* idx_write = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _Count; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch._CmdBuffer.Size) { memcpy(cmd_write, ch._CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch._IdxBuffer.Size) { memcpy(idx_write, ch._IdxBuffer.Data, sz * sizeof(ImDrawIdx)); idx_write += sz; }
    }
    draw_list->_IdxWritePtr = idx_write;
    draw_list->UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _Count = 1;
}

void ImDrawListSplitter::SetCurrentChannel(ImDrawList* draw_list, int idx)
{
    IM_ASSERT(idx < _Count);
    if (_Current == idx) 
        return;
    // Overwrite ImVector (12/16 bytes), four times. This is merely a silly optimization instead of doing .swap()
    memcpy(&_Channels.Data[_Current]._CmdBuffer, &draw_list->CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&_Channels.Data[_Current]._IdxBuffer, &draw_list->IdxBuffer, sizeof(draw_list->IdxBuffer));
    _Current = idx;
    memcpy(&draw_list->CmdBuffer, &_Channels.Data[idx]._CmdBuffer, sizeof(draw_list->CmdBuffer));
    memcpy(&draw_list->IdxBuffer, &_Channels.Data[idx]._IdxBuffer, sizeof(draw_list->IdxBuffer));
    draw_list->_IdxWritePtr = draw_list->IdxBuffer.Data + draw_list->IdxBuffer.Size;
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd.
// Use if your final output buffer is at a different scale than draw_data->DisplaySize,
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * fb_scale.x, cmd->ClipRect.y * fb_scale.y, cmd->ClipRect.z * fb_scale.x, cmd->ClipRect.w * fb_scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3; // FIXME: 2 may be a better default?
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    GlyphMinAdvanceX = 0.0f;
    GlyphMaxAdvanceX = FLT_MAX;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in Dear ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 108;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
};

ImFontAtlas::ImFontAtlas()
{
    Locked = false;
    Flags = ImFontAtlasFlags_None;
    TexID = (ImTextureID)NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            IM_FREE(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        IM_FREE(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        IM_FREE(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)IM_ALLOC((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty() && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = IM_ALLOC(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.SizePixels <= 0.0f)
        font_cfg.SizePixels = 18.0f * 1.0f; //default font size
    if (font_cfg.Name[0] == '\0')
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    font->DisplayOffset.y = 1.0f;
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT(0); // Could not load file.
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)IM_ALLOC(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = IM_ALLOC((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    IM_FREE(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    IM_ASSERT(id >= 0x10000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlas::CustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBoolVector        GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsMap)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBoolVector        GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBoolVectorToFlatIndexList(const ImBoolVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const int* it_begin = in->Storage.begin();
    const int* it_end = in->Storage.end();
    for (const int* it = it_begin; it < it_end; it++)
        if (int entries_32 = *it)
            for (int bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & (1u << bit_n))
                    out->push_back((int)((it - it_begin) << 5) + bit_n);
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
        if (src_tmp.DstIndex == -1)
            return false;

        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Resize(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Resize(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.GetBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint, true);
                dst_tmp.GlyphsSet.SetBit(codepoint, true);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBoolVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels);
        const int padding = atlas->TexGlyphPadding;
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096*0.7f) ? 4096 : (surface_sqrt >= 2048*0.7f) ? 2048 : (surface_sqrt >= 1024*0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, atlas->TexGlyphPadding, NULL);
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)IM_ALLOC(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];

            const float char_advance_x_org = pc.xadvance;
            const float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
            float char_off_x = font_off_x;
            if (char_advance_x_org != char_advance_x_mod)
                char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

            // Register glyph
            stbtt_aligned_quad q;
            float dummy_x = 0.0f, dummy_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &dummy_x, &dummy_y, &q, 0);
            dst_font->AddGlyph((ImWchar)codepoint, q.x0 + char_off_x, q.y0 + font_off_y, q.x1 + char_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, char_advance_x_mod);
        }
    }

    // Cleanup temporary (ImVector doesn't honor destructor)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        src_tmp_array[src_i].~ImFontBuildSrcData();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID > 0x10000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas glyph ranges helpers
//-------------------------------------------------------------------------

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 1946 common ideograms code points for Japanese
    // Sourced from http://theinstructionlimit.com/common-kanji-character-ranges-for-xna-spritefont-rendering
    // FIXME: Source a list of the revised 2136 Joyo Kanji list from 2010 and rebuild this.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,6,2,2,1,8,5,7,11,1,2,10,10,8,2,4,20,2,11,8,2,1,2,1,6,2,1,7,5,3,7,1,1,13,7,9,1,4,6,1,2,1,10,1,1,9,2,2,4,5,6,14,1,1,9,3,18,
        5,4,2,2,10,7,1,1,1,3,2,4,3,23,2,10,12,2,14,2,4,13,1,6,10,3,1,7,13,6,4,13,5,2,3,17,2,2,5,7,6,4,1,7,14,16,6,13,9,15,1,1,7,16,4,7,1,19,9,2,7,15,
        2,6,5,13,25,4,14,13,11,25,1,1,1,2,1,2,2,3,10,11,3,3,1,1,4,4,2,1,4,9,1,4,3,5,5,2,7,12,11,15,7,16,4,5,16,2,1,1,6,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,
        2,1,12,3,3,9,5,8,1,11,1,2,3,18,20,4,1,3,6,1,7,3,5,5,7,2,2,12,3,1,4,2,3,2,3,11,8,7,4,17,1,9,25,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,6,16,1,2,1,1,3,12,
        20,2,5,20,8,7,6,2,1,1,1,1,6,2,1,2,10,1,1,6,1,3,1,2,1,4,1,12,4,1,3,1,1,1,1,1,10,4,7,5,13,1,15,1,1,30,11,9,1,15,38,14,1,32,17,20,1,9,31,2,21,9,
        4,49,22,2,1,13,1,11,45,35,43,55,12,19,83,1,3,2,3,13,2,1,7,3,18,3,13,8,1,8,18,5,3,7,25,24,9,24,40,3,17,24,2,1,6,2,3,16,15,6,7,3,12,1,9,7,3,3,
        3,15,21,5,16,4,5,12,11,11,3,6,3,2,31,3,2,1,1,23,6,6,1,4,2,6,5,2,1,1,3,3,22,2,6,2,3,17,3,2,4,5,1,9,5,1,1,6,15,12,3,17,2,14,2,8,1,23,16,4,2,23,
        8,15,23,20,12,25,19,47,11,21,65,46,4,3,1,5,6,1,2,5,26,2,1,1,3,11,1,1,1,2,1,2,3,1,1,10,2,3,1,1,1,3,6,3,2,2,6,6,9,2,2,2,6,2,5,10,2,4,1,2,1,2,2,
        3,1,1,3,1,2,9,23,9,2,1,1,1,1,5,3,2,1,10,9,6,1,10,2,31,25,3,7,5,40,1,15,6,17,7,27,180,1,3,2,2,1,1,1,6,3,10,7,1,3,6,17,8,6,2,2,1,3,5,5,8,16,14,
        15,1,1,4,1,2,1,1,1,3,2,7,5,6,2,5,10,1,4,2,9,1,1,11,6,1,44,1,3,7,9,5,1,3,1,1,10,7,1,10,4,2,7,21,15,7,2,5,1,8,3,4,1,3,1,6,1,4,2,1,4,10,8,1,4,5,
        1,5,10,2,7,1,10,1,1,3,4,11,10,29,4,7,3,5,2,3,33,5,2,19,3,1,4,2,6,31,11,1,3,3,3,1,8,10,9,12,11,12,8,3,14,8,6,11,1,4,41,3,1,2,7,13,1,5,6,2,6,12,
        12,22,5,9,4,8,9,9,34,6,24,1,1,20,9,9,3,4,1,7,2,2,2,6,2,28,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,8,8,3,2,1,5,1,2,2,3,1,11,11,7,3,6,10,8,6,16,16,
        22,7,12,6,21,5,4,6,6,3,6,1,3,2,1,2,8,29,1,10,1,6,13,6,6,19,31,1,13,4,4,22,17,26,33,10,4,15,12,25,6,67,10,2,3,1,6,10,2,6,2,9,1,9,4,4,1,2,16,2,
        5,9,2,3,8,1,8,3,9,4,8,6,4,8,11,3,2,1,1,3,26,1,7,5,1,11,1,5,3,5,2,13,6,39,5,1,5,2,11,6,10,5,1,15,5,3,6,19,21,22,2,4,1,6,1,8,1,4,8,2,4,2,2,9,2,
        1,1,1,4,3,6,3,12,7,1,14,2,4,10,2,13,1,17,7,3,2,1,3,2,13,7,14,12,3,1,29,2,8,9,15,14,9,14,1,3,1,6,5,9,11,3,38,43,20,7,7,8,5,15,12,19,15,81,8,7,
        1,5,73,13,37,28,8,8,1,15,18,20,165,28,1,6,11,8,4,14,7,15,1,3,3,6,4,1,7,14,1,1,11,30,1,5,1,4,14,1,4,2,7,52,2,6,29,3,1,9,1,21,3,5,1,26,3,11,14,
        11,1,17,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,7,7,5,17,3,3,3,1,23,10,4,4,6,3,1,16,17,22,3,10,21,16,16,6,4,10,2,1,1,2,8,8,6,5,3,3,3,39,25,
        15,1,1,16,6,7,25,15,6,6,12,1,22,13,1,4,9,5,12,2,9,1,12,28,8,3,5,10,22,60,1,2,40,4,61,63,4,1,13,12,1,4,31,12,1,14,89,5,16,6,29,14,2,5,49,18,18,
        5,29,33,47,1,17,1,19,12,2,9,7,39,12,3,7,12,39,3,1,46,4,12,3,8,9,5,31,15,18,3,2,2,66,19,13,17,5,3,46,124,13,57,34,2,5,4,5,8,1,1,1,4,3,1,17,5,
        3,5,3,1,8,5,6,3,27,3,26,7,12,7,2,17,3,7,18,78,16,4,36,1,2,1,6,2,1,39,17,7,4,13,4,4,4,1,10,4,2,4,6,3,10,1,19,1,26,2,4,33,2,73,47,7,3,8,2,4,15,
        18,1,29,2,41,14,1,21,16,41,7,39,25,13,44,2,2,10,1,13,7,1,7,3,5,20,4,8,2,49,1,10,6,1,6,7,10,7,11,16,3,12,20,4,10,3,1,2,11,2,28,9,2,4,7,2,15,1,
        27,1,28,17,4,5,10,7,3,24,10,11,6,26,3,2,7,2,2,49,16,10,16,15,4,5,27,61,30,14,38,22,2,7,5,1,3,12,23,24,17,17,3,3,2,4,1,6,2,7,5,1,1,5,1,1,9,4,
        1,3,6,1,8,2,8,4,14,3,5,11,4,1,3,32,1,19,4,1,13,11,5,2,1,8,6,8,1,6,5,13,3,23,11,5,3,16,3,9,10,1,24,3,198,52,4,2,2,5,14,5,4,22,5,20,4,11,6,41,
        1,5,2,2,11,5,2,28,35,8,22,3,18,3,10,7,5,3,4,1,5,3,8,9,3,6,2,16,22,4,5,5,3,3,18,23,2,6,23,5,27,8,1,33,2,12,43,16,5,2,3,6,1,20,4,2,9,7,1,11,2,
        10,3,14,31,9,3,25,18,20,2,5,5,26,14,1,11,17,12,40,19,9,6,31,83,2,7,9,19,78,12,14,21,76,12,113,79,34,4,1,1,61,18,85,10,2,2,13,31,11,50,6,33,159,
        179,6,6,7,4,4,2,4,2,5,8,7,20,32,22,1,3,10,6,7,28,5,10,9,2,77,19,13,2,5,1,4,4,7,4,13,3,9,31,17,3,26,2,6,6,5,4,1,7,11,3,4,2,1,6,2,20,4,1,9,2,6,
        3,7,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,5,13,8,4,11,23,1,10,6,2,1,3,21,2,2,4,24,31,4,10,10,2,5,192,15,4,16,7,9,51,1,2,1,1,5,1,1,2,1,3,5,3,1,3,4,1,
        3,1,3,3,9,8,1,2,2,2,4,4,18,12,92,2,10,4,3,14,5,25,16,42,4,14,4,2,21,5,126,30,31,2,1,5,13,3,22,5,6,6,20,12,1,14,12,87,3,19,1,8,2,9,9,3,3,23,2,
        3,7,6,3,1,2,3,9,1,3,1,6,3,2,1,3,11,3,1,6,10,3,2,3,1,2,1,5,1,1,11,3,6,4,1,7,2,1,2,5,5,34,4,14,18,4,19,7,5,8,2,6,79,1,5,2,14,8,2,9,2,1,36,28,16,
        4,1,1,1,2,12,6,42,39,16,23,7,15,15,3,2,12,7,21,64,6,9,28,8,12,3,3,41,59,24,51,55,57,294,9,9,2,6,2,15,1,2,13,38,90,9,9,9,3,11,7,1,1,1,5,6,3,2,
        1,2,2,3,8,1,4,4,1,5,7,1,4,3,20,4,9,1,1,1,5,5,17,1,5,2,6,2,4,1,4,5,7,3,18,11,11,32,7,5,4,7,11,127,8,4,3,3,1,10,1,1,6,21,14,1,16,1,7,1,3,6,9,65,
        51,4,3,13,3,10,1,1,12,9,21,110,3,19,24,1,1,10,62,4,1,29,42,78,28,20,18,82,6,3,15,6,84,58,253,15,155,264,15,21,9,14,7,58,40,39,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesVietnamese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x0102, 0x0103,
        0x0110, 0x0111,
        0x0128, 0x0129,
        0x0168, 0x0169,
        0x01A0, 0x01A1,
        0x01AF, 0x01B0,
        0x1EA0, 0x1EF9,
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c < 0x10000)
            AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    int max_codepoint = 0x10000;
    for (int n = 0; n < max_codepoint; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < max_codepoint - 1 && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)'?';
    DisplayOffset = ImVec2(0.0f, 0.0f);
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= IM_TABSIZE;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size-1);
    }

    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    int index_size = IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = (float)(int)pos.x + DisplayOffset.x;
        pos.y = (float)(int)pos.y + DisplayOffset.y;
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui:: functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + DisplayOffset.x;
    pos.y = (float)(int)pos.y + DisplayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            s = (const char*)memchr(s, '\n', text_end - s);
            s = s ? s + 1 : text_end;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((ImWchar)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = vtx_current_idx;
}

//-----------------------------------------------------------------------------
// [SECTION] Internal Render Helpers
// (progressively moved from imgui.cpp to here when they are redesigned to stop accessing ImGui global state)
//-----------------------------------------------------------------------------
// - RenderMouseCursor()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
// - RenderPixelEllipsis()
//-----------------------------------------------------------------------------

void ImGui::RenderMouseCursor(ImDrawList* draw_list, ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor)
{
    if (mouse_cursor == ImGuiMouseCursor_None)
        return;
    IM_ASSERT(mouse_cursor > ImGuiMouseCursor_None && mouse_cursor < ImGuiMouseCursor_COUNT);

    const ImU32 col_shadow = IM_COL32(0, 0, 0, 48);
    const ImU32 col_border = IM_COL32(0, 0, 0, 255);          // Black
    const ImU32 col_fill   = IM_COL32(255, 255, 255, 255);    // White

    ImFontAtlas* font_atlas = draw_list->_Data->Font->ContainerAtlas;
    ImVec2 offset, size, uv[4];
    if (font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
    {
        pos -= offset;
        const ImTextureID tex_id = font_atlas->TexID;
        draw_list->PushTextureID(tex_id);
        draw_list->AddImage(tex_id, pos + ImVec2(1,0)*scale, pos + ImVec2(1,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos + ImVec2(2,0)*scale, pos + ImVec2(2,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[2], uv[3], col_border);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[0], uv[1], col_fill);
        draw_list->PopTextureID();
    }
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

// FIXME: Rendering an ellipsis "..." is a surprisingly tricky problem for us... we cannot rely on font glyph having it,
// and regular dot are typically too wide. If we render a dot/shape ourselves it comes with the risk that it wouldn't match
// the boldness or positioning of what the font uses...
void ImGui::RenderPixelEllipsis(ImDrawList* draw_list, ImVec2 pos, ImU32 col, int count)
{
    ImFont* font = draw_list->_Data->Font;
    const float font_scale = draw_list->_Data->FontSize / font->FontSize;
    pos.y += (float)(int)(font->DisplayOffset.y + font->Ascent * font_scale + 0.5f - 1.0f);
    for (int dot_n = 0; dot_n < count; dot_n++)
        draw_list->AddRectFilled(ImVec2(pos.x + dot_n * 2.0f, pos.y), ImVec2(pos.x + dot_n * 2.0f + 1.0f, pos.y + 1.0f), col);
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

static const char roboto_medium_compressed_data_base85[144680+1] =
		"7])#######hb'0,'/###I),##bw9hLbu2@$v(+h<CiTgY#3_Y#)u(###9'e=N9o6U.0bY#9D)##G)te=1?`b&6K)##u3k34;vA0F[Px;/3''##Tj&##-V:'ISHUPSCX%##[/k--'/Y-G"
		"nvS2t0))##:I0R3&7#=BejUS%ns1_A,$j--r2/-DtuF'[dvp;#9TM:#%Z$^DUj<d6jDH&#dg[0#'m@aDGigjs*GCgLspIiLh$tLD8-s8&;.#wL)mGoL<x$KDLG?a.)RX*MgXL=#+cunE"
		"5im/klnM5#h,0%#ano:Fur6D%/Xm--nGq92>4^=BAE-pB6r[Y#_arI0/4HiF#a$AFDpN-Qctq--AcFVCYfwrs2o[f1s<Mt-tnRfLV_6##oN.gW;he<6ba?iL)B$##?a,e([_a/1t-?@<"
		"*8XAug_B^%KDi9../5##%iBB#At2aE4QZ9M)<4o.ta>T@<njk+'ug>$sZ>hL3J:2#2O>+#DHoY-K3rB8_X@%#4L.U.lf5N#-64V.sU%h$0r9S-J7r5/_5WfCXFB`NfV$##deCvujh$mK"
		"DMOV-6*w(#Pr+r9d9%##'Gc##3lOfCrU:##9sh+V>n$?$@F7#MtPG&#7gDuL3bGhLuwSfLLxhP//KWuP#+:JCjn3pA&ZqV%4%q:#S7+&#*FV,#BBR8##tE9#-hW1#cFL/#+e_7#7D)4#"
		"S>E)#-+L'#L>:@-26T;-r6T;-p5T;-n5T;-R5T;-J6T;->6T;-d5T;-o]rS%%b.v,91j7ng`mJ(6IA5J8nnM06r);m9mOm/,C&^=F9Bm&KYe>5t,)Z5Bb5)N.+XMCW%L_GB[C8[QC>58"
		"<&P>Ga,(Q&5;Y&>kk-?#:^quZOU/,Nh$^A,1l7pAo'Uk+gA^f:W*E_/l4o1KhMdf(eH=G2bq)20e]>8%IoU`<0b,GM=7FG)HY$s&rEt%4qk$v#@H^c2+Obr?%aUlJQ)5D<13o-$AJM]="
		")AHJ1XQ4A=l%-tIXYm1K19sL^3P-F%47ZxO#K^w04qXoIdfYw0883aOExSlfj;a]-p`.>YLZfYG>EMcr1g`fqN*As$+UAVmdA,F%OV'8[`?nY#[+2F%tpB&=dJjY,+;@2KA$32^$?YJU"
		"U31kXw]KxtSDS`s[MecMvrAm8e[UVQA1IYlAr5&F6Grw'`)/DNZiMiK&_>PAX)7;6jX*c41BfxFv+YrQY`+29KD=YPM-e%F`hI2L-GxQNBj.;#Vp39#X9U'#&C;,#s7?;#G_N-vcw8*#"
		"OO*1#[HR8#NMZ;#tnU7#mui4#vEK2#vQvtLxd.3#[Yf5#_qe%#Oq53#p6]-#A[G+#2T^2#^[qV%>'4m]t2;Z57Y)m&<_u`NG(TlfB+YAOt/lfUrPm`WBBZfqnCC5/o*Y;6gc_rml,NS["
		"vSO`jS'&pJ<>6W&YT%5]@Ric;(S5Giki8v,02^`EPV'##Y((##_</F%@$0F%a..F%.B,F%nxno@_bUw9`.mQj)GL/#.`C7#gh$0#g/w(#-?B2#F`o5#)l$B4`*N<#s4/5#sW`,#mL*1#"
		"D0.8#NstgLN,P1#1t7-#Lxe%#L;:X3<pbuG$tu=YR*()*I:p.CHCIJ1d*o2L[Y1A==Wo-$]:,87xZJ2LrNn9;qRJ8#c^g2#Ai+6#ooH,v'7(/#Z/,>#NKM,#rqN1#=E92#v[`,##L/Z0"
		"ndl##kZY##tLcwL,iNjhpc.#YgNL>Y`#c5&mNuPArQv%tD';&OTu6jTh:5&t1SxA+;]'&t]GOa3k`uf$;>ZC%rc@L$?a^>$UnwD$EI5/$]kJI$GG)w$gRK>$5nLC$FrSI$%RPJ$:xGv$"
		"%M#<-$K,hLTMJcruujig(_glgKdZkCmkuYPG3uloi8rD*c;'^FU^UJDsH,A6T;eGM)<4#GkYK>G?HZ;6OR>2g,[A;m/d?)OFC+F@KFg*#ViG+#TK=*vaoajLW;.##f&;$#Ns7-#wq&*#"
		"b'e(#`x;<#:JQ;#6n?(#;<P%2,GF&#?*W<#+i$0#<RvtLG5g%#E.M$#P@2X-EHBk=S`$;H:+Z[$RY0F$#t*E$;cX.#s%a$8d,MTJrCW@XBM0o.lQvf;_Z*]$$p(K$*5NH$vH5/$GAaH$"
		"NexvL94%QK:js:Qb,AYPHgY`3`:OV6:(;SRQC8MKdtIiTH&;MKfEm+DKqT8B3NBJ1jx05/s]m.LeB&;H5oJcMdV%)*bJ%SeeJ8_8N=?YPX3Fk=T27MKWEo%=)P*8@,]`s@#S[`37Da-6"
		"`t/A=.S9GDfg(5Ad<1;6*E#,2@x#/:_qXEed`;7#_>d3#i/#>#]Em3#NKG##[9_'#(5/5#g6+&#m3),#V$0kL;i/%#T2C'#`APF%^0c6#/f($#X>:pL/9S0#-8TkLJb<4#xRd&M#JG8."
		"d6`i9):QZ$Ec[-%w3dZ%9#@;$H<ZiBj(@D3%V%)3mg[f:Q0I`E,cFS7aTt(3p)IS7s1ES@$WI`NVL,AOYN)AO-A^i9rd[f:YLL`E8Xjx=E<FJ1Q?#&4ADOcD<LBPAfJSV-%YQlSSp8SR"
		"=Hu%42,VlJhr%5A?2XV-.*fl/fV-kbiL'-#J%@0#5=?;#$,-;#PQq'#mju##TV.%#JwZ(#alT:#O#=9#'^;4#gEK2#wg#3#*<W'Mc$'0M#d85]Vvn29[7VJ1fSfYP]gB5Ju5qJ(8mkM0"
		"_*=m&O(88@`uclSFH(Oq3[1^+R^$#YQ(DG`%hr8..LmZ$hi`w$&?#+#>&lY%`/'^%ds*E$?PJA$^i(c#:U4Q#6le)$mVPPNX^tP&wm3.$i3]nLHZh8Mq?jV$Z:nfU;f&mAt%</L#o]9M"
		"N/hxkc:3>c6Q(FXZg8AkBvNxt,N<JhQ4J8@ExkM_P7Vs$j[p(k4cYcacM3)FsamJ;c3/F%eFgr6bF?_/AVp+DV=ro%XLdl/ERbc)iZWV$4sZs7h[$,;Iu62Lt+ds-'S,lLvMpnLlA^nL"
		"X0N.N_^H'$pF#j#DFJvLYqA<$:Ye`$^M.h$@DiK$/[Ae$_iu8.+R&I$I=XjCWG#d;8]m92W5GkFZST-QE1u#$W,>B$>kWV20@l^$l4.`$w*iC$m:ba$UB'wL6oQ#>PxMv>6QbPSWVw.i"
		"_YJAG2]ZZ?5u/Z52XdM9;Q/>m5`dV@?q/;e/*2Q9O=4p@f)r&PDadGMUbw5&OXRv?S1l;-@n9/DM7[2LM%6sQY65dN79<s-pSwuQQ<NMUX:aD+G24FF=)*/q&X0s?F7sxb)v0pRjPQw9"
		"WjDuLb7->#1fm&M`C]7MNqN//e1GM9/(i9;`Sll/,vUlJId8w^7e8D<&J*8@kd<G;QIx9)C74R3CJM]=WHo%=d[1DEP8p-$j2LiT=n3F%HA5DE/v0PSn9.AFT:Uw9Z]ii'q@w%4XEEp7"
		"$wQ?$NZ0F$EC($%<.q?$OTQG$t1+e#.kNZMi;p%#%&4xuU&,F%bs7-#D$,>#qC)4#sH`(v&nA,26TnIh0PHJ1cW9on8lef(nGUA5^4N,NF0MAO<O:5809@58P`1F%?`q-$jS+sZCh>)N"
		"s,B/$P4o1K27iP/e]>8%+Obr?eKaJ2W7i8.q<.D$)uT^#Iwtm#XSR2.xd*$MT^PM$xu#<-@EvaOg<2/#,7^.$/=H;#9c/,M)giFQ^XI'MON_PS]$[lL_f7iL0$3FM&88Q/Z:l(EXQ4A="
		"Gg;<[-NO>G6G=58w[E-dmS1F%1M2F%sf1F%Wk3F%h?@58G-bP&h.958%vfP],?>m&RVq92Ie@5/>v,Q&./gM1+mF&>^/+F.v%t^f#QOA>sbvfV;&CF%g%cxO+MC_/2qXoI>bm?$A8=i$"
		"jZ0F$;e%(QNCHYuAFxof-WTF%v$SF%]c'/h=EwV-l`?I%RPHJ1,kWS%<6<)Nv'>9#?l>lLh;2/##Fh,2eH=G2<)XV-e]>8%nCrl&.IH?$I,iC$au@(Q?E2dM7Y;&bJpZGMoFk`NF$B5J"
		"QS,.$SqMS.$Rso%e=e2LVdfxk;IAZ$..LB#sq.P#&*HtLwmMxL>_&L$Ulu;-IO[fO#T+1#s<g.$1[WV$JNsE@+<g.$*Sl(E[[hw'Fo`4#)g5s-P0'sL$3tuLaUTvL1;Y$MgF`hL^58?9"
		"[(L#$86Tk$'Jg9V,WTlX19(Ks,lu8.DioA$flQ#6(nLDNY@%>NdHo8%(U5,M2$I6%u,L'#*QJ878lef(]1q+;:N^x=x%`u>k3PxXM6oXlJk$P]WB?`NAw-Z$C78v$hoU`<`'AW-?$%La"
		"C1;Dj5Xnxk?<Y/(;U:58r6S_&0L*1#3t7-#feN1#N,/5#9rha3@q53#D402#2Cu6#YI/=#CSY##PS]F-iM`.Mj(9-#-qK&9]>BZHB8+/hAUo&Qk@]O$raD[$kl=<M*]b5&/M7:V#0U,N"
		"fE1#Yg2;;[A1r;-M(l?-CfG<-[wsnL_d%b/+Obr?qIM]=):4,NZZPL3CESG;Gd<#-Q2k%$]kJI$<OYb#X'wg#@@xl#39_N$O$&;M[HrM%-0oJjl_jr?f0alS,`(g:07D5/==.G`_E*?#"
		"`c/)s(5kE[Y-o-Z]tUZ$mAl^$ahuD%DsBn%DR<x%@=:H/GW(C$vZpt8.&RwBe.M$#2c&*#qa;4#aRU/#jXqm8dEOW8=X-m8-PdZ$V1rC$<3:5MZ(.A-1A4Z/eq(K$G%pe$A%23;22H9%"
		"$0/d<P[#F7V;d5&ENBVmr@VG%tq?YPB^?PAj8PiT(+ZuPbq)20LMou,[[L9ivdFJ18uTV-I;Tx'Zd5#>x[C0--$&?nV&n+#>aXwp>r9#vHN>'v:_hS/aW#DW$*WuPfM&WRS,:SIQMYBO"
		",JI?$C_v'T08F?%V]#q9OP5)%6ntPSmX,X&0BRK<-ou8.o%`C$:QJ]$@DiK$&[lf$lq(K$ErSI$/Pd$$(EJs78($Z$HhdDNv(dVHr7*s?[7`d+6HjB%k.(/#Sa(vuTmA,28XgIqd2>ul"
		"H6X:mpoa7[#IV)N_r)?#lwZv#FVrVot1NJ%MRq?$=6OE$;g-s$&aU3=b6*KV9[ml]GOrWAek?Q%$AsG2fW<1#fI#+#h[&au5ne[upCY)%sxGYu(vRlf#--^4aBjYGlIQ]k5dGm&?l)NL"
		"4p)Z5S[aJMW0fF%[b@j9XpO?Ica2^8'pX_J-vkw9oXm;-fF1X%U7*Q/hMdf(2$-)*1;m&mO>o3%3Qeou7>m3#L0=xum'+xu?)<%v+Kn4v2ThfuT&a8v2#/>-)E<j$u,L'#1'AN%DPX5;"
		"U53eb>h=)#2DE)#5g%-#6p4:vqscW-+<jK322%a3`wLxtCFBR*<0t8.7equ,6-IM_di0>mTvRDkw@8,MK4O'M:X+rLu;]0#YL:_Af9Ik47Mf9M3G&:)>%LZ$01@<$wU6R*_+WJUtJr2`"
		"[&h>$o*Mc#aLh/#$),##k542&2/###,P1v#;=[s$/cU;$&A^;-&;TV-^5c'&W@es$'=dZ$7uUv#cRXGM.IG,M0L>gLSnM;$0cCv#^BLe$PchV$hih$'ZfLv#81@W$1VuY#GN7_8Zf1Z#"
		"8(%<$7+%<$8OC#$]cPn*X1[8%=CwS%ulAv$2Bd$0Z9[s%a:+gL'@,gL-4pfLX>tf-^uxa[v[B#$$,Tv$8VMq2'wJe$Bu'Z$IH[w',eGq;Z:e8%Dw86&A3n;%2at?03kMs%i=YB]$WE-Z"
		"V=n8%H0%Z$g9)Z$9qm;%]TNh#_UET%<L*9%]V%njaR*9%?+i;$.N5lM>%KgLA9gV-=7Fk4w62[$?Lns$AF[s$$:Lk+Ljk$0vP/R37V8U)[LEp%hU4gL&qSs$:@RW$3JlY#->P>#/u6s$"
		"^(]fLLkV;$*$Pg.8=*p%,.VT.;O<T%N$@E.:1Is$DB<s%':vV%102R<UxhV$wbT#$cY$@'aLes$=Ujp%nNp$.^=4,M8FPGMFrcW-(G+_f+)Q$g$bN1#9cBv$>Qeq2gMBh5wEn0#pc(R<"
		"X1Is$cC+gLHICp7t4w<(5U-Z$n9B$.ee'HMOE>,MT13Y.;..W$A#PwMGOx>-U4%BMT0gfL.;#gLPs@jL9Bs#M/t:*N):)>&nf]@'I*(KE6j]c$&+?v$b)Z#@6Rrt-_It(NT?JW$#eDe-"
		"PY1Z#u:qKGRwBI$W4Is$47l%O66OGNIgMa-vdoQaGB*W%?fo'._7ofLRXaVOu:TgLfn`;$%h@$pL&hw0rMXt(:r+7*[Cn8%`F+gL0/8W$c?%W%$%Qv$IQ%JM#4pfLC>/W$9(.W$[7xfL"
		">IG,M>%$&%d$m>$+:HxLi%j;$bW<L#T](Z#/PlY#LR(gLXR]s$B[eW$a@xfLhRGgL=dPW-r4nw'-`O,MjO^[%T%r;$%x5Z$6,q<1a&v>@>+/v$S8f#$oJ#2T-:Ir7+&_>$uad>$B_bd4"
		"W'j;$hO4gL8n_DNeNLTM:98Z$'FvGM:FWYPhMYGMj48b-8$xjk9p&W%rog;-Tk-R8&c0^#t&Mt/2o_;$0YL;$w+TV-NO9dkF`ni.;4Is$04`T.%####)flS.B5>##5:#-M]-*$#]jN1#"
		"rXwqLCpw;#THY##w+ST%pK/2'BG)GV(H7VZTd5Yc/aGrmhD+YuEEPJ(rNRP/SSU]=AAx+D##[iKGWarQxG<S[lVOuu$sd]4=64G`dMDVdNbg+r-S:v#A-,j',*@AOHZsrQ&TNS[W)MVd"
		"veBMg&g=p.J9I292K`MBj&(pI+n15f:d9>l#/u>#PsU,)Jk;;7,D,MqM.`(tf0uS&4+-g)Ns9v-fbbJ2&6s]59V-p8WkulK^+_]Ym-sr[1pdf`(PT/)HaBv-eR4j1rt<^#'5Fm'>hrD+"
		"[%Am0f.:pAEWEvd$2Hwbx1G:vw(Y:v*Pj+M'5oiLS?ZwLOJU5#V+78#1BZ;#k1PY#GF7@#tvTB#U*;G#tVj3N([EO#dlv5.j#<+N#uU%M56O'McTC`N6;RtLxt-/$av@1$@Vh3$E_d&M"
		"?/:'#,acG2m7v`<T[2#G.%l`N)ODGiZBv(s@6uJ(H_vu6+X:JVGS,Jrg=T'#1lK/)EKX>-,Gj^o&*WA5`1rGWa:$#ZnAaM_#Lu_ogW:_ov:]_olZl^o^/i^o^/i^oY&r^oV&###Tu$Vm"
		"6F/]b)oi@bZhG]OOU/,NK1niLIu62LGcUPK?p&vHN%)XI>I.8%t>n_st,Of_2HU]4tYNl8>0[Fij`mUd%w>xXhnE(WEwc@O9`)vd:c0I-Llm4&.9L^Yl>a[t/Hoe_%0mr[pw@DXnXmJV"
		"mjqbrfs9Mq<lOg1R;%Vmb3mbm[h-CmDg6_mI:RGl]mc=l9`_Fl.5:'mJZF&m7hiClcv<>#b.i'M8QKOh,qA1f:IQ1JtPDxE.BpIBV0C4Rj(V@N<n%fK#s7uFs+vR6C->bhkw7^k%'iuY"
		"*=D>#q<u.$fsQ&#-74R-,5T;-NMT;-dELW.:YXI#kbR'O`XBMBU+Ee-#d'##Q%R_&.jIP^wUk]uK1?5/g$C2:w`K>?V@L]lk>$jq,l6s$]WHf$v@gwO<C`T.d5+##2.GDNDW#6&)WYb#"
		"lblb#pn(c#L@Eh#ua@d#6nRd#:$fd#[4n0#b0xd#B<4e#D6f-#/Yjh#JTXe#Nake#PZF.#3f&i#k`?g#olQg#qf-0#7r8i#w.wg#%;3h#0xAi#_+K*#N2^*#d]=m#4jOm#8vbm#h4^i#"
		">2(n#B>:n#FJLn#HD(7#C@pi#Ncqn#Ro-o#V%@o#x4B2#(1Ro#_=eo#cIwo#*AT2#>T^q#3n,r#9$?r#PKGb#n$;c#6ext#pr4u#NIKq#/bpq#W9J5#'sc+#,>i($%.),#3mk2$8$o)$"
		"7rt2$;q7-#>==*$=(13$?.:3$A4C3$G?o-#EFx-#CG_3$Pm0+$T#C+$X/U+$Yvk.#_H$,$^3n4$Yt$8#]+78#a7I8#h_W5$dHe8#Rpb2$r,96$t2B6$w43-$x%I0#w$X9#*NW-$Y8e0#"
		"*?n0#/as-$.W#7$-B0:#2k>7$,,R0#8#B.$;.T.$?:g.$@+'2#8'Z7$D792#MTv;#T&/9$Lf;<#Z8J9$PxV<#^/j<#WJT2#]lY/$[[+:$^b4:$s%u2$kH(v#mN1v#^?D0$`9vN#,Gc>#"
		"wO`0$$F2O#2Y(?#9ve)$<&YQ#Ree@#X-Xx#BDL3$D8uQ#x;Q?$DNbE$T'_B#%3qB#,Z)@$9q@H#0g;@$-X7)$Kf6O$eF,F#4sM@$='SH#8)a@$iR>F#9pvC#=&3D#CD&&$OrHO$oq1($"
		"S([O$T)UF$qkcF#HYSA$uwuF#IJjD#lWrG#Rx+B$1eI)$V+R4$)e8E#*m9F$&pJE#(TkE$7C%D$9a'F$(8>'$ona5$s*'6$w696$>-gE#rN7D$geV8$V9#F#]qi8$,1_F$3Vl'$.4U+$"
		"W#WW#QvTF$[&(J#D;pE#`m:$#(X@p&m]sfLK[D;$3r&i#$*Gb%laR,M[3E$#^_kgLw-gfL>2oiLGjqsLx3pfLmQMtM8Oql8oUdfVvW9>Z,owlBwaTYZ$]0^#hM#<-x6T;-37T;-86T;-"
		"8[lS.@7YY#nM#<->CT;-NN#<-;7T;-WZ`=-uM#<-HIpV-KI.F%Erq-$_5<R*(1/F%K.r-$L1r-$K-o-$M4r-$fJ<R*.C/F%RCr-$SFr-$M3o-$TIr-$nc<R*3R/F%_t@.$S^0F%[_r-$"
		"x+=R*:h/F%i<A.$Ta0F%f's-$)A=R*C-0F%l9s-$m<s-$Q?o-$n?s-$0V=R*I?0F%wgA.$Wj0F%G&,##x05R*&hqA#C1^gL:bQM93?kM(%i^>$9T0j(KU5s.&#xuQHRP8/xIO&#XN#<-"
		"IGpV-TQ'KsQ=UkL:[2xLZ_ZhM)$[xOTjA,3U5u`47Z6;[ZJ:&5xIO&#ZN#<-W5T;-sX`=--O#<-`5T;->k*J-[N#<-b5T;-&Y`=-5O#<-i5T;-(Y`=-^N#<-k5T;-.Y`=-<O#<-q5T;-"
		"0Y`=-`N#<-s5T;-<Y`=-BO#<-)6T;-2gG<-PO#<-+6T;-6gG<-TO#<-/6T;-8gG<-lM#<-16T;-OgG<-XO#<-LHpV-A21F%[g4.$]>AvL2TgoL3)l4SrXHJVkIdfVt)WSSI41kb,VI_&"
		"kfH_&h68.$uqUxLvq+(NEp'&=t.J`t$]0^#9N#<-q7T;-1V]F-l7T;-65T;-75T;-J7T;-85T;-95T;-N7T;-YGpV-HG1F%%o2.$di+wL7PLwMKNmuY>OR]c;Lnxc0/PMUAkNYdxIO&#"
		"QS6p-J't>[k+E*M1YlS.un(##1CMX-XZ/I$#;I_&$k/.$qbLxL.nCdMtqJfL`x,j'6rj-$sn_xLH`0gMIEm:ZQONJMNLjfMx20W-:,H_&r)SD=TkJGNxIO&#'O#<-Q6T;-S6T;-r5T;-"
		"YBT;-#N#<-W6T;-[6T;-v5T;-]6T;-L&kB-(O#<-_6T;-a6T;-%6T;-fHpV-`72F%cso-$dvo-$(om-$h55.$%IR#M#/gr-MM1F%*um-$k>5.$&O[#MqT`9N,r%qLqQMtMRAb1^oUdfV"
		"1x<2CqhDGWxIO&#.O#<-n6T;-o6T;-.6T;-tHpV-eF2F%u`>.$4U/F%rJp-$mo3kX0J2F%tPp-$$dp-$14n-$%gp-$A3;R*2P2F%=0i`F7M'/`xIO&#;O#<-07T;-17T;-:6T;-6IpV-"
		"rn2F%7S?.$=q/F%4>q-$rPM?.jKguc7xs-$@$0F%<Vq-$Qd;R*D13F%>]q-$Biq-$=Xn-$Erq-$Z)<R*WM[ih6rj-$n`39#g7Wt(x_B#$kEm>$8I,hLo4roI$c'^#wCF&#Wai>$61^gL"
		"p=75J[-q<1xZj-$DYm&M+:#gL&L>gL7XUsL+RGgLU'3$#$`v&M1*`dMvt//L/qR5'$`B#$nOp>$A1^gLw'KJL1-4m'wCF&#_E.@$7G:;$6>uu#TH:;$3GpV-/R3F%YY###)=5R*PT0F%"
		"5H0.$J(N'M:a[eM%LccM8mG,*6s(d*v&0W-#8D_&xeK#$SN#<-<GpV-/H0F%<^0.$N@s'M>pIiLUP^88ltg>$IfG<-%2RA-UO#<-GGpV->v0F%Dmk-$b<6R*Wk3F%QG1.$Y#auLSC_kL"
		"RU$lLrD?)MZw)iM0Z8;QX8YD4V>:&5v5#5p[SUA5%fK#$_N#<-X5T;-Cae^$_=)mL*>pV-B,1F%au1.$g&M*McH<mL`NEmL>?0vLhpJjM=//crfXfS8%fK#$cN#<-gGpV-NZ4F%g12.$"
		"aMJvLimsmLnMpnLsDj*MvoukMR8+5]t,8)=%fK#$eN#<-uGpV-Yv+F%u[2.$cY]vL$2DlMJIdY#xPOA>w`K>?8/95T&vgY?(>m;@'D(v#);dV@%fK#$kN#<-*HpV-d:2F%*(3.$i(>wL"
		",YVpL*fipLP@,gL3:43N%G;pL0r%qLe-m.#ZIFgL=kFrL:qOrL/[]#M?wXrL>3urLHKihLC9(sL@?1sL>go#MHa6pMYx>J_FA#sI%fK#$0O#<-C6T;-&@:@-<M#<-Q6T;-ZgG<-IM#<-"
		"S6T;-&s_d-=p0F%9a6w^XpMuLc1tuLhikjL`7'vL`X,sM?1YSRc>srRav*j1dG88SaDSSSuMk]>bMooS#NYQs/'=$MhhpvLen#wLg#0nLft,wLmSsq-3Y-F%h,p-$U%=_onV1F%YGTw9"
		"T`-F%l8p-$m;p-$i.m-$n>p-$1X:R*Vf-F%tPp-$Op]&dO^;;Z)$UYZkHZJ;xjpuZ#t5;[F7$,a(3QV[%0mr[2OpfD&928]'BMS]mZ;,<(Kio])T.5^Gts]5.jIP^+gel^cK/W-jI2F%"
		"B6;R*_(.F%3G?.$'./F%02q-$E?;R*b1.F%28q-$foc>edh[`a:+;DbGG^ca;4V`b81r%cJD(W-%@J_&:Pq-$01n-$;Sq-$Pa;R*f=.F%=Yq-$]0Cp^B:.&MYRjJ8/tg>$EO#<-B7T;-"
		".6T;-C7T;-TD^4MC4O'M*>pV-HC.F%PF7.$BD?&MUZ^$Nm#DSeSc8GjS%P`kof-8fX:l%lU71AlxLfP9ZLL]ljAQfr[J`lgoVm+slS2Gsmf,W-Ta4F%,J=R*`-4F%oBs-$F)v92kL.F%"
		"qHs-$0V=R*b34F%sNs-$tQs-$^g&##[a/.$h'AnL&=PcM6<:Po$c'^#.*O2(up/W-ruC_&#og>$:N#<-05T;-$I,[..5SY,+VjfL)kirZa+3v6^(N;74?:>ZM+72LNLjfMID(W-p%I_&"
		"npK_&gB4F%g*s-$pvK_&sf1F%i0s-$Ej+##h9%##nI2.$J+&5#o-Ak=G_YgLCB2pL6'2hLJvf0:3qa>-9fG<-:fG<-6N#<-7GpV-vm/F%5?k-$P:pA#Ep8gLn&:v-]`+#HKU5s.EOlS/"
		"G2'vHRWaJ2#SkA#FN#<-P5T;-Z$3b$vtQlLa$[lLdQLsLbKjiMx6,,M8camL*>pV-*?0F%@k3.$E[gsLO5wpMrU<MKqhDGW&og>$;N#<-q?T;-<6S>-xgG<-s=D].oE4;-#IpV-MI.F%"
		"#j5.$M6ZtL,d=vM$Il(N1/FM_.,bi_'9LGN4JBJ`1G^f`_KP)OIm,Z$#SkA#fp8gL8;xiL**exO<#H,*#SkA#hp8gL(;xiL,<EYPL_P8/wCF&#7Z)Z-b1^gL-EauP(2H;@'AD8AXeg>Q"
		"GW9^##SkA#bN#<-'>T;-RsEf-7pC_&vRkA#cN#<-3YlS.6,>>#dN#<-<GpV-F>1F%?g0.$bYfvLP:$hM8S6MTk0C2:#SkA#iN#<--HpV-JJ1F%-(n-$s<Rw9:S(##Fo6.$>;v3#@;[fL"
		"U)9(/p[(##g[6.$@M;4#dO=R*L2P##tXM4#R.b(NLE?YYvcS=ur4+Auf@f,;Vr+/1#BQa</JM>#p;xQ#33sYMS6xv#j2vW[;5Y>-r3GZ[c7vW[X,-Q#L$tW[d7dP9B1:Q9J&0Z?#gKDO"
		"&`vI-Sf&=BbI<p.I#Pn*@f/f?#Ug5#f?_qLPW&D?94;^?LR76/vV%d53F:,MJmbkLUB-f?qf7,Et5.F%@Ss20.i%X[k86C-M<Sd<+eQ][_x%V[rS.g+vcG59=i]C?IMtd+$6oS1[.MRW"
		"FbR5K-WBeZ.%jW[1?tW[qZ(90kA6g7H]IV?aQ6R<qnTA.UrbNO+O3uLK@;QMK[JRPd>vQ#8Uv<-&44R/A(u59XU1n<#Ki*%OH/a+Rep;.aAxiL6w3v86Q(01`LG;94O#Z-'gb9&oII69"
		"8^:011@h*%L=/t[I'aI-igFS_h;G##G+s##hjFS_89>Sndb&V[DUof.?PgB(B'm-MwIH01UF5=_K_[D?Ca_l8x#KM0+ihmL8'$k0T(U'#;3%[#f,Ja'205d2S`MM2t*a$#drBi^Y'FX."
		"0m(C&Qp0)<#LIg)-:q#$KlI[-9]5<-=*oq.Ji1$-3f&:)XhD0=.E].M@L1T7>l/v.aVGb5Q1':)93n<CRGP/M(Y%9I)%>]-3w7T&dk:mM3]rsHxR&D,A^+6'NCCJ)4(MT.Gci3'+9vt2"
		"pFlpD8[/g[^&&A]Q',;H38=I$#GD@#$]Rw,SMxaFd=L/#pGe8#+w=wLb8bsoHqh4]LQKY>&Yq(<3pF;$=t)##Ala1B*;ZiTmYDY5rG9lSY@lV-mirY5tsD>P>T[S[U]<a*TOn`a;A78@"
		"@R%<$3f.R3,V.pAaq@5&-9Sxb%U&Dau-(GrrliY5/K,/U$-gulDMb;-Cve]OEvg5&m7.&b;v8vP'=>Z51vsS[uB[DjV.CW-Dj:E31/L&k67]pI6VAjpi:P^Oe[HdiJ3wm8^FQdiD*UNK"
		"T^-_4`fSQf7+Ct6RuPHi8@Hq7%m;nJX&R<d,]2=$Wq8wliq.URM5m'=weaR&w-*=Ql^DhqQC5CXFPN=?S'RerdTwtZl0/V.,S7.`05Ac3[&#]P93r.)ZQS7]:1OuH,p'cs_vQ%cQ_Oxl"
		"Zm-p&D'8Pgr_+&>#kI)+'rHDOFEbihTe_A5#/2#Q(#f`b5[&Q9P%^Vn,)2p]k5ZDkpgN#Z_mVpJ@D:gr=_3EOX(r^,@txAu.YPZd6$(wQ1T9nBCE3O(dt?*X^E1@$pmW$6sF]9SF[T[$"
		"8],RK2#S-a=;-hrAiDU8*[DhVP9+UfJbP[Ha7T$m97gh;fC@(,[QKes8bqnT(1OCPO%(M2WT'rfPA8rSh/$;8'N(irE`9YIZkDG4;q`S'uhNogr<<JW@t')P>hS,+2(sue<r=5_x/>a5"
		"e$&so)7FpB.q3vn9QpGXW:=ND'jvv7hiC3)toRaPxV'wIeTM'$9:_ZR_m>h*p:<<fS[#U9Tk3'dNC[9Kb6k*5sMtw%+6:qpH9HqgiUq'Hn0&l2w'cF,L,HXoM22.X]BcOM;l%VKuK0iW"
		"SDV.X0AQf=msND$hN&s0i%j]@60pP2hI68_^XrA%(inM3P](dG<HPm2Iis]es,Xp:`frYoUe$HPZXu#8`re;pOH7Wg:='3WYo*h48#Wad/Ets^p@-kE27[$/j@bWp:#[*?d1vW^alW0X"
		"6dr9L&Hpq:w4:+-mUp9q8oNUU+q%`7?u@Il)4Y/_-A4$FTh?&Fb0u,F>kx3FSoT8FUCOZFeP1PFpt;UFauwuF2HYsFEnv=Gr4B,Gc%lgG(^c[G<Va'HT@LHHQ6DaH@9OvHK9X;I8R[UI"
		"u=pcI2#IiIBIYwI1*]+JvGoNJgINfJ;iJ_JRt[bJ=H+0Kf#jqJOm['KNmC]Kt&ixK_1R6L+/QTLm2^gLb39oL]J`.Mnx<_M&uXSMJQPdMrConMeg&0N5NnYN?o@NNo;0wNxvSgNU?1tN"
		"pA5-O]'&ZO&X=SOt]n^OXA#lOh&%#PP)^0P>p-YPeaHmPlm)fPRHvxPWfR/Qo#xJQSKdaQ:^?6REmx(R3_g@R_,3nRfpgdR5h.rR&(X(SCX#=Sd6MQS'%#1TbppHT/;C=TX-^STcCl-U"
		"B^`#ULNt0U?'m=UKSZMUZe&WU$<psUGmg-VG6[bV1E#xVi;6mVeN<CW8Q77WjUmMWWWLeW#D9-XMCmIX*#m]XE@uhXLqk%Y?#gWYPVenY^dqeYo2*$Z%<l1Z-Wj?Z(,ZIZwg_MZnLdQZ"
		"nS@VZexc*[>jH'[O5'1[p@d2[Ah+P[R(=2'Ca88bH^A/R<oNp4:$.^C<x5mP<&u]_t]WDgpg<2vx9]#2*iAg@2GBpO09J)^7e/ml?=X^(B8Wm5E^WvDM6=dSIr)WaM8I)pKU=B(10LW3"
		"-l8K@25XsN6QxD^9R*Tk58ZK%/bx#221#-A.mevM/hm/[1.Rsj,agN$1ukvMHDaQ5@$iaBEC23QGA:B_8=IWj,TBn#'1&b0,PE3?40OWj=]K9O8?8-]=_WTkBxIe&>9P$20]W3?-K`BL"
		"*:hQY1fL?i,$A9te8Sq+J&7X3F0rEB,tT-J((:qXdksWaYJ@-o=,LR#4XJb0k6.I8a`p<E^atHSXCa<aaxaEpf#NX*RJ*:43M[q=$Ik0Ir(s?Vl9g9bF[Iwi'_%XshmZ[)B9>C1(jLX<"
		"oe[nGoVHbTogh3drn5_qnY=7,`/BRPD`M1%K/*u3QWebBY0JOQa[/=aWj>RlSOoI&UfJ75RK,+TenSXak:t*pfsMx)eCpR5Jt(i@P@H:OVi-(_[2MOm`6Cc']uAr4VNiIAYtiRP`@3%`"
		"hu3.om8&>*eKS4Qe[s[`Y^gUkW[=J&_2>oPdLE%`ju*inYbFJ&UGEY3P[9S>IVX%M9ILuW'0%Scl%4in]qkf&WMNY3Qq>P?7'UlH-PA`UmhW%`VHKujE8?ou?bJJ/:8V];'*r7c,I;`q"
		"3i?5-,qni7[9QP?6[48Gg'nuN[/'5Z&F0A`F]9MegsBYj14LfoQJUrtrT(g&LQ:)1Q_^S>Dg2/J=UquWpL%,^:d.8ck`@PmnZqG'a]<#3e/=,BaBx%iKp4>s%^S#*sOBv3=gK,9mYB)C"
		"FHL5Hg_UAM1v_MRQ6iYWrLrf]Znicgr-Y&23$]27+x@vEZfJ,KE=^DU<2'mdBZbYsI$g//DPrA;A9_5HDwvVX8^R8croI5mdINT#ItSj.PI9W=HB1TG#?CmQxknG^R7Q/f-Y4mmYVU;t"
		"$b(0&r_cs4Z*Zp>c_Z#N[ZRvW5NIsb;w.aq%?NE(/$bj79_bsF?+,EUHiGjeSVd8u.<7-'(F7661(8?E(6GTP+[G^`5Fd,p?u-t+U_#t=@[]ZEiL(*LCoagS$r<H^tc%9lud6h&p.t^2"
		"eBdT>V;WNIBxJHT76;?a,J+6mwWCk%iJ.e0OS)e90VZEC#_/wNqCRK]rJ;<kmkSq#b#;h/Sr.b:>O]?E+w8wN[Br^V<EM?a0V=6m%eUk%pr<b1e0-X=V)wQH;&V6Qx:2nZ_OdNeEe?0o"
		"*h(4%Z-Xq,;04R6.8_-Bw?3_MjG^9Y]O2keGkH0ox0TU#X-'7-K5Qh8O3oKf`J(_r:a*.'k,dk.ENFR6vp):>P<cwE+_E_M[*)FU6Lb-^cI-RdEUt3RgQ@+(m)Tkes'>r#]qnX+7=Q@3"
		"h_4(;Y,ARmajQLS*AvqYR&a_`s<jke=Sswjho6@s`B^7-hw^@<se$fK(S@4[3A]Xk>)Kf'Ig^47H=3fB#`lLJS+O4R.M2rY_okXbKe?4n,C#ru]X%A*7%_(2hFAf9T<l@E5qN(Mf<2fT"
		"@_kL],K$chb&^Ip<Bio$m^BV,G*&>4xK_%<RnAcC-:%JKQ].VPrs7cU<4AoZ]JJ%a'bS1fGx]=kxtoUu*At+1nR8J9HOJcC-Fe+Lt#AcU>:JoZn-AleGrJxjh2T.p2I^:uRS0/'<`^M&"
		"#508HU+boQ<x`rYi7CYbCY&AjE2+##7fo.CNI)m/1R%v#21no%>$gi'Qgm)'h`Rh()gI0((XvAM(RcGM+e(HM5YbmfoQ)29^B:,2g_1v,Ket$''K_$'BYBt65i.Z#U.[]-g7,+%+<,+%"
		"+9#+%4:<ajpbu'&36RHM,2%[[IM8a'-_e[-$9L#$GqOJ(IQkA#(_K#$2G/a#b.%ip?v#m&b7>Yu#&p=u'1NSt'OG>ul,citWCLJuk/EjN0Q]Y#PdH+rbA(Ra<aiu5Ni:k'Oa$W[@-,Q#"
		"5K=p&$K=p&(KAw$iaZ?\?-HfIr/<nW%1<1,2SO;8.'d_`3bR(f)0Zc8/6^^Z*sAN@t+Fq6:Iw<VZ>Jmwb,$5F5rYCnL+&d*$a9c#%';G##=N]s7YdB#$:Iv%$70Z][S%Nm#0g/mff8/X["
		"Dm?g%1iq,2*2k#$:c&]$'uBB#>O?+[-51I$pO_fu8Jv&O+0@k=umf6<JuE-d,8pu,44ak=N.9TB2'2hL;UeEN)(^fL)(^fLlfg5#6?,T+AJ`D+2gM8]'Jx+M;@+/2bZ^`NR`elf]kkT["
		".(`5/fwBd&AEW&O(a;0(xas9)trO#)hO%0(<xo'/60?j[QRHs*M.+^,Xo-I/*Vg/)w4YJM*1F%$D3k-$,Q_#$_VjfL;j4:.x66`;V0#W#W@:X##la:<nf6#5Pxd+k4'L.Iv%8V#$@=I#"
		"Ab@>5d<)kuE-3v5(la%Vna,#5_-@>#`@Vf1D4*jTxhn1K[SLk1Km$2u]Ktgl&Ue0uJ``$'@Nf0uGV`$'GV`$'t@So&&`/v.ZqK6]U<e0ug>Qq%V$vl$>2g'ON]?<.<e@:.3WDa0(4qdm"
		"gXa:/w^v)4OGg+4BYB+*at]:/.m@d)p7#Z,Tg'u$BkY)4-NsI3^^D.3Q1oeEqE=5e4/6;aI)$wAQn/##v>iUfo7-`h(jffGnj?K>:,u5KU1;_)?Soerae^VDNL5@Ag]&+@wK<_b_edsc"
		"E^g.#6]SiBI:>&OPA62'e'IP/<r3>5evYj$g5`9VdvjT[:V>#M=&xIMJuBKM^M,rf9&j12Ax0X[r>Q][qi/V-/VV:%Lf`$'ptCt6@0M/(`c>j[v'H^(uP,T%b,K6]SHK01'4Am,GB9/2"
		"g?L6]g-S:.6Jt;3gDTK2'*fF4_R(f)?4,<.cP#J*S51*4*/rv-s6L`N_.d`OWD4jLqNad$q0%lB9boLY9$:F[CfO#71VD9;n8XVNUooku]LEV<9`^0<@T7bum[aucmp=WMH?Z=:,L>4P"
		"oJ4G>QUia>:E81g/UcOP>:@oXM6#'?SDAU9a*cNr*l-;:'6GY>&R'K(_q##,$NYc2Gla#2VrSh(K3*4(83wAMTATS8@5x/3H1o.2M2#88?()d*?8)d*2gM8]?CGgL[)e)0&mp*%;W1p/"
		"4?b2(J&q*%Lthk+,M^b*DPV)+-Tp*%0Yg'++HvV[t68Q,u<&9.#wOM(<0Ws%H'JeNl#eh2-M>c4;xDu7;h.ZPKIw8%n(tI3sBwX-?<pV.=e4D#YiWI)huJF*RD^+4nGUv-;#H4W=d@j["
		"TlCmD3;-Y6KRopuMp]jur#GY#AoJeuUD:sL2ge+-A8ap9hHjXCCV<@SCvs@`D-VW;BJTgIVHoTZ3%RcmgU*8Ip4bVCN's4>?=7`4es:uA$),##w&5Y#45###,e%o8iv$9.3.*v#6>eC#"
		"LfvZ_$Hc4J*8>>#)_f(3-7#QAC9,/(UCRh(^l/'#c//X[nS,kk1.^h($J;Z#O;Ih(j/_^#ici;qm7nF[sIA`TGkxjJ_:#W#M)X<8fNGf:^0:cu@gUpJ71FJ:Fw]-?EkX-?o;Z-?0.b-?"
		"HE3FN+rugLnl&E#BuH8%NxAL(<lZP/TF,i(JQ'&=NHiU2;%TjP945<qC6XS7du[dud1GIH54/)3R_[@#r=FcN$h$ouE1QfLsWP]#Blvc$7$M$#9DQ&3#@@r[3r#^?iC1<+qo[e*g:,F%"
		"@3]e*,il&42XNv#@1;H*EL`v#ENAG;*B:D##go&(-4f_uRIu%^8&HSeFCnJ#;[(1Ht2UCsX7F]u]W6XC(pTrQb(LM^H/<A+bl$-2+I#W[Jgq/%<8_s-*XZ&Mw'tY-8*x9.VF3]-nDarZ"
		"$A8cM=?<L#^*tV-QalF#Jd`oRS)n(,9Lko@n:vo%LDn,&[MRh(mNvG&P*sM9<frM2p0DE4N,tI3+M@,Vc5sB#k9UcD/aF@@_F#L[ue%LPPW,p%IlWoe7qFJ(ZA:,2<xRRN5;H*4CkYFu"
		"(7%[#8q@kF$hD(ssh1##Oukr-B)#n&arv20%@@r[%A#^?/W=(J9q@.*JegD3M,NG)@)Wt8Q&8F#9nDD5%#vo;H4c^#Rc9Yu*9P'J`?.S[YWbg$S%<e+.fsJ2:E(B#/S+o#N8?*%R5YY#"
		"$fN1pi?'#cK>W]+uCw2$]Kc?Ka/M/(3Rn8^+XPgLQ1vv,<kn;-*lP<-^Vpc.KCK6]f=`&M==[s*ib#B=@N6/(ml:^#(rJh(X(#.)s77<.dN1K(^^D.3bk;s-2SM,qI`%-qw3Z<G3Ku+$"
		"KL8/G7/fduVf$S#46U'uD:XY5k74lf%v2F^gOTgeDEn@MwDt4#*Yhr$4IY##<We^$V,4kO*NTTpZ_VV$9a*?$;td`N6%Zm#H]*(4tf_;.>lIu.KeC+rMV.#5R?CDJL@*F@Mn%##1E(#c"
		"50o(<r>D`+bJ,<-9s`E-IYA'.eCJfL*P5^(#Ka&OAheeMim,/(HxdT.F%xC#FM0%%OMG:.L/'J3?DXI)RiXV-<m9f1xv]b?JZ*sL[isORK8n/Pf*T`WS_-XHNct9L7H1XhDFq9F.:tFM"
		"tE3D<)q5d)Ad360K6:K%M5nqVwg5qV&0dqV:'Xs-j#C-Mr4-Q#%W_A#=Ic>V5L^>]h(MT.PRK6]+.>-&`&K6]YIV9.j_UB#.In;-Zt@:.d5DD3WCN,*=p*?$);Mm9X_Y#>+87<.c4MbR"
		"J7E.Gp:sQIROa<tO9`>KpDHeg7Q1eLcX;nN.j$j9l_bRD)?K,G8kUtb3Vk60QhthnB.U^IS?mxJ%/5##[G:;$1Yf5#;xL$#C&(p$u*N<-jMfj/#A#^?9-7<$a$C0lPM%01[V>j[E2<d&"
		"`Pks$D0]Y-6YSb.PvNa3:0x9.(&MB#42tM(1j,C^V9GxOGoFit0J%2KW7s]b$wml8;-F9uR9IE-$),##@1ST$83TC/^B)@]g/'s7e[bA#FofA#Ndx;-4lP<-m;IuLP*u[&Z_82'TowV["
		"&$=T++e&9.QI&B+@To5'.8p'O(M4I);)N8]F-VhL,Qeh2*76g)[?%J3;8XI)7J-W.T_C+*]h3B6s-G>%2OiM'l6RNC[d'8vvvGouxOQg'Gbl=KqaGZ@0/'q##jL%X59,Xl4`(:mZb-SV"
		"GEdH.X5YY#<mZEeC`qi^Vxou,M;bv)Si>nErFF1(]4r=%73$hcRRZxgC@Dn8^ZD8]0r^wg)SO,Mkw_v)F4'W[d$Wp+f>KK287E:.j--aNMii,#$$b$#p/Qv$jL0+*m)l6/s2<Ve[FPY^"
		"kM7xto9'fqN2.5AZR,.-.0A<LW$L5$Ac4guu:fquUkAE#QKYiKts@f:a)U0W9^aS%v->>#iH%##ir]S[[Nt(3Kr-=-E.'K/v@#^?*6Hq78]F&#Up*W6IW'B#IYi>$h0:3vxrh.(F7`20"
		"=(+2p?@uu#/O:w%k,K%#p&U'#*X(019&+_$;.2ofH.kT[N;dt$>HIwBvPSj2+RGgL0RGgL=3*E0Gf?X1TZg9]3N0>G9>-t-AVkjLwFg&(:V)7<WD,s7/MvV[v)h,/d29K2p*'u$&[;^%"
		"N9530M.rv-iL0+*tRN(&1N+Z#4LG]J'8A$jaS'0GWK`me7.daL'_Qmj<W(r_%UZ9Fkv^hXa3ZNHi6-]aa6aL2w)F<jmf5sE[chOEvBt=u.AcVCuPY/#-sZo@4*:g(W1fc)*(k2C$Jg;."
		"%&aV[Qj&@)vRB9rwXK9rvs^q%n*EpJ1]]e*)?gW-j[x6*o.x0(LMARjGW=K:==2'oR<Tv-p:gF4j@A(#Yn`$#_DsI3pG;)3LqE.NpY%)$v4]qa66lTPW.5Yu&@>O,75gxj?$oJMIT.h#"
		"j%=2'RJL)MsYdLp1;_fuUZE5$Q%D$>3Qa3c2481Y#)>>#GLSX#loY+#Wou^t*Mc##4,l:$2(xU.`-(R#fY`=-f,i*3_mY+#K&Ih($),##`rI1$'JfnLve67]vq5Q8F,$Z$hIGZ$X:woL"
		"^=?Z$povC#r(E`#-L$4$n7./lOh%2b7<w<#cAgM#(@J9$*%ffL33v&#f8/X[U.j2$F4ho7XPv;%26J/:q3hN(HPM#$ab@1$-g1p.PP7Su5/DkOn%]%X%##mOS#4kOV5OkOv+^3Oxjl4o"
		"qbxlA?xilJ-=l=Pa99xpH_=@6_#plSZLb`*O4x4A(iYs.X*s;-THi2%WhR<-?\?Vx.AA#^?I`T6CZpg5#)8(,2?m+/(euP2(+^5t-m@JfLvw_v)n]9K2kHeF40b:=%nY(Q95P)<%:%`v#"
		";&@T%M#DB#Lp-V+Yf(d>qoo1p+2S.T-B'Gr7WIm:8itE#]t2t`.0T<AB*@x[pKUWWmnvBQq([P:.;/H2ji=#%=DB2#F]&*#&VbV86:na.%qlT[c]nM.aO(W3>x@H;KBvV%JP'W%2gM8]"
		";0?gL3Ag&(0Z;W[pu@g%nSii;R`;W[B6.W35cY(O;qU3(:?f@'nPK6]&PpgL<;Hv$<f)T/u;Tv-t]Ol(*^B.*wspD3*aXl(kxq>$DTCg(-uY<-bcrM1R*.0)IU,H3mO7@#.C#7*Ws47/"
		"u9Ss$E<;@a&,OINTeqv(:usUF/8ri3Pi`6MP%[@bQXxnuh7mm&I3>Sf@'%</C_;euq]c>Ap6$V#AIxX#=Jr:$W)pI4VRd-3P'Xi#XJ>xkGP]6qBN*s$sxbDuBws3[uLO`Nc-b@polP_^"
		"u.D%0xF/)h^>mf(Scb^jiFg=PAP[g=Lpe3g7wbf(k@T585w<8%te>M9f&iB=`He,=Z*+c%Yi(Z#0?5Y[Q`Kh&0$7HMm&eh2jnlV-$CZV-0mv5/`G.oe?k=u#V>cP/a8lIUJqpT#(f0j$"
		"wc:c#KTi,#fJg2#MIlwL.GN$#X3=&#6oVM9>%U^#qL5nf&Cg;-5)3/%<XKB=:0vG*/W%RND$g9]wm/>G)FTa>_]Rg%;bu3+_#K6]r<a/'L(]@'4TNI36.X@$Fmx_%*a?['<29f3%#]HF"
		"DnO?.OG_^sPb^RDjtR@tY^DW##<?bneO27IHZB6se$`7cu2v?-Q7*vjD/@Y5A>mo$n3tjE/*,##%1ST$.)r7#3EU;]ZM*;&Xa5-;Fp@p&</Q5&_/M/(1/x&46I&s$7J-W.aY8f3j2E.3"
		".>m8/NTU99W',f%K0DPo]D.<tYdCUQv&UJ(E[>5/3gp9RaUUm#G7hDSR0S0op_%P#uT;Cf^#1lu7@2nOnG-$p?i_3#ed3SROQDMg9ETP&)1?M9Z*^F*0?5Y[P7Lv)kbEV.B(K6]fiWZ%"
		"^s1/(uRIg<,rXe)R:M;$34U-M-vL*45OtVR@rwR#BF]kuZ7F7X<Rx4fJoXMZR/.a:d'IoYD567nF/>:pN+4GMPO5OMf[C=81Cdv$%-i/%s`(K:No.T&[G3hY=20*>P,UA=bVvV[I#Rg%"
		"O&L9.60fX-;?j%$a6ol#'Bc'$wSlb#Zeahu92cS[9$T4NwD_kL#mv##ue:B+5_K)ub&)N$Y2$q.Q=ZV-ML`kF6u-i^k(HNu'PPb#PQx;M+OpnL'2Z*v`]4q$[/4&#UC.w,LKA_%o7'tq"
		"tUTtq[4)<-Q[x0+xUfS&^OE#.k/=O:LeGs.Rcp)5<`c7(%.X@$+ocV-hsHd)YQD.3mRG)43Mh^<v7I&_P>C0$'(c]+S9IA,?oO@tJVluc>Xjbu5vPg`GW)P#EKSJ#(Xh:qYPncrnW6su"
		"@<D>,SdxZ$mE0Y$4h1$#m1hx(AL[Q8UKvV[r>Q][R*fs-wF^l8o.#R_b&eh2Dt@X-Fl*F3h:iI3;MTXuA*cu#5EBDNP?`S[$o85A<.nGWorh+VD%Y#$0@*Vp0bG[.I#?3uODxjLTM.$v"
		"%'g(McEW$#pA`/(@m4`%A]iQ/$@@r[$A#^?$#730d5DD3p_U8.E^3bl@95SnGK.+irj[:HeWG'St<)2gCwuPH.,L&8q%Jp&M;1?75L>gLUj;Z>4/sS&?\?sS&A]]&#FUwZTknC.#R$no@"
		"o$@@')FxVHaGCpL<*j<%Fl*F38OZV-HT0^#PScXu]2JP#97vr#55Bq#,?]u5,/2Y>$lfXuL,8$NQMR5/lR>Z$l^RMP;JQ-H=Vd-Hqvcg1J0fX-O8BVQ&e%]FMBB/8rF&WR>[H)E59+Q'"
		"_jnw9R%B<?hSbGkTag5#KSqYMD;@>%/*gs-V14,M:O,I#T4tgl2&@@'23_5J/rY<-8:;t-5ro,MA*WZ#2K8n.[W'B#st@T%&`gU#n^`O#*Ye[u(3Zx=DtdS[$ojS@P^SxkB,6+V[gP.L"
		"'Rv']8UqhLZqv##uv#&=Kn5eQ@7%q7OM@gWDDcD%UwnV$s3ji-h7#+%u[e[-QQrQND^QJ(qX.@u?vaXu[+-V$M/r*M/Q6U$2/5##&:op$a&r7#Q-4&#?oCv,'c&Z.CujT[)iT%%af,q7"
		"-j7p&b%YgLhZw+'][e[#a/d/1XT'?-W#_:%I0Tv-7-)m$^0l'4/Jhiuviulu?J>PSG@#/C$D?N#&g<rWAroTX6I'5]ebHkZ9cK8g='OM9;<*`aGG/X#ft8locwr[tbiSh9/h<3h8<95&"
		".kRc;_$ke)2px8&w;**NXR=,'=VYb>`xNd&DeimLHeZM9K'0K24uv9.,T-T%IQPs.75^+4qV_Z-a7_u#f#+v,]&+uuZe1INkuMgu`ve@ugTflu1v5QeuRVJPl$Xv17m-W$]&r7#[Kb&#"
		"oaOg:cO;W%5`5,NRF?&'Nv'9.fhv8+`ZFT.I=K6]En`u&i#*E*G9<j9OE6?$2j?a39xlX&`[W*I76U7eCjP,8g$Pu[-=m6S/j=6Y<?;J_,%%n#wKkLgvA#sr*e@;'@W4uuPAGuoZA/E>"
		"<g<o$7$M$#G7B>]]tb=?;2O2(8Ke>$m+7K:f`M8]g74,M#,E$>)cM8]0X7k9KEOQ'd7V/(aT&3i0tM'%Vi(?#F.lo7/aTp7bEO7nviAxtBv6]K]%xeq>EJ;$>O@UMK'tc2QCcS[5jLMc"
		"ZbGC'jrT0Gq^9##onUf:I=PAOj)OP/JpI^(0kp^-VX[#'DOrZ^`[Ct6NPP>,HZb;.`65?,(6ZV[rlV''u2iRWH`gJM;_&Q,4;[30t%<T/,C'Y._61;g$e<R%]dLT%#LJhuJoUS#Z+5^p"
		"i0frMjH1N#P%?duEVujuQOA%Xs4SL##$t3E]=Xi[rx=dK60[[AqWuTLuZ%?in/8%KDUMfLV_i'#2-Cj$0Oc##WX6#)>q6w%W;`/%1S/c&aCk):VA.+4MOfluAHw77>+3;m$L/;m?>D]."
		"<1ST$0>D].x,1;][k`*%95GB?Prmq&#^YN9L]Fk2ilQC#lF18.6R)P(<A0+*C/8L(gIcFiEvaCjFScM#O@/5u0c'fqvJ89$]vxwQK'N%$b/[a.:PUV$aq$*.Z6l.<?J1@.mB&0'hsbp."
		"8'cJ(v5+qi)LkA#kT0^#-6LJ#x3:'s.'qc).Y-vGH;Z-?%.=G2e_[Vm6<op&;OXj$5_0.tv=gs&]ggU.r>Q][E9dh%LT?8])Pp;-:o/gLjs<F3_:Sh#BvN1)g5c5&ZQ?o[UT2suW9?o["
		"oH5)*fkBF$ftqC$%fln$9M6O$fJco.PG:;$M.$L-9Q8C%o/Z9Ki#?&'VvR3OIoBt60GD>#Huwm-h:,+%Ncqr$9'8$R3]DY$iKkA#/pVm/76,)3:n)J:kkC(jZ&?ucmMC]b9HWS..BcM'"
		"p0m7.$LXOo.::Mg[P0K-cP0K-4fg,.IJ::8ZoI(S8%B$erGwqM^*#:.XT0^#9ccF#QX[[uSS*&+.$*5]p4%on.2hlS'LDd<_-)#P2iPs%N+0@Bs199ge8;N$-CD6jeGr/(UbKH4`fn,&"
		"qR8U)=+'u.Z_G)4FF3]-jQj-$EvZo#+kN=$'ksuW*[i]Vv?9LMRh5#,je;8%Km.S[dl%K%F<Dw>x*cNbkl*F36n@X-7077X$qWlSGpQXY>$3BMMbJwu;w-qLXQ?##>[Fm#Ach;-BJg(%"
		"#3=1)9Y,K)h?f8qQ5s-h0=jl&1B-AXc@9s.N3)B#;-W]Oac,Q#[)Y<-_#(p2kax9.ulGju4Ch.%A+;h`&vuEIX%HciU^kEIs@HP/5[WN9Jt1pf6n_v#8=Es-,_Z&M00M8.gkjr2[kdGf"
		"QaTP#q/IAX/fg+ie^&_S<&t48jC(##AvV]+*ptP8XfW>fd2eh2JF3]-'R)`E;&AXa+KXg2YM1p%abf70NR+Q#kKx?#;#Po(ke#'(JOvY#4GD>#ffm)+qo[e*Pacs-kT0^#>fA+re2*2g"
		"S[$##*Er(vi=(V$:.4&#/;uB]iFC0MQ=,l9tB;H*6r6W*7W:-%[_SE<vp9^#JlIn31*@HM`r<d&*SX1(vq%t-'7(hLd,av%pObi(Zg/m(91+N$1>Eb&kDK6]f#$.mnxMT/?]d8/9h-H3"
		";U^:/ru`;?\?W`H*uR0f)=G>c4`Arh$pa<v#]Hu.YBMN>#h.`vAKJV6IdlDnlK1_Xu9[7/MZ_m)-'wT9O&NH*>vq7^?/xTkZ$H?/?fEA'6@mPV9%Da+2;1ST$[5###Mqn%#R4ht?HKK/)"
		"0Dh;-i+O_$HWXb+x&t)#],rX%ff1t-eFffL8l3H&)0QV[N^It?(pDL)<f)T/<l*F3%jE.3F8t/M?S2K(pTiWh;8D)3I^0=Ej,N'Etf^CrJ0>e7HlexJ6(;$uM`$fHO0vhg#BBqu/Dqf["
		"'Y,EP02$#]L`Jcr<bux+w7%6#B^ZQ:2w(hL9P[EN1bWf%t[.9.E;Yj'RrN8]%o=gLc=T&(?vfR8?hsJ23AP0P^TvgLY4HJ(k8;D3)x3i*(I,b#4/-H$Ns#)es+ZCs(nn=b]m`S$C3I;L"
		"h&G;EBf7#DNb)v#_1;20=i9^IUVAW[(7YY#`lM1p0PY##J/w%+QSBW*QX<eHxVkT[R%5S[,-Wb$&2YB]+P1C]3p_$'#1L-k?s:1.ebbgL8j3H&*6ZV[5pG1'J%>3`:,>3)$RD.3:$nO("
		"AOi8.=*8M)+2pb4JGUv-%ovuba;.$khn'<aM%vdH,#_vRAW(p.r,;XIJ1_=#K(,)3P(6wM1>U9`)Yu`=Al:U&5xac)oqf+M)5+#-Qg$W[Wm?g%c5fGMAfkd8$IrD++^F_/T?Gq]<@DW+"
		"A+QdAtwI]#-@]D+S]Ps-Ze]+M9fZ5PNL^>]um8o%?=_1<rqc#)N,Rh(V_Zp7IL?Z$&n)`>0[Tj(+FQ,*qt@T7IuUv#t4:W-Q8`^#E$uXlB;E4k[K-W&I2AuDg'LwWk]2@#,MQS)XPEt,"
		"S%9iukCJiBK@?@K=n(,VeZn6J`.v1%-lc3%LF.%#E&Od)x@B[%t]%Z-VwnvP?5)N$pF,nf]mbW-.jf?^_-=t-1:rkLQu[J3D@n5/=rte)J%1N(`cILdg$X$%bNcrj>__$^h0b%)S/a)B"
		"OT-0#xd/A=3Y;m&9F#&+v0g+MCn)9/dG%W[g2<d&=38EN__Y,2($eDN2_YgLmcVK(?(jG*EYMH*'B;>]qW6.MXh&0'n1+L(9s;8]N$/>-1]0W&`/M/(7''c&2[e;-buj/%jVp.*I5^+4"
		"[EFW'bTA:pm7p[&-A5Xl-G(&H>6X/PB44_qP]ODRgSwcIxF=&u@:Q3F)B:?sD:%sM:*0_HB=8_W;o=`j_lD>#G;'##%5c##vR'58waRKsGB*R8jZ'^#:VMp.r6GB?S>k2*V-t_&`)D/("
		"?,<ac62@A4E;JL(ShNTBEK2N(;<KCI:u3a$jUX&RV-x9$<;KrHq'Rf_q5:0uEjF82M>m1K`?%;QVm&v#o%fr68Q5r7]XbA#SF1gL(*X)'5+@-2O:t#&YRM,DB2-pK_P4uu*F_6:-vtY5"
		"k^Tl:&X1P:o5YY##B+G;J+V-Hnd$)*xLw2$#Nmg+Js.N%kfP<-@ILD.%G#^?wqJN0K7kT[$'Sv)m_b<?;`>L2%B3I)@f)T/)_D.3M@$&OIUZ2Xkhd;/:b(d)]LKatF)1^(v+BI$fj.B0"
		"Lk?0#dU]x$7;E.F-^,9%%J6-D.bE.FP;_&=/6@@'S.bU%B'rwEX-PF#sNk]uO@1nuLQUsFs*`##acNH#Y/<DuX*,##[->>#S6###LUMENsNlD%V/Ps-cUL*M1%c41PZqr$D]>+#k;JA]"
		"[#V9&XGhT.&@@r[MSB-'Y'7b6<E`hL%5*(=$e6$$Er+@5LrHd)(tCv>&brd*V<SC#+jE.36gP,*aUI+%wAp8%;Kt6/+N2K(eu&2B='Cxiu<vg]#jX6@^Vsn)1,6)a@g6U$P$2OXPNARu"
		"1T?@D#*EEusJ8Ya7@xT$vUx;%Zh%3(4_Pd&xNa#PFtJfL$K(DNGsugL:HREN.:<f%W@Iw#JT+W-X[bg3Ltnc8<Z$C#pFxibq*1,riW^>:>hW6K/^-Ku*B$@D^KttuZM#-k_6wlLAMW$#"
		"3Wq;]$v0JDw2ZV[%*h`&ZQ%U@_+_>$-_]<-I=Oj$=SkI)u;Tv-.XlI)@?(P:hN4K1k;TlRrG>R&rI>MS?RoDrgB^LPM@Xi#t3v@O,`.W5Li/M#Y@>^XU<-tWk@CwT#Qe1B$WcwT$N+?)"
		"oIMEl<R$29xknv@6pu,2>6&39bXB99`HXc%VQ_X.Fl*F3(eo,(8#dG2<e3WEn$;EExV9(rI_])8IF58M2>gtum+B:vI;;rLK3r[t[5SeJ'q*9`8)MP-2*MP->Mei.Je[%#&ZGT'4+N#P"
		"1iu1'EA-;0v-mt&C@30:-o(a4:s2G9NQ8N0-GtF^iQQBr;SDIoJ/AGQ^j1JH%`'^u9i^QEB1LxtfIJ-sYOOK#/K9e8,c0^#@1RI/)j6<];*8k99s/6_8%Q<-`))x$k8'*E6iMs%Gb#I6"
		"R:SC#C=-W6,'&J3xs<P]ie?l5_#h;?oTV#$[1@KuL)fuIfPJ'J*aM1pB2Q'J.LMcD.@rp//VLENNqIiL7vfl'pu[m#A$Ee-c)D/(l1?j[r-@i&rRK**P#Ql0c?>$,@='^[&wKg.t6&9."
		"stL^#?qq#$%61^#Kn<>&k^KIEbxG,*Zq3W-W>,'Iph/a#b0ww=@jCTbq,Xe>8@DH#'%]nq^c7QHF3CvBeZ95/Z?\?r/mduU`w83w@xbex66tg,?u%p,Y,C7T:XdIF1psrs#1gs)#<]N=]"
		"+FPH&R`SBfeH8Rjp0x5/M-,Q#''PfL'30X[`5VT&[UkW?oR$tfx=Oi%[H/i),A0+**pTi)(=lM(kDsI3MOBVa(].T0C>8buwI[j$h_)1^;(6d4WDl@#u<2w]C7D'S]:O1p]2%Z51BK]="
		"[9+9.r>Q][dVFHM)^[_9K;4m'C%%n''B;>]tn,q7'qimh.`nv$oNv)47.5I)r6l*%:b0m&fXTD^^2b/7;#UXKv,Sc`aQN)LG(stcliq,*V>k(jPE)xeE,1x&/aPt-,vA&=?#a1)&X'B#"
		"8#`qm&+$xgQ`9Y#]/^]tCAKoLsWP&#xC6&M_@)^)7JC68e[bh<e?OP&IN?8])Pp;-/`Ps-__p+M'a;v#[2)9.+`a@$EgAsu>_QSm]GbHpkssQ#]=-vt'9?Vu#)nCu&.6;#+(B#-UeMVn"
		"]Qh_dd/tb.)@@r[j6S_)f;c_d`KA<$2Ql:][aO&=59-0)hVXV-]pSe$#xg4f<ZjL^ontbrq9B+r@+CR^_8Jfhs@wkoqtQYG-.)p.N=-eQH@%G;g/4eQ@5+87Yb];06CP)'Ib'[KIuTt6"
		"4l[V$xACh$uANlfZIRE-]gdh2,<ta%E0Tv-thF+3tt4D#pjN$k#e8]=?t0Y7&`Sg+vHNL#3dId#u8b]u:;%4$mh6E$]n#XLDq%##@]*XLT&P68dN2cY7=6/0mJ]S7/:)p.YbVD<JA^k#"
		"3d;<MMPx_uUwC,%xXI%#6.Zb*[lw2.*stg:pJF&#YPW&#rQQ5.cXbgLkA`/%Wj0d;cXM8]/i>m8<U?T._B7f38&9+4QS>c4]sHd)j%'f)hW+]#*#=I%`9-2^cUD/%7Vh]4uOtH#C@mL]"
		"oh*TkkGdFt@3/^@f3X=5TaCvZl^=>>)E@l06l*F3tO,oX/2?)aeb&a+D%hYH<1/d*+<<Z$972_]2f:_]<%v^]=+)_]$K)S&<&wV[(1d#)2<<Z$)r1_])K=c4?e#;/0`NT/K>'eZZwFF3"
		"/rte)`SA^+E?Ja^jr6R^eUme]ClL%v3F_c>mHKY5-YIc_cHkPli8f]3DH=VH38Y`jm+AD*&XH^(p6Jp.mSv2$,h1_A*HC,2-%DZ[i8H/(af>j[wvj/'v>kT[mujh2bkh8.rA(&=aJt87"
		"S,m]#I5^+4Q;ZnPtGla8F;g-b>VBkMc8RT8=''U[;41f-h<-VjvuD&G$05##>U5N#dV5+#5(V$#+Nr/(Z*(W[@$g5#:?Fm8#oZ$$xgl2$-0Z][Op6K%K)Dq.vL=F3i@c0Pf/w)4HjE.3"
		"#A%w**oH8%7W%6:7Tr5:bxktu8mq1;)kh1;p),##A8R<$Xm''#SI^>]BrdGEde$HbTl(='ZTT5&#<o8&95x.PuFs0(j+?j[1h[s*[mn?9m.x0(]l)q.=G>c4a_aCn.&.&4g4<[G#YTp7"
		"Tu29/s0S8%I<FD<5mD;mHNHJd,*5)_H.*O$6:WD@E.WiKifthN8CVMoBRJcj6$E;-$gr:,ocWaH9noZ[mo'kbZ?&##mFe]bRh5JC1`tY-PC[j(1DY#)6Vg5Agv^>$;?#6/`ouJ]cO5RA"
		"5f<#-N#5q&(*C2(g.vJ]Q0A>--^-a&f:SC#w_:W-_>sI3Ah,V/+XpU/J`)m9PV.K:rdS@#k$Ka#l<IRn$2k<%dTYAtD_t%sVRC5Dg.N`#]hTL>1UH^FH<L%rm7&F_,u)UD5<aY#gV:rm"
		"nX=Jqg3el/m%[m#P%B?Rop]2-:;SK1%0Z][pNvA(k)D/(J_TQ(fFpA(?FMdN@S2U2MIi8.E$4Q/?W8f3%a5J*+g^I*9Ze)*kY*E*Uf/+*->#G4qYqv-qunV6M8%&Vt0A=C=/DiP3w4(U"
		"CmvSEG_S1uP]Ktj%Ytn`DijOE6*(IQ-^mOd)g[fQ2#*gQ<E1;b6L1j#[7@mH$fPme0Ui8#=^v%+=YZ`aDS*)*F_.A@<i@.2Ga/r7]EYn(YKkl&n(G4%vWuJ]LvUE3%.m>)<U*.2o2rw,"
		"F='^[W7/N$@jk2(@l5#>[KvV[eEZ&(R)u8.XYJa'F<a8&xEmd$TZ@IMVeRX$>kH)4iT0^##?WD#^O-K#)YiO#v8m3u$+GMB7&D)+aE1,+..flJ%?8QO9'tT#a69%TGBaY#HH?+r/tbKc"
		"Ql68%0/@-dslg;-Vvm1%[er>#^,eh26$nO(n;5I3^VRom3^%^+'S3;d1CGe6(39P/(++#c:.KV6#U8N-e=XBo+skT[*&5S[p?0C-7wCX/Or#^?6'];%s@VA5vMC9.$//2)%ns;-bBtc%"
		"gS`N'9s;8].j?=.Dj%B1Y.sD+kJfwBRUr)OTFo21f#2.2DsMG)jCXI);J-W.e_1+*xeoJ1ZD.&40]39/9h-H3Xt12O1LC+*Fc*w$,R7x,p19f3L=)KLHG?+r:JY=l`#1(JZruZ`*1h+8"
		"ceBtmk]e,r4`MjO8okNbE(i;=*Y-$SV4[lNSM+k?ag]lc<+:AFB.%l;W6QFF>HC0@BrsdY_[[>ET*T+>Cqo=T@wco/wt352c<?[H-[]Y#e5T]kq5VSe<*GJ(Km.S[qkg&(;L^>]iL<)u"
		"+,Omf-Ed68)@pK2'*fF40k'u$n:$HNJ.=Z8Ccq#IuD%,9qB(q8Jg'r8TSws8t`Js6(?cu>M`_Yl]_BA+,/7A4.qs2-a+Th(<.L2(1tvAMjkiU8H+MI3Ro0W3%0Z][TU5^1:mQD3&#/31"
		"nw#'(HImY#jaCK1fhn6/fD66#swFgLl):q%.7;X-U_H_@WsGm#0ApbNvT<T+%hd--6-w3(0,7aNWSR4(;?]@'W:SC#.NP8/RA<j$qu#<--/3j$_44`#;)r>$K94=$IKbq.XT'?-Tt>nA"
		"PpDmJ4Y`luUwk$Vns'e@RT3RD3YuG#NP(ipUuC.qnCUJ(#A3,)cV7E`F<u.L;]W?``b>:bC%fn#3H:[h#rP]hhoeg=dw0)I8rpmM>7XxYQg#ruWS9uXA%]`XgO()d:_ikuT(rhdGGV,#"
		"`2^i#EmEX$Wme%#4x$D][9gZ$j8kT[kv$#%WpXb+FM*v6WI3p.mJI#%wB>j[[A?Q#/42X-k@#+%4s--2UbK6]EO=s%mZ53'R`DW-_L,@#5pnn&6gc01w=vH;bk2bELBE$PLe^>]aY/>G"
		"snAkjmY20(kbSC8d:SC#@<Tv->;gF4H)9f3.m@d)41]T%GS(f)/99r.MGg+4hm?d)U[[T(<YF8MT[tEHaC<J[?OfQ+?H-'>K8Tb>(@LH=E@B%J94hL2W2'eQx0uFB4im)+qL%j2&*>>#"
		"^*^.$U)&5#[Tu]o+V(?#Flc+#$),##s`Q,$dct&#Bke%#,Gr9J<4ZV[t$6X&BgR=.VF3]-Zk+SuEvJE#JJuuYNvY&QngE$#pJ6(#XwNE]a.lP()PxG&rR>nfRZ&g21n3WSYAJ+'NQXb+"
		",$Ct6@0M/(rl^p9Xf-]-.n[@'vFA8&U9P/%rBO5&PbT$-&]uu,]+oo@D.##-bA^,3Uic(Oago&,18jq/6E+qfa-L6]x9]u-/*+78'_r>R:-;hL'YB.*jPe)*%`^F*bQv)4su2T/Oa4W-"
		"_,9TKj>vJ(OcO$54.IN#P=^fITEu5&[=Vj;Ij)w@WLai#'3T['K8GgWl&a<@'/QA=2EXD7B]-(9csZJVxG++71eYD#V@'DNFi<5A1R%v#:9.S[^JZm#]96WHmW/&$U*-_#[6=hUOPMiK"
		"d)W]OKpto%p`2>5wL$b6H,f3%3xw5/dn'H&L[Z58tAdV[m:;jrcJ]]%^IcI)?]d8/(Sx*3>U^:/vu/+*[VKXT1x+uTQn_79eU#R:iH+qj+jr'VmrO-4X+=ODB-G<11E(v#nxo=#=(V$#"
		"&@ap$##%q.B$g5#m5`iti(tD'KQG'-mD$N'0r+@5XX/^+T9nAIpF(<-FF3]-piqu>N6-quS+[QiX/aR[Sv-,MhpZK#?veoIu0NF2aD.;6vM&5]P(FD*7UTg%H/l;-LIIIHA@C'#uKp5#"
		"+B;>]2N,T%PI;##Yt6E.=3V$#/7*W--m6l+=oJU)9Z3IMgu5H.pK[+4O2IT%#^B.*^w7[#LL]PuO*.AG`t8;8Ia).QoQ/-P5Z^iTd>C,D^7a:8=N3?Lk7]*?@1?x1CEpi#7Al6#x`+B]"
		"+c@N:BuR5'l&X5'PTx;->lP<-0rM..vNlgLSCCa[xZWO^ahlF#N7A>Gi<L>#NmR0(bOROX';o+Msa,Q#E4A9.b7(B#a@=8]*7^Z.XkK6]'#YkLt+E.3K)b;%UMlGDuQ)B4Mv(B#;s?O4"
		"8%:m_)%<SW,wQ2QVQrE9cd9/NQ)J=Bq@@<.;+IpD0f]^PRHYKG56;S0bjIfLW,8G$EjU$%&5>##N[Fm#lT=[#7)k(#)8(,2[kou,3='^[>?c2$l]c2$wSS3O@u:8.n+jc)-gi1%wRm&M"
		"viUiuUeT>$p:r$#h('qIIZX&#-##F7gL#Q>SCd;%@*:D&7&`$'j44X?<-Soq9W]_%i2gbcduNd&gXdd$oSj**WLsu$+i*w$KeUN(2]4GM<Jr-Y9..P%/tOp;R+nCu1&O'JPCstcqWq_u"
		"`gs6HJG?$$/Yf5#aq@9]u]d8&xDA9@=qv5/=B:K(%Weh2E^Tv-*C'Y.5UJ[#3'Truc0FsmO0fS%$a.6vFZLfL1Whi#l=x1$rB2&.%*[)N.b5,2hSfj0Lp-cjF3=1)TKvD3st@^P8?s0<"
		"Hpeb#pm51<-2On:+(_*F.a6Vd4UJM'iCo,&:13eHjS<Z>/GZv$svQ)NU50E'3E&F.[d>2'TasJ2j<$'$x]lS/J29f3SC4D#Z7mJZOJ^p93GE#$qF)sD'ZJR0Y?n]OgFR3Lu$rY#7wwr$"
		"f`?Q#fAsTIb/-0)r1=N$j7n6MT)v2$v*901>aZ&4*1nY#mwT]F>oL.4BZQGIG^t7I]2G/1N5ql&(`FS7jtm.OdP)0(r8rP%R1Kd6;k%>-=B2m$f;Hr.wUXb3F?HQWv$s:8kLt87<TEA4"
		"2Q5wU?;Nj<+1ho@xe6E$V&cUbvL2nbHQlMBZ2qj<rG4rC#)>>#d*^.$j)&5#$_x&&+V(?#tuH0#EO<[?V7=,MP3NP/)UM(jG>1F%?\?e]+3+@8%p]->c3_+/(6V3q./iQ:v)H1R3q%,sZ"
		"J$d&#KUtGMS.lp#Ak?x-KMJ(OWRGgLU3mv,spCR3a82R3X,:,`%J%&Y5kUKie3%##Y,]+iF:[iTL3k9TlJeh2h('58J7x%X9F#&+ROt1B/(Zs.P(*W-OQ.h*B8xiLjlaG<&5O2(?vB3("
		"-0Z][$78Q,7r`9.>j+/(KYIu-w@JfLrvdh27Cn8%o7VNr.FBx$GN1T%BS,G4dFGI)QNv)4+gB.*(t0&4;FpVB3Dk6rQ%MIfL<d2<-3CR@l5E`#l&aS<cq>@A?3@x[rZ?9X5VD'H>H'=o"
		"]nZY#pc+##7+_S[DHcf(EVE[8AOUs&E30<-6Ovo$9hf9].;r)&]CwL*crNd&La&T&>.WA&;[DM9XW0j(tl0K2DjL#$w_0B#8+U/)N,2N9O-S8%ZH?w'9?YmuK3,'$8GQW(,@aUmw8kPt"
		"QLOj+$2xUZH++ou1^E]4l4X`N$),##JJg6$EZ<1#guv(#d>f9](YB58sX`8&1x19.;?3p%6sM8]`FofL[5d2$eYslf@WW5&pBb5(55N^2jlG>#tQHD*-Y(?#Y0:;6'R3N#0wUYRpbHI$"
		"i&IS#)jAE#eGdCa;9<`uf<po77h)##:JO*vF.6Z$)[I%#mpB'#Zt@kB'O'3('@Mof&Cg;-h_#<).bEW-c46eQIfpJMwGuJ.-&b.MIWbSpx^*T+.k7pfE1K6]bJ?L'?6Tv-VR:a4feP]8"
		"_AmO*UYf)*g%KF*:GUv-f,-a3?#WF3Lr8]$r?cLp]i1F@dl_IS[2jPcEMbIRX*T19e[x9?a-In#fhe^uF,.IYeB7#i.rI>;n@6S#OR)U#bNI*QFNBxbm`_-$]$J:v0/)lG4d1D#p`+##"
		"-arJ1WYf5#CF.%#D^wc)xBCVCKsoK:$$c&#cx.a94VM8]]7f+MH<,,2sn((&5gx9AIebA#FofK1END.3TTF=$H0x9.+/`$'GX^:/p.9Xq*:rZuBOFErsu]dQ5A`S[n;:TdYshIhc;ou."
		"H^]NkT*###WZj-vaeI-%e,_'#,S0N1f18V%ABXb+M.0jaE)^v&ITbs-r_JRJBD;8]Q;l`%0tDg)1*sp$R5>o87>0Z-Q=2C?p4WO'm?`[,*Ivv$a%NT/pq.%@&`R_#a^.*l=JtA%x`ppm"
		"KaXt+s8`;]f2bu<SVUO1?\?)'@=9pUK7;tN$>7edp)gsSB)c5316pU:;tw4*+)dC^>&V+jNtCO`dqr/Ns&5>##_B46%QiP+#+^Q(#21>J]7HBv7M^EN1b+D/($;+QL0@Us&+9_m/Oim5#"
		"l^tW$oL8r/KFW2Q>:'^[c(2]&7&NW[0^Kp+=:')#ir%31_^,3-pkX`#r6H<5$`Ps-XbY7MKs8755$po@F'AD#VS@^&_#K6]pp6Y%a;`/(l0ST.MIK6]A6Cd%#l'2(hmV@0V_k[$?<p;-"
		"`xjj$mwt9^H[^C4>:Ye)#6l8.(oRs$C?UT%/Kur-14G)4i$grmq:7mhhX=cp/l:E?8%RWug[`7MX31Wu?;u8NImjP:&AI1T-gA'ce0s)$A3_<(ke*S)m@-][R=(&:,uT>8^A5wMTnfXu"
		"nJ:uU_oT45-9*p0G7JW9HHq7nhW;AGJ05##&1ST$@?_'#l&U'#juAmJ>Ev;%Mu(t7/Gi)+8.n8+=-x;-?Zud2&A#^?Z_a&+0?5Y[%(H^(qJl'&0t@aHe4T&(Kg,t-km4kKfmZp.Bamf("
		"`Bf[#?]d8/>;gF4+g^I*I$4Q/4t`a4I/5J*D+39/(wuZ.8j@)$t3TdmF8AfK%.b*Rg;Oae?326Wo&D)^xPB;-LGhjOn;T1O6vn_s5BbIX-Qm]KbnWGnv-wFJ2YAM=vdJINvF$A+FKS(Y"
		"SSdLd)@uu#Oup7R/mMSe7E46'9T0A=*(wB7b;Q/(UD>j[19kM(soMH$g/H/(4os9'?kdM9VLb2(.NG7*NeP7*tl@Z-sE]c;umB#$:tH,M%cw]4$Q.Gre#mm:?#fm:_CfS8<0*`#GJu4]"
		"#=v3;)CO+;06Yb5tYCnLDFe'v5pfA$IN7%#fWt&#gXW5/eI/Z$?/'JMqhApA=Isrpj>l=$X;C[-+.$+%(4U2(O^Sn3Y$TQ/W[kT[*FZ&(n&gW-iEsp1Hk#`4/WeL(vUU?^DUr8..$'49"
		"'+?v$No7T%(nXUK`+HF?'/I-c#Vd)AiX%vC':8E#.>'>IJnIZ5Y.tH3Q/`P/o@S+.r.Fjub/v[O;dwJSY?r^BZ%[e`f[[GuG4M7#RlIfLiQ#^48Ufi'LOp.CL#$eD>mdv$%-i/%$QnjD"
		"C83@$_YRI]6'kBErigCroEWW9R.Ctui+B:vKAXFitABqu/]%x_Tdq/1GPUV$wYf5#MXI%#G[&c$0NKHD,JvK)^YRh(PY3.2)#7IM?S`-2UD>j[cU?W*,<_j0Z[Fg%eYslfUbK6]&U/O&"
		"tVkT[Xl;q/cl>j[ZrO#)njs&-2dJQ(Z_Y)4V$nO(4>eC#Jc.Z$=4v/uf/L1K*jY]OCbZg#&^mG$fC[br$NY4J,xVO#$)PY#+/`w__3VuPQlRfLvZ;b$]#j*'AA#^?`5Ls-fUL*M)23Y."
		"aH:;$=sus-jiQmMkSG,2&nQDNSB?2.I5%/55#Z,2,PY,2Osi@]m0eo@AR:e&V(%s$v[#gL3e['M$^bA#6T?ip#a%X>LS1/1>0/J_FP&;Z`HuY5$V9S[<qC.h*7>v-Df?E4;W52'7nN.;"
		"x=F7iYNtl&3qYW[Y,h5#a%;/(Atg8KMa4d;s@x;6BrjT[aeb,&`u1/(af>j[`ujh2x1M:%60fX-Rj3c4`tJcV;9joR>qBtL(uXt9HNKCsIfk_/wH(5SR5<m$+viw$.f1$#Sg#d.)@@r["
		"Ifd1'CA#^?'x6w[t4C,.0dqGMIn<mf9%2=Ll)M[KqW'dRn$l9rhN9xQrtE]#OxT;2UoQ'J,%*G;<?[S[HUl+DZnje)df<@G]/DTh]ag5#kk^;<bv^>$67A9.Ddjl&&@G8]gUS3i`BtY$"
		"tqXF36/=xH$W?T.D]1RY1B:n/Xs2fLISiiKAJE%+%@AwcsET@$%'fp$l%ST$@a2#2EcCC$&[>+#Qb,?]MABcN&Cf3+FQXb+j9TeZm-+c%b'Ox#_1fV[/6Fr1Xvc2$2o7gs0lo#%Q&KCO"
		"g3^j%qwJS@oU*i[<ooTX.<:n/L,JmDM;7U$v%IxhowCo]upu+'(1GkI.7B=ld9[Y#uUN1pwa'#c^O18.ijBQ#%%a8.Te)T+cw8=-/I&N-xKnQ%(FXK:E9ZV[hdlo&f-KM'PI@j(gdck-"
		"*Aa_ROZ;XF_CdV[>kw<A]jdh2meF,6@.Fb3j2E.3BhgnJI$$Z$Tb9T%VT3I)k/1'u447jX[JPj#jYc#YPpaS@qw[Y>4%r+`iLfQDB2%I[)=*:R2UJP#s)rf(D*fouDlUo73^;#5hR/nu"
		",ZC9#,d$8@_Kqfq?wUY,*gu(35b%9+5>l9'$Z:g.#bk1(x;b[AWBqp&S#Yb+MM3O<=j@iLBj@iL`AVj$bPOw9?)GI*5_a/1<+Hq]%w$12+H^5P]EXJ'+NpW-Smx34@_%pf<=&bG5PN&1"
		"i4=1)hqC.31_d?Im%>c4j@h8.Gpk-$oY/F*POgm_fL7N:pi$HDpl%$lGf@p^T81Wu6(gfHMKjK<StZPsg6_9m9sZBNx$ERNG+O.$<Z5T)MWef#tk3,23dCH#@,[#OToL-3F<I#[V@-C#"
		">(Jx_X`C:aF.;(J&S(0#Ze>]OfsaoRJ$kl&a]v2$dm6-&@?Xb+x55LOF^0d8V/9K2xOFb3uD1S%f#`Ob04M/+'b^L$OLTx)I2ppt805###8.jkepA%#O=K>],UHo$(9U)Pu:8('m@Rm/"
		"?9V$#0?5Y[Q'Gk'aYuW-WFBTr.Fd;-5l2]-`GUv-:k_a4sLA['PKZI)-4,-3rN7g)^Gp._gjJ(a1Bb]kd.ep3Vb=;jvJ8DET.HVY3l]?ux,P@#WbK-O<6`1)@T^au&=dWEYBaL[ZE/4-"
		"HpD>#R%-G;j-Gd)R=cAG7K&'J@c7.2FZ&r7*5O2(7EO2(hJ#<-DlP<-:NL%'EG#^?A@Cs13J7xDVvVW[GM8a'/C#nfPc8riWaFp864v,*$?+P(2.a]&=CKd)Qa2edQZvH4:vxm(T+QRb"
		"1:`/+#=+O#t>[0u0Xw_(I;RsZ^:QaBp@:I`BF(.#(9`(vqm'/%j_R%#W=K>]2$`N(#p)N$H;^EcW(N?>Qu@4;jYd3'5Ts'%[r;GYa%tD'OZ.J&'=o6UZddh2[(bN0+._C4+WkL)lb87J"
		"Pft2(Ymg/^gd2H2bFbO;,=%-?1g=6Y=?;J_^bR>%4xTS7ikP)34PHtLeV)aXax8tucUv;>UhQ_</S]H<%N0,`<DED*LE*qiK#6W.7]mJ)3v$0:0igJ)s&YK)'B;>]O'`kBSnR5'f*oJ)"
		"w=#-MD,qR&A=)aN?kXI).:,L%R5^+4nGUv-0i)HmYBa44f(=5Ds9gDn3'J1p6#<UhUJ&hVSH$<):NG_],^0O,Li/M#](wo%XqR__F*87#L;m1KS;3W6BTZ`*6kl)'Z73T.r>Q][;U8N-"
		"FXPL%8)`$'FBj^%ETgs-CV)c<U>vV%Pf>p&jfc<-G[C-Jeli)FdPlQck97T;<&^L&XPY(iIJ<+J-Id<I=xCm/)m%&+>e9K%Iq9et07Us&)lQa*d>?30L:kT[$3fv)po/eZ=tf9])[A/."
		"v`?IMP-3N9d8KK2p*'u$fj)W@'9kM(:V5gL,.Gc4-`s7WN9.:14KGw7ms<8R.BV#uG_=V&&>l58hh@>##B+G;KYwY5]4JfL+DaN&BxM<-7laY(gwH^4d@h8.eVp.*$?+P(rfk._X'go'"
		"Z8PH$<2(C]oNPF'ThO.#)NCV?bpM`s7.')*J%x4AsoN>-_::W%sq.Nrixf+'0JHW-T=5qrHt^$H1JZV[@GCx&kGK6]dBuW%?JL-3b@XI)#T#w-FEY)uZKo8%uck1)lxDT%<7ZV#jw&@N"
		"@[OYY6V2g>=gfU9VvM]IG;/FGJ7cIh%YLmt_6G>#oPqqV]XZ-7RfXdPPdDu$';rxkqpw>#qU#29]ML,2?tc>#]mC4M:kFrL_>Hm#+D:,2jF1<+rr[e*c?5w/mh#'(RhDZ#=:X8%nV8R."
		"?QCj$C-7Z$B)+hYkA2T/.+xC#aO?dbh'wR#]9GPouslB'8`###okWrLjiZLM_2?dN'Q>o$bQx?#nr$O=JDc>#%>GW%1iq,2ho)d4:P:8.8UfC#Xk8pu96,loNl=tTmT;W#DA>uum8lc%"
		"h*axF3.QT1lgEp#aKQ;#^>$(#'(8'fb&eo@[V>j[rw#'(Pb;Z#D0M/(OKPQ/E%kT[fwBd&c7&N)OHv)4^S<+37Y:]t_+Mk[Im`t:*V;<UuoBQ::K['AbMU]k1aNJhID8>,C#TQ&n3L&#"
		"v.J8]E'/->L7Wq%hS4NLW(emLbUsV?r0N-49HM*#Hg9SI?QPD3qxo?911EM0[B(gLq;#gL)hBnftof9][nW#fd;Z/(#L[@'_JC<-^>@>%0iRv$X3f8L5P8<K$1am150#TBd6`TJjf(gY"
		"5Vjt7Z(<U8h),##Iq1G#V9E)#]:D8][TQH&n1jQhW;Vj$xppm/fm?d))2pb4m^Gw$GsexO8>Y61b[.[@$>t<5dxVK*R#fC@FuA>#(j=Dj#F_cr#PFS7[GC,2^]>j[v-6'(,Zl,2<@'^["
		"0=j;-?k@u-(g&bN>]rX-)be@'l>kT[><bJ'H^-T%H)TF42bfrKMRTiDo6v)4&;BH5U:<E4iND(Ra$L?5_vS@S;A&(#oh:J:RbjY>+A(v#5j%RNLC#m0?ijT[Y@Jj$k)D/(YuxfL:HmV["
		"pgnU[LIoI24ddC#*&E%$J[xWppt&PQUNGk$KxjT#PYv&MAtogu&fM8vBh1$#x=N)#wl/X[rcaQsW?X$#Y1P;-D_cS7/6vlA;L'^[nEZ&(mppp%ZH+9.SC_v)avsJ2`a=1W.l=Z%JcUv-"
		"d<.4<5#YU:qu-R2l-1O10k%Rp'?=6;ud79:mF[w-R+/(#Zq1uP_2a+DOiq`d5ls=%h?,<%C:)R8APXE?3S3]$%95[J2-CB#Uh,2gv1OF7jw7T.Qr2Q#u:r*%fu6X-Tbh#&Vg'#vBOZG$"
		"T87`$vOc##iO]=#xof=uK@QruB5R0#VWt&#D7ct7TD;8]7+k]$,^5Qq[3a?#<w9T%=sxF4<0<a4?DXI)CRr?u[=Ow7l@S1ZO6hrmOo/;.R'5F?Rg+39xlSxtn<Z7v#hl)M;ed##,U1nW"
		"Tx2U%G>I#e,)xlfkl'R^^&6v$d;wo@)+xM0?ijT[mgnU[Z8=r7d13D#&ZH_u%r,fhNu's-1i'V$h_(ZuS[k,M/]#]u;CH$vbTl##<[Fm#f9=W$G[[s-DhdJCTE4AuZrHX$ciDo9b.kT9"
		"Ya2'#.d2W#%XDmL$]7.MO>0<M([Ht&RbM8.>aH:POds`uFF?9/r7d7$lD.#Ma9<$#t1<)#]B%79OJd5Lsb1Z)De`AOD9T&(x?%'M[MhZCa[6C#ciO*R0=u#Ies<#9J?E3aW8J8[8Z=OC"
		"aotxuV3lShU'/s$u.j+M`KR3#dRJ]$.CP##g55@&%Q`>(k:fNi7t>B$:Vw]4,To:$OpBut)7,</>e[%#K:PwLSL?##qm[e*g0X:.'B;>]B+js-vB958pL$9.H<>s.btV]FRx?GtJ:`c#"
		"K=rg$?u'%e:t%v#l:N1pZh&#c>/HK)g`YrH#*kdtfSv/(VG>j[?4.UhHcCx&#Rh&4N5.L(FICa[wHwn]M5Uf#h,RZ2C+i*%QO`Y#Pil4)e=pA(WNO<-OS1k&c^f;-Wk_WJ2lA&$]Z8E#"
		"?]/`Ruc>g[XVC0$%TI@#2<m6S-gOQY?Qr+`+%%n#k9tth8xclY6LR/ACG(?[d,F_ht(hV$eh3C&oG4;-:9:g(4nwr$h)@>%gRR<-il3M%M*;i%EA#^?/@Y>#N?ri%*leC#3d5juW1/X#"
		"oiOuu[%5m8<`bS[om%##=YO.#-2BkLRS?##5f1$#_gZi$+&ua*&qot-*Uec;QH9u?9T)W%AK=/'N&oV@*fB#$SiXfE$:c77)=sWuwN/2TsC$2e='-5$p?E>#speQjYBCwK/3mx4J:Cv$"
		"m>l;-AV*^%Ko=jKd:Hv$vc@:&w[e[-4ctM(*FdZ$];V.L5buA8%ctA#trbQjoSrE@8g.;6[5J#$1;F#$6VJ2r&WnlfGFVl-EhX*v.<fX-:b#&4FUbp.B$x[uSx+qt=5Iou>rgUupn/i#"
		"c*47eVv]a#qYNSu&Auu#o23wgAwoEI'asx+^^I`E1(QFYif;0(o:?j[`F4l9YZgJ)[vkJ)'Lh8@IZ)1)eJRh(OS*.2WVuJ]#[q-M;)v,2bi>j[fZOj-EZa,MCQ:I;),X>-9+/-*gCXI)"
		"WN4Y'auj^._48Y.oUfC#v4oL(i)-a3c='C4%uSpV>SOoNxMeOuQ#V:QZZW`W;%IoYMJk1$eb+gQ)O1;$@O)_T^@4($YrixO^Ls%vfJSPu@?Lm#:lli^twdN$)(?P]$),##IL-T-fqDm."
		"ICT>]n*VL&CQgaE:]4l9C>j5EnldR&Q4WY$t;ElfPkdF<EZ;s%6Q5F4(M+F3Au68%9Kt6/-cD_&X.P?g=[F%$,[+Mu[RFAsN+3on`0iuP;3^P8LZ5P#gP3)s`(d[u;eiP*K.LX#vI@KU"
		"nPe`ECPJHEZ`p%kgjno.-Ns;%k3kEIU1%T.9@@r[i$=G%6#`$'cr=ZIFJ=jLLSX/Mpr`0(quOg&Q+wAMc5Px>HrKN(,_%pf-Hv^]^&Ol(0<6N0vF9F*m>sI34gX:AHj/QAd@te)tv#E3"
		";u?]k]*6JU^K+$L$FbGiAFOM#tTxO#GmnPE,,jQf<[B9nfUQ$j@--sut8kMeIx^iUZVG>,CQ7ul*50ju3&`8#&3GY>7K,pI6+')*ixUoI:7h;.N_'W[l%l>)l9>sHY;]$R-nI,XH@C58"
		"fYM8]$=cgLXx_v)*6ZV[BJ_x&XJ,n($`k**vwi8.@<Tv-sdA3(]f^I*2#gb*,tKT%b>u:$Gmaa*%f574H`:4m?_7?Dwqx7J^@lT9OMnH$&[#lJHf8GW2E2cs<lC?#sHaKuc:prN@w'Mc"
		"P;k9S^e:)EV(JG)[-6JC2/^Ic-_:3(<MpdmZQS495DQV[7>[p.'B;>]<t0kbNw502]oNUiAp:T/P[bV[Yk2/'oA)^,v`tJ2:]G6Ux-C+*K(P?gV<x9.*D#G4L77<.9U$.MUQ&K4/]NT/"
		"6gE.3Q$=&r0KEsL^&VFJnnOU#IeTvAb5jNG`[9[J`Z-#cTLYsTk`Y$M03ubu(?`1HmNq@F0.[TD7KP.O<7kM#3.>>#L1FlAZ,%Z5d5mx4>a/6&9h%;'@W=gC@]Yn_Tt78%_[hp$7DeC#"
		"(o=N0hG_M'E'i:QLdEltb9RuuD-,A4Yi$Q&%/5##i0ST$,LF&#n&U'#Vb)r@-gG,*_(IT.x?@r[fk_P%=8`$''bS='>[_p7oXM8]xeffLu59a'cb.*OJV250x3ed$vpqd2))'J3BY0f)"
		"?;Rv$$]K+*3?(]7fcfF4#,39/=ofY%MkSv$u5p^::XXsDj7P6laTeJl46+ipn[;8@Z`8(JwV;5Fku+5o6;%`+8b,g3gmIbu=]MlTqU>8.xhr0vQPu)OSl;X`0LWDeOkD>#C*TEnSnwc;"
		"+-@]OOIL.;q=Mj;GPFjLvRFjLE<@W*;HkT`A;1W*`BXg7N]-]#sPVZ*XR5W-k7pnLY/v2$NjAb.vK0X-6o8U)%]45'?'Qd&(<mV[M0'H8E]M8]RQ%iLvMMN$&91^#XWq^@bVg88Ur>K)"
		"`Y0i)S`[e%-)H=$BsJpARr8;BY$g3vHWO>A't_*dV-usCO[nsul5ipuVA]X@aOHsI^2DC^8p#l;Bf3_ZPoO&?k%ov_Tjqjt,e>iKQ1cS[6qVf:./6W-8Kc^-U---&+9G9.H&x%#+AaV["
		"sM.A.jgS@#Y*,f%efK+*ig'u$hi]+4#2<9/age4(]pH)4P<h1BHdI2;,xB=JR6Wd#_ER%;`w+djFxbS[+;s&FM<`%@TrTPB`8Z07t)v^6PXjhDvDB;-7H5xb]v0RK8DCs$1SIJ(5hl)'"
		"3@c`QJdc,2P44W?FU1TLQU)v#g:n]-a2kW8]gdh2rFK+'$53K(`GIS@u0^b^&;qNA]&%3r'?0_SF<WIURl8oD9:oT$a5>##21ST$o&r7#FR@%#nG]A]Z%5o$apjT[>Y8_A(_Vc*hI/;."
		"FgN5&.:WqJ)D]e*b;8$GEb>v]$i;mLXv#6#T^2`[DK8o8v/oP'&AggL^19S8RCiOB)E-Z$h`#d$#jlX$.M0+*r61A$bLHukVxU6nUr=>uXvOKJ*^CK#I2%UJ*VpY#Xh$ousd8E#qt+&4"
		"T0b$'*'.K2XFHJf5DmGgd2*###x@1vSchpLA*<$#FDjF&]I7T.(@@r[*`fC'YK-3=WOn.U>b5n0VW-onUuACs^R&H`t+W%$<ST81,<fT$osf=#?P<=]^2M=%]h7j9MR)W[r>Q][9#jE-"
		":,/b-p1mQ:lOg&(QMRh(<Ymd$I/mx=`SBa<.Of@#I:#`40<Tv-wq;v#=j:9/Fn3Q/.eAbEh=ZuYThlM(Ha>+F0q$>sGP@JLbwY$.,io]$l=WetA]+?#Gk4<?Q>i?uH>-*j*R8M^<ZdA1"
		"U]Z,$m(r7#Xus=]#OV5%9c#F[=mQ6W#9+v6G&iA&.Z:K1<DQh-8P(_Itj8Q,O(]A,Y.3t-,8&NKRF/x$KIgG37=lM(hDI)*lp0u$5gZa0>U^:/f+39/7ve@Hi3'E<K#xNO)(oL#T^xhu"
		"^=.&+_3<c#Rbq348%iXu_.XPWJgUK#v,;BK'DA^>BJ4ou4-VC8[Bc+FrE95CNIS<.?<@)4q<&2cic+ABg,f'#6t@O1AqPPfv1EG)=$Qd&Hh*9BGnto&pV#<-A0fe%FA#^?T_K6]xf>T%"
		"mLga*i1aN0FDsD'.=pmflNi,B=v00G@+&^-60x9.'C?>YY7m;/GY4cu_Oh>rR83-$=G<078+EX*Yi_]=$)rlJ915##]6XE$Q[>+#Kke%#YCM_+1Yvh-dM?El4JvT%%x[Q8ZuN'.Vkv##"
		"*%oO:xNoS:g,eh2$lAL(rRC8.3h]?T'1wu#f%ST%t#ZY>M6R=b6)<X#e+]vR[<YgGSf@)<2PE.q`i6rmD3(;$j1bV-o3ccZmsY:bp->>#TM/]OD5F']dBSc;8:6W.brPEN7h..2q-QF*"
		"Ew1(Qg7O2Bl`M8]:%KfLFUa5,le*X-8?RcZloI)*Q6378]&Vv>i1NT/ND#G4fvNFpG[>HGk4kf15;XiKLSoR0(:#Kp(7hR$m@wEBoINS+'YFG5l^WKaFcf.<[w:n'Rrkg/ESa1#xd/A="
		"=&`MKqYhi'5$8G;l>7A,Y./g:7&oP'3/uP'cDP<-AZlO-1Z5l&@tMm8s@C'##()ofQ=ZV-k7CP%]RS(#.#JS@T4]v5EE;(stFwnQL,&X#Rs%p7=G2CJi8@eHEO#sH-wN%p*:$C#xAaW#"
		"umnbiXwEw`$il5#d`*87UX]lo0>$3(v:@Q#ej6:&dsB68j04m'B_uAZQUI(9t#Wm/.*t>-8RG)43b2suA;pU/=JxhEi]XYu0^F2$1^_26RvrA1_(%#>ixsb.>+JT$Yvsb.vjb:]cEU`*"
		"N^Xb+00Ct6ks):9Vag5#Br@Z>D62N9TKD.3co%$$8^r0(9]d8/$eJYGvUK/$KWBfuPbEGga(nCu+WE4ou@I>,@t5rugYenl>Yp2#ib8au)$KI$)le%#i:p/FLeIa+i-d8+4]h;-R_CH-"
		"1fJK%Vd0i:LlY,GbI3qgoO%B0u3$E'%4;of8^%%@#geh2,pq%4cv4J*Jj-H)@PEb3*^B.*c0/%#p*h/)Mpnh%0HUv-I/fdu,INfKoQwN;H]7vUR6<^NKk>(sF]W5X_%wi40pZ]4#IeV$"
		"b(LN`oDNp,ZG=)R3#2;riFm[uY3e5+)=*Tac,>>#xfJ9i^pvY5*(_l82S%H#10idMTAVhLLo3FN%C_l%ihN*,Xp,Q#tB=1(G-g(1eEZ&(l.^mfAWQJ(=A<WA:YkS4[Uvu#3`s0(9NKA="
		")1>#Gm.qXl7WL*M[3LX#9QB>$HY'k+%sMV#^.fLpccmwdpYdC#FSCD3sQ^1p1%fCWh*?tLQQ5*vq49kLaRG&#oE`4K=oIO;?W%iLW-I+<e08o8$_`C=f)R(#Fr+@5do>j[L%l>)<U*.2"
		"ck3a*T`c;-TM83%3cV,*OLo;%?GC[#@V*w$%'?/(sH1AN?=IA@6-WPA>bWB%S,)Yh;N8^@:=oTPJaG4o8bh`USsJauq>$He<gpc)')Iou`_+V#uXuXnFTXrHWgxlSc?;;-cg.DEV_$c?"
		"kx`0(Vundb*gY1'ai@%>VL,W.ctW<UIep/2ERE9._-Ud&;b?v$[oI30&4Am,$ht1(2d?mU^sdh2FsHd)I#%i.KqB_40NKW$Mfi:/_Y8f3*^B.*b8=c4f_^F*JGUv-U&aj5b<-*h;Nc)M"
		"3A>=gA./>c#3'@;t7,DK4C5l3)A[P)'DxTuT:tT#vSp[k:I.:M/I2,)vR+fe(JnOf_1:8.S-hA#LXl*#[ruXlq%ifU[[BA+nr$6#l+]a</*S5'm?E6''B;>]>.^QAQWx#eneP#)d7V/("
		">?[&MR99C4T8Zp.`tv9.hf]E[JgWjLHlq0*Ng^SgpPWW9lN0n/])>(jr$#o-h'ST.91o<KZ`(?#P-TK+.q&Eu(%/t#h(-B9j@l1C<6YY#CmL1pgEuY5[[-5/NEUg%v/MwKe)kT[n'4,%"
		"-<t68WIV#nKQNn8@eM8]qqF,Ms`tmffu>j[%0o;*j[70(1Z:9.e=q6&a6qV.gJn;.?lG^?E4Fb3)M3]-=V_Z-GxCe$Hbh+4BYB+*p6_x$Ol?xd2a8^@$VYV4dLO]u'4n(u(7r]=2U7I#"
		"6v5$AH'.,MS/8ulhL:rlp['#,'>:Quv_v+]6'gfSv6+9Q1;E>#8pD+rE]bS[TN*AF%r/gkBoI.2uH2c*2%Fq.r6GB?-Ql3+t1oZptS_Llja>r%9a*?$afi#>hDZV[j9/#'/7-eMZ5eh2"
		"7+'C4GdQ/(N<--3&L0T%<1w9.VF3]-u)6G)'r/Y#(eMsu;m=M#A;XbIK6WH$$5?ruX9XsuD`(.Jr1[I-)V2;mat]C1'1ST$2)r7#gmi@]-ep&$O_x`ZR5k^%@GhgGu+<o&)em]%)-oU7"
		"Nb>v]Y$k4<Ykb&#pS`l*qSUW-X+q-O2>9Kk?%<)FH]Y#>.K@Mp[;ivsYaLqQt&UJ(GX#p.4p5URb$Eg%#;LV$]c_HRSh=ciW:N@XPU?gJ.3E`YMw%HU5Er2eb?SXU%/5##XlQS%pXf5#"
		"_EX&#w[39.(;co$E_MmAKh8gk9@gK%gla`*[%e=.qZIs**]Q<-'3RIkLj0A=N+qj(%4;ofCN[&M>o3b-m]WI)aCtM(KJlY#O?Z)4'2f]4$28L(75^+4TYeZ$Qa$7T$F+lo&5[ouGL2kB"
		"<DZs/kpI:&D=SP#Ao-/QCWMO#oq2Cfe[.;m<R:%tQdh*eqSn6ij..Su%i$RRHFti#KvslTL(0m9v,G0)r<(DNh;[],_:r;-A'Z^$_?bG%OmXb+&O7%'1jSa*Z$F9.N1e-)%H.9]wtdo@"
		"><%[#8l0gLH'uQDW&)N$`VB-'f&pBd0@uM(Ca'd1#DXI)'CuM(4o0N(,iCnsj>:PSNJ9Y#Yju,LM*`Wu*e-0ux^^sL(:<vQ'tB29bt#ig[@Z'f@R_Ju^JJPutpD+N15%1#UJ6(#t+h%M"
		"Ba8%#H'sm$-B'q7waZV[G8L='C*ReM'oXQ`[mg5#rR>nfPNSg8p$k>-l1bHZ,BLH%VHH)4/ETiuHmXM#48,#uNTb7NX5,J$m4?ruWt5pm%KQrdC/a@u&qClelh`e.R<rg$wU)..G.qWJ"
		"I@($'%GfY%/A-N943QV[<w*#/1]J:&]ocERI*nO(13t-$*F1N(,;_u#Bp-G;]IS]$C20>$TKYbu%F+.#a2B2#lS'%M%AE$#G=K>].%4uH52ZV[73j2)NZ+<-BLbo&^wJ6&UN;^#Ppfa."
		"M.EM'S:SC#aveN%+Ec'&lr.H2#*CBuPwMKV$=+lo=7T6$4_sZ5>Mu1P25?ru,=$#NELqV$.LuYuMaEMF(nIfLpel]uxPs'M_GN$#O$Q?]93..M76&1(kH,F[FTXb+^2<[&'4pfLloeD*"
		"hX(W?bdDW[>-,Q#b+D/('ngd;GVmT0eY;0((%QgLQ2u?.4*x9.LPOl(JitM(T=uM(NfhV$x7a48Bi6,?oQ$$$KQ__jA0p:?E86iT$q8iTUc<G#dQ%)NWxaA+B>AfC$),##@c68%q1G0."
		"MkCB8uc5r@SmR*Hg5.q&#&G%>'vh$'$X^m/O9YJ(/1qA(>[qdk^5)N$jU%0(;HtR/k(TmfND,/(YNJ'fJ0x9.t$TfL;<5+F+CCW-s:m3uhqYjuj'bpuL^ZC#[<gOSZFec2C%g1g`L;;H"
		"4Tx-$ViCZ#nNik#-N5s-J<7FMsl-'vD^/e$frB'#VDFQ0<@nm%n%Q<-[mnA%+mu_F)*Dp/Lxm92W*U)0-/,B(__>v]93Vs/B4m)BEns*%j0JW[,TM8],FgfLA^h,//X&9.4L`p/3WZV["
		"^iu[&:5mr.&iiIKvHlD4`V(fDqCKv-pu_W1H-C</'EsI3GHa_RJrw1uqfmOtt]8mEH%eIh0AMLKg8V5N7w=.Z,?R<$;n*,Hn<c?DxSTKXS'e%LR.G4aRjwZK<G]FddYfWR5b]2'73to%"
		"@vuYf4F5gLJXb5ALRC&RULH##)qoj-j=#+%p:SC#Fo0N(6l*F3F+D?uItQKu'1NSt+^G^tt9jot+(>G-mRU`.4>w<]CC87&P.YT@W];s%L.7W-<eKsJ`V[,&k5D`-rN2Z9nB6Z$x=u^?"
		"/gx[9Wgq'#^(E%vK?2o$8*V$#.)nI;&)DZ[]g#6#kTFg$A2KKkU8`/%bR@9.MMGj'2.KT%BDh8.vqXF3pt4D#=MDO#ab[[uFI--l88$Y6D2S_uH[e[#(>Wwt*Jt/%s`&ZJNUvu#7Wc=c"
		"36hloX(pu,KG->>JKrS(h`20(tI?j[L_p?-YPvG&G`<.2_:H7&><QW[m,h5#g1H6&A=iT.lu@g%TvmX&d7iJ)22eT%>Riofc,))3c7XI)bkGg)pCn8/^?7f3mMA+4**Qv$$`^F*@q3L#"
		"5uTF4+87<.<M@R#l1Dju,>@_*Qjd/T<Wa*ahh+uYd2[V*>w0VagPa5/X6STZV)wjS8b$5S4j=iZ?=9jctP*^`Uc8v#FxsjfU`Ud9e)WgVF`bS[jYiu5DmuP9mS?v$N`LW-ZdS6(ik1Z)"
		"v-bd9YEx`tj.0U#@&u_.aH:;$xh=(.aIe;8Ye<T^5B8C%eEo?#3r+@5o`f9](a-30AWQJ(+ch8.`59sT3=5H3LT,=$c)Oe[hd$crKADMKi=1i$wc:c#Y>Qlnx@Mc2QC1_#IV<b.nd68%"
		"djQT-4C-_AiTETB&^qd%_Evp0Y5Dj$#&fNt@-e<&TOqkL@Gu4S4U;Y#jA34S827E+Y=;%=<cFB#^OjCWd:i'&VHgx=:[99@6Wn&K,uDF5a;^Z0_`>j[[%Nm#4as*OV/F%$)E0c.fj+=t"
		"<`g(/f=H=l6bY6NL/5##@PUV$:Gq-%W%AQ#2`d8.HM8a'=xju-w6`j99EvV[GGj)'EM1<-`NX_&<4n<-54$u)H=:DN3&x^p#bmfpkqMrHdAF70<.Q%KUMA.hZ@d'feN#2p+>#kVTR?##"
		"pZqr$'cf/.kk'hL%oE@]2anIM?-lT[_%Nm#9>;N9[Yl=Tjv,&'>qfs-+qeg:`2O2(PM'W%Gea9.M[C+*#o)W-.,He4x(MB#s[]$&cVd`aYM<o7;2`l-XQ9nCA?4%=*;MK:w1ST&/nRc;"
		"@c1kDjrV0(1;1kDPiUa%*u#EN5bhA&kS(`dWNT=;vckA#mIk0Qi_,p%gC-DE[6CJqb6vu,lwLXJ%,a''rtg&4mIFM(FICa[(Iwn][4j%&^)T)gAkw*%j-DT7-ZV8]IB/>GKMC<-58/('"
		"NcUv#a4]V[tCpW&g>Qq%uY6i$hJE+*KG>c4l7[8/?;Rv$.(f(#Iv2g(fBf[#mYY&#sNKR)AD70Z(/lu#*eqYVT;@$uT=-jq&fT9RRpvbrK8iIh^.Q'V3s*kebdFuUoHG#Y4[2gJcqq7Y"
		"hTd;#q%%GVtX)9%&W_H*r[3MK5LMT/+6t;-qJSf&sc(K:Dw5k*N9Cp&kT53'nAeSLTJ@7rFC35@dDdV[X6Q7A@-Q@f3b1N(F$nO(n#GU.nFfFEsof=%V?cLpw39%t.,g5uu-jMT>Y6G]"
		"q[I6)[G+#,2'+i$c;9cdm*;r*4^FhLrxG&#6Dba$5n:$#Lb,?]T/cL:xRV8&eKab*x,ojB&O'N(:fRn3,QB+EuA6Z$xwAU)'WZeOU2SN2Rn@X-m975/o5Ea4kgLG)[5oO(rtVv#k4Zru"
		"R2/X#&+:,)_g2Wu(Sa_o&j4i#$%Zr#o.TJ(1s%At1Y*jJ6<b(N89#=@O#[;6VG<A+H8hx=P.KFchlV0(p:@T.0@@r[1%2D(^t&:%iTs;-Fiwi$lo><-imK@'IL)l(b0/%#$$HJ(]]J_4"
		"M)tI3JCn8%T/cD4mK(g%5p_:/dj9>s3RZukV*$Cs[cxI85Nx;V/'C,Ix/[&J0nBYu_>p/'Bk,V#=J>M9rNWlS=2uG4$fY@.em')StmF@UjNM5Pu6>##bH:;$F83)#FL7%#kASA]GhAZ$"
		"_mjT[V2b,'I%H9is]=#^t0%<*)xG9i@2l;*`BXg7LV$]#rG;?*XR5W-%2G)6_]Rg%*fR=HGYp&$]mdh2?6J5BsebA#OZXs6*8Rs.G&9f3SJUM#U0N2AE2DjkC]oX#Qx@nu%Q]8c&`WoT"
		"BLsG-^3rYS:Lnou)ft*[xjK##TE31#K?]nL@$`H'%&6FG&bs^)GL5buar*;d$Yt[Oe5a^#^L`Ca#.j'&jWCG)[jl'Q#NelfU+EU%GEr9JUp,qT;I$29vn;w$'i/mfK7kT[-]GW-bf_kM"
		"<bH29PXXs6SS>c4Ff*F34t@X-6*SX-(^d;%Q%k<9*wGipJ?_+;=ZbrH^sC.qm3cRG-IRfUNJF=u$IaCa(xJ]XKtD'M[G5&#^M:pLY`8%#)swB&Ew2u&vWe1EwRuE.jQYJ(pYc8.wNeo@"
		"GgmT/I@H>#F*nO(/?K/)H2pA$N?hHLI:9U#7;fP#SNd7rc/;20:B[cDTO:P]K@v-$,p)`#,9`G#x^lS.<:O7eVW7F1a'89$9W5+#5lhC]/`1L&bMYb+3$xs(MwCt6n#Qr.4dD>2C#b.2"
		"IRn,+Apno@Mq2%l*H;8]n^T)&x=>(+s>,W-.E#V)#aI1(Dlxp]S224'X[W,/8uu(OE3N)+`)Zl0CV_W-V`-[-6>+p@^=M/Cn[HXJKMGf*Q&rT%gfO2?A7YQe'vFrqbX$mDCh#)^sX<i<"
		"/2`d_8*0g(JbWs7#XFp=mkJ^?ODfU9FG.C835&&QSYwjt%d'##x(sY5[5S9@D$sMie6t?'oBARLU98d;<EN%$(Xrsuf-4U$x,no#&];@$46$(#rH(#QGx[MCE_Mp&knap9VCZ.'=eA68"
		"^w-W&*ZEd&'B;>]@0lJ(FICa[%Iwn]<s.5%%.Wd8Zei*%g;$u$*9lW-5#hsL5V7lJx=kI#%*_^uEmrku>^gsLG`BXu5IHL-3DHL-*i`e.2>w<]&CDL&qQ5*<cSjEGTNUp&1S<-;Sh`]-"
		"Q4h8.0#,G4Ra>=$NFpOfQGCf[G(le<lY6hGn535$uLNhu'88m$nGjV#i;uw$;i<rL4)<$#G1Y[$n0fT.*@@r[nGHI%XgCa<tj*XA:SNv81_uGMtRia/$*x9.YW'B#_#&^-goWS7Gitsu"
		"uF+fhK`ZC#]fY^tb@ISu(WnSuk@0^$#d'##pl(C&$d2MK[:Pj*<J3Z8/t.v&'K(<-%4q`$2pu,2qB'Qf>O^>]_D/>GO#)97S#s@]TLJYG&1CvuT/Jx]RGKkL>1W2Q?_>v]4Kc<MDYW+&"
		"wGf#5DctM(`,=-rf4JErj.>X#'dCC$EGsD#k?6M&Ei`N9,mN:I*AQV[/1gR/,dssuV](#PVT,N0GKb&#P7KQ$'(N&FcU1?%H'xbmh/Dj$NI5a3XRG)4mc3pu^*j@XP1woI$muoIBk(M3"
		",-d1BXr^##bbLS.'.OiK>mch2P6,Q/wR+K%49opf1>9g)duhT.2@@r[lY/N%GLKB=P^`8&QpW>&sqGb*dqE6/@#Hm,>4?kLsZ/1(]f^I*Yb'C4VlAL(CZYp.ql:^#HA_FE'>*CR;_V,>"
		";Vu$7h+Oo_,;*q==:hJ<]L><$gC`U1$,Wej]ME4Ei%&Q0bVaM'f5Z]4dPwou<XlReK*%tu(2Qu>;j0A#RTPS^0PMu5rN[0WMj=VdEtD>#iu(`aa:xY5[[i;-49s>&Sk>)'&FS<-UGpv&"
		"]M^eb80i9rRZnQR:rthg<<;%O$M=%Obuso7;XmX00?uu#C@<)#$gjH=*gMs%?9J1CbSsD[Qs.j%eLwo%QiXrJ(A]e*JGrp.tf_;.^*2oAJ6(<-+2pb4Keexkh8sKk-OckE4l;X#@u36d"
		"$cP1^k>.]#ZaNb.ZZqr$?uc^<xue;o/=-Y)2xxp/9W74rfRTZtN?,S.bxvtc<cFB#a](`ajOi'&JLr+;<JUm8R>h#$xgl2$Io<Fc]/1E</tIAG]`oM_w?EF`K8*NMNKNB/D3=&#?KG)M"
		"`9E$#F=K>]3_#l$VlEpIF()MObdkl&2r+@5_#/X[x@br&s[Q.'[ep5A/QXs6d5DD3AS'f)f4:W-VF3]-UND.30]NT/9Rk&NX/b;c]6pUdBa^p?Holc#x^jRFDS/v#B.,rmoLmiW#pUf:"
		"2)-Z$Ap9`0=(V$#ZTD@]Ia&S8^0x5'gIr/(GsGc%tqqjB<fnw[C?7*Nh-4W<4J9:`Zh-dM0'TfL-1X/rVJIEr&l7oa-m6ct]gn+OeN6##bH:;$_MwI/6>w<]3*%.MA-^G&qp*HF30.EN"
		"w,^G&H5V_ZI1v*8X[gXUgYRFRNXj%=Q4ESn@07;-d`/B(MO&0:p./NippR8;k[M8]141hLA-Ca[=Iwn]Eq,c,84x2$+,Wt(qV$#$?'Qd&;;%a+g6TvGIjM8]CH%iL6pMN$dEo[#8;SC#"
		".FE:.L/'J3HJm;-]i)q%<n0T%d05>>jco(e)'`P&1,g`JbX,5Ikh=$'##Z]=>F_]OEHNB[ZTn8u^b>%Onb5&Ags81Y7*(##b;g.v&xGlL?g/%#%;uB]Ov&f-M_Xo&eZgT.0@@r[02q$J"
		"$Do-)j(kA#@Zhpf^5kY#w$6E#jT@a*3kE<-usgn&MtrR8vEUB#9n-H3.)gF*aV`5/L0x9.+Gf`*^8RT%R#/bek9h8erG>R&rI>MSSHb'*,s@X#<b]$O-PD/Pv`6vs,`.W5Li/M#7k>V6"
		"8lpbit*6H2Ld0'#`<)S$krC$#Mb,?]sH]f$qMkT[d^F,%x1VK:a6$Z$f:,Z$ljU8]dlRfL@UP#)#MN6&X-fv#BID=-aAiT&acvM0*Vg/).W8f38QPD<U$AZ5m/xOnELpNEK'A_CGdCo?"
		"sm6ctxu&iux<`a0)c_5b_Om'9bQ]x?$,GuuLZ(($x7###-`UC]UJ4jL[n.S[_fld$-fQ<-N;,7%18gTD4dD>2G?);[x47#$+jZ_,CKi&(C(0Z-vJS$9FRT/2cG?9.I5k=$0HCe('OL5i"
		"]mg5#$.2ofd;_6AC$Qg)k*M/)]>=O%5v8+43?go%%g[)4)a0i)hF_fuH6gQ#8n]TX/9(R/D3C?A;N@*Vmm`P&I7tYO>sstulFhEa5&n1pq5:0u'Qx1Zv)D>#e%N1psBE)<fuq4JxJmc*"
		"I)m,D44K&v]t[b%v[Ol'aI*6/Nmx*+7_Ed8b]M8]o^?iL8PMN$?.vY#fG[c?ok(?)EkLk++=-/)?'Qd&PTjJ2J6*5E[0Y_-nZS[#9;d)%N&6)eu@;%t'eRxa_&SP%6:Y=K3E`cuO7cYD"
		"D]`ACN[dY#_1;20=]XaHBjwkuh7YY#A($##WowY5X(pu,Xjq,&NWf;-1_Qx%4V?39Og>[/w[WiK^e'B#I[DM9sI+Jc?=LP8dQvV%';GG'5Z)F3W/(9.57n8%YY+gLcD403>U^:/A*e$t"
		"%</vbw9J7nN,cI[M=LF=5xe[#q]ZPDrLuY5ju>G`'R<o##J4`NZQiW#XPw8[*J[SuW#eO?aDxF#4]7,N`6ur[qgoj(lbJfLsg8ONhDguGd+L^#w^Pm#J&v'FGxGUr$3VK(vS-Q#9_qcM"
		",KG,2L06V'Z&)N$aPcW-3uXpU`pdh24f*F3+q8N(hOMINYr(B#7L+=r#>r<@PpflS6&i9CwG;O#T.W2/$NAS.pu,kkArAU9#->>#hJ+##%5c##`]UoIfuBl%/)&?R4F5gLxvd29NVo'f"
		"tAY@#;Pm0M/*D-MqLAl'dUbR-qmGA'(PO,M,15e9,/1'#qL5nf]2?nAIZ;QLY`,T8[m1aO2ajY^[Q.]%WOC:rRS02^bGi%+(2Ta3)NRuVq*,##G?iK$t73)#tnA*#XHDB)M'wa*rUis-"
		"f=+^:</)d,8n1N(%N_oIixxsu)KvO]FKBp.*@<iT@+Pk`25l-v1G3l$G/4&#T;Ba*_Sfp@w-^KOLH#GI1QG,2b+0a*RMVt-Ek2&F<UiD++M0g-k?MhFRTv)45s]-DM/9tUGh`#5c`Whg"
		"'UE%oLItFD_pOJ#V/Q<BC8jot9t+?X=w+R$=-=rHMG%H$>$xUHNM7H$'),##/4fp$FKUiRU1xN%qnCHM_Vhp&R43g)GWEv$n>bhcDj-2p/3a3doLCD_2#DV9.YFn@rL$E$ltv-bPP^4k"
		"A<APuk.$lEWRGd2[lY=lHftV-Sdcf(Zs&>GusTpTo@A1(6(AT..@@r[oO(P%u;gs->U;N9lM,P3WvYm#=k$_#is1/(af>j[b0Kv$Ov+49=<ZV[as1Y(grq5/k;oO(w&pS8SaTp7DPDku"
		"Or&lm0`r+)vK>X#gjBIpIi@*?*?6rdQtiiZ9VrY5WUH_uIAicuepYG>:3>>#TQlLpmMH>c,B<YPT3TPsIJ=jLE8Nd*?BFEN=-[A%>R)29&H0jLLCGm'nc%6#x2,kk/4f9]`'/>Gtie&,"
		"/=$Z$3#ggL_=tmfO.cvA4S9nf]uCw.p<[s*3PuJ2c?^YAbY@D*T'7a$@H(6/*s08._'3/:b.r;?e,*6Svu+]q`ds$uN>RrVp2Fon8egxX[%IW)_/E.hlC9%tlSS5u1n$1a1rQju7@2nO"
		"tpYRl#O#*olFVO#?A)VH.7)##UNW-v)g-O$?Lb&#D(tT.$Bji%gOM<-.qdT-n_eF%[ko;%>R)29$<tiL=75m'&7Ma'RPH;BNM=T'6Nno@UD>j[T4)]&eADN'K^Xv-InIw>Y@ZV[,N`p&"
		"(aik'SMlm'(FVofMCF6hJT*r7WdB#$GlHT.#0Tv-CZ#t$uN2X)hIu;b9<>m/A1[lJ/`w2KJeSC%+IPpus]A<rjXlLks*.*B*F28ZJo[k<H8cdu=RSA$`5YY#?x###Jj[S[<nJM'+5v;-"
		"N(xQ-nZjR%`gSKC=Qf5'gO_?^=x80O:-VHMw$NE%pklV-$CZV-K_;v#%Ub7HYqFu#wi=##QPP=lwr:P#Qx'5MXD]S[.&FGD::n?TQA###>%4nWa*,/(w:qOi3Yqg-Mw*@TM%<9.eQm&("
		"8FY;.pto-$8WD<%+?4gb5:aX#bEbuYpj#jsfex@#QBcDMHItY5Vp^xbHoR9C'=l:ZZT'H2A8.,)arQlJ4S5a*X:%b*Jr;39,ZQV[q1?HM;IuZ%a=M;$$?L<-9(Jr%-b>j[MM8a'AoNIM"
		"vjdh2.3O9`8+M8./;#s-nP4ID4J$9.Fx]xXd=mi0p%#YudsSgluQo[t>6k9u0/fH#4(.,Dfc5%=]u2N0$&;Y>9SYI_KVRfLjKe3#E?k=$/Gk3.EWRNOb4PD%X+?\?R(1Pn-n+qTp+60X["
		":(@O3:POmf]Y>j[N.1Z)5hYnf-#`-MNngk.HDeC#t[kn-#>*Ug/%gG3Uw)?#>'7Rn2e6CE^CuPD7iC?rT-C#tfex@#]'Yit7IsJ%>4.W-I;Hv[Lv-tLO2Ac$Cme%#nMfA]7(cM(:?Fp+"
		"__,b*[wma*gUoWQ:uKbE;ND8]fYV3X%6(,2Gq501mEZ&(`u1/(pIvJ]YB]>-';Yc';f^F*9Kt6/AJw;%GCg;.*-p*%,f#G4hKkA#+D@5JlYxxt>>KVD4#0-QF9l:$-(VZu:,$Vuu53G2"
		",`Yci4,KM#6VE]&/.XS@J`)VQNuo[jl;udmN4`Y#PV'##vvuY5/Av6i':#gLZOZ>>ZkXw7uct(2=wFJ(#p)N$0g/mfAb*X?_V?(@mWWp+Ct$Z$A$4H3QS>c48ATBoakY)4uOQdNqG`hL"
		"v-QlLb?lou[>&tu@_xn#>hCdGtr1e1Vb]>#jtE+i/RYH'#Q]o..PEf]tglf#0jRH#nEUs_]Dxr-cVB`2$nQS%7Zf5#gKb&#X-QV.8Mm-.nRHs*LwDjBiO03Ec%(-']KPE<[(J*+8.n8+"
		"'B;>]_3.ktKMI[[n+],&m4gmfBrjT[-94W*Me#gM&xKQAXs#[-l6F]-Fl*F3E36B$ig=-(4oQX-%*C`uk*u:$3wfI$#9r'8^eS]$Ow3/Hp?0pn]qblAni/iTR1<lJw>>oIG3M^4/F4m#"
		"m`:guG9ED#%7YY#^((##4WvY5FGl;-B&HX$^GBQ(mMn([eU/I2KgAQ(]t['&`e$eMR+Xk'FUNu7DJ(0*YocoRf7L._^:s%X]K6Gr<E`=axV25A#;x),LhxfR8[eD$/nTB#1CBN(x4Z^-"
		"9J;^unnBJ%(f0'#X^TF]AernBP$Vp/&/iA1[&(T.$@@r[p9]EdQV#D%B>(<-ObAn&,NvJ(=Z0qJA7^e*cCCB#KXAmLc8k)B7nb^#BUJM&(nN9.kv'w-c<Bb*mK$*3he$C#n'n3()l?j["
		"?x#'(mfkjL#`=31,u^H3Zq1Z2`:SC#m>:Z-Z.X@$Ynn8%;Q8A$/p@d)&@n5/0gfF4OA#G4&p_;.YDD#$9KKS%fb)5S^T2rm(WWW#q=,J#e(InnieFQ=S[ao[Xkn1OZTD4O]W]GRuT6jS"
		"6AAQ$*sh*dhY#PWlsuXc3L^NE3[^m^bhOi#P?>6rPrWq*;ZHg1WPH_uRBG+%gd0'#*HX)8_mr/<?oI.2Hg8r7BG0j(>v^j(%0Z][n'&ec3b_j(iGxJ0Ol#02Uj_v.n680u=Oj#7a%k)'"
		":uh;-^sdU&n9AX-;jw51F0:t%1DXI)cs8I$feV<-qk=w$5a%90cZ[EH9nlUmg[SwsWOY?s9-DHQ:H_j#fr0fE%#rqQ$ja-RtEhMS3)Wp#`gUQ_s5H@IxE5GSOHTNE2k5R^D+fL#FX`MY"
		"4e'b&%kPD*0>uu#pFN1p_t&#cJ#@D*RnPfCocN>-oZ;?$qL5nf&Cg;-.lT%%OC<0:Zi7p&<HG9.O;9m&,hEpJ/V]e*Px)fuv@81(ZcCW/Bk;c>3M`>$pLur-xVY8/`BRF4R(C+*J/P1:"
		"l&.Cu'F&h%I=?m]wBAH&([e/_rwRw*[%Ak04Iq&j`X<7eW'nKoWQEI<Q/wV6/Aii'Vl8>,A[=N$oO#gLX;iHM5:rweK-1p&>d@Z>ER;8]tTi*1:Iwn]?L0f+Hp*?)>D^Bo.1kp.wUXb3"
		"Lp8l`aK/@#[hR/)/$:N9j8TlRsSPR&sL52SIjVW#rr?=(a3qHM9n?=(f6-eMAUnNu)1CMo/.v[OThH>#'+?5Ougc%kSWH'S$),##7G:;$b`0%Mh'8<]O$^7&CKov&KaXb+pH7xGKHLB#"
		"bYRh(+5Ba*>YHt-])NhLul%I*ZEo8%jhYV-h4P`+TnSEJj]@%>,X.Wr>w82'<dWiFbrVcS%qklJj;aS[nI]>-kuZd*1SIJ(DVpnEl1$Z$m@13BnlGa7Sh`V$3[H&%MdEX&Ygdh2U;p_/"
		"]f^I*@GUv-tTEZ5O5V`)1Y[Yf[;<o&XdI8'AVWpuwY;AFP[b(as,Uba1Vm&uD:3)#/dI)vgIBu$e@%%#Roj=]TFhgM]J.T%Hh;6ALXvv$mnR0(mCTe$DS5cNB2.W*k$'a*okZgLiBrQ4"
		"OaHd)LS)'#l#b$#-a0i)r?8R/'pXjLiESs$xL0+*-hXU%fA^+42QVFrgCkru)-lf(k_BW#)o7bIn`?UM`0i;L]C_CjOgvPK9OguGB/EG#;C2E.)Jf]u3#24%lAIpAnO1kfZnn4/Rs5,s"
		"rapK/T*pI$F[[#MDxu&#WRN5/o+xQ-R>A/&rSE<?Oo)I-^q%3(m4gmfO'b<-`@=R-/B&N/pujh2V,&IMQ5DhL9<>GM`x-&4M1euNgvmk%dKo$aY(ftk54N9kx1L%@+7mW</0/7_(qUU#"
		"DHN1FM`X<k>bD(j7aI+-'qUqu6pKdS-_xoABSO,S4QdJD&5>##H=+6%uaa)#5>N)#,&WH]gML3MXG74(_`>j[feF59&]N&=IRC[I-CZt&ETXb+<Z*jKh1ZV[?A`.(cd49.gerg([7B<."
		"s<41(Q#]&Mdh$T4`(j12,u?j[]ia&1N7mR(1VjA1<jbqf=xg9]*3dh1)jqG8p7nsf$r@L2J-xe*Qfn/1?]d8/q#fF4>gQv$=bHA%YYoO(`^D.3)VFb3?ibe)C]R_#=rte)8vjI3w=Q>#"
		"N,tI3Ynn8%3n8&rv=^5mftuLf1=tjfYq5[AKLNX#4ro'&g2nb-FMc[2h);_osJnrF:Eq&$n-s]$U?BUu1B9+V;$Kq#AA/oetSXLp+geEIuDc4J&1%>LGftgP]6RF.FF4:(+7oE<l7[O="
		"a1[n2f,$##E7_+v@Opm$#uL*%[cs,&dfCp.'1;m5,xPe$JQJpK@@on&HYJa3K$g5#)8(,2J4kT[bqt,&dMRh(G.=24Jv?j[hf6#'&MtAHLmn4(A_@j[&g6#'rM^Fc>Au`%3M0+*KG>c4"
		"I_AW-CftAH%o`b5;Bh.hdA#tBq]mQ$aJo?@i#Q=AqD25Ags#A@L'Da=[3Kh)$@s2/Ik%fuAH7>Y`q*j9FGVnee1v1lf#_oelA,#K&T8/G8_H>EP:gg1aBRF.FC4:(*.S*<Y<%(8xmd_#"
		"Be4loX.8K(h*.5/YZu/%J.e;-Fg@/&_;*w><A6?$`V9T%Q$:0%BBBaYv1N0(k.?j[)r5^(+m<iLi1lT[6#Hm,Gn:R*K^?j[N/rG/Os,02D-+T^K`k/M.CuJ.q?@V-qR3]->BF:.sR'H)"
		".;/&$uV&]XWo._+&d$BJ$b^Y#]P(W-P0$XqI1YXqYFH^?>J'e:OdPV'KC4/2^eJG<GAN8]Ff@@>?,=Lu<K?j[B89V-U]V$[_&QJ(ZM(GN&-oN)=Ai0$gwmO]BFve+9]I^J(@?C#kFg=P"
		"S^G>c9ZK]=#4%w'pCba*7GE<-'lP<-v6iD(j/kT[hn'H&faT<-G^`e&c:SC#(<('#Z3<l(E)H&#7n7X-;5.T%KcE%$'ms5v^1L>#t0>>#r>^@k8U,SOO%BM#Gcdi94jW9[,cQju,D>5/"
		"WfdLpasHb.RHicuRb4U$dX6iL/;mOK-tg#A<b<M-#2cr*,1WgLN_[&K.qk(0o.9f3JjE.3,WFso/W4J*xp5BrU:kPji2TrFG@KGG-OKxSm3v(EDkocjbkY@OnX^D@uIWo+-g;##<`X.#"
		"k(;g$L.LhLY+9a'VHts-L:woLhOf8%,Vg/)_<ZGM(+F%$[l.K:Mg[V-SjEI^>V=xk[)BvdK14F#<p.Mm?W-buttHOZeJa@b4_-AL9QX*ZCSMM9dCg%LwhJxt0ZV0v`)#>#;4i$#a5f9]"
		",d8q'KaTh4ceE]-u-s?uDRiY#R*Lj$]e'MpnVK:#1wI%HWEw@FM?gi'W;/S[kl%K%0ch,2FQVmLt%<0(YP>j[h:=d&%TaaNnDS]%QLue)(:b=.t@q8.;I9Y.ofi#OPl3ul#(esIWM@d)"
		"EDoHD#YWL#cCb04c)V)*P]<mA4Ijl&Y;1,2ZS>j[su8_8r6_=%f26lJprHG=v%$CS.P@>#uXZw0=Y2x0gc$793uTw08F>V/bje_sp4Ec`HJw)GFQVS%['X=cC]F8[Mc<G+*#r%4``5D<"
		"?Mk(ENw1S[Fr9p==$=62svrN%LHEa*Pr[N09aoA(OkP6(snXj9PFDH*^j%8(%oTe$04B/OU$<pfJ5nT[MM,W-2_]7*R/@j[^k*K7;L.kX7va.3m$&9.m('J3BdK/)P@9v-5`_m%ot]:/"
		"fK*$Px$)*NBjg)O[#_=7eqm4al]gL2eFr@kuGLaH$iu31r^1&AG[p14(=mReDkc'&+6*I3vW3%0Z`=CHQ'Q]u'&P?-g(^,MGE*I3-T6PDE)Lb4tT(loGx.I>jxd'&i8r4DEvke3+sF@t"
		"P9m'&X6l%N($Rl1Qb/r2KcYA>ScpY%S)TAt)-xH#&4.8#0Yu##D@%%#X'+&#md0'#$u<j-HfE.2qY/X[)wEp+m%MT8rlFo1sOa`#w^Pm#QTjP2t-Fx#.%eP/n-[>#DU&r.4Ll)40l[u."
		"Gx?8%x=r#$B$s1)Up9:%+j/Y&)O#b5ib%T%6N75'm;)]XK%HZu5cR)$XtNS7?5k9u.G?%bf;em0&k.ac,<'URu]tS'2O,2B.kn?#j82kuuTLZB-g:I$u&]A(r;b7uLd?B9*Hvr#Y;Dau"
		"16vr#&gvE#*'cSL_:K-#Pqn%#07Qt(erNQJdlocPcF5^(KwLe&^jG2'+,<X%,)Tmfbi>j[81fn&A@xu-Pkut?E0@Z$*Kq7@-FZZ$9OjG]8;-pLWbZY#c7(##b_cS[.Yo^fIc<N$JXe;-"
		"fCY0&*j1-DeD2-*J/Rh(&.L=?h$C#$C,+b34t@X--dWI)pH3Q/@=9C4/]NT/k7,=6eC4D#7p[J1X[WYHc>/(Ju*$>Ql?nS#P+p%HLNJ-@@&SLI2#Um4<v:l:(*:YuxGR<ClC1s*X&Rm/"
		"XEX&#)[^:.QT9f$LpFk9:Mo->9[:*'gSD<B_Ub5A/UM8]qk4gLF8Ka'*6ZV[n0]#'BGXl;-Fl(NUb)T/(IwG3tE+rRE&1wgK('A$F1dhF^uc@]'1=%P)-7R92w0c?N5bHrg(o^It^8%P"
		"mp;]-n1+F?_3VuP[g#a3A=78%_J?Q#?;=a$MIx/(Odee2JV%a3$9]u#^*7;mF&]S[_iZ-?sJ'##Oe7vGHr0m0H)w?GAchN(_K`cupU/Po=6(k$*)*)#lSBnT5wSJUBAIG)@Ei&('&xKY"
		"[To>e^$bm&h'/<-uAn)'3@h-M02d2$rS.0(k3/(Qa)H/(]WamAfr@Z5epq0(J<--3at%p.6o0N((mQCuOY$s-V1TCs$d@nN7%=Y`oUrK$f]7du1Z;suSo8'`l#FPJ4<vtLIg-##1>28$"
		"=:3)#5,[<]5ERIM'vLdMNLG?elV@Y%9VtLY:4PgLLu;M-/gmx%-o[h)6B`c)JCn8%?29f35ho?TAkZM9Bj-F#N9D8AwYqIk:?n3rhh%sH6/9@kFsBM,s=gaL_Prp$0=r:Q_)dA4&2)K)"
		"cRXvBE_c)'KbbZGr<Fm'T_._S:bf9]0Leo@cq89.C5Yj'Kpj[$o$f0(<JN^2+vbV-*HF:.2%#f%mF+F3)`OF35%DZuM5JluNk@08ro`XuXf8D#GIJPutKot#cSYVuxhLE3i3lu#5->>#"
		"]%(##jUxY5>J@S@D&v&?lC,t&A.,)'c7tX?f_&5sg-Is*,&.t-1;g&=fYuofgbIOX&d*PXi,>=$'x?S#s9Z]i8=Ooq05$b//+nCu?8eS#4l(mu`23`uY7YS#xD4(0^Yqr$&Zf5#tX^@%"
		"v]qg:4G.K<TO>)1'B;>]D0YJ('>Ag%o1I<-cM<`%g+k)'/evU7#KSj0V$vM($LkA#T]BM^I'Zl/2L7M#]hd_uO3=]uekXIujX^;c$=1/CpDp4&ShSZ6&1,#Y.,V9V3M@W87Zb/'$GnpR"
		"Lo06&$N=t%=Uut%*H^5P@mtJ''@U&#s&Zf%GX>sqm@X`Ng-hB#tU6K#<(IS#&;%)$XE_%=xOK@tqElKY78(##dZ`S[ce'DN$*#78hsmYo^LdY#-$A6%hLXfLY/2J#T*V`k3<Xm'Jvr@]"
		"L)]AGPsA['0iEC#-:SYGNGEb.MM`Y>UEZV[m^&g'En@X-H:*&JPX+F3C$V]Ow?R4S<+:Ju#0Dc#T^rF)$NY4JKq]5/QQUV$J&ZlLp6R$AN9`phN)HO-@s$./+=1,2UF>htNG[93=s7l#"
		"`<x<rf4JErbu?]b6_mLGGt0'#joPjubuu)Mb.B%#fo,F*Br+@5usMm86[nE-SSR-F=O$29rAnIOWs>Q#,vAa>YJ%K%33s29d:1U:gb][>Ww&K2#vH79S(LN((K-Z$:+jZ7;Dl1uLf5V$"
		"&&t89r;,)O4O$gL3GJgu$j-O$v_R%#8mm*<$I.eGE,A/P^2+MaZ/$F9RVM8]tK:dMb0hL&w+?+E4nng<w-Qx%1mA@$^7D8%Y9K)ZvG#pDUQvgGImm_#<=^[kEpKM,EK-;Ds'D]4a'SX$"
		"X/5##&4fp$q.%8#%Q?(#O;^F3#u$*.hRHs*FC+K:h(+B,<R/Q,(8<R/)8(,2NI]>,$EAa*RgP<-Q[>s'R5>o8vb;W[eAmD0/mic*-=2k0xU*w$9vL^#qJ=l(lS$Q/<D3F3oHMN(l1)I3"
		"8)Oe).5WF3f_^F*J[FU%RHUv-qo+,iX76#_]sWTYh2`:PAk2L]NS`IpNqYE`w`>uPD0t'BCmdm)bsqP#=Q?M[drg?hSP:?iDnT8e%56Dqn*&`a8j6ou4il[#4CcE:=:oJh$P*vV1As1B"
		"2uWVdBBnr-3K[xO9T>L2i+>p@gk.T&I&rV(>d7iLeI<c<6#G&#;[1'#'B;>]/nMr7#ZrS&liLHD(&l&#a%[#$fiv20asT,/$ht1(%L4Y/Xddh2_[[5/Hm7f3)F^[>n#pG3wJtbN:=vg)"
		"25WF30(^C4r,k5A?;Rv$l)e%`RhHfm((]qYrh>BK#Y]gRF#Eq;VamTI't3.35NtC5ljE;$B^$m&W]$aJ7PKp#'Lc8`S2sMIHT3@#-21uB;9OON]nA>#FXJ(WAMbS[,q3D<$9jQ8E@,wJ"
		"M_4<%m_eT%ZJ`C=/PHZ$nt[0(*hhT%Y0N^2VxF)46otM(D$nO(8D0N('t&iu/M:nuCs-0uw]_utUY^;cr&)S$eTB&vxu(^u&bJA$6k*J-/oX.?8;4m'SV$9%kMtg3#<@lfCi-]KepjT["
		"^If/%Eo4RNqcsQN5IWRN[Mgt#7'nuGh7$$$vPSRuQ`i1b/W]0#dTVo$AaR%#nevP81u[@>edLZ%nkh-'TUj20>3M$#0?5Y[Y;2[KVjK.;)/hN(V<SC#xbK+*fZwe-mS1I$+*ct$h3ae$"
		"&N0@#'<?.4[%L:CE[(-GeO+Y#Ym4J1w>fZ$+i:%kEbK]XWBC;6m8,pmJ15V#G+,##E?uu#X@<)#=VE=]o[B%%>XtOVtj]0'WhIW-(`VVp38V-2M9L.tX&)N$se)<-2&LK-g?];NZC-$$"
		".a*k2=$]R#=QlQW9D:X#*MQAOD1Z/]$J9o[PAXP&&F4N'_4JfLsw41#hm%p$:6i$#&5=s?EUm9Mbn(QAB95(-Y>Vj$WPn#%Olvs-;771Mr5[@-FwB0%2N2K(N.tE]6Osl&UdIj%ee3xb"
		"rRuru_LOsOqjDr?%/5##('Sp$T(r7#dQk&#vg5r.Jm,t$)W#9.-@@r[J;2d%eZfs-(o9-Me=4/2a4%:&&O>j[b)wG&j2Cx>kHZV[j$S>'wHF&#aU39/b[LF*OX2H*&DwDE-fXI)+2pb4"
		"Gcu>#)U^:/rtVv#&iNpurNVC^N2aS%wa1?#Q4(HeqZ'v2C0gr$lE]dR%xk@#5Ia?6rj9`^o4,#,aRlIU@Q(44'LCEc><f`EDP&:kFXtReJFl]O:O>A+&$[xO;:h;.G*_>-8/Vg:>Mx3k"
		"oB02'svC(,8mC7,o$oo@n.G`Fh/Jx]f+LkL>1W2QR_>v]^txFMQmJk9kIngu;KFhuSl<T^l3@W*#G_5'[R8]_%7Ds-,&qs76bLE4j8+l(+FQ,*#VNT%nYqv-.jZEhvq2x#>c8YD[AC,`"
		".sQr6Ag?/Y+EN/jaq0Pf89NJJ7w@?3+&;)S%a[$+df>B4HOo6J-w'##_,Biu/Jop$?T@%#>hA78Y4O2(AS9a*q&#n8G>wo'uL9K%L2Rh(;U(K%uT56#C:DZ#b#KmflAf?7`J%K%/;gbN"
		"<jdh2@&sMO5/l/*Hn`$#IIlM(mclA#h$^XuIc=Q#a,&snjEYIq#+wo%'7]lu8rbIugJJPu$GMGuNn]Fie/&tu*cT]FOc^##pOF^PT`oP8NMhQa_-2E'mPZq'fE4$'[lHBF>j#K)6-#Ra"
		"D0gQaB1]b</d;Z80`M8];ZF-;akP,*2&R.%s*^:/E2xh%p-^M[]Sl1H>&:>PCu(*rgc8E#2;G<0%]Xp_he,&:HS,(&I3+W-WFtKY5F*G;F^[S[V##;HSaT:@xASSq`v-q&4lPa*X[#6/"
		")B;>]g9$IMAsE$otFi9jYjGm#D;;BFLZgJ)_o$5&-IuM(E^9PSbPYCa]3pS'TajJuWudS[$).2K`i]mcnW]g_S;5lSUuHp.%s>p%J)P9`qWQd&K^IZ'Ts5O-;#i-/ct1c#0qj'^7@R%^"
		"%Te%X$jfTX.9(R/5$2S@T43`uwHlqt/i*`(]1iR#gJHA1`5YY#ai2lof3'#csaoIc&)X?%IKXb+*tBt6Y>iZGb&o5'[S.;?K-V@G%:f5'P:8WBc)eh2UsGJ(R:M;$AZIa*F5''4oF>c4"
		".M0+*-mRS71,-ku7=?%kos9uH4IXs6e<8203l]?u,b,#ce84i^hC']b7=alA-%*1lx43[cs$)=u?PCM_9.>>#k13lo(]^S[T<.DEpJ7A,xjYi;Hr9#]+q%v&Th.Z#MY6=.SviZ#Q:rpJ"
		"6l]e*]fuJ]<1)X-niBX8tTEp+C;Go8JHqWJh,g@##DXI)4c(gGGN@+4oPq;.XsoXuKLgM#L&r=j)HDvO_Xx_N/`4AT*H3;m7/B`<YpWRcb)4@M<.M<Cnf2>Mjnscu<PcB$og(P-A$)P-"
		":Z8c$A9Xb+0TZu6PAYvI[5;N$26J;N,5n5c'wROrLT%#Px.AFe&M$joVR`8r(19aR9[:/$NlWju)Xf]aNr+OaaD'=LRaS-ZWT^`3PM/&OT935/&&j+#)uq@kM588%`o)&4#)P:vC<Ef:"
		"s*U?gb9uOoJ/HP/KQBfUAw$.-RA5uuRlIfLS@5,Ml1hT&23O`<Zw8+*ornPJ(U]^duBem%ID(v#C^g;-?8#9qVWg5#W/<k0DNAX-rZv)4_:4D#9]4K1iL0+*mEwKu3+c^pPI2Ro/V^Fi"
		"7lK(J.mvc)cQ$;h#Ds:$XK/s$(c[cQFksWUZ^&##O)$XU-APV-=b9K%9)-T.Y[d8+%fLF?**Bo%Yil##0?5Y[.^1['8W<eMG($of;*EE,xTe8++Xrofs]Cv->r2M-T5s#2OS>c4/OHH3"
		"]q[s$22#HNwOTYGw#>`MiXh'u>N0c$Y37froqM;$?xX4]]a@HQSV?.hG=(Z#PtN]#O^/ki5*iG9nTkrd*:aIHY&XfL-IB)vAmJ*%Z3=&#//cB]QQ7O(NL-[Koq>_fBEXb+&79oi<(Sn&"
		"$4U:@AGL#$vX3B-]oQ1'A%KU)@/tIMSsc2$Sw*t-n^Bb*riQ<-s&VK%:eHIMN#IH3fd5N'tZnP/ql:^#?H5<.m4SNid:gZ/#1oXWx%3)<bb3I$lDQ^**N)2K<xK:vje_`_k#`rZ68juK"
		"<L+A9:1i?#2T%YI^J#`<@:Wi^f83n?VtC7tn)Wd]_w#V#m)$.f#>rx8aD.;6IY$vlWT/20]h3K.:I>4i=EZ'vSoho%$v@t%M3o;-w[/g%lF^a*q$f<-AZOv-PguA:%KiD+#8*>.x_t'4"
		"AT$N0[QR,*WpB:%?TWbN)aIIM+Tf5//U^:/iQgIRD.&tu)YDKtw>[CsC^[J3r#F9rmiI[+x/-5J8VYF]go]e#mpt%F9%;T#e-OGCX.e(<_%;M^i4(eBMr0rsTx%lYNiL0#;6o02_8OYu"
		"0nMS.ETi&(&^LlkG=+/2ScLP9gdPb><i*T^_S7K%GcWEe3g5r.,HvV[/CDZ)1dKW-eDU:@+IKv-N>u?0x*'u.4bc1%F>Xv$I=K'fbl2T/K29f3N,m29+&J1(ZaFx#$.%%@w;0p7VGp@#"
		"m<Q$VU/Y-nnKRKe>HLG)>1n]_pd-d#s&uMY053Yed$k7MW<MO$X?No#309&;TIU?gS7I+`ju]S[O:=PA(`5W.Bc]&#SGP?..UL*'n#YKl^UCt69@'p%_@fH/f&Y-;amsJ2P_N9`PT6Z-"
		"_cDv[G;0i)lk2Q/?O'>.s90T%q9m4fwJ+logYb>#Em0v#h$goe4;Mb.C2PL6qn(g#c2muGEIm4$fbE(D<W*CNWd#C;k2#W:QU3LGs#kJ/DDFM$U7;MMFgC)N]LTCO-D?I%&.Rm%lYo#,"
		"Fm<aPU'9@'V]>>#ZIA&,Y3@HM%J8+4#W4jLaF^I*jpB:%H85@PSw.>.77weg(@pXl,Bhxlfet+]Ix:I>aupx$e;=]bW4ImP<4OCCRb,U<I_e>ZMr5k;L7'c,Lbc4#+d298@>@jK#PE-d"
		"Z(&6#9=@u?>:Fw7KU=/2t'1m'.0Ks*9)U<-lkZ.'5dlW-onQ&Qq9@W*URdTia:SC#KJlY#3@Y)4/dk1)lc1b?dl+G4[m199Ck=$.4q]8%TZ(P'%I>w:=:st's4DV;T=nH$&]4=,%^`iK"
		"qe#oc?^O=lA*M.h(_N5$)t-#,iTjb`6xMpX;xG*d]]tbMPMnWh$62lomWuY5QR9MB^Mc29[u5UpE&]iLjU9GNGnaMKQ4ZV[]FxQ-09Su0d4./;XeM8]A?[aNq$oQ-mxnQ-+G0k.`^D.3"
		"Lu]H90(m29@G.$$DLOF)3xxo=F1kR:%`v=li)JB?(6kS7YO*%$0</GK]J/v#0kPju%=krdrpLimrcg.1j*qBTNP(x[Ir//KG/7wgxesRngd`S[Pb5JC0,)5Kn?5RC@^.iL<UdCFvak0l"
		"EIl3+8)`$'@qM29YXM8]qEqhL*ds5,*6ZV[xYXw9_mdh29?/495]dv$rX3]-&r+T%u$b5&HL0YKdiC/so*wY#xon:`q$J+MHJMMK<rhb`:P?P]cANn?TbcUs3F]qbYX.<;sIF&#iss<I"
		"j8)T@(%ux+j_Zp+*DQGcMcgq'8%Q<--0,O-?8MP-AlP<-C(2t-2#Fd;s;`pfjYKZ#'[.?%EfvJ:gAZV[<h`Y&8T*IM:@B[NOVnO(Gs-U%g]:v#+G.4rh::>rriK<A$3kS7V47_#1EJcK"
		"lBE@#'ElB$KlSE#_m'F=o3uhQj/rMa.r//K30QtL3:9)v)sQk$`AO&#?rqr-QYutreIZE(UW9e;A=B+E3];8]=/..%AE,W.<+Fb3XF4(7KNwq7cdt877aGluGxCMTt^Tokjpw6W?WC%V"
		"*3?v#3J>0)=#DBoC/j`*;]E9[9Y8h?p_X``oVa,o``t%SRlOD?@(j.#4_*C>O(*8n>tUY,-$*?)Kt7@^DcRn&rCRj9C]M8]tNLhL4Yl>)]7gK1AS>c4s>K.*Y@h8..([62@)Gr$6Xue)"
		"TJrr?$W<,)2XM;$=`aruMoa4R&2OnB@T)cR`Agp969.P=QOhc2x:lPt>n^n5@GUPT5jK@11q@p$3Yf5#<>w<]EY5=T?FHB?#qY,'YPCo%o0bQ/YP>j[k/o;*hh+''l9x[#Jbn8.Z0fX-"
		"Zg]m85hDw$UONul:*:/L32+^mGOQfrotvY#wc[:`-_iulCG%`N;l;M^i=CeBMuB7t1F]qbaj$q8,o88e#MAD*.]7d*[#4G',VI9.%0Z][:;2g)4;2g)Ee2E3V,$?$ZZWhumojhureB<A"
		"#3kS7UJ#A#03hnu:O:8ID:WIqc^@GMFQJ'KC'v%r[B*1#LZ,OLi]kuul5kl/p^0H/K2nqVPvXb++RB*&w50<-E-(t%5#nR&$e/qJ?1^e*oc4B#mx*6M_1c0&W']v#dTX;./%HW.)25a*"
		"<2?9.3`F^,`eK&4^^D.3+WkL)q#t>-+LBu..W8f3p?i6;LXeP)Z#G:.wcViKTBl*[UikXua.&tuiR-tH,L?C#Je(xO'l5HQduKR^%6fO#w2XWL,3iwD'+j:?#`@HcEOkKItfouB6&&/#"
		"7xk+DpdMfhZ/^V$K>W]+)eKQ/VG>j[e:=d&W*:T%l:SC#,W5x#:n.i)(@n5/^=E2G:^$duNSXG;lYvlSxY9[uD)9YA2.F$@7+P:vvV,p%IlWoe:$GJ(J8Z-246qF$FU6(#8KQ2l(w=j7"
		"pKxMLjB62ge90gMui$RMd&h>$S_.2TJ.HP/hK@>#(c-s$hH.##K;'M^7F@[#P/wo@36.Q/#I;BLX4<GVn7dL2U),##VObA#/)f%#Prl7]*rgp7h(1'#uKp5#B='pCid0wg+xn8%r`oF4"
		"F%>oJX;'U#M6?f;]M;QO-4QfLqCb@$:i^'%oBXB-enWB--xm^6Oim5#r2=1)BGg+40h'u$W+W)$g%K##%b$vU2,M_8QGB+iSuIrdw3+d*cBes%lvY2'%9b)NJkSs$O`))$e4odu]JfaR"
		"kA=vfI97I->:7I-s@7I-ujBD&]Dg#5m/_:%/aS+4Kl%s?]h/(%-r'8N=q1BDX*>>#aX'^#3*f%#kXD%&+V(?#?@M,##)>>#jAYY#4A;A.p-f='?5@T.BL`,#%/5##osHo##(5>#>:r$#"
		"k+=-)>r+@5J4kT[T%5S[D?50.;ZSvNKHo8%-O9E#'0E&L/d_c4%,aH?hmvdu[-aXU1@ueGPoXbPJT)'C'Rgg/hG:;$LXf5#]h5s)=,r>-N7R#'FYZ&MFde0[U[+F3xX#Z,Hvwsuotd7r"
		"hQaYG$BbYGc?Ufu%/[a.86XE$%`CH-G0NP%t]XQA$w0i:n8U^#w^Pm#ZAf3%`3wa*9R*<-$Q#T%jr[0(TZLN0_`>j[qW;^(n6Ba*q;?Q/D*x9.VF3]-3wxa[9pc'&Z(m1M5FLM#pOs-$"
		"3N[^u$Khfu=ssQbx-e@MrtVRu_[F3bBV2s$Hk2DjFg$)*K`:k'*]*)#c5Jt[%t7e*A;RW-;h^OT1V4K1[wl;C7VU8DZd5v##dP&J+UxAII@AuuXj&.v7+1J#I^''#8,###=C7@#9tXR*"
		"OGYdN]xTgMTH4v-0Yj+Mpn6b#PT0^#I7>##Gb@;$wAS<:Q:s20SO;8.XCZV-9l1p%C[qr$Z]M1pSM)#c[I18.)pU`3^Sp(<b@j)0+<a-61eH3(*_vAM=&xIMHi0KMWndLMfCrsf'ep/2"
		"_p,K:5a%D?vG[lf#<V8]l9_-Mx;+mff8/X[YM8a'77R-2P2ow#@vp8.:5)N->/E8]_5J^HW&d2$LB++O?3`3(8C@j[U_nJ7WVc2(ihW4+xoK6]2PpgLlC5f*1^v;-<ac(&9*WcDX?NX("
		"P4xC#fwEx#V9b5/*/rv-$km]O1tH]OfYQ$$tWFCS1O[8NIft_S4njPO<`R%Y5.k1Z2mNP#7^auc4F:%7#<ptJk>v=NdSUbt<eO#71VD9;bg-hY6VN4PZ9:SFqj>.$x_ElYuVgKYoRRx+"
		"*l-;:I'%'?SGAU9a(Pn:3ECW-U2SE>P,&U9p8j*#;LH`E8HPD3+Xwr$96.S[bJZm#fY_5MwqIv4:s$&4?79;6M[V=lC0Kl#X+LJ#Dvk(NR.6&>n*'a+ZHd&#.SWPJJiJ&$r2=1)^O:8."
		"6v+Y#vQ9uXkq=>YYX)[u/:TM#+S4S2l%U+$lt0'#/Yu##_#/X[QiZX.DpID*]V6T-Svts3#)>>#E^>j#=Eo1$Y=':#dJ^2#$R3r$A,-9.cujh2mv6hD_.UuuG2mi'3U5H&Krw,;gekl;"
		";5D-2f4O-)Tnr;-.lP<-11M9.<ZJ2'nLg#$Nw,6A9Y5L2MD]C4C/8L(K#6<.**g*%,N=;-XM-v#$QqvBmSJ;ZB)9rd]PX(jP?#igOQaQj[B&##mFe]b.H<YPF<A60Z&,<-C5/F-`;fN%"
		"-^_$'HlBt6]`/w$Wd'W.2Vg9].kV^%vANlfWqAb*/2RQ/_`>j[q`oA(T62=.Lag/2+>X#-FJ`#$7J`)Oc-;hL#/lT[%94W*+@2u&##22:jH^Z-w_:W-CJ>f*sKcs-r%-f*dA4U%_<xX-"
		"(gI1gvqq._ZrpY=s0l%XQ4haSfj&5Sv4V[A?3#?#-Qjl$/Gb.hN5Q%emkmC@KvoGQkhp4PCf-LQLBP##JYqr$(Yf5#^3=&#m^''#h7sH]Z=(kL4kwG&)L`ofBusY-HLP8/Ke:p/)T'9V"
		"^vvq&HWXb+Twd9g+RGgL0RGgL`AVj$j>?K+&ZIlfb8f9]<vdo@,[oM`Z8Dj$w>N&OMPFjL3X?j[Epv6*:@MofX@c<1d.j2$(FVofxU?j[5S-B6#()of'f?j[neb,&.k7pfVXkT[)vjh2"
		":#G:.=:eM0$iYV-60x9.C@.lL6f*F3]5gG33'901P:@<.(J)B#OAM8IO[15/>2Rou$[)'M`-7Y#Wh>ftr9?;r-UbjAqvr:2qEBn0$==bS+E@@#q[jfLXgtA#$X[@#-]thS%_@iK6JX-9"
		"i2KlSPv6Z5LFYY,@L](Wb<+8r%4Y2(SiDT.4@@r[&]^@%@A`$'XYl[P6qa$8:9MhL:5jk'.0Ks*:#-b*l5[O0tabe)0?5Y[CI^2HG@XB,&:Dof%#0X[*36S&R:SC#&hpn&*=Ss$,0x9."
		"H&B.*Vg;hL4p.H##MFb3Q)rD3%;m;/G>8bu@.lIr3F(Y#Evb%+G`OqtFjH6nKGwKuG43;&-P$&4IY.>`e^.p$L_MQ#xP]tuu7eCs))P:vO<P1ps.^S[*)7A4*c68%FIiYGY/>)4%No`="
		"dZ6%-nBIP/'E^f138V`3I5&<3-)jKc;SlT[EV>#MDPbJMIo9KML+UKMPC$LM;)ge*?FOU.>@@r[D-sB%BG`$'Gf7%'Zca&=]Ukm'w_p,&%gFnEYf_:UhRg&(7:'^[kIMgNIwZm8dJCW-"
		"-x?j[8q1Z22U>nfH.kT[H1tD0AZrofk.?j[g&qG/xtR3XS^(/*a%;/(sF?j[#OvA(9WFqf,u?j[5>b;3%ore3V<SC#8VE%$@vv;%:EL#$&e&.$,XpU/i<Uq)IbLk+^Kwb4(k&.$;v(B#"
		"[4b)<0(A8%I#QN#`6)iB2Tb>m7J&9r8WL6R=>kXuv,ugR07iG#AZ@TuQH19oJ7Mh-nL.x_e=:S[%8wC3PgD]XFU[8%$2r:?ZpiNuTVa/M64p,MX9$>Y]-)=MJKF]uUGi^_E6=&#Le60#"
		"=ew<$krC$#]H2@]vkOM('>Ag%VO_a*9)mT.5@@r[Ra@N%rB+9.r>Q][-@w586YF&#P]-t-p$GgLdP,B(#M`/sD>B.**o0N(m>:Z-)+LB#DNAX-EU@lL'h:$%[a0m&.+f+r?/Y7[PZi;r"
		"8eab?`'GipTXFfaOsbQ#3Uv,u=n6ctl'OTu2&fe#9)@P`O+,##t6]T$rvh7#.YLC]IU`?@GO+o&4lHW-o+:hPJ_g/2qrSaYd9QjB(#C3(bi>j[kM8a'CJv0)<U*.2q2rw,&?#x#NAhXZ"
		"*uG,*?Xpg1RLkT[(OvA(*Riof;xblA+Hp;.5G#m8w;cD4A'mZ%RH`hL/W%)$/^[-%-9+]A8@%-vZ9rk%,XUiZ[vBBu@OSiCwe5+&#$pIUD:&j'm)ZVecEbA#lx]KP_9ex,ef[rW1R'##"
		"H?O&#W_5n$Cme%#gQk&#8swL]Q2Sn$k5kT[CV>#MEVkJMFvRN%9ZH30v?@r[v@#^?_K)'6Rw275d9HX7guN2(_LbJ2I$uc<4Q1&GBd?s%uYIr5-4Sg[FwVof7K?^7]i<2(Z@OJ2OriY$"
		"I>.69e6+V/P#602u(SU/W2xX/BIE]/-ai`/]>]5`B023(XM>j[^Z_/%*6U2(g.vJ]++7mLEBHof#9(4+LWo'/0*?j[hb3H*V<SC#8Prm/<MfD*P[)H*3[jJ1Sg'B#L0fX-bxMT/%*Gb%"
		"QPBfh]ct0#XlUJ1eL7;6tvV>#H>TFtOm3x#%$[P8&8a.qN5sUd$o%2'(Zfg#%@;8$waeNV7WZXLWb[JV?B%Abc5V86A#tc4L:dl/3R8m&HxAA+Yf'<*?W?q9.#vl&Sc(<-BtDY&wANlf"
		"X_kT[G=wJ)AHj6.$XM&LRMngu_gX-;oG;s%S?bs-L<TmAeer=.51^=%YCn8%[2lA#;)TF4)SE]-Hm0I$=+Qcia=&td57SbjkkEpu^@c>#+V&7*<plC#fv(quTVjrZ'k=RC@'1^(f(-[#"
		"c(6Q;Btj8#b8IZ-f9A&=_sL^,-U[p++:ST.0@@r[07:XSMwCt6SJOL(s@?E2r]/X[(94W*rcED*LG^s*Ld,6/q[Fg%0^LgL+AlT[qwBd&*%KmfeBBb*.`PQ/k.?j[i3$E'=)*W[qEZ&("
		"3)*q-?xDgXvvSm,<78v$lC;Z#>f)T/Rj3c4Xc1p._-h_4qjs-$$0xF48d:&=SEcLpU&/ojo(o%u:LVS%Oj_m#NvHpuS`k[#LD$5?$K-5?<k[3@*IA+@?^)xtiX-(<ip[H<7t=j'#MAD*"
		"O6uk40bR.'TNxm8xS9>Ak'`v)RA1i::v7p&G(EQ/_ujh2Mg(,)Od5T.d[Rs$#7fT%MKQ]4b9+m/b_U;$b:-hGTxE9rauP]kXwC=0K3G1pQi=(qc]'f#IgE9dOhRG)<1;6gM=dl/`b%I3"
		"DS*)*fi:SI/YtY-Tv,d**SKw%lIM<-7K3'%Vt@Z#PpbZ-Q@E$[/gIlfE%kT[dn'H&tGm6&8bnL:VfgJ)MJ-g)NMC3(m4?j[gHo2-%u-K2sGxiL@H,1%`7K,3TTF=$,0fX-@p$%H=/q#$"
		"g0c6u&LVS%1iZNq2?$ip2#@S#$R_R#LuCMPKB^Ln*rnYEalCrtipd9$D6tRKM@0/$B$axFI:>&O]X'&+(mU`3E]5SRd-U`5Hn=E+`;Elf&Cg;-P1%Q/1&5S[pt?p4i/1=-8g4k/CA#^?"
		"H.kT[]mFs-kF+,MJN)0(84E9.TL$<*%H,9.P)4p%o/3X.'@Mof&&0X[4WDa0H8wV[4AXv2.'?7]*K;3tJH09.aP#J*eEGT%<;Rv$f&LB#8IU>Ve7]wUsuHkYgUW:8hD3E#O.lnNaM0on"
		"MQbfgc[(xt?OFcKRrk(Ru`*nLb$724L(Pn:2uhj29C9OXb?F?*N.7G;Mk?;Q99HD*I``r?r1g;.F#8B#xwunf/Eq@'EA#^?=*;$#g+Hm8S;ZV[7$X+(_k,I6Pv3[5PNv)4_;W.3-0xF4"
		"_R(f)UUI8%@]WF3S)qIEt%b)<?F-Ul4Q5<B)&,aT83fK]px5$<rql]>cY,pmaGu@#X$>-^<PKluf1QbP/l13;j5N<7l=)##c2B2#GM<l$<7>##O'+&#NWKF]l410Mt/>2(VG>j[FvJkN"
		"0'2hLcd$29,r,02Z1ip8W@pV.%#OV&Cd4%P&2tY-H4tY-c8j8@$k%nj8k>j[h>Hm#+D:,2.:P>#v:qo79OvV[[1(?)8UZv$3S5cN&65N-+1E'S=5q#-g;^_%bHeF4.Mte)voC/s%oYV-"
		"jn0N(b9Rs?HdgMu$f%Hu<d&#5`/']b>>a/=%6Bj<@?V:d#%AM'%uL?#U5u.U4:a0VEk[:<=gH=WcL#lEO0s/F=Oh_W$37Cstc@Fr4J###@%U+$nkn`$5n:$#Gn2o;<vcUrcHRa*:R'6/"
		"7B;>]mt+gLSc,Q#keLs%'lls-_34GM-Zelf=?wo%PPL3(af>j[MJ&E'$^Uh$o#;/(er>j[Wi4^(2U>nf3.*v#HJO?gN1,.)63(B#01]T%sL3]-uR+*7p=hXLGNpeum,NJPj9Jp%#?0;H"
		"x+4#,$#%2Kp5Kc`$Ce+`CQ&C1U1ST$]]>+#Tww%#3$]6iOT*r7*_fRh;euh%kh`Z#/on)+tGm6&a+@T.P1(?):4vW-=f&+.iAK6]A)7W%e+'u.?]d8/KJ=l(*^B.*1heX-Z^D.3/3m;%"
		"7Et6/YK1?`mKGIQ`2Y-mS$#>#QSjOZpc)EJi*9AA5dL>#JK-v#(EqTQ2^iTFE%D9J-WeY,?*>>#xpkq$8%?3#w.'6v^AdtL*E80#v.>>#aqG7$D####k)mA'(u_kL&?<(v7._b$TL-X."
		"p;b]#daYs-%/[:Nj+w9Oe;D-2rK@w,spCR3Tj1R3OG/#Y$+(gip/+8R2s)&PZu38]V3dA#O3dA#VFM>?kwsD+$PZw0EBsr-leER30_TR3%-+uZ5sE)#2ve)v[Hc^$2h[%#W=K>]A$`N("
		"2Lm>)*RiofBmcc%,2?W-MGY,,%6#,2gI((&L?;c#_bp.CTfT/)t2)p&?$]I*`GUv-`BtB0-+l$%&e%lB467W?\?[g:/2LJxZTm-]M/A;>6/P=2'T)G,m9g^FrnKYpMwxBpMxXd;$)`F>4"
		"ut_^un^124`ZnsXM-OL'.qfBbfCXd[[)DP;.*,##>D9>$#[f5#Qd0'#I]2q0DqXW%csjT[iAOs*k''U.(t@X-=_&g1eaiWuX2E)<3`@WtnPAvL'ck`u`V'%MJ1<$#beb2$rfxV%<GwI%"
		"a,K6]To6<-5S'>_g.vLuUR<dtGM,Fusps)u0UR[#FfRib2Iwo#D%RRuZV?]/4-U+$GV6(#uU9S*;bm;u));c#X`[w0-+%##-hAwT6Z.;6mft`6?$D%%Pfu;-3J(`hFnCt6@R1eYlMK#G"
		")t%N24OBD3n0M8.h4uo7vO4quUFfHgc:mi0Mn7%=Y$N%MFgB)vPK)S$6le%#1&*)#4v=Z5fr+@5F(kT[I1[e$%w`,bfu*a*:EK<-2d9s$j5V/(l1?j[xx7K%L.4O42$?j[+CDZ)@&H,*"
		"coXgL8:[s*9Igm/.%@j[9l>p4vmfe%meRs$a%NT/9Vd8/=/9f3jq[@-F:K,3n/_:%nGUv-LsHd)DfWF3l]fC>GTCa42`NT/>0.TI;UWFLhhE(oWY;xkFsxWLgtYS.FpZS.o>2L#p5CmW"
		"-*Ek'+nU*JNsT&R5J,&1%%AEF+l9j#[QwUQ$45i^a+.t)L5VuP$3ji9L*MK$C+.&;$2LE>Rruh$tl+NQCgZm:[s)##(@fluuUk.%ML7%#+g<)*FJvT%J^Xb+eW`HcKwUt6VY.^&%B2h+"
		"lj[DE&X:a4BsZ.<'R4K1+OE]^baLm(V;Ci#/62^?<=?IWB.R?):$&L$ODTx)KGY6#5f4JC>Q-d)'tc9M@8;SIIJj0=q7j0(la49#fv<O:hCvV[`[Fg%8Nb2(iFGb%PtaW-qq4Hlj7tD'"
		"ueYnfJ4:e;@>Pb%=N?j[]WcTq%jO.D^HcY[2TM&1i4=1)N8^F4EfWF3R)2u$b+WT/#<Qv$>HMs)P;,R235$P;16eOMk3G3=Po:+>M:LL24]2M<0QaLNoKl3=OV>I=sD.%$tuX`#2DXPK"
		"),_J;s+2B6L32X-'Xpw'7u,jLdh?>#5`n.L3G`]bnMQ]4fs:OKVR?\?$s<41(e.^e$-q>j[dTwPj]J@g%=^b2(r>3t_v,d;%/'[)4u(#jL9o-L*>gfX->aQipGZN3FSi_V-Y*+e#6%BuB"
		"/(:HK6e(/Cb:khdOO/67(x37v`:E)#vM>J$K<x-#C####(?4>5qW(?#W'ZNu4)A>-d(A>-q/A>-*:vx5Ubi4#IQk&#m8]p$.CP##mjb:]OsUK(%,a/%;-tf'$;Z)Ev?Qv$%H=_/.n$HM"
		"7FpK24ddC#IFH>#M-A@#gru+MhG)ous*UIpdEdS[pZ*M0*CHfhuj3M0)EQfL/,44#<oZk$Qd0'#Oke%#fHO,MV3]8%=Z5T.sCkh2nBKW2`:3h#I2*)#-Mc##^0XS%CC_%/Tq39#oejd="
		"S^[u69l%>-CS1s$FYZ&MYP1N(3Xs-$EdiZUHtkZUa;Kf_$=If_j@#29;<lr?2(RS%dO09T$^?j[_8eh2Lk#H2v`kCL:HeS#&)u'HZ#.x@<o%;3UPUV$X_7%#>7p*#*Mc##6]`mLRL6##"
		"?G:;$O9X7/Uqn%#OZ2B=TF;s%trHT.(@@r[CPuD%+W_$'G;r_Z:Hx.'K[l3+uv_w.;^.iLAgI.Mh8Q/()D7TGde;W[Yeb,&BP6e;JZ.J3kDsI3wcMD340_:%?;Rv$@T/i)PawiLa>dg`"
		"/2w(ER*K@R?1+iGM4_ahuUJErV>m#;-Tb&.Ch.7B%nU5u193)#/X@3kL&0kL#mA%#kow<H1R0j(b&5j(Q)ZGl(Cd]%95GB?^PpV%$$lgNu(W0(ZS>j[*=gRJtY)W[^ujh2)O;uMH&9Z$"
		"F/'J39+w9.f)Nq:@/(Cr_6-0iRXkl8j/3T']WS6W7E16BujrEto%x+M24P8RQSkq/Zcb1^EgKk10?5Y[dAqVEUCvV%*E&W%#Y=_SW9.m0`a.m0^;#<-HlP<-IWB?G_?rJWOcbjL(m#t7"
		"[I5s.LX5s.V0iP0Q0iP0J:'Z-D#1s1(dqG/?Qml880H,*X80H'atV29ao5g)UQa_(Lt@X-4I1N(<[@r*v(FW-Q?&Zef*IdN@VYTh#Il=P38NErpvj&CnCU*a+%'$8,/L#$%e91&A8V,%"
		"`d0'#>5ht?S[MCXke0H4)a(m&cop-'0:$qr/G1H4I]XjLcfps7(?pV.=s#9.r>Q][Pq@<-lr[AIE$n29Kt,g)Z$TN0@ljT[BG`/.A*wt7b_8B-G?n68Q3'Z-HU'5'3+V583IO]ufmI7&"
		";CV58ONU'uI*,##A0ST$fa7%#*;uB]Xu?5&/&]q&3rQ<-j`]d#f&2X%:rBb*lS:<-aOsf)E8Gb.Of^a*CpC6/`RHs*W7Go8]TgJ)?Ij5/3Ow9.DII1M-K3R8Pb1e3*^B.*-Ya@$9_^[?"
		"x^4:]+DPOW%-QRbHb,tGkGwKuAQ:]?pWFq]Gmgptt]_$^83c75ej`uL8,@'vGsS*%2C,+#gH+k;#s+@5^ic;-6seQ-(8J;%hba$'JwK/j))Vm&*Nn3+9k/uQch)W[r>Q][f<Or.KA#^?"
		"h$4?%JK@B(.k>j[O@N&:0ASU&I:M6]DZp4&5SL6]h[7q.SO;8.H796_PLNT/kDsI3+87<.uiWI)wZC@co&n#8U^v)4acWF32sBb.j.<9/K)'J3Mm,+%MMte)@%jPa=LMb=4*cM#57CE?"
		"_]ZY-H-Xe>8@DH#uI-^V#]`HEim`)pZJVR1Tt=VkkGwKu*L)H`4_xk#6#aVBa$pa5e@Np;O,`$7F4Eg*j._T%9qYs8*A[fXEdvbEe[).eN0')*C4MuGv0-sZJ5<A+-8R]4N9DP8gu5D<"
		")Ubr?9a;MBIlk(E^E]rHr]6MK,if(N<t?YPR*2B(g6Wh(-)o0((XvAM=&xIMRO6LM_BNMMaNaMMi)TNMmA#OMsfYOM$;DPM(SiPM.xIQM0.]QM4F+RM:kbRM='(SMDQhSMHj6TML,[TM"
		"QJ3UMUcWUMT6;%gX)6p9Gs?R9fp5a97B;>]i3(j0NdFsLB?uVLK(@2LZM>s-4^=2M'o(-2hpKm/l<?a'sY70(L1An$'O>k'hw`pJ5j`e*+$EE<i.&d<o&Rx-7lYcNe&d2$?I`)O,Pa0("
		")l?j[:V9a'8Q=qf9lg9]%*ho/7BqVE2VDwfVp^O.$Z1K7+X-B4fvAj[&DX8F]Rhrf#klT[,83a9bl?-O?k'vfWKAj[7<`I2agAj[Y0sM?lWEtf.6mT[&Cc0.lRE.O_2_6(`dAj[l[X/I"
		">dCW-6%oYM.O*<(0,Cj[>I0sN2hN<(48Cj[FnG5P6*t<(vJFF,<AUv->7&V8i&?0)Z]B.*jPe)*C4^;.*=pk$8M0+*I5^+4'.5I)M2Cw-$68IMk@kM(_T/i)7,B+4$LPJ(<IHg)Z$vM("
		"<2jc)G&9f3_r2]-Jb0.$/v6`&L$7L,^Pq;.12q8%GI5s.c%+H;C]]<C5'IMKtl?h#M^odGYNpqf=6^w7:mv<8kO[f_>mlC?*$5=04]'SG2.b#dOW'J#e>nlorK6.us/<XBYH%@u=Ar?u"
		"A]e[#?nwm/Ci-g5V]feq;<pr-@'uRM6`b.h+djAABnPaI6okm#3t%+>o2jQ1MJ^99vO,*G0vVm/NvmCNtj/f=3d)'uIZ`xd%7VouePXA#^wvOY$x5VY)7To=r4rf:=3TV-8>*CA5#nxb"
		"eN5]unB:NtXcdhK1aZp./s?S@+nD,iLD#v#`C5;-pNel/v@0S[I&V,/)<Elf_#/X[Vn7`?n](^#xi1h$UjGm/W7/N$h6J$#4XiX-_MIftZS7K%xL[w'&4-J).Ce20;_sv#uL>c4I.U^H"
		".)%E3jiV/1J=Rs$2OHg)Lt.i)*`rS%?1Ox#t9E)uCD1[,@k+gXAZek#Vdpo0M5vA@^m'2e#Z(Z#.9gA=*gP(3xO<t5$s5HNguLmOJ;wX74N1v7'1ucY&)Z5&r9>>#^'%##r<dS[gSXL6"
		"=<u4oB$et#?+>'$RV+Et[VKvb%/5##wNRD$7[>+#e^''#u+5)=qtW#-?JQE<;2r[G&P]p&+da)'wn%<-MH/6%^0x;$Br+@5(Dk-XiL,B(CR`X-7)ik+`7/DZY&)N$2fG?5wUXb3JjE.3"
		"aAoO('*Tv-iVp.*I5^+4ATC_&bA9M)ch>^=`#6<.^A3.FJCrwDUw6xtlm'_:Ol_iP?Q,&L;FT9QhRP99oUbi#tBiXP5lm=,?F@F2?0s6U.(A/Pt39qaPHChJ;.>>#t2+G;+eGoe_?]f1"
		"Xm0<-XS4o$C,8'fPLD6jLv'-;U4&X[TDsD'61I-2f0DN(PVK-QZ't+44um1G=2D*5eb(sJ[_7UdW#xZc=qeV$5`t'#p#mN#(i=6#8:r$#@A2(X.:]v5SO;8.c8>D<Os=iTB1ZY#nKhlf"
		"UMp`jOi4;-O$2S[bc`/%/]_,2.+Ot$(2sw$iH@%%9(f2$JTts$[#$6/K;W)'3phgL<GlT[pNvA(<YGnfVhGX72*]q$Cu7F<Y@wt$<m;b</IvV[iq;T+vP@Y%:#<8]L1Z<-Lvtr$*Pl;-"
		"3tOU%CnUhL$nt;/.m@d)=)'J3&_D.335#DscCrk[X<J6:OAks9&kF$-]mD<9%->_,k?x]C`.b04#lfC#0d*u98DIf;-okX.0EMNh+(ET%LGsx+4rU3k$X>j[_ND.0*Tc,2k+vD<XbkR-"
		"`nkR-7mo(%);Mb%2HqJ1xs7Z)xl.O'>r+@5Ep:Kar^Wp+)P<X(R:SC#W.M)4=#4_$U^jd*lUcT%QK[(sQ&IG[Uw;99K,q/:EG3gUV_VE#`x,$Jm4eC5v*>uH7C#D4GY:32JD3)#S:4mg"
		"UfiMK7S-N(S95^Q)fp2?o@Gb%*M^GMi9Yp'n1fP'XH3T'C`VW'ZmEp@OCkT[$hnU[bXM=QRP_=%N[h7[^TD,]L]kN;jwr@Lb7vvJrRa@6#f6vaV,P<nZk>2gCv;A+bZ2S[/Q8C%piCn<"
		"<$]e*uAkRk'/I/(ZS>j[K/av%[ppY$F1>^$9+]m#N5Rh(=#Kmf#-Pn*:>;E<[kM^F&av>82>eKY3&J5'V<n8'AS;<'wTeKYAfCZ@klM0(u)?mUjY20(.qe@'rMkT[Zn'edEcE<%+dxb4"
		"b)MU%n)B/bL*xR#Jh;k?19Xa.q8#a?/N^B/U?,8ndIJvk/`,61LRIsA3^FfM6WX5&Y7`KlfrWlA/ZS#$`^Mi/Ad$K%0ch,2W$U8]mtkgLp7Ka'5`S8]3fg58C]U$^wJ&32ov:g%,Xu^_"
		"#n.PL4M:Y:Hi*8n-cbUH0vh@,pvI5;3(Uv5r0Ic;b^cS@0vI-ZZ#%8@[GC,2Yf1u(tE>j[Av[N%c>-Tg[I+p@6t6s$eW_A=DDm;%BwYB]m/@g%q5hh$qSm6&0c4:&vPx>@[;?Z$*&E%$"
		"?[d<ViV)gVu`B<ioYQqP`L7H#0/5##8rrS$%'CPOS_nlfNt?R9:DPX7Ziou,<@,875C'^[>6Gm#9`b^#wm:N$-0d9M/p?['=*]e*BEas%2o$-2&wtM0e?Yv,dKav#ieh,MNahU[L/&iL"
		"`v]G3$cE<%(vD%$15+tVg*@BV$3]D3(>Hh$swPiPnr<(sx'e;-dA+B6/w>j[&lNWBjn_m(nDkT[3U8N-teCQ(`AgI*.->6/6=tmGHi8d8g+)te:.>>#%[vcDTvFj0Jq0_FF3p=%;;gF4"
		"c;*F7OvWT>ktX.FZVXe?g`Mi/0G:;$I.'2#6Pi-(4Z_R)A$is&awBbZ_H8<$,r+@5.&QFubi3H&DlSj0vlLZ-,aCD3jhYV-hLa`uBhMX#]T-JhvR(?#l&'@t$tN:H=+Mf_GWXrHK_Z8&"
		"YjxQ'X(pu,CTXm,=I+L>'mkT[)V>#MKmJk9bVbA#=SfA#IxsNkEA#^?Cq,##Sf`s-#u%K[iV1]#g]8P*-u8>GCgn@#ni#F*0sl2QHK0]#*j[pL'`$dfq_N;7,a.p]b%h0MI0bX*:@3D="
		"n]M8]*'7.MV^[`*s>pW-6(B@8#]gE[oDdJ(I5^+4-PWm3E-C</6_rJ#LtQp?2<(fl/wt(DbQAIhYN7UCwX[4Xag1F.&AVNWvbftuvCAmUsg7/:SdM*#tE3D<[roP85G]]+']w2$K97J&"
		"teYQs#.2Rs18ds-oL+,MK'WZ>B$C#$U5H9.TAkP&bThD3s5Tv-Hm7f3JjE.32>m8/H=lM(v$0T%<7M58ZK['j%*KgK/h5U#;1e0'U/bLp&HE#KvWxQa';$C#rDC1vg?N7Gc^5>:f78-G"
		"FTXrHQbiM0k0SP&'+?M9h?ke)6k#W[N6Gm#KNgu-RbAb*9>cjK<dPH36.X@$$GXI)3Xq,25VZ@-KsOq.f.GmWeM#j#TH8=Yr5hi9KYjP$,OQFV9LMxO<@C@tp+NE#7E>>#ZbP#$c4OJ-"
		">P&7&i[v;-XQ,w$`a0t$tq2M-*i+d&L>v]=O)-TMbsdh2xm@X-Ngq]49D?P&0wJ@bnFC._k)<`s_?(P$hTA+r=PliUPnQs%i.awKC'$'6HjBd8hELwK/rP6UA(>G-naxr19]Vi'@gtAu"
		"9;/X#QPj/M]x3'MeQ<(ve^uB$2#x%#pn,VIU`V8&aI_8&h'St-m_k,MS]20()lE=Cfx&a*+&dW-Cx&+.Qj]e*%>E01w11W-6j`'4CqC.3bj6u%h/w`4'M3]-s)>7SE'RtubL0IrRf@4p"
		"plSeu'$sl/Pi/Po[h0M#GS^p7Z'//1iZ*_@Ni7p^<uh.];UfZ$frBF$gg1$#O&Ux(?7@dki-Is*XVw5/'B;>]wF^l8%J4q]Oh8.&uR-K7Qpi@]RbsfLt.t@]33^g1YKJYG%(NjKT/Jx]"
		"*/;EMLcr5M'/nr%:2ST@lisJ2Z+4jNE&oO(7Ya@$sCqbro'/m&wYAc$W=7@#j^AN-Y%pN-Hox/.t/NfM,G5,26Xvl82b^t:HM)8Ia8B[0#9@W.&A=/C9QOQ'R>Vj$v+*fO(:#gL^C>,M"
		"Yjqp7aajJ2/-eQ'kjQ7rgKCSls&j'?=SC=lhLCL[#j^#8F)Jp&n7r2D5L>gL>`q58'j)W[r>Q][gd=q(Tk;Z#^`l@0E%kT[]$:H&74ja,bJ2K(-X?w%1?[^N%P0J#I*_^u:pNpucH/CM"
		"]5J>PQTaL#?:/XuVfYl$_A8vL&UQ##.wII-3S]F-/^Xr$0M/H2vlLZ-Hf*F3fFvi0egoZbO?AZ$i2'##D>-C&a]WlA;B+?.%rJfLgwZxO]49)(E+)[p/EJa*OYds-U(ofL(P7K%.RV8&"
		"UasJ2o=Zbs=c$9.)puF#N*glq/.d3Y%1qhqh#]L^6VL:HFCbY5u&+#G])$YcuQm>$9sus-LwejMb<#gLmiUDNrTqx&iHA39Kr9^#9xYbs9Tq>$.r5HFB`g9$Nqx(MoE=8$A5YY#d:fIq"
		"8mPJUMJsx+uCw2$S@6b>Kas39-RW9'N#-a9[?mRA.KX2(n#kr$pm%J38@8R/.RpT%trkeu2';suf/+-U(xTiqbGj$Tb,ebRIi4%t%u*&4aqd?Za#khu=4hn%jhr'_A(Mrb3AQx1HH:;$"
		"6-'2#@@%%#n$jM9Om^>$eDLT.x?@r[AnO<%C7W$oj9[s*qN-c*@.e)E)b,[-?DXI)5FGH3CI.lLs:T;.-$<X##xseqR1+bDn4>6r$=$GD8EumZ[%qxb;:hlD[>xK;oJo`E.^HG)6X?M9"
		"rnx8&2c-E52-;hLp7Rq7sTXm''F7^(8`M<-u#c?'d*'u$U_#NBMc(T/c-'?7<qbfEch3aU*h$f36&NL#$su#q%9ojScI>aSxWUj#-JcH:m#C/&[/>YYd[eV$#+IouCWdsa,cYX[Y?/':"
		"7taC5mB42'E(t1B`B,c*0?5Y[MVS&($CWQ8slP2r/klgLMmlgLi=Ka'6rRn3p7&3ido3H&tnAU);Omt&S:SC#Aqje$$qB:%[2lA#'^?T%1igdkg>m=j_wsf(RHg%4;X'oDiQ*>Ya@G>P"
		"x=1Q@T?jZ#8L3.=/*,##U?]9$`:d3#_n>?]i=mp8.P;s%]6@s%WX]78Z%U#$A$Ee-e2?q%5T@a*EusW-4/S49F3tY-MJ[V[FWBk&W4Dd3?*M^>R8T;.FUe8.%cK+*GBtA&u;M,3r:CGD"
		"KW?p=ZaLpuTM-[AqRf4k4nnVO(mEX#JD4J#3x^O;MYG&RD$x6=HFAa5&tP:]WV;,AJI.%;HlKS.=WH)*gb;8%-C#wRPg@N%bAi/(@Xxs-EEk):<A.+4^X@[unKwFM;+OVd$NHVd7>D]."
		"*IxT$=N%>/pWF:]<[Qq7q;oj2n/hd%)@=9.PVS&(6R<N(*6pV.PJvG%>j<P(D(H:vm^7>#;FVbmbsaLuL#J=k:uoXu)qc9hZ%JfL@SGgLS#YrLeVC_EP_N$'CHXb+fN#QUR[[b%NfqV$"
		"Xv<s6>OZV-jhYV-ubfh(HCH>#_k:tun5G>#LAx'$/AC>9*vUp/dXRVm1q7p&fVc/'J#BEN-76u)95GB?dbnY%ID(v#JT?8])Pp;-8j*[tjow`*(bPs6_j8suUlwZj7>q.p0r7U#ZZMgt"
		"-H<)urlE1uTw-qL&H`hL[2BnL9#3$##)Tk$=e80:h=7q1o1Fa*KfZs-V44,MU](dMNF?##W3E0:6^sJ2T-qtm1]$9.ZW.#,9hhxOSN%]t4m?JLq]IA+`N*8R>lgL^$=$GD`g;58]t(Z$"
		"8/_D4.Fn#P4/9%'G`BdMWM(SD,LWD#e6FA#<c1Zu,0+JCVJ-5J==7S%1#McV?/RfL?ot)#G)D*Mq>Xa*`_(n8CHpAZn9[s*>8iW%?HLPJ2D^`EZLEGV93*/$Y`h1#q$r+;qnOYYSN02'"
		"+7?M9`KP(+wup;-uG-.%GQXb+FFQ+<ugB#$++L#$6lP<-ZqSj'JP9OjC>Uv-p4fY8rWDh)orUv-1SEF39-IYY/=@;V/=IVVuQ1,$e?g>#V^FlYQu<-[%D7?Yki5`TFo?2L`tG^#Q5[`*"
		"us(<->.PG-P6rH-0W8(%b;`/(5x0W$IUxbIUI1N(cp6H3UJkjdSL`[#BZQGISNT>6u$58]kdtG*qi)O[]P*s9Gvlwfoo7)**r+@5l)C-Ma7T&(pu:CCe+k)'&[Uk+8a#+%u[e[-m^++j"
		"D%E5KpVHnZN8A(sJJrQW@cNiKs2]Ae2klgLsDIEN`(E4B_SM8]uNlgLV;Ca[uHwn])h7-##I;BLT(*GVF6Tj*3jEt-,_N)<]>pV.aR`s-;4tJM>US1(<p#V)b_wb%8GDp..FE:.:&4Gu"
		"^O&%><acd3%Utl&G'Kf*8/bX-?81+jm'._#Xn$##Q1+d)U`Il',jk0lnt`s-`L4,M->,,2WflqV,a&mf@ljT[ehXg%F]+$Io'Dp$<8DD3>HF:.L&MB#k9@+4@mGZA-s;iTXfpn`wUBb'"
		"D$-TW;fR%tPglB*$),##kTot#Q`Nb.ZB)@]lOH@&>9Xb+<(fnNjX-tJSmGm#nj-F:`S7K%-C/bI.ah,2cDACSl49a*.NXbEHhM8]70%iL'U<T+,Ng*%j2=1)80fX-j@t_$?<Tv-e>K.*"
		".p6T%E_&k3hR'VZ@,C3MbB/YL<`?k$;p7<8rFXiK;W9dRc+@aILDeK=E'&t/%/5##2Z8C1WPX&#T-4&#e$+x,:/&#%,pu_Flj6LcDcDW-8;g[S`&wH&P5@LcoY,'Fx,9K27.*u-33P,/"
		"M;gF4C5Wt(7UYI)I5^+4swS[Qmk_KEOo`:N2UKZk8nkp'u$7'$KPIWB>;%EI1+7t25Ht:?jb*ZQj-E/#2V5lL_lP2$eH1P'v*(kMd;Z/(KBDFR5PG6ErZLi5jhYV-*e`rZ@.=S#oUlb#"
		"H)&Pu:vCBSkF#m8)BZ0)dhLS.tY;K%>^=eD]xGK)gIr/(`cg'-F+5G-BL];%*N2K(2aPrJ=+^e*c6V>QaDZ#$RZI%.o?9>G`-b2(lqfW7eI<^#(`,3.`'vW-m@$uU-.V)+bIO8]ED'gM"
		"RXb,8XMtM(M.-SHm>C:%hNSs-bBl4:*]Np@l+Bf30d%H)MuDx-?S<+3eb/NGeVm1PY-QuTmNUjuj>Tv7Pi#WSWiom2h?ti4qv9@Jq>0e6Y'o'IV6:]38Zkw?Y8]l>VnHL2lL>M9l4<DW"
		"Y/),)M:XlAM=)SCp@81(DL)c<2#b-vfn+f%`'9U%&%q^#+#pA(*s8bN+pGm#Yu@Q-Dt^P';<Tv-^N#I*Ynn8%9/TF4fF6Y@7$saeKNg7nqFPQN+`Q7WKZ'D)3TL3<EGkI3-s@A42J[aI"
		"Akx/6jB)8RF+X794nOF5JO@>#=R>DNk;v*ZEcMBu(EDk#XA%v#[6I/:du5pRDD$o'76o(<f.I-2o:?j[%#8K%ZC2a%uH>j[*U$A%P=?J=KvH+#Gmk.#>%`NkxMalfs6lfL[oM0(R&cO4"
		"V:SC#2Kv;%A2O,MfL9]?[YOg1nGUv-#aE0nJ1$OF>K&oD=t$uB&u0K2&FQd3aPRUk3mBn;)EF6;%.'u.34rkLOG*Zu;L0>$EqB'#Gc/*#i+Wm8C>Ms$P)@j[&1;m5CjG<-B$<'%SvXb+"
		")g:hL6b13((i?j[opsr7b]Rg%%Tj'88lg&(#Nl'&7bf9]q2p_%Ikl(4@X%u-,^uGM<MJ1($]1N*hMv/(Q/g@'/AL6]'PpgLu/e,*hg'u$7=_T/jUKF*^HeeMb]p88.B(a4dT`2%*9lA#"
		"I,6p/*)TF4.Pws.>u7e%&KUv$7^oCOv5lDLR9Z#HYC75Q<e5quxFa[HQKD:d6a1>GQKLg2Qe3IZ6ALm7r#CAIu-O,$Cg84*MXtp(n5n'$Pj:J?A;1*JG@?4:g+ZkU[[`-e,,VD)+QWj;"
		"6h=%&&^$?;Q*,o(2#Ln;&3GY>s`iPA0J./(Y^*AFw'x5&6aEd=0pu,2:hdp78--f;aVCKEp(A*-`?oo%,r+@5lC#+%ox68IemVs%B,^r7`a>^=><Tv-J>f4(]f^I*N&E,)rf/+*KG>c4"
		"sc6<.]roiB3'^1n/QwVGR[;+HC5o+K6XVQ*3X;*HBFRU#Hi$@#<Z>1iqgpUuNYUbVQhe=2HN+8#A,GnTpsUfqDS*)*M@t1BcdSjiv(C+E=s;r7Q8`$%>p1P-sASj/.A#^?F0Z>#Ke/g:"
		"wcMs%U.I9g4;N&fb9FC&AUg)*wUKF*QxMT/+87<.'HSF4$^(Z-fCUu#:q)`sT0Tau0I]PJh&tT#H0jguV<vr#'Pki$`:A5moMTCsB)nG+PCwGHMap1Fa.>>#7&Vl-Kf`KcTd(;?=29a*"
		"=__D=M0X/2OjaRS*f0a#cY>=u^>]P#g(%[#krsp#'_(g$q6Dk#viAEuj.;&0SH:;$_5###q1F?%4Xpo&Okd,'4Td^=YgD<%P2Rh(d,$E<KYq]R1?x9.VF+F3t$TfL_nPuFt.XI#Ntw[u"
		"R+9iu9@0quMsAxt_0mju)[/-8)igj(9ETP&#B5E,JSK-'_QMCXnl^DlhVJt^POvu#SsEf-g7,+%ELRc;ouV:MEp==uEtw@#Vl@l#mB*p#'bTb$S2<h#+RHCu@,m9%22=a*=/a:MG1w;-"
		"X#^&C_W7I-%hj*%Stwr$c_0+%[)a:MTa%Z5(i7X#QtK^uPPKqu@W)suSXr$uF0mjul#b?Mg(u%v.Q+q$lwK'#.6ik0n(<n)6v?e%tJMs%M?b8.bi>j[UDhW-:>G=pjO,B(O2c$0,#lT["
		";G`/.d6B9.'j*m/*BteFhcf+4KPr[G>*pbGo.Jd)sU39/E`[b<?jlSf5uNIekPCT.IQFt`.F@#'@2BB(Z[Rba.Bi&&;aj](mHTwcLtc3MvWx(#EE'>$55=&#*X0>->qo_6A$%B('wXX$"
		"C2[V[a*Ds&5@(lK-d]X[_[Fg%`u1/(bi>j[gM8a's<41(`W,t-N&4fMvDR,*t0;W-MA#G4uKZ)4gWad*RWZT%YtwX-GTQ(Nj*alS;.)@&&lZ>#iA<s$XOwwibs-l;/Tc>#=-4M]&oDf&"
		"$P]8RuW*Hh*>PXK]kA>#0lO?gsJF>P&ZOoRs]H.ML12m$UudT.7@@r[LMlU>a`X&#fK=wTd5mm&$YPs-YjSh;e*VY7c+>v,DXP87/0dPAA*vV^0B)S$x7wo@4avU7FdV8&2Z#qrE:&Wp"
		"fGd/(i(?j[Y_tD<;55H3voId)?;#s-uxxiLt]lS/XUfC#mg9B#=c1Zu<Z`Cjjfd_uUwW7ecKwYu<sfCa+r&K1abQP0OGLnQU5?j#0-i^oi2'##,Jn^o+Lt(396n8%&-$A'Fl,hLpp>R("
		"<huM(&9L#$*V#Brj^GVdxhl<:G/5##nmTm/.Sl##vWKp%Cidt$EYJQ_l;)U)S[V##<qmd*[G[T%r[e[-&vD%$vukV--hwkS>*i'E(L$V#B)'iu.urEuA@cY>nN?$$U%5;-SN:K%=/n.%"
		"wANlfF(kT[Gl7T..@@r[R].Q-4LI6/&A#^?l)C-Mp-Ca[xHwn]OAq+$h,RZ2D+i*%RX%v#`J)(ogG$>$`vsH?W>U9.Jc.Z$^dCS/cLj7@Xf`]4X'B:.>4P1I`B9>-hb#]%46mx4Ev/6&"
		"Q7Zp7Hl#gF[qDZ#kc^t7I]Zp.nBRP/4](Zu*73<s3UM:v&3).)$tN:HkNUk;i+'58&6G>>@/h'&^FNP&8@=N$gsSrKRag5#IMF,;x`X@R0RH]%=Mms-kmk,MsK%K%Q6J/:QAdV[wQo.c"
		"gfGD/HolV-e<MhLTcH>>$[K,)@+3h#&jn'&Fr&#5cQS#c*M6W-J8gER6s*87R/C0'BEXb+3H;,>U#fj%arT9@EQgZ-'RdWuXn]lo<7[s-@U`mLO5$)Mc1<$#._52'(H=$&rQdW?ZZbA#"
		"wa@.tZP@g%>(`9U/=W+22r+@51rH>#Z_G)42S,lLZ5UB#FIN7nFdqOA$[]@b[#B+`,7629TNH+T.0-AXe*g:QwTN`<vN*d;eAcN(Ucsx+MBUg%2.&T.U.1Z)O(&b-msv&61'2hLL4N$>"
		"lZbm''@i&(U9W>0nP/X[Zqt,&QOhsJx]wlf&txi+f4T&(dk_gLao1Z)%4;ofbAt8@gcWI)gHE:.-Od)4=gf+4_`hV$r#-a3PE=U.b[uW#dVQJpG=jOouK#Erv7l>#*$89$PE8L#^5%l'"
		"[E#Wtq*Au'_*bYs_QE._7_w[#9V4(N.u7+i9,>>#QAJ#$gaBd*XqTK<93DhLgcs39X=Z,*.'T6aSSaEev4+1(m4?j[j/o;*SJQ<->;/?2+K+P(uen8%e0w9.`h2v#COHa3v4oL(Q'7mK"
		"DaBGDt-G9m01Z<Gt-^%kQg5P#XSB5m-41]u:%K+*OR-##+&fLgQb:]##^9@11H:;$I9d3#UCT>]4qgE#>^.iL07bFN>(5h:-HvV[r>Q][Pe[s-QZ^6AQTOQ'v=Elf_&uX?DrKN(*Riof"
		"*9ZB]HCZB]-+n;-4I]f.m>sI30,^3%+Mac)Ynn8%9<1^#JVe_RFoK,Ix(AX[I>XHUGP'nBbE1G23,Ui[C^=PYj'B2dpCG(UH=VXQxbQ:mBP1lk`bPfL912#vlG%D$uf[%#pF)c*3X[EN"
		"DsugLqToW?9))d*?8)d*38x7*X*SZ#d$Af$lr[0(JCwnrZJ@g%6m601Q=ZV-Z0fX-)jE.3Jj:l-r^08q'm>w-`2(B#J)f1$Pv2ROn07HqncXGH>db9r$eM7c*Efpc^+&7>7B0@/O3bH5"
		"o>,&#+*'a*cXpv>;m28]HnSX-Fv?3<D(8.2AZ[@#lVBe)3=.K7JQGSf8SFq]<v8q)P;(K[eJu[#lrw5Ms.';Md+1n`Vsc2$QgO<-6d%3+A4gfLd3C+*ww%SB-1>ate$[guZ169MQdW_4"
		"A<T;VM3^Y#G:$##&wD>PZ45;-P3>N$J[*T.URHs*W]&c$9s`Z>5YO&#1,*q%/el3KTmrS&A_ZX&%Yth(-r+@5/V]e*^5]gLoVsu&&on8%$9L#$T4h8.@S,G4pgSX-Z]B.*irqW$`[7<."
		"_<Ukf0.F%XAl7DZfu75/Hl#e))paAjN)gPq2>1sCxl9`gF8IouYLO:vp0P8%3-%j0lk(-'pGth=4:CS@l8d;%ta;2/Rww%#hGxk^+3q/)2Mu>)]uGEN7'2hLt$(U@(wX&#KX6th6FFq]"
		"=PC7#uR-K7Hpi@]RbsfL#>t@]K1JYGhhQ['3^c'&r6r.#l.8>GR9=ipS+0p]I,KkLGW`>V?8N8]eLx+MnQG,2[eZ9IQBZV[CX4o&'k@vnN4+F3Z]B.*ig'u$Dn@X-I0F='b_PI#?VS<P"
		"896igCF);$S6l.qiJ0)I+B2O#3cr[AkF')gA2-W-*no&-Y_-##LPUV$]-'2#B%'>]lhe5&SPmT.3@@r[<Vnj'_/M/(N&$q.YhXg%,<Y#[[TKD&0iQ<-9US/.9s,T8vBp,N+sHT$CIg]8"
		"9,77p9Xcou>S/vJCc]Rn$),##<^@L$xo.F/0J3=]m-lgL+,vG'b9f/:r>C-OJ6Nj9:Y2W[_ujh2v=lKP.e)OBoUX6BJ&xsuG<qV$bCDO#5//&ujDVJ85tg>$tQNB/HOg>]0<IIMcb*+&"
		")3um;5ANV9^/v2$C1o34)+/X[Z5&+'qBqT%n:SC#M^J.*s_h8.x&lA#J29f3O[1N(9VG._3r4$ttPB#$pq3YDe:C3s+At7$]P*fe=Q1]42H-x@Bi4wLmw1^upJ_hLYMW$#F=K>]u5xhC"
		"L<HZ$]lrD'mULT@Sm%tB$b37%k8kT[,_L-Z1G,nfgx>j[cmYS&6#gb4^NOf*KT<U%fkx<8Z@7w#<08<ugiO>#3uVsuGmS^b.lHaKoC3]4a'&&==M[G)(votu(+,##CYqr$?Off.4'XE]"
		"9PLA&gvgT.-@@r[e'x_)'q,o&,YQ<-dV_@-^7O3.ow9-M6J;-2x4X?#^=&w&oS'df>iOq]U65+'r2l2Q2_wZ#&B#nf@j$q.^hXg%E1Fk4cdF@#nf[^$P6;hLjnlV-w[3@'_NHkAt2stu"
		"SMicuTBvFsccjLsYUw[bmGZt(F)q.1&uhP/`0G>5$xQxkwLOAu@OM4#`hk98T7*8nA_MP/1N1H/F$068cF$--N,u)<?dM8]w^hhLXh-H/.N)W[P4%dfn.+R-g.7m/n0&Ha[O^>](%)t-"
		"8%OjLder5MWVg88B[M8]Uik/M^O$,*dg0N*addh2,bo;--OxF%./B+*t_U8.d.<9/K:0%>GHt87:5^F49:1PExSJEr#V4[F^ZfeubuiA[3dLk&f3m.lsm85$BMfU9OWFpXr.4`/)HK5V"
		";U'=@K$=8:,#(N$/v?j0'.H>P#C7G;@pSQ&.Ak7'joME<]KQV[BV@n&LA>##bIO8]%YTp77l[J3jQ/@#F$nO(4=KErMFOEr$tN:H$MC@k=SC=l,;nV7KqMp&?uG_?<s-&'pjPE<R?Qv$"
		"s7U0G7Z_,2rfw>#W]-'%oS'df;V4q]IC<1%r2l2Q&:@Z#.g>=&v1s0+l<pD=v#_og.=V-C%.C>,CL>VZv*-Jh4gE>P$xQxk>AS7[G@n;-&%lG/3>w<]6eZq78cCl2_Wg5#b*S<-;)l[&"
		"+,2T.4x-s$3Bj'%TVQ>>u5D%(iti4P=3FQB>FX=$#B?,$$EZBpEX6K#Kq3HV7/U'#%xf#-Ma_>5k;^m'ou$6#c_x)<SV7%%,Wp8%W6bS%HHEFK`orQNveO.)U7.[#g`5J*YOZV-URP-r"
		"H3j=G9Z2*@BCpuA36:pLFeRe4pJ-iKH6W]$L@=*$%oab.MrBj$tjab.#Qh;](KKs$aM2QhM[An&ug73BcKIgs&K@lf2e6H3Jdj39e@hRB**u9ub;qV$rl?1p]+G>P$TdrdUx@s7]fsm$"
		"G%?M9jtgM(TeMKCXBf5'R_YQsPG?*<QdaJ2^cD;%]x*F3igLk$WGJ)*``HBo#%?wFCgb9rsv*Qg[4oduwp^@/BU^2`^)-N'TrIs7WNg9ra9f;Q.8wM&apjT[#tM42XA`/%4BJN9hn]G3"
		"#4Mw$3w^6r#[;X#@0n?p6WSvHTiA2gH*]m'x'Pd&@TJp77$.B7:@_l8g`;W[feb,&/ZU,Mp,9a'XA=-M+CHj'(n=XtLvB&8Bf=_JU.35&C5/^+UK/q&A`w4Aa<^F*MKo;-i*f+%E8C39"
		"E>Z;%puC9.?3`v#_Gt8&SWtc;Df3ia]gdh2<;pc$Z$vM(;m#l9wG&]$]D3pu1]%l#76n<t0G5bH;t4+;%EX'c_M7puw'+M<l+DuPDTbrHq8CAYL)]9/-;###Cov@b8`-F%ZET/:pi2g;"
		"LfDq0>w;/:.21l2jS7K%'U9isHBeq7snl'$1[8N-PhD>M9l?a0x`3ig(w,*r%b<lN'q$)gvEV%FWNfIh1Hw'#aD.;6HqjG27Y=>,&1Bkig_Z.'^,Yb+<TCt6I)BA+i5kf-CU01Pjleg("
		"_dDm$18)<-xF';%C3,aE4`V8]TLJYGt._Z&Y6<r7)X;8],X<nLTEKk$nF3]-q:Rv$bPNCX^vp58IH,,2R[=sL,hV.Lb_H_ti'j,ri#]fh*v:o&&@I,U]0A.(&x8YBs6<S#F5mLb,oDGq"
		"vIZ%OdW)##FCq+v3XDmLJg&%#Q8HG]ESO/Mow*a*#YNa<i#c0jq^>o&d)$U%A32m$ZF%K[Tp+[#]W(>'#V8>G9*i?#ddi3'0sl2Q4e*[#*j[pL'`$dfq_N;7,a.p]VSKkL%]Lq7qUk2("
		">j8r7U;ZV[`Np3'Y1`$#+d[`*k#:W-+G07DV$Dp7$72Zk*>)I`/%Q[)3j;@E2>U3('TT>$]Fvqndp%.GYL6##kG:;$90a'%XFt/%GH+W-U_tX?)eAQCpa=c%mw39%0?5Y[p([7C[?=5&"
		"qJ%u%b*c$.YGFK3_V8f3jtuM((E_#$>]WF3W^Tv-.f/)*kRsT#h(16nQHs0k%8+f#N,>ZWM0_J#&0>'$?%lG#p*&##.`($#Sr[+MQ;E$#5nYM'Q@+(*E9Xb+$bBt68%JZ#_J[#%b*G(-"
		"wAIlfBhXvIbP%K%l.^mf'FLk$CEsI360fX-gZL]XB,?Ehn>whg69Qru%?drY;d$oulcNb.G$_/%f_Nb.>K9F]IQ22%o[7g:'gaTBTag5#4-Ks*r=2*Ei.vG*bj#UVB$g9]An/>GU2[vG"
		"dsv(@=Xh58nXM8]Bxn[>o[nK<B5I./lQm&(hY>U;wrO#)q0x0(j+?j[hn,L/Hax9.O<sf:cb?\?$dt_*gsn-[[hFoo@tXWLplfbf:mw$(#D:UfuWANH$l/`$#Qu%/:,r^>$Z2(T.+@@r["
		"vMH&'N/9E#F:=f%hBgm&)U&T.#;b?#a=Ct%YQ/Q8Xb3-XWj#6#BrVq2F7Hq7,r&K20,4GMtvDJ:Q?q^#LU<>dQ'&]bA6t8.<cl=GunW'?g(4_$uoR+M'g/%#qp<d)(f#]%Kp$hcCnEX_"
		"1fQ<-Eer=-#@[Q%krLW-<_k-tpHIq$VKA>#nu09#CN.]%BCj2Qpk_Y#qTjlfMwQL'-$vJ],:Vf:1?Y)4O#K=@.Dp;.kwdLMq%:B#Youn4$%(#,DUYrZav']b@S.3rPn8Sk7bTUuAK+Q#"
		":NcouRcA^$l:$##X$+C&0VFcM>P(up6bU*'rkng:r^9u?\?S2W%?`gHQ5gq,2tr3?#Yod^%SAj'&fV<g%J=no@eNW,VQPt,'K,$E<c,[IMvgh^#cpvsudhIo:;Dh#$&<DV-SlO(#uWc'v"
		"[CMK$AMb&#;f_C]qtq0MHBA1($e]S%W/Yb+:+:IXVF<R)GpTW-/-)lK1h;4)_/Hm#0wIpf]wK6]gS1a%m_k[$@Oi8.&k5&4Jk.i)vrse)_g`I3h.]w'sQD.3rx@1(uLeC#9.@oB^n:Z>"
		"axV/mb&+luXWbM$$dEUGJZ2r+3:I`j3fwU#+aPGup=^O#@U7B(-Y9;6vn3`u)/3,^?J'/fD:cY,Hc@2'T;8H:0K)d&@:#cr4)o?4&A.ig8=q(E3RO;/LH*##`lp@k3Y'fh99_c)b8j]O"
		"oTki':k5H&0Dq;-W$of$@O4-;GnrS&?%Em8Q#j8]N,VK(&T<4`co3H&u'd2V($SX-GK4W-`8c'&([@DE[[Vs%a%gSE^+u]u2l`W#J[QH&pgrR[(&IqV^hp,&n3-^#jh[B&pc,q7Jb6>o"
		"o7Oa*9vcT.`c>j[tm+G%n--B;lgZp.ZH7g)2ctM(5Ji-cWp[19m6')W,q3s$pl?1pBDIVdO<_[tHYH[oefI)*#A7H*)i<N$p697&_^kT.5@@r[><+8(jqsl&0?5Y[ZS$E9Md;W[dNvA("
		"$BRhLqN_;.VsHd)sU39/Qd1T%%wrI3bV=P(eJk-r65Lee=3a<>k@%C?.G0;*%dSG'v+x(EABvnDu]g3&50*/MYYV/#J6JnL*IUt'B40$>@BF<Sb2Q/(g=dH.lX0E'YZ96/[b'C4bn$c*"
		"g0[T%CT`BNv)+Erj@6qMR)'iuHZMHg#x+2B-8fAu[15###Q=q$t^D4#bKb&#l'tT.C<2A.d*&Q,?=O&=mBj&H&uoP'CgF?-X*G?K,a[lf`tkT[Yim5#7<T<-iRN6h0q:_$;g:9/qVkL)"
		";F9a#2tGJ(7tIX-0;6J*T%1s-14G)4JnGVd25H%b`.#:birUvPR71##Nx-%bbj01NBPe&6VccTVe7XwR#27;o%r_-Ass;Jm0/Qru+?M1g<<Ck-<H$?X7)j'SR'-l.hHxT$))-l.W-4&#"
		"Fdf&El*F$'@/9163GXI)*K3jL)7q-4_c6<.%o2Q/*Sx*3rN7g)3vD>5l`ru5BKJOSbq/lov_lP]IoxV1]Vt+TK/i>ujjJDWtBSOJBSsL#3EBpR<D2O((xu&uAi=VMra-##U'89$H/'2#"
		"X<v?]TomM*)H0<-`OZ[&[h.Z#*r+@5QpiZ#.Z;W[wYDj$;M*eMPc6L'xoK6]'>tZ%U:SC#^d3d*hims-I(OC=oWOg1K4=`#;#d7$<#8uc3s;>#kXsErr?M*H_`D-$LmWjuCw`qO=w0wV"
		"k#,g=DaBPA.MOQ'T`sx+S,fcQ0AtmfYR<B.s(+s)u(#*@n`QR&GIu@2=T*.2d2C6)[^no@[lIU72#/9]0Yeo@?K@@#,vIi*S`$t-QL(N?)<TZ/wqur-=h9.V1qTcdO&RIqAJ3[#tWucI"
		":v6?u`M5gH3iqB#lqK)S7*QbUJa(_BZ*;T/T]m,/5ZYT.<@@r[lC@:(uPro&b8aY[pkg&(8:'^[Qr2Q#+(LT#vebv%tJ?C#_bcF#VC.'M_vYm#Jf29.Hp6K%M_vp@gjoX[mEZ&(gIr/("
		"gx>j[R%l>)b+D/(o5X-?$SHs*'$:2(lnWQ1x>Pj-HLb?K/[nW-8ofh2&0fX-+B0I$)FGWgRn&>.#urRB7l$C#l8wgu%7?PJ#F*7/ns1T%<%f6SUOZBTnbuZa^bh<>vFq`$`q_D8G`>-4"
		"%C$bEL6J$R.3i/gpl4XKH->>#ZueIq$:ofU;1&GVoT3uH<9JNMe[?m%1a6r%,r=X[6lg&(X_>v]B):v-5o%6#Cgma<T;C'#l.^mfb$lT[A65d2vjr>)=v5B)WI@3-c^.:7LR^-60W28]"
		"hTv]+=E?j[i<]2-Xx2m&bo;T/9,B+4`ohE4+g^I*@GUv-8H`hL&w(($Z)NX(-H1xIMHD:h)r%SSP_SQ+3*O@J63erhmb6gu&G<L#A+).,BW`8cj@qlbKY>7,?cn,#HZXrHGU4vctq$v#"
		"OV8>,JE6YYY.x?.+e8T.C@@r[g>p[).XqU@n?>Q^7F@[#P/wo@rO@<-Yt#w&h,0aE2mpU@ssPs.^d3d*'^-L:nAdv$^W'B#Z2Ov5wUnXc>W%]O07xeane4;b&SEi<*&-N<um)]XE2^#5"
		"v%@fqt-JBUtkBB_+(MSC/*Y(?C],,u6p.I'$G$##d8J>PVrSY,pr3E'see8.^I-W*c$Aj9<<@62UqVv#FEx;-nm8o%dlj`*IxYa*KiNd2iZj,V?6+w#_YS8&%L8>GB$W:2`x):27*?j["
		"Y@h;*ULQE<1c>s.v.jsB$PJ1(u<gVId$<,s2uA0g6x,E3.3l>s%1YCs$?7(sg#XcHYn5BT#>Zq^2qPpuMh1d;:H+I>6h]Y#7IO1ptp[##L;<A+wCp_.i-UH2%@@r[%A#^?\?B7w#2Ql:]"
		";xJ,M@r]?&N2.n/DsMG)<f)T/bY*0Vu7W@$;'Suai6qArR3CDqdw;ZTn:9EJa0XHrP@/DJO0vhgu)tpu3o@=`g5%JN6.>>#_(2q`HR<)<6wr+;BN(6;c&vC-_TYW%?k/9.FDu/(6YmJ)"
		"'WD&=+GVO^fXl)4ND#G4/$j%%$10T%J;c#@7fVS$odh(nr*nJl%j%7%9O/a@VTb*[H^&_[^$U^u_[p_N=Yk&IRT-YZ+15##pb'`0@5###Pww%#bQ+PBP6`D+@`I,;BW`C=/4*:g$F;:'"
		"]nS2'/dxS&)0QV[]OnW-]j/Zf]gdh2G/EB1:$nO(AOi8.rZv)4,nE*c7E-)G5&5;Ei0@bEH/`PH`FCaRk(KAK#Yd/V'XDg#h9FVH(=L.U[up_L;rv2b#)>>#+6YY#;#>6#GJ>R]8n:g1"
		"nV@D$+)V$#2Wq;]4W@iLXr;_8Lixbe/FLmJe)Uw0OU2ENaO`c)*r+gLm^W)#DS0+*J29f3i^oG;e?I,n]7/?p.xtXl<ajRRKRYj#t9DxOOM-M'L%KM#RV*HVWX+?V]8aY#TPe1B0W'_S"
		"wGW]+^.$9+x</_S0SBb*J.BT.,@@r[@OY?%NHWu&;+l3+w55_FTjh%Ia#h2'b_N^-lC#+%b6dQA,-_>$>>^HMR>hW/Fl*F3+jE.32j?a3@Dn;%Z^D.3gaZdF6SCsEl0%bEDjeN&aFl)R"
		"dSwcIi&4hUu'HfuuS<UG+[H/p%*&iIv##jb:5`P-46`P-TA`P-XN`P-lkQ[&g&aJ)CjvddH2Ned/Mq58>Y;8]C>)2'AYpa*Qr)6/+ch8.[j498DXo/1I0cw^g6qArCRg<s-_'dI$HA*P"
		"Y8D5/%M;uL_[KB#X&*#,<K_NcXM`9HWFRi.h0ST$EARi.W-4&#frgh(@t9-;s:hc+N,Rh(W.t.2-ZvRfRb>v](^Ji+k$/3-KMh*%S;tl+&BJ&qxJU_&1[t<8>xIN'I0vbu]gdh2Uh.ZT"
		"%rXe)0(C+*aZ$M;qSd&486nsu42MuO/&n77jv5M>aP<>7l<MwJ#tm5HX'&HsituwO53^kJ@<;@-8/(&$<QTN+#YfKlCuRu>:3XKl1`LS.&ko/.jr-W-DpCl0]nJ&#^/=G'On`Ad)>'.="
		"MZBXETkq[tO>8p45K=hP(64;aKrP-H,pKZRBaC5/)>>-N@bwgqqfP[->R6dRA7/,2U%$wM+TA?^$),##*4fp$j,%8#3EU;]If_A&HWXb+TgT:@gnW9'[vj78OZaJ2ZZ%g)/`sc)<D3F3"
		"s?L]$0'*igZe?%ke?2O#M@2Y#-'%D*&rJfe%ivxTT0S0o(bXQ#M:FPhx:7mu,On,R-V2$4->&8QfVfF/Koj=]qB,a=LQk5Xww$0C@Y;8]f/Jn$Th/IE>+Ju.s5Tv-PI(*4lU,H3>LW:."
		"kf:^#s]@h%#SG)4a;CQ9qX+saKf2m#Is*m^hDK$-4?G4o8=DSnTiJ0-w;3v,e>v@]pOu#3YtJU^UoG8%%&6^3uo4`2$=,ca5I#_$p?'Y$4h1$#1j6<]uX4_$IUvU7lw0B#vT5Q#5bS:("
		"Na%HuT]`r?pd9T7t/oO(7v2g(fBf[#B9cJ(%]k'4]_D8.G_'Odq4w2$a6Zhem&YwsPecS[a$_Od<I+`amT.;mkb[M'&&=W#sX@^0*4fp$a-%8#,'niLL3#0&d$&<-iP0K-vSiAKB%rv["
		"^2088FY0hP4-'t-++5gLXw:^#xL0+*i',f%hDqYGVCR-^X>BqYJBSM0]G<5/S7t$klUD(DqFGXim_MIfH/7$`n1,#3SmHhaZ+xmc=d&s-b-Z2/0/aL#DPYw^-lHw^'G18.0W&W%sYc8."
		"bExM-9kn;-8tfN-+60'Ck>X2(QsjY&g;K6]Wd(h.n6Rp.9QPk-<wSu_^>qYG/:2?X$p#$7[JTqYK6sl/`oTP/dDJ6a:9uRk^aE([;OQ1eX$ejas$lj^luA&2X^C4`X7KGf5a*v,ckkW,"
		"3&NL#9^+##>JSvLY_gT$aHY##Q;j]$&'>39DQQ?$K;Rh(@NL,2qX#'4tf_;.>lIu.<cxDrN`I>5)K,@JBDGk=ip&##KUjfU@>DV?t9Zqpf<=$'>$,d*)<V8]kXJfLb0v2$hGg*%m.x0("
		"_3]W-%qJU)vbtL(2>Dp.F%xC#^pgX'7u.>.L/'J3?DXI)RiXV-[RO1BJocE5uO8[JCXE$NI.JJFu2SJ:iosK7jqE$@,L9S`<)G-@T;D+ins,N02L+#c]Qt(3N7Na*7X`v>aDP81c;d/("
		".t=C&Xddh24NAX-4t@X-gi]WupZwf1FHfr$>5F>P3Xfi'g]Rfq?DV)+nf_kr?J;-2r6GB?v:Xb+2amu6j)=;&qum8.>9rZ#wwPZ$ufI0(BGVT.F4K6]bRR7VoHw8+7x#4%kF3]-(L-.M"
		"i<V8.IdDf<UR9T7p?Z)4'5C+`:j5fU:pIiu^2#JfL3jrW=XRB:D/V4Q`UUFsEw*bMR3/8_Z_m`J%/5##v;<P$?(t1#;xL$#qNTW$^M9-'^b8-;qS5'veY`/%eYslffDf9]`+f+Ms.-/("
		"d%vJ]i7]fLO^iM2m/0i)6o0N(JmCD3^,(:%j`Qa^slE&%pCGXuJNKV*of#YcLne/$RxM`##Q?uuqQYb#cYbi$<b(?#94-L-:LD>p:8)-2o9%w,&=rA(.*%1MR==`#,(nk$EfF?-r8nd*"
		"AJlgLeNpnLqH]YMtK9_MUB.eMA$[Q%(u_kL6B^nL'j@k$w>9C-JZ9C-9.PG-:1PG-Q:PG-*lboMKB`tOl:#gLG:#gLNNc-Oe`T_MZ#+fM1pKb*J6#9.$X;^(L.C,MUJ-b#2WLXMEPrtO"
		"GM>gLPL>gLaC*T+W3r(#4/odu?`K>$wxw%#j0x@,Hr+@59I=w>Vt(%cunkA#MWOv-];1F=H`M8]wpH.MitZ$&kUG^(E[%u-4/mOK87h#$.^Ol(L[V58oW)B4/%hDNRk'Bu,vO3N]hFqr"
		"Xc#$;Kum.Z9uGoCcXH,$bGEuYX/5Yu7Xh;r18W&C@x-7WlnKfLEv0au)*1hL`dZ##OR+Q#O8h^Of_[`*,&]2K(L$9.Ad&i^U=<Du*&0Q$=jvk$C-aY#d:fIq8gPJUUoSY,(()<*:Le;-"
		"[0ls%#/f^=Cxk&#uKp5#<Vh58+crS&jpF9.FdNP&0om;-r5-e%[s1/($M1h&m2<>/rCba*NKM017.Ds-Hm7f3N'(]$?'G@74G*^4C1Iqpptxx$*s]auBJaCj6UuN#/r=Qu'D8tux'0et"
		"</q>Ng]*@#>etg1d;)^O?SA+rkpH)(KK#r%..M:]@-<+2N1UN$rJ)4#`Qk&#=Awi:1b<#-Sch;-0haf'KJAS:)+(J)VL9k.C#b.2bF%VU)m=fG:&M)3e*j8.<39IX^)HJ(=(ql8aPT/)"
		"xhS;%A'2+*e&Us`+6kCgN-B[NglBXQhwK#V&^#?g.$'xK,*K4O[6#H#M(#BptaEpu#JOru&nNS^a7.mI1Ba,]FAk5#I#2QBCcscMM:M6'.@?M9f#i%,iM`V[:oMq%-2na<_8H+<A'oUL"
		"lDeh2W:18.M,.&4CqC.31Z?a3w=uA%/7qj%CL3hu8TI<rdpKiq*C&tqNY/LTb2E_SeCW`u`t<oPNVRS#]Kp`PwuU@_EsuB1w<N+^.<&M^W@$8Rjj/s$jL(##DUU'#_->>#993)#:p%VM"
		"`ogtu3/m<-_L,hL()C_gf$;YPm@?>#le>3tYV2/UTn5JCr1g;.4kAT.4@@r[Ou2:%FS`$'KBxGXsiK'F7j<#-joC9.jg#g):xi8.shXg%Y(DW-&^2nUg1;Q9@'Fd;LGUv-(o0N(JjE.3"
		"jA@)*ig'u$;)MB#Et9k;pN5/>7#-x[ag=gfw%U0tNt%O[oO&%3%](s9W1;>593b]###dHB(I([*oC//ecF`Qj?B6b#/6Vc#NOY##4####sMJ=#sY4q$8*V$#v(YB]I/+/M]*j0(2xSq'"
		"7@40R+K^S%@u$[@2g>,2T]a>#v*d&$cAHpF?`<#-k:-/VCpbm'Lvr@]n`Q*&.w>j[I^;e&xkJ2K8,aW[Y*&Q,3VNI3,k9l:aiih)R:M;$8u0N(I5^+4+87<.:fOg1Nw@R#kLpi0<f2lf"
		"]bH]WXrS8cI-uxOLumGZR/.a:d'IoYDMs77FW/9pZW9]7jA>C#MQ*##C4Z##Z/#;HmjQ8`o]e@`amah)CKi&(AV%e*klRh(TrW.2oP@`(c&6j'4F_>$EguQ/gx>j[e%l>)<-g8.k*M/)"
		"c5RT%j_Y)4)a0i)]RaCnW)or?1HG?AaG/wbrD%T$sQ,]O(QMr6XB^%AM-1V$`>s.QQh@J1<('W[=jli'Wkf/:t1([IIwCt6HHm6&Tm*J-1Lo'0YP>j[NZ_/%<Es='p:+U%iH7g)6PE%$"
		"'L0.h8kxA$Z(=Vf$_ghp9eJP#9UlouR.GuuPvrs#Dgs)#esr@]liQ.%dlMa+:@Np+@<a2Bkl[a+aANlfNB24),=5,Mv3.W*@nN.2=n*W-h.J1C[d;W%LAWq7kWXb+&:DofOG?L2DZ)F3"
		"HN`hLmL4I)oOV='sM[)4l$;VZTv#_i>e5n0V<-on.skG]c[P@t'j4luA2*;&-P$&45<e7X5U(vu5=r$#rT:?%,e''#I,>>#pqSE#rU&P:+u^>$E;F?$50Z][2:sKN]l_w-;<D:O=Xjn%"
		"`.<Y1_5RF@Q78]b@+7m')QJ21T,$eWwna]42w-S[sHi;*Jb(^#[pH,E;5h3=n@,_H/*YG/3,Lp+3WWeN>'2pNrqWO#<Q=.#*9)A.+KSv)PAnr[J*2'#bq,V#whHiLAMpduT<]p$V`pO0"
		"J####O[6K#lU@+MJih2#%9@@%pX`=-qH(]$0O)W#1[_=%FEXb+8#*v6tBV^$_l_w-_)v*RZ?7*N->(@-hmV&.1-)+Rc,B$PW3@aN2`>.#s_Tt7JA44O<pVk[$[Hp-tmdE[xW/X_ZW4X_"
		"f7q5O%)^fL-3#,M3a:)NHiE4#-Q@NO=LM/#/H(@-K2(@-L5(@-RDC[-I3=X1I`S,M5fV&.5GPxQ-rU)N^Gl$MS>HF%icWX1h2NiB#a:4O4WVk[Gp6e/c7(##svf/17kI,3(B#dWr1DE,"
		"*TP,2mv[k=[84*v:)H;%uKx>-vdXv-:/7;8V471aW<98S>UiJ#)J(@-[XkR-][kR-tP(@-amV&..ri3RNJD&P]wruMtiN4#P9aM-%^Hi$I,<v-T*)^#>oJ,EAGvPh:KcK.[6m>2CS$T."
		"<1ST$nGqS-@LqS-AlHr;qI0j(o3ZY1mn:S[Qj<s%-?]/2A=,G.[k,4=?\?No[ZEi2D/?K4MP=(@-ec0o-g)A^HTNeE.?cDj-.N/<%Sv[fh`tsY6D%$ZH=?\?(A(m,<-k%;]0n'89$'JYY#"
		"inINMd*:*#j-BP8sU`H*6r6W*10Z][4,`w-i4SnL)ugO.FfjA1IXcA#$#^v$JCO_QL.8bOlVx(sH?tcW*:U`J((]x0oE=x0(8l/#N+-aNQ'EejT4R^$P=4R-[lKk.S#-;#:/7=.m)Fp4"
		"%+woJ_/8(#ahg:#IH)8vrQGbu]mFU$+Y`=-R-&N/O[6K#4T@+M_83(v%U*t$2X`=-@khx$^.Qd47#m,2i_P^+5p_2-xO^Y1Q#SS7nqDs-N51d8IBAF.r+.4=?m(JU:'IY%V09dbP51[-"
		"SE=x0];<%8?J6x0NPWk[(]tn-Q];qV&ax%t*a)&P.Odv$:)F3tFKx;-rt;M-Ahe(9dvH#Q9:_cDp:lYHn8l?-I&l?-oo>q80sel^Yd1jCVR/.?Bp(JU$bU%Ow>7v8$[txYgx]AGNBtX1"
		"S(.D<2CO&d#5Qq8OlYsIuME`s&pu92%Hk?^HsIG#2w-S[x)b5,;Kg5/d0ST$>#JfLNpUx7.gIP^Kj`s?$eJZ1g69>>a,@QqG%d%PrFl(NH]Z##1Rc2MD<5x7Umt?KH1q?KFV<`WnY2I."
		"-/rG/xV/Z$:_m-.<r5tL&09W&^f#^?.IWk[+vjh2E^'8Iag2DsF'oY?I1=;[RY0<%&6.K%.XdjD_l_w-n[bkM^g?m8>IAF.K4p3=:x^G<S@gR<Z/oGM[$suMTV6@JIs8aNVN]$&H@%G;"
		"-'`-Z&eZ-Z'R1)*IuxA#,miqVK2s3=HB[v$XkIgN&x(hLN,D-M^5XbjS^%L*?E:5o*RAT.SRUR#/'Sc*;8N1p1?&m]Jc$9.5lI##jPg;-`3%+'7x`R<2dN(s+Io0:^%DR<Ht?32CO*4#"
		"WmNg(ON$F@X/5##6?I`$qXf5#Re[%#(l%D+e^d1%:i+@R7KihL9kf[>wbKW1(xSfLVKx(<0h7p&79]p&)B;>]hW;<&fjjT[rW;^(0(qI)30Z`-3,j@.rKw8+9%9U)i:SC#S<Ss$]WD.3"
		"Euh8.*W8f3,eK#$wZ/I$)ut]+7xB&?eNDk1=1/ZAI8$Ju`SR@tYCgH#SNcS[Zgw:A>+e[PQ;aGP$/gau.B/5uFUi,#YGqw?tdD]tMPCJ1WTu@Xa$U`5WQf2(^PVwgNsBT.r>Q][S*#O-"
		"CZQx';0%4+Rql3+r/P0lvim5#toWD*Rr^GMau<d&*SX1(Y+NI&r#vj''iSX7g[l$,Z3)=6GC'^[3wKg.mV19.n%]#2'(/9.#Jt;3c,0K2r.NT/pr9a+qD.e2qF>c4N;Rv$eKx[#ZYC<H"
		")OZ.;Y;iP094ds6,tfH-j:6(AWUuvI$bN]_xr:h=[IYK?R[j%+`3Tq71c*(PXL;'6@mPV9w)>>#%1ST$VGVn:oF$C#KChJ#0B#+#>aV)'*m16/c'89$f[>M9w68F.uI$##2w-S[,VK32"
		"n-kB-bIK$.ZI'%M;-#1:I$i?@*kpC-u2m@.FR/_0Bg=Q87<w8^)EdD-Rs%^.ov.P#':&C%aHk,bIGJcr;W..ZMK:/(^gFRNLG8s.QTIgL-<50.#aFwN(@#L%t0wiU1;-f:6[8WROQDMg"
		"_TS-HvF$##uxCfUt2VV6EB$W[tm*T+GsGQ/6m=Z#<wv.:MQ.C#$[C;$&JY##QM?L20'`=%25YE;jaR&G>Voo7W&6l9JPio]7qas-W=6,MBhl,27gF-tP%-dN5P*4#K@P%&iRNd;Z&DR<"
		"W/hR<KE*v9xTF@p_'D^+vf5gV/DKM9<DmJ)4haW-pA*_HE_NO#$C0)&7g0<-+%Lh2da3SRp:_G;I+^]+hrJ^(p-Sl)ofV''KaXb+FV8^d:+gK%UAPo8B;iD+Lw]B&]I]_=<c2W[bEZ&("
		"9:R=Q(@ZV[ZjIF'%2tM(,Zd&QKSPf*dVQp.a6ol#bX+<J4v+LujJSPuuBic#cu@#eiQ`v5jD_S[^hk.##nJ]=I@wP&CK38.I$Y%X@N'O1$#@#GB4D)+_HK)+wgK%'T8K%.W&Zm#Z[Sn3"
		"Pn`39*aM8]NwU-Mrg:02Ns902t/Jx]X>m,0p&Y2Q__>v]H1Y7Mp@:00xR5W-)ma+5vhBb*psZW-meS_8vlq'@_mdh2E%d29*uc8/[0<8(V2(;E$4vr-14G)4n((At,x-9'qnRXf/PAmA"
		"9/^[6@^$jXaQ$DC+Y-Pfl;L<Eg.SV%wgs:'I%Iq5S%9iukCJiB%=,i#@a.t(Vr4x7iq?2&.Je+VUB4=-6x+)''f*30p.&##:<?g%`ZEdbr<Z*vbokQ%Be''#O,>>#s+d3#xLZ*&GTXb+"
		"NCu?eO&ebu:Y3t$b%B/;Qa[v$D39W8),b-=N+q,NHFdD-XaD&.HQ:6Rf?RUM4rorm]IUe?\?2TcQQir&PNJddNA`NO#cl84&o5PRNVv7Dk.J,<%Ah;5/``BaNXZmHNFWF@t7Y.#c.5O'#"
		"$VVO#qC#/vB^*@9&L*VNH#-J_ub3MKWbr;-H[D#&kWg:%(U=Z-1Rl2;m#m18D@Z;%/WqH%x+r5/J?uu#Y`LpLCal0N5tf@#vepC-#XNn%S_46#hxUG%KJwG<2//dWF_pr6fl;a<D<Z;%"
		"Iooxu?F_w-5'GE;47/dWCot[trM<^#mx?WfYi5f_&sR>QJcH4===4D3elMa<qrkxujipC-$D2].]4XxuI$5P*QUA(M4+%C%LmV&.c),1O9ViMN?<.&MtJ8`$_oV&.g3OP;ZpZX1aRI[u"
		"wiRh$90d52xLwCWn=P:vuZ8]=:&OJh0djg;)Yqr$Iw#:#)+.NUs@*DWUoSRNX]xx+,lCaNIOUH%3kP1pZA]S[w,BJ1kc4<%@e:58e6T?.CY(($En7R*-P,<%))dP&lSkN1kh'vY4-A5/"
		"JX=p^^9;GD`G]p@bMi>RW#?muaYNRMJ6vsMFOw10B4ZKWVa[u6vP%?[6SLMujpZIM1Ch0M,sf(WS#b7RK>v1KUmcA4uCQ-H0a[R*bBHR*38(##5SwA4c2bJ2G-wYHbnukOxcHd;[x0'#"
		"Z6YY#K<suLG#;1N80,A#P))u&s+5kOSGl;-RsEf-^FGkOxSR&#Xj*J-WYh`.r^xX#5=@uL$f%Y-HxtkO(L@#MbQ#D/(tJ5&1%'5A,$DSCWddh2vC/&P:q[;nQ+-5/]#rtL0W?J$JPqW."
		"'@@r['^+x95vM:M@MMO#G3J?N#K#6%ahN^-on4R*WxkR[X+`8.'&wA46Ngs-rRh28rbCT1nZjR%n;'##%5c##1P2T.KChJ#j_wq$aO;QL<)5SR7ZvD*Z6+4=blsA#CMJN/BqX:$IgbkM"
		"p_`=-S,w_=B;u`4Cc(jU2KdXL_P0eMkU=D<XH$jU%Xad=FTD2%@``4#nBH#$MW<<%'uo1B<-dp0=c7.2FZ&r77dTjC3s3Z,s>JY&i-v.#nk1&&a`6:.tUul]e4`s-?;oY>@@%a+SKO<-"
		"9$?%.[M)'O`cL:9%d0x7)fc]<cZZJVnLt'HHMG`%&/pGM,=[&4$/;)X=f8r^nw$m%JDWk[@o'`$&Wexu&5;)X%cC:KqwS`=s[8WR[i5Akh*o92P;Bvng50>Gu>J(-FfuD%nD'##1FTk$"
		"B1DF.u$rxL>_Xv-e66m>taXAuZE;o$'u8k2#Me(W$@`7RsH<D<'*al]eD7R*&i?R*<iGQ8/6GDkb2LFIn-d%O`JD&,7qn;-xXU).UFOcM6(&s$3V38#>CcG-Vp'W._0ST$fIGO-E&qg-"
		"(d8@9/=/X[s8q8/_'89$w$JfLgM%2N&@WiMxO,Q914kw9Vtow9sa[uP^at?Pi5t;MH]Z##phbQLTLHv&2&A#/G8JAOBDi?K0Y9(#O[6K#l'>uun9#bu-jjP$g8'WMK'U&v,n@u-F>#xO"
		"qRf(W?<a7R*3R/;+]r:2c2bJ2^R9Y_54#AXmNY#>^v8E#29lG/jr%##`67#Ma[:P%i34eHV8bS[:Md&5*c68%E3E;I3lYW%:*]YGXs6R*8vRR*</H]M4Ai^u%p`:2)`&aONgL3X:/k48"
		".O]:2kRq/#CMJN/TG:;$B9;iMZ.A8%Z`PlS/#6=-)Jj/%A@K1p,_G,M%Rc(N81MoePMT:v<rZX$Nb@5/]$%##%Y<`Wuv+Sqgd#W%RvU&=oQm+sL=iY?Qa[v$oQiF>isR5'6ZkDNMK)a-"
		"K@xe?f4p:Z`1IY>J.NEGu^^C-'&?%..w^G;.cB.X#e#`-tw`R<vke:mT%r?PZX;ANRfd##D$@k=o[K.Zi.1pNDfWO#8TKc$&x.X[1TLd%'-Nv$1'x:0U/?;#dl:$#5xX(/PiC)4jS:)X"
		"Uasv@rn5]%<pVk[Aojf%(sM[$#Kv%.$*N'M*PToLUE`hL$i?>#)rk@OE`bS[D>du>*j)d*2>MT.7@@r[_kS;%gntX%X#la%RY#^?Z'^b*2Ql:];Pvp%^1f/5n0Is*C6Ks-@)CI;3HH@-"
		"GL+F3<]WF3j6+=$x,ud6.TZ9%7u6>#:=2oR3.HZ$FVqqSsOfZ?jA$X$B#aE#A:goe*J`]Tx&LYcTt)<Ip?-DsJ)P9`#v>T+n2(A%93DhL6wTb*@iQ-MCZPgL#^MK(>p*Z$b&@$-o@#nf"
		"nP/X[fhNQ&m&/oAntsY-Ea?'=Qf)ed72pb4%oK#$q-&r%Pn`DECjn>r^H22r-MET%C5[WB6>Po#If9<fUB#^t?+>>#YYqr$ICN`#/2<)#+Pc##[A8`9LCS?nrnPV-JwTSe/#6X1RK^X1"
		"c:q8K/_YgLh]JK:gW@5DaN/a?O7_MUm%E5Dm'<Zux7s^M9?hW$Jf@(=G*W2i)EdD-`8nqM_`>.#X/b(&(G)9./pk8+r=`5/xG:;$pm5lL5OaMMtViJ#[WqU;#l6-bM/e+#B>E1%H2(@-"
		"U?(@-L;?q(d[6F@;wPF@T$w(E?+d8pT]*W9:8,F@[<(xNC`NO#^Seu%N?sDNQQvD-amV&.M<QB9YFv;%VQS39U.r3='Mk(NI`Z##xVKfL$-L8OcM]Cs,aLv>f%Ge-pr8_fZE:R#Eh,&'"
		"Qr#^?bhET._B46%F]^C-*R_w-A4PY>m0N;7[3$hDA_Hs-(`C5/OdEF.)x5V#lbCu8%&td=Jvd'v6#,^$G5(@--R_w-S-Yu>xS%a+P@GLNP01]XQbiM021D5/=bEX12f#7vRik.#;on2-"
		"`pi@]2L.a*2Qv20a+HFa$;Mx#2G^t(#`?dAeD4Y^gk(akbvi@]9&sQ0jodkpNb>v]arKkLXjdh2cq3iA@?<jLH`IIMSH@&Ml,w>#H7<7#3Ux5#fM_5Mx.oo%&5kVRTpxjkJ)k--?J^l8"
		"_6ZY,H`L<-7C7M&xFuYHA)$Y1kwNj9?GQX1D6?F@?m(JUdQrY8Q7qw0ao6x0sO0+-Ij]HMU?15o7ZM,E_E0_JS7$-Mj#M1&SZIv$'*nt.nko=#4,8;29gVk[oHOg$<165T_/8(#bq,V#"
		")>cuurQGbuZ=GB$9e*c35%###O[6K#9'>uuDCU/#7Q60%PX`=-cx$#%&*NW&/]SY%['AvP>UiJ#'UNe*[]>Q8xGQX1D6?F@?m(JU13[Y%eZkKaP51[-rJ>x0/hud,D=>x0;*9<-n,NM-"
		"QoSN-B&pj-T2aRNFBNCRU/Dj$?E0T.S6YY#Wa$=.];Q8)VCG#$J3M#$8%qw%qeUxL#&>&%94Ze?Mjv(*ZDWeX=l;fUVB#mBfVWX1ET[o7o>FaNnhj#&k2'##S3(H)tLvxc,aYdWv+tu,"
		"2w-S[>NYT.hHxT$:>aM-afXv-rUF<Ooa_S%P<FvP>UiJ#=de0;^,*N3/9K4MD,m@.<N(hH.'iR<I>T(m^SF0MA:1[-dGk9V0C/X[.Ln;-H[4SM_Um-.cld6Oh/Z@.W$6^1rog;-RZ@Q-"
		"a`@Q-ih@Q-WHt//'@@r[Eq;(?bl_w-xNZ.OXf^a>CQrM%3o3qiE/wE@JwVA>hdwe?*EsGMfaxvM7a>.#n6usN$xSr%L2###DZJw^.<W_Hx#KJ1<]wR8x6p;g+RGgLb4Eg$;m/o%:N@'."
		"%%J>-F/+k%+#2MT1AK?$#q1N$tqkV98O$(#-SuYuh&)K$rS@%#_Y)D@fwN:I)(c`6Tag5#x5kJ@j^N8]TA%8@HsF&#`nT11@Cr8.KdK#$gF,JhrIfr%HgRcuBhMX#7.rx=[c*lt^`xV%"
		"7u@ZoWvaLuY3l6#sH<D<[roP8i2>KNU-A8%R`aiB`ev58hrxS8G;KF.?m(JU5P-^(UAm>[BKd^$r8v[.f;p*#bV1E%dOZgC]m'@Kr>'jU2dw9g$3v[.<H:;$uNW^/7.>>#lJtpsBe(B#"
		"L/B+N2qZi$Y17EYC;Uo2F/u297K_>$CkRJVhJnuN,ER>%M],edoiGs-lR](OnF(@-Qsa'%gk<]=nYJ>PeWsb*?)]W?8=8a+^;c2=gcjI&Rikw.8shZ#6&4</*W+Luc*G4K`iOZ.7pV_A"
		"#gvnuLaB)&t$NW-.d6K3Gl;:%8JfIqNmH,M6vM7#*JY##2o9x0JQXb+:#nu6K(PjiSJh<.PmBB#Rj*<%`+MiTn4CaNKT#9%PK.Z$MY_w-UJ'x7pamX102r5/t#uU$=A1tM).88%2'w(E"
		"Y*<w@aKVGD)C%)b9u_SJkHxQ1H9e68QwK#vb6YY#.GUhLR>X_$V;W`*6Q;v>MXSSSE-.Z$p%u;.N2i$#1G6N9Kskxu]=C[-7&kWqh8^JV<jMXq<+Rr^ed8[u^H>J$b*S#/xLwCWT^F3b"
		"Ox:w^u]txt8F1B#rx5ntT;9aNm4TK%]ddIq&F^?pjal34Ap(JUg+-lF6l-<-&QA3/<KI@t.gOkFft)###Me(WUY4K3A*WJ:/A7AcjS]v$9ckp7Co5qTM.M*%d&N#$W5[mLlG=E>QE^vH"
		"fpNSRELZ?^34cA#eH2sR=dnFlYpnu6u(wt7ArR(8@MMO#`%xjMtH;,D8#LEYoTiJ#b4*$);W/vHND@/Dm0sxcd3eVI95hs-8k]cN)Dn3#)v+t8*%v^?Hen&Mi:I]Mv%^.G-0_-%E_ZQU"
		"/X1)Nped##`Po@:[Cwd+q^e(%nV_w-JeR+M-4<$#jD/X[vt@b%i2'##$(F'o]g9eHoZ=VHh(W^Q)NG,2/YK&Za03MM#&Us.#Me(WO,Ykk6#6kk8R:B#L0l<2@J&_'w]eIqM[k'AIS&(A"
		"hSn'A;#O-=JTx>-E=Z=Mp[Z##6NiPqd,@h%JGv5/Orx&$:C1tM*788%Bb@D<#)P:vm<b^-)&i3O,5;9.C#uU$?tvGM[e'KMXL=jL3aIIMWH@&M+i(P-/)<V(Ch1?%o]l]ugI[mLQ8+)#"
		"OKNB/*IxT$3FUhL,a*^M/&91?iL7F%jqv58'2hw0OIfvN2ZKl<,N'xn&^ps$>kVe-*`0)Nped##8P7(8YATEY('$`-[fqX15R^4o,2ml&X5(,2wwVk[kt*(&G@q>,C69)XO_YW&EW4?-"
		"'IEe$M),##fX8I$UpMuLu#.?]2HaSAx;2H*6r6W*d/c/:x)S5'I,G6'-0Z][r>Q][6Sv8.AHr$#_1fV[+BQ%f'Dv?G^3BK2mnat$7L6n;Ir29/9,]^$qJ=P(oCw1KY=l=8jx5ve3NpgK"
		"(kg;$,I.fLoFp%@GJ+^4Q`qfh:=5>u)qc9h->P:vKF$##3jWv,*UtofJ+dd$[7O9^LDPD%.<E,Mqen&M$]Z##,mxct1HS_%ADtGM`?15oZ.tw0T,fP;+L6x0l_*6/h>uu#i/[iLt'X^M"
		"CViJ#k#T%)2bmGMt7qxtG*xfVD>7-FN6uW%D2tGM<X'DMR]Z##PPVE>w3sx&GXJmCIHE)#ALBVa#,Y:vrG:;$qjs)#,N93C*Mc##TA31#52>a=OM)mJadI21o&DAP?rq^>(J1v#=E/5/"
		"CNZR*5>_Y#vhSxXG'T)3+GhG3$.kB--D:@-vL,W-SjHR*6X=:2N[>R*$FR#M13jaMEGL<N9j;aMMMc-vP024%;f''#<.>>#.@o1v/[2<#'1/B&Wf#^?$+Wk[xAOs*o(:+Hfds$#pBY^$"
		"1OY##6I5w.c7(##__sw'VR=x'#Xpw'UCow'5>g--dorw'k-pw'PQ.Z$D.m<-X.m<-:aCH-w/m<-tWS>-`@2X-7C$.-rYF.-Ccnw'__ow'G]N1pf3'#c3dbG3v8)2hqo#d38A,HMe**MM"
		"1F5gL.;t^MjHWMM&f2i-O2vw'hwfw'GS&D7ATW@29#3lo=VFp7Gsk%=7YmZZuj%##2w-S[:s:g..g5T.u0ST$wxCj:8tgYZT5fl`aTtG%&a*MB*98F.B-Fs-Q;)W@R^gJ)l.wAY&,82B"
		"#$`DlS_Z##fCe/9ajsDYMv-tL6Gfs$<4?p/D.>>#JCiP,b2B8%5a[Z%M;Wk[-#Hm,O4rp8c<n^P=XQDEjj/s$R'<G;Ad+pob`>)<(Wo`=nSEvQP#Av,^o'vHFGm,*V>k(j1A-v?K+=#-"
		"iEEVn3SdV@,rkxu:qM..`.TH8A*Z,E)r3lopjIa*tN'9.7>-Q,aj5_]<,dtN%2ekETF6eNFGtbjoXHg+@LCp7HIuWUPPc##5AAs7v#q>$,-EElwp5`-5q/4=N&g5',U,90R:R5&*&aw$"
		"tgt;.G.>>#CtC^#s;RBNEG:jCh,)24diHiT_,v]4=0Bk=wBXk=>#WfLhHYvGtEvV%uOa/1_0ST$x'r7#an>?]OHXXA5su,2n3rv,G;l;-R?#h1<A#^?S+vr%2Ql:]/0RiLYS[,&2jhW-"
		"?g4l9EU;8]mDqW._'L6],`2A&Mc[:/W*U7JQ%xxumBl^$*nOU2rZL1pl*dS[Y*]f1#)P:vjL(##@Rfw'n<q.9IO`d*3*A5/RDHjDe.M+?#/6$&5VM:%ov1lo/&#9.&=q(kBw[JV+<2WS"
		"<GV-2tWRw,f5L+mKsaw,V_l>$E&hK-NrrJ)[SJ-ZBAl3Fha&4FAWf(OI3qS%Zo4gL`4b0(^p439Awb?K7U6IMg9?E9^om92?<]p$EOY##(pFL)h6tQ/k7N1puHE)<:9H59N%?a=uk%Q&"
		"vQ_M:ha5-N931g(*mF5/gV['f3aG/vT&CH%[E,a+1%A<-w.8s8`*DpJbheG2kw?&>YCnA-=NO;.o6E)<=REdUP)>>#'X(($:Lj`#vfVv8:vF&#eN&I3S`'##mk',r79uOo+i?8%[`q@b"
		"h<r2B+EvV%_Vh5/_'89$nJfnLE<DPMjehsLdrvf$U*m<-#,m<-Podu1ReFmu$I;UMUZGOM(K:2#<iBb$F#=J-s`#pLoPhl8&Eh4DU^[u6+v4?%>2(W-j,>X1Lj0W-gj=Guw:X?%W+i;-"
		"qb^g%>EgS%mkm,DZDCli>Fbj$RFgY]=-UE#Yhn#)nB<G@exN]X9.ip&+da)'hlv2Qn_.g#W`*W9hD/'-+QvD-2BG0.:dv&M_=<$#`&/X[)X$E'mY]GMj0VSe=mM,E/nAk=`+g?70`,Y@"
		"=p=p8=WvkX7s0<-aR$6%Q]rT.:G4?Iqivw$i#6N0w&5Y#7Puu#B:cjBv8tY-crsA#CNOC0/iQ:v+[1GrgbQ_/HQ=_/wJ9o8rgjUIhg/rI%+/t%f`&mfe5/X[N=,$Q7%4;dT935/=k>e6"
		"^fUV$J=$##*nd4]8%ruG2nXT.VQ/&vofAV'/+I##:#^##Dkm%F'Qlp.3*A5/KKAWA1T/;Q9)1<-64T>%A=9E#u&80#%RBsLSp('#H?(@-bosn%#2r5/Q7?c$gT_>;5BA=%Eo)5J&2P:v"
		"C.2YGuHE)<xEG59(TlDX-n<8%lQZ=.$G4W65p3?[:-UE#ubH#)EgHsHQ)5p8GA1d+vqd;-LY*Z%N3Pxb?dxEN<+:*#]J:pLC6[##1(>uurQGbuxNaH$fMY##c:7b37G:;$r:Pu#cbR%#"
		"+Pc##sG,F#WBl<%Y$/<-gKuJ%dm:*M@T1Y$soBB#jc6=.D,Culi=`N(0;>^(10Z][.kq)3p0[[%/]>+#Gnl+#*Mc##B38VM&Npdu.BFa$LZ`=-ib`=-ZT@b1mhCJ:5ktiKj)F5/CGER*"
		"PU+Q#.0YwLWa6bu*E3P$*Y`=-'$Au-+5cwLnCpdu_']^/VPqw-#[_XM61T>Ir&5R*)EIR*&HPt-$N_pLUUk`#2A<[;.>H,*n6.mTG'kEI<BDwp.@ri9)$S5'o3e5'[+D?=S.X^,02wS@"
		"]Z=X19TYX1dK.(8346$A(Vi3:;%7w'/Fx=H#^?KP:/<F0:8N1pK)6pI(LJaHec]S[Wo.JL#)P:vXVM1pWHx`<wamPh7fh2#%9@@%M.Ji3f3fp$%r#r#:rA*#+Pc##Qk$S#vQs/'Rr#^?"
		"nk<.&L/K9i<IaD<Q762U:ssw'roNp#0gZw'OJY##/ZgxO(pP%P`g#X$FB39'eque#MF#v%JN39'GW9cjuoLdk)s5X$T%qp'ekYI##up-O/82U%K,AN:?2%jK,=bDuGfe2.E]'LND,nm%"
		"I[8<-8ae2.Rb.g8)L8q`]w3j=HPFjL$[b/Mfq2ejQti,dbd=t$LjKd%F8)VmDXAN9E/HE506Wb$AZ13DS_Z##2C&.:ouw5`(]7k:FF;s%;cA,E1x[7&67nM(NABdMMm)+9&uG,Ea+?K*"
		"*Q^S%h,MjC3s3Z,wHbC0/mM1p8@/pI7haSABNGH*Ja4s$8l.@[j]ID*Z1w--Uo:Z#ivgS/@Lh(EsoVY5?AC68HR$xKap2B#]^w*HM[3K:)CBdQcN.DElCwMqk$@5/fL>db0BRV##3J^."
		"2@@r[:s.B.^?2Z2U6+T.e'89$9bkR-VcOkL-mniNZ?nY%QYWk[-sv9/L*JT$E@ua;jaR&G][po7[XW6/'X(($R]L^:OT@&G>^]f-kPf;%r<9$.Lg@K8>HqW/7rN+&=_=hMfhffNIxsO#"
		"fJ/B..0W#)^Wa'AA,uuu/N/<%jh*GVnd.HM-c[u6;r]:2+&(##XhIq`riu8BYjpi'i;;o8'H`)+Xe,gM^)0-#bKMo$:cDE-oava$NTq8^hi&j%RTfnT,d_;?ZC-5/jo9$#x/BJ1CBl34"
		"fLh(E&Our6WQt9.OUtD0oH_#QG[XVdLgLk4,48q`P6[xO,jmk4^6@9'Yog_%H>Wk[2(>S0Fk__$,lFm#^%mlL==*?#Drh7##)>>#;bCC$:&ZlL<c7mM&O#;dbm'6/.GYY#K*fNMW+:kL"
		"Z%,cM:hp$N46k]FUUtY-9kZ;%N(<Mq9LL#$Zr@u-kFMZ:))L>ZT2Nq0.kjZ:*/U>Z*i]`*iZ8<-JGii%Qq3W$8iqrKPWL=%;8N1pXF0dM,kBT.b:92#dem)%*c*t-X*tVNJmKi^Z?`V-"
		"bAuKNfgbG%f5Hq%/7N+<9JwK>%fP3%RH081+ah^:rn:$#vi/X[T;2U%YS###v:9sZ1nqP^jSpi0+5K?$#q1N$_LWPM/jBnf/f?uuBuXWMF.<mNJA=<<l2?D-K6?D-/*?D-Y]V].Hr-S#"
		"Wv0<.h:=d&St_,MB$5&%lV^$.FARLSA_5wNoZ]#M&+G8R9)C]tG(mX:otatfx/w]u_V:?%LU#@.=%*JU=G$E4l.bo7lFHv$hB#w.<x)JUg@6L,xa*q7+lrS]DXNG2`QsP'B?Xp7L[Qd+"
		"&__,JMa[I-lc:a%dk6&O?QtsW21hdS)>Ys6V@@T.qu`W#N2:g-9cQd+#O:gIn1:g-k4hX:@jrIq0@VX:s&N<^YbkR-OW^$.2$G)NH,WejZ-l<%FW9lSfS'K:Ylt&#@>)4#rrJ)8hB-Z$"
		"WK<NM96]0#RJ?6/YW7L$RF7#Mh7--N2r]8#-)>>#dp1K$XHfnL*NP0N;k+5=7emJ;559E.ZvGM$2h/*#8*_/%imir^N83(v%U*t$9j/v.+:5YYK7mv@#u1D(Fu1x9wH)AO04vw9%(DJu"
		"KVx(sq`fv$bZPE#kqBsHgEFm'ZCL@9VwO6#jwXT8V2&=%&6.K%69vx[671n[#Ak_$c_EwT;[9mJ&jNB#Zn4<#c67#MoDJAGT(L#vRI:;$`K^sNS0H6#hpCINw@,gL&o_DNgXN7%W)AVM"
		"7/<mN>*LH8l2?D-?<Z`-:I&L>ptjVm0eU5^rYIA+a&q8.%9s>)NQ@,M61s$%lV^$.8aqkR7?A@%HE'C-p96L-/$_$._$RqN0t9H8EjB_--P7'oP@MX:nd0'#QtP)/hMc##qAY^$U_v>#"
		"fsm&#(#L*8=IV58R/&HW6tXJ(arriXPQ,2$C[gW$((/X[2bc1%H')B#(tpsLVPu1#+Pc##57x)v,]u##P4:W.R$mN#fxKV%RLl:ZOour$fIqVIU-$K,?n@u-bMOgLqLpdu5lgY$a[Jl;"
		"aqT>Z><o(<<5EvQb`>)<MEh9pRmYM$?L`.LY2?Z$x[X3HRn'^Y,G8;6_&lJ)bg16/d0ST$Cls-?(bk]Ya%3W])NG,2`e52=chUB%x1`90^1>fhQHkl&HuTw9%o>_HpE^$.`5v5NBiv>#"
		"^aTj8@T[A,w/gs-eNc)MbntrNG0H6#]=>-N3ZBGN1M4`W*>(sI#=_P^At]X:0>hX:.8h;-KW^$.Nd9I9-erQjX`O,M?Z1'%lV^$.O]RLSA_5wNoZ]#Ml[/vPn&co7gNdfVw*(p]&CkY:"
		"Z8AvL30<iMbCIP/,@iERS)Bp7Q@Hv$g9^Z.<x)JUlo^`$B'>j0n9op$`**`#6^,3#YrBZH,uID3w6f'J]'a##(fcpS+i?8%GQ5;?o&,<-1?-.%Ta?p8(SX/6C%%J_**V;.on,2_Gh+^Y"
		"#p;>5tUnfME[o]Yth)-+-ju,2hTRA-4ijY.J90U#;Jn]-6I5.6HV<`WH7]uGZ@*j1D`x]unl'/%6S<>.0XY##ZB+T.sh6O$gudi2,Ll:Ze]&`sK>v1K#Vdc`g0li+9_`=-E(3:/1hl/$"
		"wAQ`GrtFR*v)IR*[Fe;-dwmt.+:5YYYo2dt#0#,2x);ZG#>bKlI0&##'p[YQ[oFv$DwOw9;:c>%O%kB-3x9W.sA9U#Ux6e'kPAZ$kZY)M2729NkDW(vp&[&%mP1i:pdTx9nFxx42FA:T"
		"#_K$.SH#tN1jG%P0YWuN1lci$c?F0NRM0_-^f0.Q;YVMT,8#.QPb<&5e>ucW[4PdbS1fS@/rg-QI$&x9GV<`W74E@9`cq5/1Ll+%BDOcMKL88%C+T&+qlX:v.n@oR=;>8R'c?R*D%B*#"
		"#)>>#'%oL$Z[oP>muFR*r`2/(Dgop&aJSfh]mN_?l<K$.hO^sLhkd##Z$)O%k/Sb%jaqrdR'f8.c(],&tPP<-=+Ld%McnV?]-)gLtIDE%<JK21gJ1dW;g_]4WWo;-+I0_-O,;FPaIL7+"
		"AKf>-j14g2uH3D<rQ;5]?\?$)*&$[xO['7f;RQt2(cLk>)?YWF&x9<R%03$o&H9Xb+$bBt6rZ/1)CKi&(=82-*[fRh(;@)c*F30t-gX+gLo7Ka'3mVW[u/o;*qL5nfP`HcZ%9@ij7b;8."
		"/Y*w$&0ZJ($RD.3svZ)4)a0i))@I1C:1]v5g#qrjhn'<ai:dOZ$GCFr5VvxKb:gW8_]wH$;^^(G1_ZC#Uq3,2sU4PPFQK6X2Zl4fb`V:vv4X7[QdE;/lQ%##*xq@kCl=58o@BP8*$&AF"
		"=Ne4]k%TBf/3>A#j@MwLYddh2I6smT@4fMKVEE5/iY6R3fw6S[V[$##^6`uP.@AAb46A5/[@XR*?7BSMrC9vL'O0CMJk&/1:Uh>$l``=-&@:@-xF:@-)e`=-UYSn$`66R*cp>9.$Qkc;"
		"ts$<.;d`lfBrRfrO-mA#qa`=-NaDE-VnuH&xi]igSC=#-4snS/Vnws-3B/_MluWuLOBSYM.*?%NCGF5A>1aSAFKtcWPJ'4VvqJfLuFH>>q#n92n`avOE97Pfq#n92_^6uO;79Yc%PllT"
		"DXw`4RVRV[*jnA#;_`=-aOYO-O3OJ-xm%Y-V.OR*L)6iT9M):2ePWR*0*OZM&?5]uRu<ZM;#a&Nq6dJCT.N-*'_NG`k,F5/U)RR*@nV8vx-Y:v8ElB$.JfnLvd8jMWD8cr%4V:vsFOm$"
		"Of-s$_>7R*,1Q1g>cv%bA8.,)XZ*AFg8[],0?5Y[7FHA%L>Q1'AW,c%smp6&SSL/)]ES<-ApPL1Z,h5#l.^mfRLkT[YdkURjjV8&g:M]&^:SC#YCK,3#DXI)h-2d*VX=T%;i?X-J29f3"
		"5*x%F%#NKV$=+loP:lvaNTH:l^[:-Qx5-j9Q=$#NDCU;$?gAXaOur+)gHV:L/kr]dp^HuunI:;$fXf5#tPwK<3UDW%'?Ig%+wJm8gG=6'P2Rh(['3q79pDW[a3$E'f`&mfjhxA'?W1<("
		"9-]<upj./U%'st#$(+e#,,.,DT1o-#]$6;#xvLC$Vtm#9B3ZdJL*x9.kDsI3BH`hL3UbiK]5q^W;1@ouW`1?#$JHC#aB*Dag_RfLS@7;#De`mLSZ/%#gg]O82ppU@Y5@=dmHXm'&7Ma'"
		"GGc;-v$*b5_M7K%dKl[PL^J=%jxJmfCujT[pmi#)UVRh(vhbs%+a0i)NIHg)$LkA#IPvu>g#sie93Kw*u7T01.AX1d6IYt#viAEu<vHe/+I:;$s5###:28)dVgb-8IJQV[H]_2(MEII-"
		"?`5O-&XG^A]>6O-JDEb0(6-rqU,G<ckAh`*Ua8'4)*Z%UIgiXl(BUf(,LDuubGgiuvV,0F69D:#Jd0'#85FA#iPc##c6w,v:btr&D)Ql&L)@iKrV7mAI,o343aG/vFr6]-rH,54,Wx,M"
		"4'4g9k#IwBCxEucO*Uq;q=O'$`s,/(HOKR*Rde;%)E:@-JA'wL^-KJhsPI#Q=iFp7?VViq:Uh>$oa`=-jArP-/e7'&O,ulKUUtY-Zgjum*mJgLYW*uLNd-##.G:;$ZZf5#:us=]T8f[Y"
		"%Vf#I8&2-2>0WQ8<3Ew$PDRh(BZ_,2;PO1:Er'B#H&368o<Zv$`e5b*o)$t-t$TfL2LF%$1uV7eZEFI#eEonLrPT[um>Q;$$PU;$&+p=u#DA5u#/>>#gcix$j''mB+`CQ:B'Qh%:5GB?"
		"PJP6WZNY5-'5S8]g:f+M21bmfqW2Z$2H$i$-aCD3(DeC#A6Ejj2aBkug-<0p5ovK[/p[H#XXXVQ$8Puu[n*XU@;m8.B+TiBV7?p7'2oP'^WsP'k$j<-ib+r*;p_s-pFx+Moe/O9C-_>$"
		"BBW<%q0x0(aruJ]<Ii9.SO;8.pq)W-@@X2iAnF%$s]]oRnfZ:dn-`CjZXM-HuOYS./)5L#X.pFM2ITMuOJ%@uP[,jY2R5+rlQV:Q5lOfC`97`]#]Ga*lrwm8u+`_]Cne`]OD`_]Es:_]"
		"nikV-e2`g%oORX-]7*v>4]'^usi%qur=uDO@%sW#ujCT#V]/<P$s9M0$(HuPRcQP#$1)O##)#'v.mR)v$IR<$#NY##7(>uu%25##^.j[$Qf''#M####`e#7v+6eqL6TFh#<utwlVD8)="
		"3b'W7UDe-6kv4<-%2&X?Avxc3=`L>?o`Sv$qj,F@^,'REgwow'Einw'Okv[tXe&#cxKn(<@a:5Kq+ZD4A#qJ)gm*J-A;Mt-).[h*#<GT.XOLR#B7(0%k26RCK)ebu:Y3t$ZUaQhoTiJ#"
		"EnPW%*$2bZB2X9'A-UE#PfV&&#?hLNP_5m>]DoO9P/>p8U-f3=p$3QLft;a%k%aC=W4tY-w_@T.K^$K#t3is&Vr#^?n[Vk[x,cg$COK1pj9plo-.AT.):GuuNP:@-EHarTFx;GYCH_*#"
		"%9@@%1SCl%1`X;6nEEaNANRaju+M_A5+K1plgC_A?U%##,CPuYZH$`AR'+<-=5s=%>EA=#s2<JMUG6hMa-l4oVj9BR0r_`jlcC_ASe](M.I]W$?\?xb%woOfhbQFRahh&a+M*V:2AuC(?"
		"g0^D=,j0#Zlqn&-5J@e%IAWk[Lp`a-&6gQapc$RaVNj-Q1Cd&O:SG,2xH:,M,]Zc*[s^p.YG:;$AW:_A/C=:2*FIpK./L+EIXkxuIOYO-1iDE-OQU[-r6u_AeS(<-+l0[%rAO-Q:,J_A"
		"1F4:27iJ0O/euc%;mVk[M&rh.x&89$Oq_j<N`0#ZrMn`Ab/A_u*RUV$mhs)#(8wDC*Mc##)mfqB<rhiuo5xh$Qf''#9`=R*kxLJ_-GP:v*+LxO`3[##<e?M9YdB;e`F=R*t4EPgu23p/"
		"vfWVRJV'H<inHv#?TD2:Iifp]#)P:vZPJ@t[.nt.fH./:Nk9R*,H7R*w=6R*GJJR*+80Z$11Z=&sKa5/OdJsIh.K`st_^J;v`gfVtfWVRoB@ONanv##<rgI;+(1'#OH:;$v@vlLj.I+#"
		"+Pc##[abI#g^4G.dl:xkqKI/2kLvrnCR0kX7iS7eEJMQ&NUXM$jaLu7wMP8/J;gJ2wWfT.:<;O#Pp)M-1l%Y-ZBLkX?in-%,(TO&#/lfh[7voJX(6sI>D?wTI95:2RO8_MEVhiu'l.@%"
		"UQm:ZV'sr$dJ9sI#26i:GcK-Q0D7:2LG:;$oaMmLE%<.N@i#2=HfB#$7aU)&%Qe1BT7>Ds=G5&><[7NU$cvo%LPgaEGHjVRwuT1:<]1_A7#J_AUdP+Vfk<_&KL=2$7r@u-o$Rp*:bbN0"
		"T6b9##)>>#gCUo*b;]gL_$S,v,n@u-T6np*8;mT.Buh7#7k@u-,$Rp*qhYgLaQ7IMMXs$#Lv>;%][AN-ctx/.u'W8=37X=?\?C)-*V>k(jg<.F.C+)F.GV<`W1X(G#_V*Z%@`Rh$&b3vM"
		";r78%iq[lpLb/-*Iup(jvMDL56>1#?iJ0K2W?bVm248#6v+q;-qa`=-.%kB-oF:@-<f`=-1.xU.kV7L$.F:@-uk%Y-/WMR*x14_Ac-0:2?8]R*A5AR*r$Fxu)i9b$w`''#GL%##.6;Gu"
		"vqbg00G:;$2Pk]#43tZM(_g1vw]#7v%j&kL#x>q$lgBB#%?o92J1Jlu?YJ(Mjh?lLt?,i$NX`=-O#Au-G;s'MB=+)#]^3ZMg(I=NR9CSMbf$/vA9Nd$jGU[-n=kw9IVA_AY3>)M,4:SM"
		"_30?NoPbJM7PvwLn19VM*P]BN*bKCM4.,GM*ME>PV27kX(Lo;-6.m<-K1MP-ADsM-70m<-@0m<-l(m<-P81b<6^d--5M3RMCf_[Mk>[tLDShSMDlh[MPHS>-JB2X-[^%.-xVg;-4<.2."
		"b&5wLePbj$xVJ21[^OYdqdqV.>+N,*OQgre1@$Z$aPYD4T)DPgA&-g)uZ$j:Sv(5gd%wU7(F<R<sjtd*q8q?-Pu;U/6Sk]#5Uv;#:oh;-.>(c$)./X[[%<T.U?]9$c.KV-.hh2)=HwV."
		")&n2Che2s$OkNY$NFl>$_@7I-iFB7(,#s^Qs`YxuSkxx$Qv^^#Bgt;.D8wEJ,vn8I'6&32-i5WM[h%O8EeB#$3qv,MpiO7MZ2-YuOI`JV02WPN5=93KxfDQ1CYHu'+q8=-RrUH-0(ra%"
		"AEw,MkNHB$uWu>#>(?Q)KFl^4g%'58Fd#8nBhil/LPMd/DVSh($p,W*ZEf;-+fBN%pt^)')-r$(gCj639a4*+9qAg12Vg9]sQKYG=-NB#ElsAMv'#GM.w(hL41XpFHKl^#vp2d/jEoo@"
		"O3]t(:`6@M2[oAGxYM8]ATjd*^HFW-v/AO=*f;4^3M>d3Q3<d*CC_j0].B+*Ynn8%9Kt6/N3X6,NRr=rqPN:RLw,-N*bk48VhL=Zgw#v_[h?>#3K9iT;gE>P0@TxXVhAb*4'ms-<-Rc*"
		"*FWa*RHT<-5er=-=iVH%6gxa*`Gds-kbb,MoC)-2Q2'Ea?O^>]4F/>G-'@I$Cjw]Gc:u'&l@D?#o<,A&vL8>G]vBjpNr/p]SJKkL]j<dA=,<8]n[Zu:VDC*<sQ;r9@2@kAK9(]$[<_Ru"
		"W&]t#<E$ou&C4I#')?\?$m>7@#fSjjL(&Ibu]=GB$4/4&#-:#$'Fg?-d]ALb%+W_$'C;4?gJoEJ/^iuJ]uU+,M4_0b*6OS<-rHW]&pc6#$h1Go$-xU/WuN[P/d^1+a_i+q&=DaK^`x_#p"
		"SX2R(15ns+&=.J_E[f(k4Ts%+R)ma*b.js-rU,29<rb:&`8.#YHtD5/<i###1w6S[W/?cV4(Ep.nNQMgx[&?RqKv.#sFX&#2If5#Q0jm8MB/F.]12W-&3Ye-X8)c<?OSd<vN*d;Oc^3F"
		"=Q+##5wkf(oT0b6MqaO#+$ffLaMWcu*/Cs&Kvf1^on4R*5vMo@o4eT.^Tj)#>k@u-)+g:BY,WGWbQC_A`vP+VCAwY5HT#Q/,Ll:Z7>65&`'/n'6q%v#1:<p%/YD8A9___&cg;A+u7]R*"
		"WRJM'-pl_&==(S*2)7`&#l%/1SsNF.8l4R*2un_&9&FS7,no_&]Ujl&m(i_&gNCG)#Mi_&$J-5/74j_&[5@S@^Qk_&TEgx=ehn_&dkjl&d`h_&7vC_&YVBP8XhdxFeBEYGIYo+DUw%;H"
		"vGu(3/pYc2>lj>-`f/L,LS8R*mP8JC,9@N:JDWiB2fd'&HdJ`E>lj>-3/I44NMWq)=%`'/<Un-$.R]rH/(=2CAGTD=hp^PB=^E>H@rj>-`;iq)&.AVHL++L5-#[-?+[r?9M]._JS#%44"
		"1HKq;CXLwBwa.j141[qM57M-QGN'(S3*)L5;8+DNRC+_P9%s=-?hj3MfgipLt-lrL^)B-#9-/b-,kb3FD#w9)qcSfCl[tkkA[P@pM[xlBLo])F$E3GD';AgDaB&;H/mC5Bd9$LlB58R*"
		"Qc8R*&&bJ;DbxlBJbPDF:OXMC[i[>-$FUDFcgi]Gg6YJDZq'?-*2cxF`_&mB'mZMC&/>S3XR=X(ULI=-RS1H-v8wx-e,KkLhT`/#K3*j1m1S:;HcfKYuuo+DOdmcEv8/#H%*nJ;^I*kb"
		":ap(<g8&29-pu(3,a>G2VQ&q7I(L#$t1TV-whQ9`ChGJ$R8YY#-MYY#2[oi'm*Lk+Uk)$#,2A5#SFbN;J@@lB&_S.#5GFVC1;4fEhpf.#C)ofL>rJfLwF6##]qGAOS:-##7F;t-a'ZAO"
		"^_?##@(GjBx##/#TGg;-ZGg;-g(`5/U%2eGnW._Q*aDE-]PB,.V4FGM*:>GM%D#W-WnoE@VIjl&wLkA##g4?-*M#<-WW*+.hn.nLXW0(Mjuw2/qKIP/ud*20#'bi0'?BJ1+W#,2/pYc2"
		"32;D37Jr%4;cR]4?%4>5C=ku5GUKV6rp6JCs-VrQ0#Oo[4;0P]8Sg1^<lGi^@.)J_s['Mgwt^.h%7?fh)OvFi-hV(j1*8`j5Bo@k9ZOxk=s0YlA5h:mEMHrmIf)Sn,aGG2:fwx4FL9;6"
		"Mt587[sIM9*]ScM=q->P`/oi9g]0,;lxgc;p:HD<tR)&=xk`]=&.A>>*Fxu>._XV?2w98@69qo@:QQPA>j22BB,jiBFDJJC.cAGM6=Y`N>nqxOFH3;QN#KSRVSclS_.%/Ug_<GVo9T`W"
		"wjlxX)E.;Z2)bo[;]#2^C7;J_KhRc`SBk%b^)HYcfY`rdn4x4fve9Mg*R2Gi43J`j<dbxkD>$;mLo;SnTISlo]$l.qcHh+rqTSA=&(/#>*@fY>.XF;?4'C8@8?$p@<WZPA@p;2BF>8/C"
		"JVofCNoOGDbdtoIuSUPJ#m62K'/niK+GNJL/`/,M3xfcM7:GDN;R(&O?k_]OC-@>Psu^f_O$x(a,_VciRiK/:o1?)</[O;?G`tcD%)###/.35&p_D>#A8>;$+kQo[[n5lU0Am`j)I<Y7"
		"Qs4xdN^hLi&X#Gk=$Y`,`O?:oBOd&4vj7`>IWrGrE@S`,WA+YIut=)#B716A$VxejJ2<HV0Lv/hn8w:Sgs$fjI3isdesP01ujOe$OXteji9RxdsK#L>[%Yp'A'#J(pig')^IVj(/nas)"
		"Kfgb*erLg*=xEI+?(FI+:Pjp+8(Nl,Ex(]5>>2%.?N0u.v-.e-.O]e$'a'dsb11MVm1B-3Uu-_,R(tr/Eq'AQ>>AwH@t#fj1k2LV@>I@Qh75(>hdh@$u(E@QYk.eQGUOY.X36]13Hrr/"
		"[qo(PYx0lUYvR9r_`f64Xxa24GFm44Jiw946Y_CM^#>GOV>C&5?2RA-ifP8.4ujLibk[3OZn9@0j-,^,YJ>8.L3qR^lWFPpXJ>8.#N$fs]JU&5&>>jL.?LWNHE3xd[Y?#6ZbDE-s$Q8."
		"bh#;SFx5^Qja;:oF,14+vRrA5bGLvLVxO_N@PGf*V>C&5lc8GM]2oB0cG@)#fI=)#^m=L#t]@Jisp)gLvW(H4au;X4[MJF-tRB40&Ufx6u?%/3oURe$H1]?^PTlLi-L+-+k5l(P`DGlL"
		"+[NxdQ-`90[;5xdZP'/34X)qr^s$fjIT$AQU`sr/.j&YISSk?KeJA,M2Rco.XPYZ445S>-eADiLslOC5d[H18=R-L,T1IPpxw(.M.k?lLp)6.s]E:.$r#7x0&n4eHK3Ik=B/7kt]Mn2;"
		"$XiY&2)Z&5.#ur7tIC&5N*50.L[LCM^Y$AQ3Hrr/Du;B5xD:O5[4Vp96]ju5Z05##5*ofLF4$##&MYS./f1$#<Rrt-w-LhL.&v&#RAP##^V5lLMbx(##S@%#YuJ*#51g*#8:F&#k;#s-"
		"mBonLpoc&#ttu+#vmXoLo%;'##NYS.QBV,#gu];/7b.-#_Q2uLvO%(#n/RM0YIf5#_L6(#ki*$MYNm6#m'*)#6tql/sNC7#u3<)#6=#s-RZB%M5hX)#cOn8#bSd&M>HU*#*[T:##S8(M"
		"F/n+#lm](MGRI;#j2;,#Le@5&ZG1cr(/AMBpJ/]t1Ea=u5ut+D+SC;$Bf$p%End%F=esl&TELD*WZE]FhbYV-m[mi0pf^uG0mGG2xQ+)3#Dv7IcDFJ:_eU`Ea^XoI[w2>GZ/Kf_^t8MK"
		"KhIc`i@[xbkQp.LbAv:dr*Trdt)m+Ml(Soe(+%m/)Xc(N'3ti0H@UM9K&a%OeJOJ:Q$3,;t[^xO]$<>GJiqrHS6i4]0+Y`Ni;<AOr@V%b@tqxOw(TYPx_7]bHN3;Q'SlrQ1Co=csQ5AX"
		"H;nxXK5&Mg'3MYYPl/;ZQS].h/derZWIco[Xu=fh9JBP]^t$2^_=uFiA%Zi^dH<J_e[U(jIUr+`jsSc`wH7`jjl%8e2.^oe3)`4orF=PfATj(jBc@lo69J`jG),AkH+xLp>jbxkMSCYl"
		"NIX.qFD$;mS([rmTh9fqNu;SnYRs4oZ0qFrVOSlo`'5MpdWQ(seNh+rlvHcrm;/]t(@+v>(^-s?=#gY#WLh`ETa&#GU'5,)d?aYG]GurH^Klc)sG:5Je.nlJp>i`*:O(&O*0[]O2+vo."
		"Ov88R=PloRB6OJ1h[*,VO$^cVQ2HD3q9BDWVQu%XZY)&40sNS[i+,5]o':87B(di^$F<G`&b658Z&muc5#erd@Gcc;sOblfNhq+iSO[]=8?S`j[d/Akc9XY>S:&5otb8MpHrEPA[]*a*"
		"vleA+.wh%O4sG,2GsG)3HuUlS>Y%a3WLpS7^k7MTgJF/:ig*g:jdKcVo%_G;o;B)<q/-DW,L4v>(]3s?,2&>Y:E-p@2CgPA4]x:ZG;&jB;-`JC<+YrZS.ucD,ED>Y-a<S[f;mx4e.b1g"
		"M8eMhq@.)3]i?^5p6i(jP`ScjZL(Ek#/B>P8>cG;jHpi)r1EG)/-/5/wL&)*PvCG)Kq')*8D55/HB4F%Nk,)*XUm-$/.n-$27n-$c[25/M5p-$m;p-$%Yaw'lSIG)P(+)*Bfl-$all-$"
		"crl-$OTi'#7C@wuOSw8#wjM<#8(.m/3Ss)#,H6(#rF6x/Gcg:#V[2<#g5T;-h5T;-j5T;-r5T;-*ME/1a=Z;#hU)<#gIm;#&6T;-mA;=-b6T;-tA;=-fdEB-'7T;-:*m<-Hs.>-?O#<-"
		"<7T;-Vs.>-D2QD-M[lS.]gMW#$YlS.q[8E#=r:T.&R2W#?M#<-CYlS.kOI@#[M#<-jo8gLZ=AG;'gk]>;?rDF/XRYG?p&vH@#B;I6]Tj_1s4,;]MKG;lQvf;mZ;,<ndVG<omrc<pv7)="
		"3LBjClu/,D1FTJD2OpfD3X5,E4bPGE5klcE:6V)F*1DAF80i`F99.&G:BIAG@#B;IA,^VIB5#sIC>>8JDGYSJIiCpJ8a12KGcUPKHlqlKM7[2L:)IJLK1niLSP6/M<gPJMRXjfMSb/,N"
		"TkJGNV',)O/jd`OV?CAPXQ$#QZdZYQ]v;;R_2srRaDSSScV45TeiklTg%LMUi7-/VkIdfVm[DGW)H;s[qd'>c?Xnxc=_NYdRfA,jl1C`jRr4DkU71AlwRlxlY[HYm[n);n^*arn`<ASo"
		"l;Lmpb0SA=l''s?:-v,E+7MAF>>nfL0M^YGRMbYHG2'vHPSB;IN+rPK.bgfL7Y<>Pu)suP@C?;QpSwuQ^)WVR_2srR3iTSSl:hiUt-ADXxjpuZlL8;[FD38];Afl^8>+2_5S^f`3Y>Ga"
		"@Ir%cQ-8AcR6S]c?XnxcC'0;eUjgreB6,8fL;<JiNMs+jP`ScjT.l%l)eM]lXR->m`<ASoo&>Pp:;Zlp;Dv1qj):Mqk2Uiql;q.rmD6JrnMQfroVm+sp`2GsqiMcsrri(ts%/Dtt.J`t"
		"u7f%uv@+AuwIF]u*.U#v[2O##i^e>#$c'^#%lB#$&u^>$'($Z$-R)<%rUDp%)RrS&+eR5'-w3m'/3kM(1EK/)3W,g)98%a+;J[A,=]<#-?osY-A+T;.C=5s.EOlS/GbL50It-m0K0eM1"
		"MBE/2OT&g2U),H3G6G&4U5u`4WGUA5YY6#6[lmY6^(N;7dRSs7SYoP8d_F59+##M;W+5;-;[@xkB9tUmK/M1p>X`@k13cw'V6Fxkw'sUm5`-F%XSl-$n)QM'i&b@k5EFxk2=r-$RCr-$"
		"wFS1pTY%:)lvf@kXUm-$/.n-$27n-$d]Cxk^^P1pM5p-$m;p-$)(#:)<[xUmn3e@kBfl-$all-$crl-$sBAxkOGU[-Ag$XLl1+XL1lj?Kjii?KAo#JhGfi?K1lj?KQO^Y5CAQ'JfDQ'J"
		"d>Q'JJw*AkuJuPp<xj.qA#:eH*EsfC6[pfDZWhAPE&%&X0v[P^RdIqDU_HqDaZR58<p3m8mTmJ;3[HqD0)3/(@p-ipK;.ipbtm-$+xm-$,%n-$-(n-$%gp-$TIr-$VOr-$XUr-$Z[r-$"
		"]br-$%S4ip>b0ip4;l-$5<`-6@9tr$lotS&7Wk6'#Vbo7'1Zi9To'/:h-_M:j?\?/;tDOA>.ue5Bu8o7RfYooS.,bi_;Lnxc<U3>doc;,sePkY#;J[A,?osY-WGUA5_a.R3j3SnL##ZoL"
		"bqP.#$P*rL8e=rL9kFrLkE)/#Mit*vT[3rL8e=rL9kFrL:qOrLRV*uLTc<uLVoNuLX%buLZ1tuL9uFc-D51F%amo-$cso-$e#p-$g)p-$i/p-$k5p-$)F9onl9n-$YWo-$^do-$amo-$"
		"TW1/(TJp-$uSp-$2+bw',&q-$15q-$?`q-$Afq-$Biq-$N7r-$ru;on_dj-$'kj-$)qj-$+wj-$-'k-$/-k-$13k-$39k-$Gvk-$I&l-$K,l-$8lj^oK'.cri7u%FNoNV6WSM1pF(m-$"
		"h+m-$j1m-$tOm-$]ao-$bpo-$.,q-$;Sq-$<Vq-$+fS1pVTj-$;Qk-$?^k-$WPl-$L-mA#sr:T.itrs#66T;-76T;-86T;-96T;-:6T;-R6T;-T6T;-V6T;-X6T;-hWjfL7-coRePSSS"
		"cV45TeiklTg%LMUi7-/VkIdfVqtiGW(l5)EYZ?>Q^)WVRaDSSS&928],p*2_1G^f`?q/;eA-greB6,8fNMs+jbHSSo=X;-v?VDp%-_rS&+eR5'-w3m'/3kM(1EK/)3W,g)GbL50It-m0"
		"K0eM1OT&g2ZHgd*Ys=ull>k-$&F_`**5?ul#en-$Bhn-$Ckn-$Dnn-$9Mq-$;Sq-$I-BulrKn-$?_n-$[^o-$L1r-$]gBulsJk-$;Qk-$=Wk-$?^k-$Adk-$Cjk-$Epk-$d[<ul9q+/("
		"eWgf(mH7AF;Ke]G?p&vH@#B;I@gb2(UM%XL>r3'o*um-$+xm-$,%n-$-(n-$%gp-$TIr-$VOr-$XUr-$Z[r-$]br-$AexdmS:rW_P;l-$5<`-6n_rr$weVP&'5Qv?uMdo@0>BJ`v4jgr"
		"3H2&+/1Z`E/7v%F80i`F99.&GBs<BGC>./LEJIJLh#(mp7teYG_2srRh.hiUpw@DXuB+aXmXxxXuNtxY+gel^?(?GaT.l%lV@L]l]kQ>m]i8p7bLfS8d_F59jMHv6',Le$I&l-$Q>l-$"
		"SDl-$L0o-$[^o-$amo-$&jp-$n;62'SbBqM%ej-$4<k-$8Hk-$:Nk-$<Tk-$>Zk-$VMl-$ObU3OFsk-$Gvk-$j53^#NM#<-q5T;-2WjfLs<45A,V`SA)S%pAQhfcNS$GDOU6(&PWH_]P"
		"fr02U:smW_j1PwLpBdwL8fC%M:rU%M<(i%MnO&+M#%ZoL,r%qLRV*uLR`$]-jL2F%02q-$K.r-$M4r-$l9(ed8Hk-$:Nk-$<Tk-$>Zk-$@ak-$Bgk-$Dmk-$Fsk-$H#l-$J)l-$L/l-$"
		"N5l-$GwM9iTj<3t[`hi0=UAD3S#>)4L:3/M[mvuQaDSSS&928]15Fm^TT9]b_TRKu8vG8%wT5)*8/`D+:A@&,<Sw],>fW>-V>:&5x]tA>4S(m/GbL50%_(R<We)vmZ9F_&%fm-$&im-$"
		"'lm-$(om-$)rm-$Q?o-$SEo-$UKo-$WQo-$f&p-$h,p-$j2p-$l8p-$8Jq-$:Pq-$<Vq-$(8[7nWRm-$,%n-$RBo-$YkSKuA<BG`SJw.iMDWfiSo]Gj(6iA+:A@&,<Sw],>fW>-@x8v-"
		"B4pV.DFP8/FX1p/HkhP0J'I21L9*j1NKaJ2s<=AY<mPb4-OsUm2G4ig4SO.hcXI_&'mp-$KqAul%x9igkXrUmK1m-$k4m-$l7m-$m:m-$n=m-$o@m-$pCm-$+xP.h1Q0F%M3o-$O9o-$"
		"o5dw'fqU.h^,uUm]$/F%'./F%3R/F%,u5igoZ/F%WD`w'`DH_&w4I_&*PI_&6]2F%AXbw'@=J_&:Pq-$K_J_&^er-$kgK_&e$s-$g*s-$i0s-$k6s-$m<s-$oBs-$qHs-$+5%VmWNj-$"
		"vTj-$xZj-$[]O.hDll-$crl-$i(5igXRm-$<`4'#Z1%wuS(ChLnl:#vtmK4.'KR#Mi7J4vC$?L/an'+v`oWrL?9(sLkZ%,vQ-&N/?NG'vn>]nLmGgnLnMpnLoS#oLpY,oLK,@tLM8RtL"
		"$aX-vtmK4.&;(pLO,@tLx5&#M8fC%M:rU%Mccm)Meo)*Mg%<*Mi1N*Mk=a*MmIs*MoU/+MqbA+MI:H:vxtIfLv'^fLx3pfL-Z-##J$M$#p)p&vvp.nLh)9nLj5KnLF&w'veV4oLH23(v"
		"&dFoLxrPoLccj(v[Gr(v:DCpL]=0vLn[^vLdhpvL>)>$M45P$MocV3vMDt$M@Ds3v'g$4v's64v/#@4vUo^%MX(i%MjOf4vT+$&MJux4vjL*5vqR35vnGP)M:jv)Meo)*Mg%<*Mh+E*M"
		"i1N*Mk=a*MBDj*MrO&+MoU/+MrhJ+MNLQ:v=7Puu2.ofLj)gwujmwwuPpQiLK8xiLS[-lLhtQlLgaamLl)9nLj5KnLs;TnLKDN(vGU*rL7_4rL8e=rL9kFrL:qOrLpgr+v_Q$,vLB9sL"
		"BKCsLCQLsLDWUsLr)S,vXagsLGjqsLHp$tLvAx,va#6tLRV*uL/d<uLVoNuLX%buLZ1tuL]=0vL_IBvLaUTvLcbgvLen#wLg$6wLi0HwLk<ZwLmHmwL2Ac$MCNu$M6Y1%M7`:%M9lL%M"
		"F#`%M=.r%MUjK(M_w^(MY,q(M[8-)M^D?)M`PQ)MJfk8vs[X*vphk*vVt'+vu*:+vGwWrLN3urLG9(sLH?1sL^+kuLw1tuLx7'vLi=0vLbC9vLkIBvL+VTvLf[^vLs$6wLh*?wL&[2xL"
		"w/sxL.NJ#M;TS#M/mx#M8s+$M9;Y$M3Gl$MG:.&MEF@&MBLI&MHq*'Mg'='ML3O'MI@b'M]Kt'MXdB(MwpT(MX&h(M]>6)MNsE8v8I4gL'RGgL)_YgL+klgL-w(hL/-;hL19MhL3E`hL"
		"f55xuH#RiL=,fiL?8xiLAD4jLCPFjLE]XjLGikjLfu'kLK+:kLM7LkL[C_kL,re$vh_,lLUh?lLWtQlLY*elL[6wlL^B3mL6q9&v$_VmLdgjmL>K-'vb8JnLtrPoL+lrpL-x.qLbb5*v"
		"[BU/#YIB:#-YlS.[$a<#g5T;-h5T;-j5T;-&).m/mtV<#ZVT:#&6T;-i)m<-b6T;-tA;=-2O#<-LY,11'd;<#K*V$#)W)<#KB;=-@[lS.gX*9#g@Qp/,Ed;#b/&X#$YlS.p_pU#?M#<-"
		"?5T;-eVjfLF=AG;#Zk]>9-;dEmCu%F80i`F99.&GBs<BGnI(&F/@DAF80i`F99.&G:BIAGIiCpJ=p12KGcUPKHlqlKM7[2L.ZHJLRq+)Omvc`OV?CAPXQ$#QZdZYQ,IE-Z_DAvLeUTvL"
		"cbgvLen#wLg$6wLi0HwLk<ZwLmHmwLUjK(MBw^(MY,q(M[8-)M^D?)M`PQ)MBMk8vn[X*v0u'+vawWrLF3urLD?1sLY+kuLnC9vL_IBvLqUTvLl*?wLtZ2xLG-01v,hCxLu#axL&NJ#M"
		"?mx#M<s+$M5;Y$M3Gl$MG:.&MEF@&MBLI&MN?b'MTdB(MgpT(MX&h(M6sE8vLI4gL'RGgL)_YgL+klgL-w(hL/-;hL19MhL3E`hLGikjLIu'kLK+:kLOC_kL(re$v$`,lLUh?lLWtQlL"
		"Y*elL[6wlL^B3mL6q9&vu^VmL@E_&vDk8*#?l:$#4YlS.RCd;#g5T;-h5T;-j5T;-t5T;-]6T;-b6T;-2[lS.K*V$#?O#<-@[lS.XN)W#v4T;-;5T;-?5T;-eVjfLp<AG;#Zk]>;?rDF"
		"MTJYG?p&vHHS5<II-w%F&%DAF80i`F99.&G:BIAGRq+)OT-c`OV?CAPXQ$#QZdZYQE*:kF_DAvLeUTvLcbgvLen#wLg$6wLi0HwLk<ZwLCka0vPFeqLY+kuL^C9vLaUTvL&NJ#M,s+$M"
		"1;Y$M?:.&MAF@&MBLI&MN?b'M6sE8vCI4gL'RGgL)_YgL+klgL-w(hL/-;hL19MhL3E`hLGikjLIu'kLK+:kLOC_kL?N[##-W)<#TpM<#D.k9#fs`W#DkVW#ib;<#-4)=-@HwM0c;F&#"
		"RHY##hRXgLwCsb&uE^w'&<6on:%Y'8xt.F%AoGe-&im-$JCI<-@N.U.x7K2#=O#<-[Mx>-<7T;-rq-A-,n>1%L[kOoH'OP&QF7;+o'n%FJ9N]F99.&G:BIAG6]Tj_Kh.G`)pG`a8K9>d"
		"BvpudR/vWe=OToeWN-PfYg)Mgic'Xo(b%,vaB9sLBKCsLCQLsLDWUsLv>]0#4w5tL[txA49gK%M&#`%M=.r%MBJO<#FYX*v*+:+v[wWrLG9(sL^+kuLh7'vLbC9vLcIBvLqUTvLl*?wL"
		"tZ2xLG*k4#^eCxLu#axL&NJ#M?mx#MmA.>%;.J_&,:E<-7eEB-oMx>-TO#<-ms.>-V7T;-0$sc;*[Ja+/^e>,=]<#-?osY-A+T;.C=5s.EOlS/bq?LEU7LkL6rR$vQes)#M[VmLFTC*#"
		"/),##Q'6;#7Q.)vGJLpLb]d)MDfK9v.8/9v5/M*MA`T9v-PJvL6Y1%MDX[&M5m<8v<QonLv(doLNecgLC&<*MpDk<#1vC*M?&<*MnVKt#:vC*MmYg9$:O#<-FI5s-.vC*Mv%-U$n0s<#"
		"GEr(vGlq@-.s:T.v6&=#Aq8gLJ&+&+ED./LJ(RMLdaXlpi2hiq<#Pq;]vOq;^qeYG[^soR`;88S]djiUt-ADX+gel^3Y>Ga@CWXC]<K_&VOr-$XUr-$xJ#LG(5'crEol-$dul-$$U^w'"
		"Rxds@I[X7e_pj-$V;4ig2SkIh`Sd?KVSO.hBtiumO2-mph`U7n@t%vmAok--I(r-$j1l--8b9'o`j'Ylf;u923fO.q8%FrdH0vLghU7rmgAjIhu4?wm'4W:m))4igrWb=c=x-F%f$xXl"
		"efNk+FfB(s[>qRn5a.JCFJ@rd[:]7emBn-$8In-$9Ln-$0J]7e_wP.h[3svm8xV7n(%:ig2=r-$RCr-$w(=R*t]%:)bqU.hXUm-$(2Hs-]@[qLxZM+v9<*-vL/B.v`uVuLMLBvLBr?/v"
		"7+>wLsT)xL6[2xLK-01vrgCxLu#axLV2d2v5mw#MBAc$MKGl$M9S(%MgY1%MpRR&MHX[&MW^9(MGeB(MjjK(MZpT(M[v^(M]&h(M4Ne7vuD>)M^/=8vWb/&v<h8&v(XMmLfZWmLgaamL"
		"hgjmLIjH'vHaY'vCQonLfvm'v3d4oLuxYoL,)doL2kbc/@Jur$D#UP&kbE(5PcRY5'I5;6Q#RV6v]p%=H17A=s;4&>4cYcaEQ(5gh%/mgD<%JhQPWfiqkTcjft1*kD)PxkDG-Vm&g$Po"
		".mYlpejt1qg&Uiqh/q.ri86JrnYvfr>LlCsrri(too.Dtr4+Aux_0#vPvJ;$Bf@e*tI.)3J)HD3&u*&4euq]5kUE?6Ip;87H5ol8tQ_M:&^]h;BfCD<aw%&=r#uu>q%7;?EQ;dERJt%F"
		"80i`F99.&GF5bBGK^MJCGcqrHbdo(a4cYca6u:Db7(V`bRfA,jJ(H`jRr4DkcW=Pp38D2q)P.v>/4tiC4U^/D+opcDEj.eE%BQJLILC>PRm[YPk>[YQiD<;RbMooSg%LMUon%)X?N,aX"
		":mvxXuNtxYwaTYZ7sMS].vnP^WHx(aM_v(bt%ISfHm$2hZY[ihS%P`kHg3AlWIhxl^tmYmJuDSnnZarn/vPpo^fsS7kBQ58cU+p8auwjkNL>'vFTG'vi`Y'viml'v7d4oLtrPoL/#ZoL"
		"7lrpL1x.qL/.AqL04JqL_kbc/CSur$5KTP&kbE(5QfRY5%C5;6S)RV6sSp%=E(7A=s;4&>)G2W@)m>`aI^(5gj+AMhMDWfiu06)kSMJP880MDbWCTk+,=.F.hE4F%e$s-$g*s-$h-s-$"
		"i0s-$Y`IucO^4F%n?s-$oBs-$rKs-$m$?lfXZj-$-f&YcJRW+Vn3J%b@[]w'aFE_&d$xXl>=Rs-k#8nLc>R'vA^c've8V(vr=`(vWbk*vqQ*rL8e=rL9kFrLoTD+v<:+*vDKq+v6>b$M"
		"4Mu$M6Y1%M7`:%M%bU6v#Hs'MRW0(Mccm)M[;t8vS(coL/.AqL]U>*vZ@[qLn6m*vx;*-vK/B.v+uVuLk1tuLi=0vL6r?/v1]]vLk$6wLoT)xLl-01vXgCxLu#axLw/sxL7TS#MV2d2v"
		"-Ab$MMS(%M[q2b$bkclf'`&2hcr[ihS%P`kT54AlWIhxl^tmYmFiDSnnZarnxebpBPZ5M$h@=M$pVjfL0gA,;C`cG;j2B)<dFQWA'sPoL3#ZoL]JW(vQoqpL5x.qL/.AqL:Da'1S[ox4"
		"3Aq%=W_7A=s;4&>'5Qv?qAdo@0>BJ`4cYcaQv(5gfu@Mh^tmYm.)%Pop/Ylpejt1qg&Uiqh/q.ri86JrnYvfrP-mCsrri(too.Dtr4+Aux_0#v`MK;$:Z+Y%+02&+3m*v,Bb/)3l7ID3"
		"9X+&4m7r]5jXs;7RSol8pE_M:'gx-<s,:;?'XsrH2a-/LA>IJL2P#,a<%Zca6u:Db7(V`bRfA,jl6I`jRr4Dkl;Lmp-1m`E%=eYG_dhAPw@YVQe8<;R_2srRnrooSk1LMUh.hiUx9ADX"
		"waTYZ/ZMS]+gel^;r>GaHm$2hRA[ihT.l%l_XL]lXR->meQ8WnJ28p7ZooP8d_F59jpcERl6JnLQ^H'viuOoL/lrpL-x.qL/.AqL6O^-#B=Z;#2ChM8#Ga;nKUL+iPFm-$sLm-$@l+fq"
		"nb2F%f=-F.I(r-$'ac5/Hq57vu#p(MlPQ)M$^d)M.jv)Mio)*Mg%<*Mh+E*Mi1N*M@YK9vo:i*MrO&+MoU/+MrhJ+MJ@Q:v,.ofLk;pwu>YZ$v>`d$vUU,lLe$[lL9Rb%v)[`mLp)9nL"
		"lA^nLapd'vA#coLjia(v[:+*v.Lq+v=>b$M4Mu$M6Y1%M7`:%M%bU6vlHs'MRW0(Mccm)M[;t8v=)coL/.AqLAg.n$Qll`E)NQJLMXC>Pv5Gpg%[h6$qfRU.YFf9$fN#<-g6T;-E@:@-"
		"%O#<-'7T;-2hG<-TkF58^HqMq>%K+rO-]w0aSSk+J+r-$F'o9;ULr-$WRr-$lc;onOG%:)XHb5/Yb/&v)XMmLcaamLBiaf/ICL+iZ,U7nYNk5/jml'v:d4oLtrPoLItll$H5fMB5:=2C"
		"/4tiC@NDhDIhv7RX#-/(3$5on)&l-$Q>l-$SDl-$ae[=l8Sl-$ek@xk8G`w'[^o-$amo-$M%[w0*82F%+;2F%bok--NhJ_&OkJ_&QqJ_&4fJG)80K_&,i`5/Fa2<#Uj1*MW.h9v^:>##"
		";S(vuF_bgL8KihL:W%iLHd7iL>pIiL<&]iL>2oiLPIhkLgnHlL]$[lL5Rb%vq0vlLcH<mLFK_&v[QP'vS^c'v,T+oLZsPoL@)doL_cj(v@vT#vefjjL0?Pk-0M-F%MA./(L4m-$l7m-$"
		"m:m-$n=m-$o@m-$UN2^#74S>-%6T;-&6T;-'6T;-(6T;--ZlS.k%Qr#lLx>-M6T;-O6T;-bA;=-S6T;-U6T;-W6T;-f6T;-h6T;-j6T;-l6T;-87T;-:7T;-@[lS.6-Zr#P7T;-R7T;-"
		"Q'kB-m*.m/_gi8$3J[w#0p8gLIt%s?41xlB;eBjCp:pcDA[xQEST)uL98x#/?f_=lb'#:)B),F.2iI_&A3;R*02q-$Bt#:)H0$:):Pq-$WEcw'M4r-$O:r-$e#(YlbKE_/ux<R*bqr-$"
		"ctr-$e$s-$g*s-$i0s-$k6s-$m<s-$oBs-$qHs-$V_4/(WNj-$vTj-$xZj-$JR4ontMk-$<Tk-$>Zk-$@ak-$Bgk-$Dmk-$Fsk-$H#l-$J)l-$L/l-$N5l-$ht@xkDll-$crl-$#L]=l"
		"YaxXlWh.F%uk.F%vn.F%+ZCk=14n-$^OU@$aJm;#qt3xuN%<%vO1N%v`a<7%^[c1goWaEG<8b<#4pJ1:crv/rP_0fq1d%cr5`-F%XSl-$2GF+rN=m-$gwsM0(;+*vFRT6vwHs'MRW0(M"
		"ccm)MG;t8vF)coL/.AqL]U>*v]@[qL)T+-vR/B.v)vVuL%U)xL6go#M6Ac$M9S(%MGRR&MS^9(MrjK(MWv^(M^D?)M<Fgk$;`1qi5#5m8endG;h-E)<s)uY>3OT/DJfwfLW.QfCWpgi'"
		"wfkum+MqvmnVLM'GTT7nkK#:)'mp-$MeJ_&W^$:)ML3F%3]/,)5'K_&P=r-$.#VP&9E(N0F?m7vZa2<#Tj1*M#)L9vYVf9v`u=:ve:>##m[bgLasowu?Wk,%iE_Y5kUE?6G^ZV6ZLdc;"
		"v&WG<r,J)=]]M^Q9p<s#*;7[#k5T;-l5T;-m5T;-n5T;-o5T;-tYlS.rO;s#ON#<-M6T;-t&(m8<kjvmRrZ7n2Mii']$/F%'./F%3R/F%h.NM'oZ/F%(cRP&Z:n]-AAH_&h7x9)sf1F%"
		"B),F..D2F%28q-$E'$:)Dbbw':Pq-$K_J_&**u92ULr-$WRr-$mG?rmKf;onTo<R*anr-$,=.F.ctr-$e$s-$g*s-$i0s-$k6s-$m<s-$oBs-$qHs-$UUoi'WNj-$vTj-$xZj-$o^T7n"
		"Dll-$crl-$k4m-$V8MM'^/:rmbM6onXk.F%vn.F%/9v9)%wm>$kaOkL57R]4>M-`a_pj-$'osFV'E?rddPErZ-eJ_&I(r-$rJ&Z5E9d7v3DJ9vDu=:vhgnwuvXZ$vL_d$vXtYlLMEc%v"
		"JR&&vQQP'vE^c'vi8V(vt=`(vabk*v)R*rL8e=rL9kFrLoTD+vJ:+*vXQT6v$Hs'MRW0(Mccm)MG;t8vX(coL/.AqL2@]qL%T+-vK/B.vCT(xLO-01vggCxLu#axLV2d2v1Ab$Mmd#A'"
		"s5sS7I?)qr;X2m8qamJ;7FgWqEKQ?p]5wfLsQ>G2?*X:mEH9rm+#5>m,&,#m/kJ_&Yd$:)XZBul5'K_&P=r-$iSV4oUCsOoWWj-$[nW:m2Jc#m[2:rma;uUm]a3>m[Wnxlb&O#m=v^:m"
		"TY%:)r`$Vm:$JY-lQ/F%?v:rmoZ/F%[=S4obp*#mFi`w'%Yaw'28q-$=4J_&Clq-$luSk+ULr-$WRr-$j&dw'anr-$t7sOoi;X>mNQtUmo`9?mWgX:m[):rmXRm-$t`j>$ZUiiLjo18."
		"k)S]lS0?q`g77q`U[$JhZnTJiE77q``E8`j6#n+MqXZ)M>;'9vx+.i.BYZ$vk]lO-#2G@M;Mer#fj](M:P*8$.pmL-PqM..OS'%MORR&MsU?h-.;g9;7qV8vC;bJ-:;bJ-=D'g-uW^-6"
		"EKXSoJukKPugQV6>hc1gq)[3Ov'ESo'X8To)CcFrA]7lou'ESow3WSoP5dw',=hFrXUm-$/.n-$27n-$lt4loUf1F%MPfFras#/ras#/ras#/rrwMSoEsgS/UMl%=f38A=s;4&>)G2W@"
		"*p>`adHASoe^+poUU]1pp/Ylpm,u1qg&Uiqh/q.ri86JrjAQfr.G3Gsrri(too.Dtr4+Aux_0#vnxK;$.hiW%8&+v,I'KD3[;>)4cU+p8h-_M:%#q>?60trH2P#,a4cYca6u:Db;@%ab"
		"-1m`EZdZYQ]v;;RdS]8SZmolSk1LMUwaTYZ'BMS]6u:DbT#dofHm$2hRA[ih]wDVnbEA8oEVwfL@RA,;g0aD<&^OA>$d0#?/r[PB1.=2C/4tiCxt=v-u]66/wY7)vX,O$M@YK9vC`I+M"
		"Fp2vuaW-iLQOqkLw.moLmJ;q)rMIJLa3Pip9$fYG_2srRh.hiUpw@DX/+>sKKl,:$CB;=-T7T;-Z[lS.WiOU$`5T;-b5T;-/fHA/Qeu'v<W4oLslGoL4Mu$M`PQ)M7v3<#T^u)Mio)*M"
		"g%<*Mh+E*Mi1N*M@YK9v6;i*MqIs*MrO&+MsU/+MDx,:vRJLH/h+P:vD.ofL(re$vuU,lL`6wlLgaamLl)9nLQ]a(v4Lq+vU>b$M4Mu$M6Y1%Md+/4vYRwqLZ1tuL]=0vLb[^vLg$6wL"
		"w/sxL'TS#MHq*'MJ'='M4^n7vRT)8v5pNw9l6JnLJ&w'v@j=oLtrPoL3lrpL-x.qL/.AqL,s)$#J(Xj.%KSX#56T;-66T;-86T;-=ZlS.8_4:$q6T;-r6T;-@V=Z-]SQM3X>3(v@)D(v"
		"*5/vLb[^vL.)>$MfGx4?XXv+s5GhW$(u]:d+q0lomXe=-LAFV.$Z;W#BbYs-1qr8?3ng;IftRSIB5#sIC>>8JDGYSJ]$OA%nlL%Mtx_%Mn[f4vX[X*v`kErLC9(sLY+kuLd7'vLbC9vL"
		"eUTvL&NJ#M[x4O%#[J<-mMx>-gs.>-586Z>U]Ja+bHd>,=]<#-?osY-A+T;.C=5s.EOlS/@--YoU7LkL8xR$vaF24#6rC$#)5T;-r@rT.+XU7#g5T;-h5T;-#A;=-rr:T.Y%6;#W-q>>"
		"M+mcaM1aw'.,q-$gp-L5:cP+i#FJ_&<Vq-$Ji5fh.V*VdBBnFi0,Q%bVTj-$.teFith;ulH'OP&,$L1p,7t9)sBkl&$,-F%WPl-$+vnA#oM#<-34S>-o5T;-+5`T.H&`?#66T;-86T;-"
		"96T;-pkK>>'XCjC(V0,D1FTJD2OpfD3X5,E4bPGE5klcE:6V)FTYBAF80i`F99.&G:BIAGa*]H#ZjMM%'83R3N6o-$P<o-$%WYw0THo-$VNo-$XTo-$ZZo-$]ao-$_go-$amo-$cso-$"
		"e#p-$g)p-$i/p-$k5p-$m;p-$Qk<<-SJ/n$&p,#G/GIrg9g*8$Q@]9$xtQ4$lA;=-A.q>>7pcca]1l0&WLpxX4hluY#t5;[N]38]OLrLjd+J4$=6`T.F-.w#MZ`=-;7T;-K*m<-A7T;-"
		"B7T;-CW^C-RO#<-4/q>>PP;daA6nFisYu92f's-$h-s-$j3s-$l9s-$n?s-$pEs-$rKs-$8OlIqXQj-$wWj-$#_j-$eYH%bajj-$)qj-$+wj-$-'k-$/-k-$13k-$39k-$+(+>-d3S>-"
		"I5T;-OYlS.9IMS$466Z>w%3@$d8':#u^2<#)'a<#C/kKP@Tf9vr>;qMU&###=8>##h]d$vHFj%v1)loLF4$)M8&=<#aZ+oLYh>oL1:SqLta-+#YnM<#g5T;-h5T;-j5T;-t5T;-]6T;-"
		"b6T;-.7T;-;7T;->I5s-O*ChL9rugL9:7;#'be6/U-?;#Zm3$MDIRV#AZu)Mm=j?#?M#<-CYlS.h<:R#_iu8.;MTU#qDnA#sfG<-').m/b=vr#,+/t#76T;-86T;-96T;-:6T;-R6T;-"
		"T6T;-V6T;-X6T;-hWjfL`-coRePSSScV45TeiklTg%LMUi7-/VkIdfVqtiGW*r5)EYZ?>Q^)WVRaDSSS&928],p*2_1G^f`?q/;eA-greB6,8fNMs+j_'3vmK4APon5l5p<O[8%u_Dp%"
		")RrS&+eR5'-w3m'/3kM(1EK/)3W,g)GbL50It-m0K0eM1OT&g2t8])=_0x]=;-ZKDQo4;->TK%b[hm-$02q-$+<.JLa'5iphHlIq^P'`jS:I%bvIC%kIBn.:+qcWqSN98%jb'crQWc.L"
		"grL%b,*o-$MmP%btQn-$_go-$h,p-$pDp-$+#q-$3;q-$TIr-$VOr-$XUr-$&2R'JI_J%bEol-$dul-$&[^w'/L%RE[2#5Vho)*M#w'=#*>/=#UvV<#W&a<#6&M*Ml6bW#rHf9?*Sx4V"
		"BJT9$Pe]q.=oN5vd/x4VIVT9$_]=U$QbS6/pAV(vX=RqLj4*$#U1t9#[:9U#56T;-66T;-86T;-=ZlS.oMp6$q6T;-r6T;-Cr9W.?Q.)vN-f'0ODJ9v=Uf9vj##f%xDcS%s)*v,3:JD3"
		"*YSV6ae))<f*a`<p#:;?i335Suc:AbDHcofc?Wvm)s$5oWX`D<t8])==xv]=vV0#?xIU5'xV?2C;:FPgB$Zfin]5vmK@@&=vJ=a=TGuo%fD7,Ei#T]cf$H&#%k(/i$k(/i*9`/ij^iFi"
		"oHn-$9Ln-$9kiFi&w:/iXo1F%&pkFirf]i9Y'%/:h-_M:j?\?/;vJ=a=>$0>>,>$$?AP%8@mvn7RjfooS.,bi_;Lnxc<U3>doc;,sj_hY#9,2H*]/E>,C%tY-WGUA5kxuE7j3SnLODN(v"
		"XT*rL7_4rL8e=rL9kFrL:qOrLma%,v;A9sLBKCsLCQLsLDWUsLRV*uLic<uLVoNuLX%buLZ1tuL=7lc-D51F%amo-$cso-$e#p-$g)p-$i/p-$k5p-$m;p-$rGe-6;Sq-$=Yq-$1xt%+"
		")xq-$I(r-$K.r-$A:u%+)qa1T]nv=Gx^n-$bJH_&`,1F%b21F%amo-$)qi--*82F%,&q-$15q-$?`q-$Afq-$Biq-$qwD_/R[3F%Tb3F%wKf1T_dj-$'kj-$)qj-$+wj-$-'k-$/-k-$"
		"13k-$39k-$(ns=GuPk-$=Wk-$?^k-$Adk-$Cjk-$Epk-$dse--I&l-$K,l-$YI]w'mo_1T?>(G`-&H`a<U3>d>hjudUPrte_cn4f6F5Pfh4Olf%%1Mg:$aY#mv=Ppt`QfrQwL?[^?88%"
		"2JN.q5YjIqX-sF`^Fgl^j+AMhX7'p]jRSq)?8n34`el1g;@XPgNJVX(l8DX(*7.F.0I.F.j8VX(3X@F.bCG(sq,>PpKbgYo^WOP&Ko<oeGV.,)Y]v.rWJ?Mq&hXiq&hXiq3W&g`w5)GM"
		"/t+$M<.F'MFgQ6#r6p*#:6D,#[H`,#McX.#i@=gLqWI;#QFe8#Wvo(MpklgLP^d)MNQQ)MOWZ)MOWZ)MpLa$#16V(vkO3X%eVMM'&?aFi*(e1T]/d:ddU&,)c*J]=>V8lf`3,JL9_/2'"
		"tFbo70QGY>qY%2BtqSq)8wf3=V&h3=Tvg3=Tvg3=.<F>#NnvS&]rhJ)WVlM()eM]lYiL/)Pojo]vhXfivhXfivhXfivhXfi/sX5'67Bt7$v%2BTAWS7pD)5A0B[ihd[R<-pu(&0c5H;#"
		"vT;8v]os,7ko`8vLo4:vY+>uuROuuuv#4xut.q#v/;-$vf=^kLbh?lL0Rk%v]2QD-g%`1.dfFoL9A=,#Q'+&#KFf5#ltV<#_ip:#VoM<#l8Q;#XkE9#`Rii1a4I8#eB%%#$#.8#g.-&M"
		"SPPT#.:3L#q;4I#<RWL#K&IqL2UrU#]98X##]=Q#>HwS#%,JP#rvp%MB:W*M$+LX#e7[S#:5/X#m[<T#,`#d.fr$8#BTqq9B<vV%UUO#-Dl)L>1o))<s#sc<'/-?\?p+_V$&R6kO6)nOo"
		"C]v1qFfZ5p=rjIq4+Fk4X=v%FBgto%%7s;->Z@U.e[1vuRsk,%Lkv?9;@KwK-Le'/D5C>#vP6lot'Vf:K@j=cJC/Yc-Y3VdrH9lo+f*a+h/1^57;kKP[?0`a%BHT.cet*vbiQ,vi1*-v"
		"BOW-vYcs-v'#l2vEMU3vee$4vew?4vMW3B-ffh).$1-&M9C,5voQ35v#?B6vd5H;#rN28vHW;8vs$J^.1o`8v7'6-.UX@+Mw_?:v*%G:vO*>uuI-,##RMuuu]Ho;3jx3xuT:Xxu*/q#v"
		"(:-$vq<^kLR_f$vew2%vWEj%v5Q&&vTW/&v+v]&vU?5'vuKgw3]cl'v&&;(vL4;,#6<`(veCi(vMaDM/;DC3vwFt$M<(i%M>4%&Mqn+5vYU35vjRZ&Mr*Y5v@u[fLo;SqLV9F<#Gwk2v"
		"<J'%M_cXo$sZ_EeQi/SeWm_Eee?wFi*s?-dl:V9`S+JD3Z0$RNww#RN$+$RNiL#RN,Gdc;+q<D<-[+_JpuD_JC_?P8,Psrn`aHwB%<b3FrER+ip=X%OiX%Vmb<Nq;k0e=uw>g3=Sb>;6"
		":C5.ku6QwL+x>9#T-M(vRL%)v-[Xeu;b6guC*eguUPW-vabs-v.gbs3TLU3vJd$4vHv?4v+'I4vI8e4v?75O-;65O-B)e*0^=B6vm4H;#AfJC1wV;8vXX2sugp`8vZ9na.3n4:v=TBb%"
		"[W&Yu7f,##,JG>#ZE&s$:I[S%PSa]+*T*v,Qdoi0gWJJ1<Gdc2NqHa3mYbI#O=5F#nx_?#MV/E#U0/X#*bqR#^$sW#NWZC#)nRH#-mL?#+Kl>#WQQC#R,k9#Fg_h.G#l2v%lTK-UGII-"
		"mT1H-J<cG-wHm91G#4xul;Xxuv_d$vg%?D-.KKC-I3'C-13'C-)X3B-0Kcs/&EC3v#^h3v@c<Z%03oFi%e9lov18;6krm347'<kO5>dc;WODD<_?tu>*asr-f@;ul._+_J1L6lfw4ef("
		"XJmOoRw-ciZ'/F%#J+,)?@c7[n0Y4fES;cVl7i@bn7M%b<8%pf+,k--xdGucMGEe6RTU4ohUP+i'W:onOeV+`.1stnfc%SevBiCjdDd:dnQQ.h5di:Z3Zlre1H5;e;/;<ef`%Se;(gFr"
		"%oq-$mfN.qXx.JLc$Ml]#t.M^-5;]bQK?Jih6[Ym.n5kX9VFe6?2XW#@D8=#b#b1#1(35&uJe(#b'+&#TRg6v%[E5v+gV8vnZV4#C;O]uk#F'PLXU%O/%(#vl+V*M3RQ)Mrak8v[HT6v"
		".FS9vSn:$#S8R8#P)YwM`=-W-v$(F7]br-$jqVc.q1#XPFIZ<-Wrdk-/n$G7I.A5#;BP)ML<0$N;(wXl,%1'#TQa-v*m$^N8+;T.gG+##+YjfL>?VhL%PG8.OrZ7nMQVK$P6.P+$Oqxu"
		"oZM=-5Y5<-8>N+M;-;hL6r&,M[@&tZw4t;MTlT0/cbN4ouZi[9qwmqN0unMN/uQ=ltCr_&T&>uum_m$v>Yd$vZP;8v%#x3OSeip.hhB(s$Jn34Gn*=-?LM=-86T;-UKau.]XX4f:E,@'"
		"r30@'$f9#vY=i(vua:5/^hX2vEt0hL''b$#B1%wu<Ex1v_Fe8#Lwm,vItxXu;ZKo/i#&9v<'&9v9g<30Vc8igMQ*##P(*GI+4d7vC2FGNqQ<M0&`kxu?,#dM##0fh:j'Yl,]2E,26rdM"
		"brBl%0rX&#xA5C&7gg@M/x1p.`2+##?nr-$nP7:)[QmxuTE9C-?F`t-3Vm&M%>gV-hQ&+%$]'#v'L0%Ms5O'MuhJ4vn9.wu+<r$#F=7wu?gV8v1,fJMj.4A+0J###,a:cVUvQjM0+m<-"
		"(nl-M78(A&ugHN8tCXk4/Pkxu?j.FNGd)fqNKrG%MGHY-/C,.6r=P9Q817r7Z*gf-Y2]S<%1N?%hm@o[7[@,&1/jCThj)fq`n>L,:Fl'&$Mkxul<.6/3&>uuwVl)MRKt'MX7YP8BRo`="
		"C@P&#^V_@-V@*086:lxu(f$]-ZwVe$.Y'#vS.BP8KPNM,q-*edOH8t2<X:#vagR)vx`:5/JZk*vk<4gLuHe##/P(vuXte)v<C^2#2.%wu.sxXu14n0#9qdiL&%Y5v5Bg;-T8RBO;(sFV"
		"L04>dwaTYZ5+lxuc_5<-aD)B'OK6:)IH*S[:5f7[Im_PgN2jo7]KPF%i>[8#b&O$M[.$Yum9(@-oO:&O7?L1pPi[+MLg3_Oi_$j)I?@jOU:dW-pT9x'e[(hLZH6##$6+gLF4$##);#s-"
		"bq0hLt'^fLo(t$#KWt&#P;G##.^Q(#euv(#u-F7/Z%T*#^t7nLc2#&#qrql/<bY+#@.4&#u;#s-x/(pLpoc&#3[%-#TQ2uLt12'#2^+6#5*O$Ms=`'#4=#s-DgT%M%]r'#w$X9#Vr;'M"
		"x[7(#Stql/1MB:#4YH(#S=#s-i_J(M(+o(#W=#s-owo(M*7+)#`=#s-#Rc)M,C=)#k=#s-3Q7+M2hHY6vo+YufT*v#irES77@wo%s]ul&tOBP8UP*&+>&li0C-[i90mGG2Kx))3Las+;"
		"cDFJ:25T`E94Vc;im+8ILA#m/NPHY>'3ti0nYSM9qtEV?eJOJ:w=1,;HdCS@oALPJ)A*/L?Xc%OUc_iTewS`Wq9B`W#qlxXx)MYY#kYxX+K.;Z(TerZ)3;YY52bo[/2BP]0Tr:Z=c#2^"
		"5]Yi^6sRrZE=;J_;1r+`<;4S[Y/DVdMs$8eZ?](an.]oe`l<Pfa^=`a2wi(jo<I`jpAu@b:Q+Akugaxkv`UxbB,CYl%<#;m&)7YcJ]Zrm+g:Sn,Gn:dR7s4o1;Rlo2fNrdcZd(sD+RA="
		"RRY.hBPkcDnCl`E(Y[4o`'*#G/XeYG0(=lolpxrH8H#pI:RtLp+`fcMI%KDNUAf@tEE<;QbZwrQne-v#h[*,V(Y[cV*'#m&q9BDW/1t%X3NYM'0sNS[Aa*5]Grj`*B(di^R%;G``wg]+"
		"ko.8er>`oewf920/b;Gi1.M`j3l2,2B8-;mAWYonI3c]4/'Q)*$l+a*M=BMBm?nP/feI,2t`0>G:AD)3$k'a3%,huG[mI29:%H/:@R&5Jkc'g:E_`G;Fq]lJtFZD<OW9#>TWYiK5$1s?"
		"^]iS@`A7GM?adPAhO'jBio3DNKS]JCp6vcDq=k%O)3D>Y[+_Y#-J./(uRtA#ds3L#lugo.v[9^#xY4L#p7HP/xnp>$$a4L#tO)20*O2w$'?BJ1X-]f1&=2W%+v4L#)F=G2*[%9&1&v(3"
		"lr>H3[?WV$0/5L#3?mx4*bIp&5>5L#2HMY5+ke5'7D5L#:m.;6-'Fm'9J5L#>/fr6/9'N(Ac5L#H(?M91K^/)Ci5L#MI;J:3^>g)Hx5L#^Bk%=67')*O76L#[agx=1K^/)R@6L#cD`r?"
		"7,V)+XR6L#hf[o@7,V)+ZX6L#@U$AO1K^/)WP9L#idIo[v[9^#YV9L#S0*P]v[9^#du9L#u77`a4gY,*n=:L#2'<igkT?>,(o:L#&94ci/3Xm'1*8`jv>W)ku#[Y,14;L#.,d=l%4m;%"
		"5@;L#H=&Vm;PnA,=X;L#R6U1p85rD+H$<L#T;']t>lj>-O9<L#e($Yu9&Q0),VC;$M0xo%`&Ke$kaW?#28P>#R[I%#Tkn@#I%`?#Vh[%#j2qB#cI@@#'0_'#o>-C#q<XA#/H-(#a]ZC#"
		"TSGs-9x7nLw<)J#>3G>#6pWrL)<sJ#H<cY#vp9'#U8-_#US1Z#5o%nLL#Ib#%)rZ#mhY+#&*Dc#CQii1$12,#/Hrc#2aCZ#eZ_pL7,:7B<DUS%GGJJCFOcfCNbff(WR$&FRN<AFV6cc)"
		"jjorH^P18IcgCD*)AEJLlh]fLxuKq;N]&,MvmbJM7&H_&Ru]cMY$B,N9,H_&V7>DN[6#dNFA'REZOu%ODS[DO=8H_&_hU]O`Z:&P?>H_&c*7>Pbmq]PADH_&gBnuPd)R>QmiW3OmgjrQ"
		"LMS;R^$oEIq)KSRiV/sRHYH_&uA,5SkifSSJ`H_&#ZclSm%G5TLfH_&'sCMTo7(mTNlH_&;@T`W?>?)XPC,eZ?X5AX&X4&YU]Pe$G3MYY$kK>ZYiPe$OderZ)B)s[Ih0wpfJBP]9Zs1^"
		":K95&A%Zi^15XM_g:Qe$jUr+`5Ypf`97%@0p$o(a*ESGas-J_&t<O`a?:2)bu3J_&#_K]bBU.&cv,&_].$->cXtAYc$CJ_&,<ducF$F>d&IJ_&0TDVdOd#<e_bb`*n.]oeGQYSf'(Re$"
		"Xwi(jdoB`j4ORe$ZQ+AkX@(&ln;S-Qc,CYl]e?>m<hRe$k]Zrma3WVn@tRe$s7s4owohqovkRM'Zh4Mp%'Hip.-)&+_*l.q&5eNq(@OJ(cBLfq4m`+rQvK_&1[-GrtfdfrS&L_&Jupt#"
		"89kx#vqc+#)<i($*dUv#%.),#1N.)$,^Lv#*:;,#>gR)$Eqpo/2R`,#3se)$M?Qp/6_r,#C)x)$Vd2q/<q7-#>;=*$5#)t-pp$qLC%Z*$@,.w#F9f-#FYk*$vIcs/JEx-#Kf'+$S_Xv-"
		"&?[qL-hM+$So<x#]&u.#^L-,$]+Xx#@8'sLx)o,$QTtv/qc$0#m33-$%3jq/#&I0#[EN-$_#Uw/)8e0#uWj-$5w(&0-Dw0#[d&.$v-:w-_=ZtLZ)C.$*Rqw-gU)uLWBh.$2.g$0?%t1#"
		"fD#/$4rN+.qtVuLh)7/$sVnw#G=B2#&dP/$-Mfr1MO^2#&pc/$$d*x#)IAvLoAw/$,2bx#-USvL`L30$3P9#$;*>wLZws0$)?Iw#A<YwLF'B1$qTGs-IT(xLf;^1$6;xu-W)ixLteG2$"
		"MHk3.[5%#MDrY2$8h=(.`A7#MA.v2$#>%u/>#G6#CBL3$YRqw-x4O$MLq.4$RZU).&Ab$MO&A4$Im@u-*Mt$MQ2S4$7KA/..Y0%M@?f4$GH`t-2fB%MbJx4$6,d%.6rT%MqU45$E$)t-"
		">4$&MtoX5$Hu%'.ELH&Ms0(6$(F3#.JXZ&Mbtq6$X_Lv#Z3N'Me,I7$TUGs-gW/(MtPe7$$K0#$kdA(MFIw7$`$)t-opS(M7T38$b$)t-s&g(M=#k8$N9K$.)KG)M)BB9$(7u(.3ju)M"
		"UNT9$o$)t-7v1*MFYg9$q$)t-;,D*MHf#:$s$)t-?8V*MJr5:$u$)t-CDi*ML(H:$w$)t-GP%+MN4Z:$#%)t-K]7+MP@m:$%%)t-OiI+MRL);$3<pg1&25>#=S:;$)SL;$VxRfLVeM;$"
		"*x(t-Z.ffLXq`;$,x(t-_:xfLgKs;$3lq;$5Yc##H..<$0x(t-gRFgL_?A<$2x(t-k_XgLaKS<$4x(t-okkgLcWf<$6x(t-sw'hLedx<$8x(t-w-:hLgp4=$:x(t-%:LhLi&G=$<x(t-"
		")F_hLqDY=$Dr$<$/X$iLuo(>$Ffh;$^'o%#_Q9>$Dx(t-9wQiLscL>$Fx(t-=-eiLuo_>$Hx(t-A9wiLw%r>$Jx(t-EE3jL#2.?$Lx(t-IQEjL%>@?$Nx(t-M^WjL.u<@$W4@m/6^?(#"
		"h7j@$ck8g1R]j)#'15B$,4t=$.VMmLBBHB$lx(t-2c`mLDNZB$tXwm/c7^*#)h1C$8#Nd%5&_G;,w<)</Aii'uIZD<@,6#>Btux++=S>>GMmY>m]7R*MU4v>EXWw?BO')*9<hS@L1+p@"
		"*4)F.^adPAsG6mBT6Rw9jS]JC6b,gD#X-XCO3D>Y*DYY#BMur$sFbA#ECws-xw9hL`v]%#/2^V-kanvnv+2'#RSl##I;#s-1&BkLa&g%#Q;#s-8P,lLNem##U;#s-NcGlLOkv##W;#s-"
		"?oYlLQw2$#Y;#s-C%mlL^jJ%#[;#s-F1)mL_pS%#jrql/n$T*#q;G##lMYS.#bY+#R3t+.L[=oL%1w+#'#)t-PhOoLiM<,#,Nc##x;#s-d/(pLU9W$#%<#s-uACpLU9W$#'<#s-9'HtL"
		"Nem##[<#s-A2/vLJLH##a<#s-FP]vLJLH##c3^V-]*v&mF6Nj9?Bti_kl7_AnU%G`Ho6`a%.HZ$_8m:dRaKvdQYQM'#`:L#wTvLg&=2W%%f:L#,3sIh'FMs%+x:L#9/1`j/9'N(/.;L#"
		";MHxk0BBj(=X;L#8=T1p<Se&,YqX.q*14Nq(R0,)aHqFruo),sK@Ae?5tmCs'PAds0qKG)n>j@t&Y=AuS^Se$W)Guu+JcY#*M:;$+M(v#;1D;$g.<X(a(@8%8Xsl&2YYY#Q2.)*QOoI*"
		"AIrr$UJe`*H,*&+LKgi'YcEA+P4S&,s`Ke$+2B>,'q5^,%GD_&1V>;-C+0Z-.1=X(c:Ds-gS,lL';=D#mk@u-olPlLHXOD#hc:?#/Y`mLQK-F#lSGs->4SnL`8wF#1>.@#QkOoLqDNG#"
		"A?Ib3MQ=.#UfkI#=3G>#Vjb.#Ux0J#D#)t-6pWrL)*sJ#^fw[#wvB'#Z>6_#Pf(T.ZuA*#Kp4@&X2RS@u%8R*h_#e#>s_Z#E6f-#EMOe#?HuY#Y)(/#LX6g#KT1Z#nf-0#T'ng#^5.[#"
		"'5e0#5's>&<o1/:VwK#$'tV#&*]T`N4I=e?ZOu%ODS[DO=8H_&_hU]O`Z:&P?>H_&c*7>Pbmq]PADH_&gBnuPe2nYQ>mOe$os/8RcD/sRB#Pe$wMGPSgiF5TF/Pe$))`iTj.C2Ut_r92"
		"-A@JUrR$jUQuH_&1Yw+VteZJVS%I_&5rWcVvw;,WU+I_&949DW5#as[F%BA+O$o(aNdscanOQe$#_K]b<C.&c:l,F.($->cg%m]c$CJ_&,<ducF$F>d&IJ_&0TDVdZ`7qfg6_]+v_t1g"
		"TZ2Mg0hJ_&DxTigR;72h2nJ_&H:6JhTMnih4tJ_&LRm+iV`NJi6$K_&PkMci_@g,j_]uo%69J`jV.GDk6URe$kjbxkg@v=lkIRM'B,CYlbn$#mAEK_&gD$;md*[YmCKK_&k]Zrmf<<;n"
		"EQK_&ou;SnhNsrnGWK_&s7s4ojaSSoI^K_&3PSlovg,Mpwh72']tOip%-d.qNmK_&771Jq0ZDfq(:4/(eNh+rs]HJrR#L_&3hHcruo),sT)L_&=Ba%t0<P3DOV/)$+5`T.*:;,#/XZ&%"
		"_B$p@LRpiB#/8R*kYofCLT-0DO*CD*Q(lcD.B(<-PW=.#D(L+$B^Lv#Xpb.#R@q+$GdUv#a21/#X_H,$NG`t-@8'sL&t[,$RlWN0#&I0#pQa-$W.41:U/>)O6L=e?b$%#Pav6BPhtro%"
		"F<[YPhBq#QdC$v#JT<;Qd0qYQDE6_AnmsrQMOP;RGSH_&r/TSRu%0sRN2/XC&H55S/5LPSQ@AX($allS/+)6Tr0<8%eFIJUrR$jUkM+F.8`*,V)`[cVOGPe$::BDWs*8)Xd;lQa@_>AX"
		"=TPaX%(D>#+EruY%tgYZ6tk34ieIT.4$u2$)PD&.fSR#MrVD3$=-.w#nlw#M6QV3$Bm@u-rx3$Mjk%4$4UGs-$;X$Mnu74$5PD&.(Gk$M-2S4$UUR2..Y0%M;/ls%=u:/:#3h#$lr.8e"
		"v%%se'%Re$=MFPfs.wofSQt92G4$/hf,rih/=Re$MXv+i[,3KiZ;#s$2qVciXr/,j^pt92U38DjZ.gcjGG<R*ddO]kr_dxkVcqEIb&1>lae_]lABK_&f>hulcw?>mCHK_&vVHVmu6]rm"
		"w9KG)P%ESna:vrnYG#LGt=&5o%@WSo=>Hk=+V]lo%tp1p'F0,)(tV#&=u:/:VwK#$(tV#&=u:/:VwK#$(tV#&=u:/:VwK#$(tV#&=u:/:VwK#$(tV#&?l_?@UhV;$FBOW-#o_?@^-W;$"
		"5Yc##847<$+SGs-k_XgL[Q]<$/SGs-sw'hL`j+=$3SGs-%:LhLd,P=$Ck8g1[we%#NK0>$Hlq;$7qHiLr]C>$Ex(t-;'[iLtiU>$Gx(t-?3niLvuh>$Ix(t-C?*jLx+%?$Kx(t-GK<jL"
		"$87?$Mx(t-KWNjL&DI?$Ox(t-OdajL(P[?$Ps%'.SpsjL*]n?$Sx(t-W&0kL,i*@$Ux(t-[2BkL.u<@$&Qqw-`>TkL0+O@$r,2i16^?(#i1a@$iFe<$hV#lL4Ct@$^x(t-ni>lL2[BA$"
		"[SGs-v+dlL6tgA$`SGs-(D2mLET-B$u@[<$Tcs)#'7>B$kx(t-0]VmLCHQB$mx(t-4iimLN#*C$vfh;$@7JnLGVZJ;'Zpo%s=?)<8KXD<=eux+wUv`<5N9&=7xEG)%oVA=&W+a=^<F_&"
		"I=S>>M4)^>)Uw?0MU4v>S9pPBsJ?X(hGA/C3F0jCk?Ne$$m=,DKqUGDM_+,)S.ucDfJxbFZT$&+)3D>Y,D>>#-(35&CKu/(V4&n&trBeGk=]rH)>MMFw6@;$Bhl&#5wkf(*6(<-AF.%#"
		"jj9'#CVs)#o$),#AH4.#jl?0#<:K2#e^V4#=tEX%Q4NP&W%pu,,)r%4]D9G;/6ZlAW'&;H*oF`NR`h.U%Q3S[Zf`s-1l:$#Y9F&#.jd(#_I5+#1n@-#Y;L/#,`W1#T-d3#T>@#M#Rmv$"
		"$KVB-cE?H=lM3MK,T@C-[A-x7oWW.h2ZIC-#u(G>nn//1]a(g%`xuu#Gp?D*paai07VR]4a]p(<3N;MB[?]rH.1(AOVxHfU)jj4]Z`1j(Ud,<-I_R%#r,_'#KoA*#w<M,#IaX.#r.e0#"
		"DRp2#mv%5#9OwW%Yefi'`U18.4Y3>5euP`<7gr.C`W=SI2I_xOZ:*GV-,Kl]T(x5/<GY##Mke%#v8q'#FDW)#gbY+#90f-#bSq/#4x&2#]E24#a%'<N,T)<%OJW]+x;#,2He,87i72A="
		";)SfCdpt4J6b?YP_Ra(W1D,M^Vr@T.=:r$#f^''#:8E)#W1g*#)Ur,#Q#(/#$G31#Lk>3#u8J5#?[3X%l?(,)h0IP/2/;D3U#Xf:+t#5ASeDYG&Vf(NNG1MTw8RrZ@.)J_:FmuB_w`e$"
		"+4ae$1Fae$Wo.i$4+D.GZ'$w]tj-kEq[bSIKiUPKQINJMW*GDOaK%EF?GD;Q0Eg;-dHg;-jHg;->)Ep.lEvsB5;wj$1$Y5Bd1de$4p]LFvN_`a>IR]cD*KVeJaCPgbT@aF%%AGi8Eg;-"
		"VIg;-NW#%.__(aMi+aaMoOAbM>7o_W:e=rLpb2Z#*=vV%0tnP'6TgJ)]?.&Gr-bD+BlW>-HLP8/N-I21e#X#HjvTc$TGg;-ZGg;-aGg;-6+Ep.R`k[$jnWe$s_`e$#r`e$KP@i$4OhMB"
		"#2U,3pQae$;eae$dCAi$M+J5J$;qG32Ebe$SWbe$&7Bi$g],sQ%D6d3J8ce$lJce$>*Ci$*9eYY&MQ)4c+de$.>de$VsCi$CkFAb'VmD4%ude$F1ee$ofDi$]F))j(`2a4=hee$_$fe$"
		".G*i$K<+cHc<raMq[SbMvwoFMS$s;$CVX&#D[P7A<VL^#47no%JmoP'20O2(1'4m'/kR5'fRfS8heF59g[+p8eIJ88)4$$$M$HS7EIe'&ZnFJ(&FGR*`iQ>6g73v6`rmY6^`6#6,OV8&"
		".b7p&-XrS&jw'm9l3_M:k*C2:inbP9RQaJ2TdA,3SZ&g2QHE/2ZD:&5]Vq]5[MUA5Y;u`4'x#Z$)4Z;%(+?v$&o^>$6TgJ)8gG,*7^,g)5KK/):#)d*<5`D+;,D)+9pcG*>G@&,@Yw],"
		"?P[A,=>%a+BlW>-D(9v-CusY-Ac<#-F:pV.HLP8/GC5s.E1T;.J_1p/LqhP0KhL50IUlS/N-I21P?*j1O6eM1M$.m0Vvxc3X2YD4W)>)4Y/,H3]eMlS$ig>$J`uo.[Rw`aZ.6L#B%qaN"
		"8cG,a7'Hh(GFY8//;^V-H'OL#'s&eM`Q&t'E>G8J/;^V-v(OL#_o+eMM<9.(/*X2(K>^V-l*OL#5@G*W@LlTePGs+j6rj-$^%Z##PeSe%GNYY#tT3]ksvNf_HX.G`Lqe(aP3F`aTK'Ab"
		"Xd^xb]&?Yca>v:deVVrdio7Sem1o4fqIOlfub0Mg7/2SnSM]P8Lvc4oSFJloW_+Mp[wb.q`9CfqdQ$GrhjZ(sl,<`spDs@tt]Sxtxu4Yuu?[%&[r,r#,MuY#0Y1Z#4fCZ#8rUZ#<(iZ#"
		"f@7[#HLI[#Pen[#Tq*]#X'=]#_9X]#c?b]#v.Pu#$;cu#)Juu#,S1v#0`Cv#4lUv#8xhv#<.%w#@:7w#DFIw#HR[w#L_nw#Pk*x#Tw<x#X-Ox#]9bx#aEtx#eQ0#$i^B#$mjT#$qvg#$"
		"u,$$$#96$$'EH$$+QZ$$/^m$$3j)%$7v;%$;,N%$?8a%$CDs%$GP/&$K]A&$OiS&$Suf&$W+#'$]=>'$_CG'$dOY'$h[l'$lh(($pt:($t*M($'VFM$.dXM$2pkM$6&(N$:2:N$>>LN$"
		"BJ_N$FVqN$Jc-O$No?O$R%RO$V1eO$Z=wO$_I3P$COLV$e[NP$ihaP$mtsP$q*0Q$u6BQ$#CTQ$'OgQ$+[#R$1n>R$tA'>$iNe<$gZ[[#f_gq#C@r$#r=Ix*B.,#.K&^>(j.#/Lu7ms-"
		"=(pd8e)8_#T]Z_#_/uU$TRrf8oEqB#)P:;$s09Q$e3og8Axe,XTeW$'xV3&+0B6A-T2?;1V;`V89Z^?do+SSexe02gR^W>#='`d=rNTYcjWed4dln`*qkm;-RYN;&/xV^6mq6w^Ym@M9"
		"^/x.:bGXf:f`9G;jxp(<n:Q`<rR2A=vkix=$.JY>(F+;?,_br?0wBS@49$5A7X_xOKqAonO4&PoSL]1pWe=ip['uIq`?U+rdW6crhpmCsl2N%tpJ/]ttcf=ux%Guu&2G>#*J(v#.c_V$"
		"X'JJ145DD3D@tu5M$QS7cMkf:J]+,DNubcDR7CDEVO$&FZhZ]F_*<>GcBsuGgZSVHks48Io5loIsMLPJwf-2K%)eiK)AEJL0o]cM8Iu%O@$7>PHTNVQP/goRX`(2Ta:@JUikWcVqEp%X"
		"#w1>Y+QIVZ9P^l]A+v._I[7G`Q6O`aZp,>cdMDVdl(]oetXt1g&46Jh2'/Dj:WF]kB2_ulJcv7nR=8PoZnOipeT-GrY*%vPK^WVQOv88RS8poRWPPPS[i12T`+iiTdCIJU.Y$,`Q0=Da"
		"wt#/h:^kxkfMbJ:q=ZD<1hkV?9HH5ABDur$)Yqr$`*EM0knar?v['AOe+<X(-&<X(.=r:Q]DC_&k@Me$>4Rc;^i^VI3B<,<E:&pA<C1^#=s`=--x%Y-cZ-S*kp)S*,b-S*QwuR*dq[%#"
		",.;t-Np_xLS5pfLOL-##2`$s$OV5lLV'Go$i>V8#01i?#2C5F#^(eK#q=1R#`]ot#riv_#7Y6g#Rdrk#)-Z7$S9bx#.8a%$YM-,$,r8.$/Vh3$CuOU$0,W@$CA$G$_1:N$%E;o$OQBY$"
		"*vM[$RCY^$0?Vg$/Do5%=GY##EF.%#^9F&#x>?_#QWT,.F8SnLJ<Tl#qSGs-Co(*MNZGM$GTGs-Ccl)MPNB5#,`_;$]sPW-9vg'&rJp-$YLjl&:g0F%0-j]Ot=e(W'1r`=t97&>vE4&Y"
		"d)Ke$P-'T%FNxoJ#b0#ZT`Yq;P].R3w5(sH;i-,W-gLqDq3dk+#A6#?n?N>?B?BDX+@J,*>BXPK-R+d*SKV#$3R2qi]8AVMv2tuLNT#m#7rl1NkMt1^uc<;RCSfl^rq6XL#:IYMu[#WM"
		"DrQ##A:r$#Y-4&#HXpSMPN:[MCTiPM*j5WM&.u0Rj[rPMP`%QMRl7QMe,+JM?4eTM@oNr7,f45T.9:'Y1435&Ip$)*bQYrdqFriL3%mlTG4NVeT]iof/%MMUcl[ih0-n>$#<)=-:;oI-"
		"^enI-_enI-2gnI-0enI-Z*jE-pr-B&1-EL,fkI^6*7,,MG@g1$4BcG-*-Xk'02EX(TeLJC)O,p8GHL88d;mV7%T3^#,0A>-91g%)s1r?Tt4r?T4INe$u*1,MuZGM$N#)t-Xr638LvfcN"
		"Nd+<-ewX@&[8H_&-6/#P6LUDF9.l`FJ[vS&gN@s%0i?5KlRc;%EwD;IpD3B#eK`YPMVD)+x?N<-FjEB-`TGs-s`s,88#JP^vlWVRD]+2_lrPW-ppLR*D9)90&;%pIpX(,a*-/vHIVtoS"
		"oa[ca;(5_ShfQSInb?8Jfn]A,_aXMLMq+a+h[UPT$La<-egCH-q_4?-m[2E-Q&BN-d;S>-*mr=-Y&^j-^hSX(;WA5OOrl?^$<j3O3km3O1cg3O[Q`wBH^w]mIX+)W4_acWx?N<-v/;t-"
		"JTCXMKfI+8g,C#$,bo`XGtCL,J8Se6gA.F@d#dL>i2raMJq[+8Ce2Gsj49B##^W%t#pMcs%,/DtPkUv[nDTYGP71#H/dc`XB6gG*U8ulKxZ'#ZM;uZ-:L=X(oH8m/f*.j1sDJ21wibJ2"
		":9n>$rvX?-pvX?-vvX?-tvX?-.EY?-)wX?-'wX?--wX?-+wX?-;EY?-*wX?-(wX?-.wX?-,wX?-BfZm%*=wQjcnFJ(rZ*20NR`i0=&hQa5%&(#`Sl##?2<)#o=N)#vF.%#]1g*#/=#+#"
		"6rn%#rtu+#B*2,#MRk&#4[%-#Qq5tLs=`'#L<#s-Y9dtLuIr'#R<#s-b^DuL'i.(#UqG3#QGQ&M%r[(#kRVs-W4+gLoG-##S:9q#baM=-?_Zx/c3mR$?B)S$pJB40]dsh#G'mj#v(s&d"
		"s3.5/_Gk8&IUI-+CO+DWoN6j:T]Pe$M6A>#'.#>Y.+L^#pkeu>GB-p8g:iJ):qacWtjvf;x3ADX@]YF@gcP(/w4I_&=+r'/%A7(&@r<kF<<rA#'G4v-MaBhN]3=n-8b0:)HN);)2I5:)"
		"kQU:)3UoQa,4QV#u/x6/08P>#t,#)MZF6##,6HIO?26&#[,q^#%3$_#)E?_#gQ'^#KR4m#ojK^#p]DQ0Vd0'#w&U'#+V8l:EMH/;f<Rq$54x202cuu#uW[P/SRn92@l:J:t1XrZPbpkC"
		"ko[V6-Wpf13&mc2E=bY5_uxL,tXRS%S2`c)P5@D*TMw%+T/DG)P;^X%Kn-s-c6N>GgN/vGkgfVH]Ni>$cX=;.J`L;H@0wV%VvY@9(^Ow93FWwB_h[wBhe5^#VI`u&<<(@'`'(,))5###"
		"JNlQW#kJe$00k-$)kpw'BlUf:';YY#tV_lJfu72h#JGxI+L>gLx.VB#E*r*5?ke%#bV?(#g/:/#$q)`#;&<`#?2N`#=JwA-Vlw8=qUR$95qBT.K^''#`.tT#'=0U#+IBU#/UTU#3bgU#"
		"EkJa#QoSa#UuxA$V>dD-%EWm.Xqn%#1vtg0lbub#r?D0$7J9Y$>o8B;W####";


static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return roboto_medium_compressed_data_base85;
}
