#pragma once

#include <cstdint>
#include <string_view>

namespace RE
{
    struct NiUpdateData
    {
        float time{ 0.0F };
        std::uint32_t flags{ 0 };
    };

    struct NiTransform
    {
        float scale{ 1.0F };
    };

    class NiAVObject
    {
    public:
        virtual ~NiAVObject() = default;
        virtual void UpdateDownwardPass(NiUpdateData&, std::uint32_t) {}
        virtual void UpdateWorldData(NiUpdateData*) {}

        NiAVObject* parent{ nullptr };
        NiTransform local{};
        NiTransform world{};
    };

    class NiNode : public NiAVObject
    {
    public:
        void UpdateDownwardPass(NiUpdateData&, std::uint32_t) override {}
        void UpdateWorldData(NiUpdateData*) override {}
    };

    class BSTriShape : public NiAVObject
    {
    public:
        void UpdateDownwardPass(NiUpdateData&, std::uint32_t) override {}
        void UpdateWorldData(NiUpdateData*) override {}
    };

    template <class T>
    class NiPointer
    {
    public:
        NiPointer() = default;
        explicit NiPointer(T* a_value) noexcept :
            value_(a_value)
        {}

        [[nodiscard]] explicit operator bool() const noexcept { return value_ != nullptr; }
        [[nodiscard]] T* get() const noexcept { return value_; }
        [[nodiscard]] T* operator->() const noexcept { return value_; }
        T& operator*() const noexcept { return *value_; }

    private:
        T* value_{ nullptr };
    };

    class Moon
    {
    public:
        NiPointer<NiNode> moonNode{};
        NiPointer<NiNode> shadowNode{};
        NiPointer<BSTriShape> moonMesh{};
        NiPointer<BSTriShape> shadowMesh{};
        std::uint32_t size{ 1 };
    };

    class Sky
    {
    public:
        [[nodiscard]] static Sky* GetSingleton() noexcept
        {
            static Sky singleton;
            return &singleton;
        }

        Moon* masser{ nullptr };
        Moon* secunda{ nullptr };
    };

    class Setting
    {
    public:
        enum class Type
        {
            kInteger
        };

        [[nodiscard]] Type GetType() const noexcept { return Type::kInteger; }
        [[nodiscard]] std::int32_t GetInteger() const noexcept { return value_; }
        void SetInteger(const std::int32_t a_value) noexcept { value_ = a_value; }

    private:
        std::int32_t value_{ 1 };
    };

    class GameSettingCollection
    {
    public:
        [[nodiscard]] static GameSettingCollection* GetSingleton() noexcept
        {
            static GameSettingCollection singleton;
            return &singleton;
        }

        [[nodiscard]] Setting* GetSetting(const char* a_name) noexcept
        {
            const std::string_view name{ a_name ? a_name : "" };
            if (name == "iMasserSize") {
                return &masser;
            }
            if (name == "iSecundaSize") {
                return &secunda;
            }
            return nullptr;
        }

        Setting masser{};
        Setting secunda{};
    };
}
