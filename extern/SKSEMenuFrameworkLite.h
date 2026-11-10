#pragma once

// Minimal consumer declarations for the SKSE Menu Framework 3 C exports.
// The official full API header is LGPL-2.1 and is available from QTR-Modding.
// This file intentionally exposes only the functions used by this plugin.

#include <Windows.h>

#include <cstddef>
#include <string>
#include <utility>

namespace SKSEMenuFrameworkLite::Detail
{
    [[nodiscard]] inline HMODULE GetModule() noexcept
    {
        static HMODULE module = nullptr;
        if (!module) {
            module = GetModuleHandleW(L"SKSEMenuFramework");
        }
        return module;
    }

    template <class T>
    [[nodiscard]] inline T GetFunction(const char* a_name) noexcept
    {
        const auto module = GetModule();
        return module ? reinterpret_cast<T>(GetProcAddress(module, a_name)) : nullptr;
    }
}

namespace SKSEMenuFramework
{
    namespace Model
    {
        using RenderFunction = void(__stdcall*)();
        using AddSectionItemFunction = void (*)(const char*, RenderFunction);
        inline std::string sectionKey;
    }

    [[nodiscard]] inline bool IsInstalled() noexcept
    {
        return SKSEMenuFrameworkLite::Detail::GetModule() != nullptr;
    }

    [[nodiscard]] inline float GetMenuFrameworkVersion() noexcept
    {
        using Function = float (*)();
        static auto function =
            SKSEMenuFrameworkLite::Detail::GetFunction<Function>("GetMenuFrameworkVersion");
        return function ? function() : 0.0F;
    }

    inline void SetSection(std::string a_key)
    {
        Model::sectionKey = std::move(a_key);
    }

    [[nodiscard]] inline bool AddSectionItem(std::string a_menu, Model::RenderFunction a_renderer)
    {
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Model::AddSectionItemFunction>("AddSectionItem");
        if (!function) {
            return false;
        }

        const auto path = Model::sectionKey + "/" + a_menu;
        function(path.c_str(), a_renderer);
        return true;
    }
}

namespace ImGuiMCP
{
    struct ImVec2
    {
        float x;
        float y;
    };

    using ImGuiSliderFlags = int;

    inline void TextUnformatted(const char* a_text, const char* a_end = nullptr)
    {
        using Function = void (*)(const char*, const char*);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igTextUnformatted");
        if (function) {
            function(a_text, a_end);
        }
    }

    inline void Separator()
    {
        using Function = void (*)();
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSeparator");
        if (function) {
            function();
        }
    }

    inline void SeparatorText(const char* a_label)
    {
        using Function = void (*)(const char*);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSeparatorText");
        if (function) {
            function(a_label);
        }
    }

    inline void SameLine(const float a_offset = 0.0F, const float a_spacing = -1.0F)
    {
        using Function = void (*)(float, float);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSameLine");
        if (function) {
            function(a_offset, a_spacing);
        }
    }

    inline void Spacing()
    {
        using Function = void (*)();
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSpacing");
        if (function) {
            function();
        }
    }

    inline void SetNextItemWidth(const float a_width)
    {
        using Function = void (*)(float);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSetNextItemWidth");
        if (function) {
            function(a_width);
        }
    }

    [[nodiscard]] inline bool Button(const char* a_label, const ImVec2 a_size = {})
    {
        using Function = bool (*)(const char*, ImVec2);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igButton");
        return function ? function(a_label, a_size) : false;
    }

    [[nodiscard]] inline bool Checkbox(const char* a_label, bool* a_value)
    {
        using Function = bool (*)(const char*, bool*);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igCheckbox");
        return function ? function(a_label, a_value) : false;
    }

    [[nodiscard]] inline bool Combo(
        const char* a_label,
        int* a_current,
        const char* const a_items[],
        const int a_itemCount,
        const int a_popupHeight = -1)
    {
        using Function = bool (*)(const char*, int*, const char* const[], int, int);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igCombo_Str_arr");
        return function ? function(a_label, a_current, a_items, a_itemCount, a_popupHeight) : false;
    }

    [[nodiscard]] inline bool SliderInt(
        const char* a_label,
        int* a_value,
        const int a_minimum,
        const int a_maximum,
        const char* a_format = "%d",
        const ImGuiSliderFlags a_flags = 0)
    {
        using Function = bool (*)(const char*, int*, int, int, const char*, ImGuiSliderFlags);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igSliderInt");
        return function ? function(a_label, a_value, a_minimum, a_maximum, a_format, a_flags) : false;
    }

    inline void BeginDisabled(const bool a_disabled = true)
    {
        using Function = void (*)(bool);
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igBeginDisabled");
        if (function) {
            function(a_disabled);
        }
    }

    inline void EndDisabled()
    {
        using Function = void (*)();
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igEndDisabled");
        if (function) {
            function();
        }
    }

    [[nodiscard]] inline bool IsItemDeactivatedAfterEdit()
    {
        using Function = bool (*)();
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igIsItemDeactivatedAfterEdit");
        return function ? function() : false;
    }

    [[nodiscard]] inline bool IsItemEdited()
    {
        using Function = bool (*)();
        static auto function = SKSEMenuFrameworkLite::Detail::GetFunction<Function>("igIsItemEdited");
        return function ? function() : false;
    }
}
