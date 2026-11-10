#include "UI.h"

#include "Config.h"
#include "MoonController.h"
#include "SKSEMenuFrameworkLite.h"

namespace
{
    namespace ImGui = ImGuiMCP;

    void TextLine(const std::string& a_text)
    {
        ImGui::TextUnformatted(a_text.c_str());
    }

    void __stdcall Render()
    {
        auto& settings = CMS2::Settings::GetSingleton();
        auto snapshot = settings.GetSnapshot();

        auto config = snapshot.config;
        bool saveNow = false;

        ImGui::SeparatorText("Scaling");

        int scaleInt = static_cast<int>(config.scaleAmount * 10.0F);
        const auto beforeScale = config.scaleAmount;
        ImGui::SliderInt("Scale amount (x10)", &scaleInt, 10, 100, "%d");
        config.scaleAmount = static_cast<float>(scaleInt) / 10.0F;
        const bool changedScale = (config.scaleAmount != beforeScale) || ImGui::IsItemEdited();
        saveNow = saveNow || ImGui::IsItemDeactivatedAfterEdit();

        int durInt = static_cast<int>(config.duration);
        const auto beforeDur = config.duration;
        ImGui::SliderInt("Duration (s)", &durInt, 1, 30, "%d");
        config.duration = static_cast<float>(durInt);
        const bool changedDur = (config.duration != beforeDur) || ImGui::IsItemEdited();
        saveNow = saveNow || ImGui::IsItemDeactivatedAfterEdit();

        int shakeInt = static_cast<int>(config.shakeIntensity * 100.0F);
        const auto beforeShake = config.shakeIntensity;
        ImGui::SliderInt("Shake (x100)", &shakeInt, 0, 200, "%d");
        config.shakeIntensity = static_cast<float>(shakeInt) / 100.0F;
        const bool changedShake = (config.shakeIntensity != beforeShake) || ImGui::IsItemEdited();
        saveNow = saveNow || ImGui::IsItemDeactivatedAfterEdit();

        int liftInt = static_cast<int>(config.gravity);
        const auto beforeLift = config.gravity;
        ImGui::SliderInt("Lift speed", &liftInt, 1, 50, "%d");
        config.gravity = static_cast<float>(liftInt);
        const bool changedLift = (config.gravity != beforeLift) || ImGui::IsItemEdited();
        saveNow = saveNow || ImGui::IsItemDeactivatedAfterEdit();

        int radInt = static_cast<int>(config.gravityRadius / 100.0F);
        const auto beforeRad = config.gravityRadius;
        ImGui::SliderInt("Radius (x100)", &radInt, 10, 200, "%d");
        config.gravityRadius = static_cast<float>(radInt) * 100.0F;
        const bool changedRad = (config.gravityRadius != beforeRad) || ImGui::IsItemEdited();
        saveNow = saveNow || ImGui::IsItemDeactivatedAfterEdit();

        if (changedScale || changedDur || changedShake || changedLift || changedRad) {
            settings.Commit(config);
            CMS2::MoonController::RequestApplyConfigured();
        }

        snapshot = settings.GetSnapshot();
        ImGui::Spacing();
        TextLine(std::format("Scale: {:.1f}x  Dur: {:.1f}s  Lift: {:.0f}  Rad: {:.0f}", snapshot.config.scaleAmount, snapshot.config.duration, snapshot.config.gravity, snapshot.config.gravityRadius));

        ImGui::Spacing();
        if (ImGui::Button("Pull Moon")) {
            CMS2::MoonController::PullMoon();
            saveNow = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Pull Sun")) {
            CMS2::MoonController::PullSun();
            saveNow = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            auto cfg = config;
            cfg.scaleAmount = 3.0F;
            cfg.duration = 5.0F;
            cfg.shakeIntensity = 0.5F;
            cfg.gravity = 15.0F;
            cfg.gravityRadius = 5000.0F;
            settings.Commit(cfg);
            CMS2::MoonController::RequestApplyConfigured();
            saveNow = true;
        }

        if (saveNow) {
            settings.Save();
        }
    }
}

namespace CMS2::UI
{
    void Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::warn("SKSE Menu Framework was not detected; INI configuration remains available");
            return;
        }

        const auto frameworkVersion = SKSEMenuFramework::GetMenuFrameworkVersion();
        if (frameworkVersion < 3.0F) {
            logger::error("SKSE Menu Framework 3.x is required; detected version {:.2f}", frameworkVersion);
            return;
        }

        SKSEMenuFramework::SetSection("Power of Gravity");
        if (!SKSEMenuFramework::AddSectionItem("Power of Gravity", Render)) {
            logger::error("SKSE Menu Framework did not expose AddSectionItem");
            return;
        }

        logger::info("Registered the SKSE Menu Framework page (framework version {:.2f})", frameworkVersion);
    }
}
