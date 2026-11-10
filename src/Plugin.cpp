#include "Config.h"
#include "MoonController.h"
#include "UI.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace
{
    void InitializeLogging()
    {
        auto logDirectory = SKSE::log::log_directory();
        if (!logDirectory) {
            SKSE::stl::report_and_fail("SKSE log directory is unavailable");
        }

        const auto* declaration = SKSE::PluginDeclaration::GetSingleton();
        auto logPath = *logDirectory / std::format("{}.log", declaration->GetName());
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
        auto log = std::make_shared<spdlog::logger>("global", std::move(sink));
        log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_message)
    {
        switch (a_message->type) {
        case SKSE::MessagingInterface::kPostLoad:
            CMS2::UI::Register();
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            CMS2::MoonController::InitializeFromLoadedGameSettings();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            CMS2::MoonController::RequestReapplyActive();
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    InitializeLogging();

    const auto* declaration = SKSE::PluginDeclaration::GetSingleton();
    logger::info(
        "{} {} loading on runtime {}",
        declaration->GetName(),
        declaration->GetVersion().string(),
        a_skse->RuntimeVersion().string());

    CMS2::Settings::GetSingleton().Load();

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(MessageHandler)) {
        logger::critical("Could not register the SKSE message listener");
        return false;
    }

    CMS2::MoonController::StartBackgroundThread();

    return true;
}