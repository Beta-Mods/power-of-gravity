#include "Config.h"
#include "MoonController.h"
#include "TestHarness.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
    [[nodiscard]] bool Near(const float a_left, const float a_right) noexcept
    {
        return std::fabs(a_left - a_right) < 0.0001F;
    }

    void WriteConfiguration(const std::string& a_contents)
    {
        std::filesystem::create_directories("Data/SKSE/Plugins");
        std::ofstream output("Data/SKSE/Plugins/MoonScalerNG.ini", std::ios::trunc);
        CMS2_CHECK(output.is_open());
        output << a_contents;
        output.close();
    }

    [[nodiscard]] std::string ReadConfiguration()
    {
        std::ifstream input("Data/SKSE/Plugins/MoonScalerNG.ini");
        CMS2_CHECK(input.is_open());
        const std::string contents{
            std::istreambuf_iterator<char>{ input },
            std::istreambuf_iterator<char>{}
        };
        input.close();
        return contents;
    }

    void ResetDataDir()
    {
        std::error_code ec;
        std::filesystem::remove_all("Data", ec);
    }
}

int main()
{
    ResetDataDir();

    // INI parsing: whitespace, case differences, malformed values, comments, missing/unknown sections.
    WriteConfiguration(
        "; comment\n"
        "# comment\n"
        "[Ignored]\n"
        "MasserPercent=999\n"
        "[moonsizes]\n"
        "  Mode = ABSOLUTE  \n"
        "MasserPercent = 150 \n"
        "SecundaPercent=not-a-number\n"
        "MasserAbsolute= 120 \n"
        "SecundaAbsolute=\n"
        "LinkMoonSizes = YES \n"
        "LivePreview = off \n"
        "AdvancedRange = 1 \n"
        "MalformedLine\n"
        "[Other]\n"
        "Mode=Relative\n");

    auto& settings = CMS2::Settings::GetSingleton();
    CMS2_CHECK(settings.Load());
    auto snapshot = settings.GetSnapshot();
    CMS2_CHECK(snapshot.config.mode == CMS2::SizeMode::kAbsolute);
    CMS2_CHECK(snapshot.config.masserPercent == 150);
    CMS2_CHECK(snapshot.config.secundaPercent == 100);  // malformed retained default
    CMS2_CHECK(snapshot.config.masserAbsolute == 120);
    CMS2_CHECK(snapshot.config.secundaAbsolute == 40);  // empty retained default
    CMS2_CHECK(snapshot.config.linkMoonSizes);
    CMS2_CHECK(!snapshot.config.livePreview);
    CMS2_CHECK(snapshot.config.advancedRange);

    // Runtime controller / GMST baseline behavior.
    ResetDataDir();
    WriteConfiguration(
        "[MoonSizes]\n"
        "Mode=Relative\n"
        "MasserPercent=150\n"
        "SecundaPercent=75\n"
        "MasserAbsolute=9999\n"
        "SecundaAbsolute=-5\n"
        "LinkMoonSizes=yes\n"
        "LivePreview=off\n"
        "AdvancedRange=1\n");

    auto* collection = RE::GameSettingCollection::GetSingleton();
    collection->masser.SetInteger(90);
    collection->secunda.SetInteger(40);

    CMS2_CHECK(settings.Load());
    CMS2::MoonController::InitializeFromLoadedGameSettings();

    snapshot = settings.GetSnapshot();
    CMS2_CHECK(snapshot.baselineReady);
    CMS2_CHECK(snapshot.masserBaseline == 90);
    CMS2_CHECK(snapshot.secundaBaseline == 40);
    CMS2_CHECK(snapshot.masserConfiguredTarget == 135);
    CMS2_CHECK(snapshot.secundaConfiguredTarget == 30);
    CMS2_CHECK(snapshot.config.masserAbsolute == CMS2::SizeMath::kMaximumAbsolute);
    CMS2_CHECK(snapshot.config.secundaAbsolute == CMS2::SizeMath::kMinimumSize);
    CMS2_CHECK(snapshot.config.linkMoonSizes);
    CMS2_CHECK(!snapshot.config.livePreview);
    CMS2_CHECK(snapshot.config.advancedRange);

    // Initialization applies the selected configuration to the active targets and GMSTs.
    CMS2_CHECK(snapshot.masserActiveTarget == 135);
    CMS2_CHECK(snapshot.secundaActiveTarget == 30);
    CMS2_CHECK(collection->masser.GetInteger() == 135);
    CMS2_CHECK(collection->secunda.GetInteger() == 30);

    // Lifecycle reapply restores the active targets if another system edits the GMSTs.
    collection->masser.SetInteger(7);
    collection->secunda.SetInteger(8);
    CMS2::MoonController::RequestReapplyActive();
    CMS2_CHECK(collection->masser.GetInteger() == 135);
    CMS2_CHECK(collection->secunda.GetInteger() == 30);

    RE::NiNode masserNode;
    RE::NiNode masserShadow;
    masserNode.local.scale = 2.0F;
    masserShadow.local.scale = 3.0F;
    RE::Moon masser;
    masser.size = 90;
    masser.moonNode = RE::NiPointer<RE::NiNode>(&masserNode);
    masser.shadowNode = RE::NiPointer<RE::NiNode>(&masserShadow);

    RE::NiNode secundaNode;
    RE::NiNode secundaShadow;
    secundaNode.local.scale = 1.5F;
    secundaShadow.local.scale = 0.5F;
    secundaShadow.parent = &secundaNode;
    RE::Moon secunda;
    secunda.size = 40;
    secunda.moonNode = RE::NiPointer<RE::NiNode>(&secundaNode);
    secunda.shadowNode = RE::NiPointer<RE::NiNode>(&secundaShadow);

    RE::Sky sky;
    sky.masser = &masser;
    sky.secunda = &secunda;

    CMS2::MoonController::ApplyVisualScale(&masser, &sky);
    CMS2_CHECK(Near(masserNode.local.scale, 3.0F));
    CMS2_CHECK(Near(masserShadow.local.scale, 4.5F));

    CMS2::MoonController::ApplyVisualScale(&secunda, &sky);
    CMS2_CHECK(Near(secundaNode.local.scale, 1.125F));
    // The shadow is a child of the moon node and inherits the parent's scale.
    CMS2_CHECK(Near(secundaShadow.local.scale, 0.5F));

    // Reapplying must use captured native values rather than compounding the last frame.
    CMS2::MoonController::ApplyVisualScale(&masser, &sky);
    CMS2_CHECK(Near(masserNode.local.scale, 3.0F));
    CMS2_CHECK(Near(masserShadow.local.scale, 4.5F));

    auto config = snapshot.config;
    config.masserPercent = 200;
    config.secundaPercent = 50;
    settings.Commit(config);

    // Live preview is disabled, so a commit alone does not change active values.
    snapshot = settings.GetSnapshot();
    CMS2_CHECK(snapshot.masserConfiguredTarget == 180);
    CMS2_CHECK(snapshot.secundaConfiguredTarget == 20);
    CMS2_CHECK(snapshot.masserActiveTarget == 135);
    CMS2_CHECK(snapshot.secundaActiveTarget == 30);

    CMS2::MoonController::RequestApplyConfigured();
    snapshot = settings.GetSnapshot();
    CMS2_CHECK(snapshot.masserActiveTarget == 180);
    CMS2_CHECK(snapshot.secundaActiveTarget == 20);
    CMS2_CHECK(collection->masser.GetInteger() == 180);
    CMS2_CHECK(collection->secunda.GetInteger() == 20);

    CMS2::MoonController::ApplyVisualScale(&masser, &sky);
    CMS2_CHECK(Near(masserNode.local.scale, 4.0F));
    CMS2_CHECK(Near(masserShadow.local.scale, 6.0F));

    CMS2::MoonController::ApplyVisualScale(&secunda, &sky);
    CMS2_CHECK(Near(secundaNode.local.scale, 0.75F));
    CMS2_CHECK(Near(secundaShadow.local.scale, 0.5F));

    // Recreated scene nodes are detected and receive a freshly captured native scale.
    RE::NiNode replacementNode;
    replacementNode.local.scale = 5.0F;
    masser.moonNode = RE::NiPointer<RE::NiNode>(&replacementNode);
    CMS2::MoonController::ApplyVisualScale(&masser, &sky);
    CMS2_CHECK(Near(replacementNode.local.scale, 10.0F));

    // Replaced shadow node is recaptured independently.
    RE::NiNode replacementShadow;
    replacementShadow.local.scale = 2.0F;
    masser.shadowNode = RE::NiPointer<RE::NiNode>(&replacementShadow);
    CMS2::MoonController::ApplyVisualScale(&masser, &sky);
    CMS2_CHECK(Near(replacementShadow.local.scale, 4.0F));

    // Moon under shadow: scale shadow only once through the ancestor.
    RE::NiNode nestedShadow;
    RE::NiNode nestedMoon;
    nestedShadow.local.scale = 2.0F;
    nestedMoon.local.scale = 1.0F;
    nestedMoon.parent = &nestedShadow;
    secunda.moonNode = RE::NiPointer<RE::NiNode>(&nestedMoon);
    secunda.shadowNode = RE::NiPointer<RE::NiNode>(&nestedShadow);
    CMS2::MoonController::ResetMoonState(&secunda);
    CMS2::MoonController::ApplyVisualScale(&secunda, &sky);
    CMS2_CHECK(Near(nestedShadow.local.scale, 1.0F));
    CMS2_CHECK(Near(nestedMoon.local.scale, 1.0F));

    // Null/unknown pointers do not crash.
    CMS2::MoonController::ApplyVisualScale(nullptr, &sky);
    CMS2::MoonController::ApplyVisualScale(&masser, nullptr);
    RE::Moon unknown;
    CMS2::MoonController::ApplyVisualScale(&unknown, &sky);
    CMS2::MoonController::ResetMoonState(nullptr);

    // Reset behavior.
    settings.ResetConfiguredToBaseline();
    settings.ApplyConfiguredTargets();
    snapshot = settings.GetSnapshot();
    CMS2_CHECK(snapshot.masserConfiguredTarget == 90);
    CMS2_CHECK(snapshot.secundaConfiguredTarget == 40);
    CMS2_CHECK(snapshot.masserActiveTarget == 90);
    CMS2_CHECK(snapshot.secundaActiveTarget == 40);

    // Atomic/temporary save behavior and readable file contents.
    config = snapshot.config;
    config.mode = CMS2::SizeMode::kRelative;
    config.masserPercent = 200;
    config.secundaPercent = 50;
    config.livePreview = false;
    settings.Commit(config);
    CMS2_CHECK(settings.Save());
    const auto contents = ReadConfiguration();
    CMS2_CHECK(contents.find("MasserPercent=200") != std::string::npos);
    CMS2_CHECK(contents.find("SecundaPercent=50") != std::string::npos);

    ResetDataDir();
    std::cout << "Moon Scaler NG runtime-controller tests passed.\n";
    return 0;
}
