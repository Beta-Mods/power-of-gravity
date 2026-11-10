#include "Config.h"

namespace
{
    constexpr std::string_view kSectionName = "PowerOfGravity";

    [[nodiscard]] std::string Trim(std::string_view a_value)
    {
        const auto first = a_value.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) return {};
        const auto last = a_value.find_last_not_of(" \t\r\n");
        return std::string(a_value.substr(first, last - first + 1));
    }

    [[nodiscard]] std::string Lower(std::string a_value)
    {
        std::ranges::transform(a_value, a_value.begin(), [](const unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return a_value;
    }

    [[nodiscard]] std::optional<float> ParseFloat(const std::string_view a_value) noexcept
    {
        const auto trimmed = Trim(a_value);
        if (trimmed.empty()) return std::nullopt;
        try { return std::stof(std::string(trimmed)); }
        catch (...) { return std::nullopt; }
    }


    void ParseKeyValue(CMS2::ConfigData& a_config, const std::string& a_key, const std::string& a_value)
    {
        const auto key = Lower(Trim(a_key));
        const auto value = Trim(a_value);

        if (key == "scaleamount") {
            if (const auto parsed = ParseFloat(value)) a_config.scaleAmount = *parsed;
        } else if (key == "duration") {
            if (const auto parsed = ParseFloat(value)) a_config.duration = *parsed;
        } else if (key == "shakeintensity") {
            if (const auto parsed = ParseFloat(value)) a_config.shakeIntensity = *parsed;
        } else if (key == "gravity") {
            if (const auto parsed = ParseFloat(value)) a_config.gravity = *parsed;
        } else if (key == "gravityradius") {
            if (const auto parsed = ParseFloat(value)) a_config.gravityRadius = *parsed;
        }
    }
}

namespace CMS2
{
    Settings& Settings::GetSingleton() noexcept
    {
        static Settings singleton;
        return singleton;
    }

    const std::filesystem::path& Settings::GetPath() noexcept
    {
        static const std::filesystem::path path{ "Data/SKSE/Plugins/PowerOfGravity.ini" };
        return path;
    }

    bool Settings::Load()
    {
        ConfigData loaded{};
        std::ifstream input(GetPath());
        if (!input.is_open()) {
            Commit(loaded);
            return false;
        }

        bool inSection = false;
        std::string line;
        while (std::getline(input, line)) {
            const auto trimmed = Trim(line);
            if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#') continue;
            if (trimmed.front() == '[' && trimmed.back() == ']') {
                const auto section = Lower(Trim(std::string_view(trimmed).substr(1, trimmed.size() - 2)));
                inSection = section == Lower(std::string(kSectionName));
                continue;
            }
            if (!inSection) continue;
            const auto separator = trimmed.find('=');
            if (separator == std::string::npos) continue;
            ParseKeyValue(loaded, trimmed.substr(0, separator), trimmed.substr(separator + 1));
        }

        Commit(loaded);
        logger::info("Loaded configuration from {}", GetPath().string());
        return true;
    }

    bool Settings::Save() const
    {
        ConfigData config;
        {
            std::scoped_lock guard(lock_);
            config = config_;
        }

        std::error_code error;
        std::filesystem::create_directories(GetPath().parent_path(), error);
        if (error) return false;

        auto temporaryPath = GetPath();
        temporaryPath += ".tmp";

        std::ofstream output(temporaryPath, std::ios::trunc);
        if (!output.is_open()) return false;

        output
            << "; Power of Gravity\n"
            << "[PowerOfGravity]\n"
            << "ScaleAmount=" << config.scaleAmount << '\n'
            << "Duration=" << config.duration << '\n'
            << "ShakeIntensity=" << config.shakeIntensity << '\n'
            << "Gravity=" << config.gravity << '\n'
            << "GravityRadius=" << config.gravityRadius << '\n';
        output.close();

        std::filesystem::rename(temporaryPath, GetPath(), error);
        if (error) {
            std::filesystem::remove(temporaryPath, error);
            return false;
        }
        return true;
    }

    void Settings::Commit(ConfigData a_config)
    {
        std::scoped_lock guard(lock_);
        config_ = a_config;
    }

    SettingsSnapshot Settings::GetSnapshot() const
    {
        std::scoped_lock guard(lock_);
        return SettingsSnapshot{ .config = config_, .baselineReady = true };
    }
}