#pragma once

namespace CMS2
{
    struct ConfigData
    {
        float scaleAmount{ 3.0F };
        float duration{ 5.0F };
        float shakeIntensity{ 0.5F };
        float gravity{ 15.0F };
        float gravityRadius{ 5000.0F };
    };

    struct SettingsSnapshot
    {
        ConfigData config{};
        bool baselineReady{ true };
    };

    class Settings final
    {
    public:
        static Settings& GetSingleton() noexcept;

        bool Load();
        bool Save() const;

        void Commit(ConfigData a_config);

        [[nodiscard]] SettingsSnapshot GetSnapshot() const;

        [[nodiscard]] static const std::filesystem::path& GetPath() noexcept;

    private:
        Settings() = default;

        mutable std::mutex lock_;
        ConfigData config_{};
    };
}