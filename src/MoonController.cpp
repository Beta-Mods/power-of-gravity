#include "MoonController.h"

#include "Config.h"

#include <RE/P/ProcessLists.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace
{
    std::atomic_bool g_running{ false };
    std::thread g_backgroundThread{};

    // Moon state
    std::atomic<float> g_moonTargetScale{ 3.0F };
    std::atomic<float> g_moonCurrentScale{ 1.0F };
    std::atomic<float> g_moonDuration{ 10.0F };
    std::atomic<float> g_moonElapsed{ 0.0F };
    std::atomic<bool> g_moonActive{ false };

    // Sun state
    std::atomic<float> g_sunTargetScale{ 3.0F };
    std::atomic<float> g_sunCurrentScale{ 1.0F };
    std::atomic<float> g_sunDuration{ 10.0F };
    std::atomic<float> g_sunElapsed{ 0.0F };
    std::atomic<bool> g_sunActive{ false };

    // Screen shake
    std::atomic<float> g_shakeIntensity{ 0.5F };
    std::atomic<float> g_shakeTime{ 0.0F };

    // Gravity
    std::atomic<float> g_gravity{ 15.0F };
    std::atomic<float> g_gravityRadius{ 5000.0F };
    std::atomic<bool> g_restoreGravity{ false };

    void BackgroundLoop()
    {
        while (g_running.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            if (!g_running.load(std::memory_order_acquire)) break;

            float moonElapsed = g_moonElapsed.load();
            float sunElapsed = g_sunElapsed.load();
            bool moonActive = g_moonActive.load();
            bool sunActive = g_sunActive.load();

            // Cancel effect if player is indoors
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                auto* cell = player->GetParentCell();
                if (cell && cell->IsInteriorCell()) {
                    if (moonActive) { moonActive = false; g_moonActive.store(false, std::memory_order_release); g_restoreGravity.store(true, std::memory_order_release); }
                    if (sunActive) { sunActive = false; g_sunActive.store(false, std::memory_order_release); g_restoreGravity.store(true, std::memory_order_release); }
                }
            }

            // Update moon
            if (moonActive) {
                moonElapsed += 0.016F;
                float moonDur = g_moonDuration.load();
                float moonTarget = g_moonTargetScale.load();
                if (moonElapsed >= moonDur) {
                    moonActive = false;
                    g_moonCurrentScale.store(1.0F, std::memory_order_release);
                    g_restoreGravity.store(true, std::memory_order_release);
                } else {
                    float growEnd = moonDur * 0.4F;
                    float holdEnd = moonDur * 0.7F;
                    float current;
                    if (moonElapsed < growEnd) {
                        float t = moonElapsed / growEnd;
                        current = 1.0F + (moonTarget - 1.0F) * t;
                    } else if (moonElapsed < holdEnd) {
                        current = moonTarget;
                    } else {
                        float t = (moonElapsed - holdEnd) / (moonDur - holdEnd);
                        current = moonTarget - (moonTarget - 1.0F) * t;
                    }
                    g_moonCurrentScale.store(current, std::memory_order_release);
                }
                g_moonElapsed.store(moonElapsed, std::memory_order_release);
                g_moonActive.store(moonActive, std::memory_order_release);
            }

            // Update sun
            if (sunActive) {
                sunElapsed += 0.016F;
                float sunDur = g_sunDuration.load();
                float sunTarget = g_sunTargetScale.load();
                if (sunElapsed >= sunDur) {
                    sunActive = false;
                    g_sunCurrentScale.store(1.0F, std::memory_order_release);
                    g_restoreGravity.store(true, std::memory_order_release);
                } else {
                    float growEnd = sunDur * 0.4F;
                    float holdEnd = sunDur * 0.7F;
                    float current;
                    if (sunElapsed < growEnd) {
                        float t = sunElapsed / growEnd;
                        current = 1.0F + (sunTarget - 1.0F) * t;
                    } else if (sunElapsed < holdEnd) {
                        current = sunTarget;
                    } else {
                        float t = (sunElapsed - holdEnd) / (sunDur - holdEnd);
                        current = sunTarget - (sunTarget - 1.0F) * t;
                    }
                    g_sunCurrentScale.store(current, std::memory_order_release);
                }
                g_sunElapsed.store(sunElapsed, std::memory_order_release);
                g_sunActive.store(sunActive, std::memory_order_release);
            }

            // Screen shake time
            if (moonActive || sunActive) {
                float activeTime = moonActive ? moonElapsed : sunElapsed;
                float activeDuration = moonActive ? g_moonDuration.load() : g_sunDuration.load();
                if (activeTime >= activeDuration) activeTime = 0.0F;
                g_shakeTime.store(activeTime, std::memory_order_release);
            } else {
                g_shakeTime.store(0.0F, std::memory_order_release);
            }

            // Only apply via SKSE task when an effect is active
            bool anyActive = moonActive || sunActive || g_restoreGravity.load(std::memory_order_acquire);
            if (anyActive) {
                if (auto* task = SKSE::GetTaskInterface()) {
                    task->AddTask([]() {
                        float moonScale = g_moonCurrentScale.load(std::memory_order_acquire);
                        float sunScale = g_sunCurrentScale.load(std::memory_order_acquire);
                        float shakeTime = g_shakeTime.load(std::memory_order_acquire);
                        float shakeIntensity = g_shakeIntensity.load(std::memory_order_acquire);
                        float grav = g_gravity.load(std::memory_order_acquire);
                        float gravRadius = g_gravityRadius.load(std::memory_order_acquire);
                        bool restoreGrav = g_restoreGravity.load(std::memory_order_acquire);
                        bool moonActive = g_moonActive.load(std::memory_order_acquire);
                        bool sunActive = g_sunActive.load(std::memory_order_acquire);

                        // Scale moons and sun
                        auto* sky = RE::Sky::GetSingleton();
                        if (sky) {
                            if (sky->masser) {
                                auto* moon = sky->masser;
                                if (moon->moonNode) moon->moonNode->local.scale = moonScale;
                                if (moon->moonMesh) moon->moonMesh->local.scale = moonScale;
                                if (moon->shadowNode) moon->shadowNode->local.scale = moonScale;
                                if (moon->shadowMesh) moon->shadowMesh->local.scale = moonScale;
                            }
                            if (sky->secunda) {
                                auto* moon = sky->secunda;
                                if (moon->moonNode) moon->moonNode->local.scale = moonScale;
                                if (moon->moonMesh) moon->moonMesh->local.scale = moonScale;
                                if (moon->shadowNode) moon->shadowNode->local.scale = moonScale;
                                if (moon->shadowMesh) moon->shadowMesh->local.scale = moonScale;
                            }
                            if (sky->sun) {
                                auto* sun = sky->sun;
                                if (sun->sunBaseNode) sun->sunBaseNode->local.scale = sunScale;
                                if (sun->sunBase) sun->sunBase->local.scale = sunScale;
                                if (sun->sunGlareNode) sun->sunGlareNode->local.scale = sunScale;
                                if (sun->sunGlare) sun->sunGlare->local.scale = sunScale;
                                sun->glareScale = sunScale;
                            }
                        }

                        // Gravity: affect hostile actors only
                        if (moonActive || sunActive) {
                            auto* processLists = RE::ProcessLists::GetSingleton();
                            auto* player = RE::PlayerCharacter::GetSingleton();
                            if (processLists && player) {
                                for (auto& handle : processLists->highActorHandles) {
                                    auto actorHandle = handle.get();
                                    RE::Actor* actor = actorHandle.get();
                                    if (!actor || !actor->Is3DLoaded() || actor->IsDead() ||
                                        actor->IsDisabled() || actor->IsDeleted()) continue;
                                    if (actor == player) continue;
                                    if (!actor->IsHostileToActor(player)) continue;
                                    float dist = actor->GetPosition().GetDistance(player->GetPosition());
                                    if (dist >= gravRadius) continue;
                                    RE::NiPoint3 pos = actor->GetPosition();
                                    pos.z += grav;
                                    actor->SetPosition(pos, true);
                                }
                            }
                        }

                        // Screen shake via game API
                        if (shakeTime > 0.0F) {
                            float activeDuration = moonActive ? g_moonDuration.load() : g_sunDuration.load();
                            float shake = shakeIntensity * (1.0F - shakeTime / activeDuration);
                            auto* player = RE::PlayerCharacter::GetSingleton();
                            if (player) {
                                RE::NiPoint3 pos = player->GetPosition();
                                RE::ShakeCamera(shake * 10.0F, pos, 0.1F);
                            }
                        }

                        // Restore when effect ends
                        if (restoreGrav) {
                            g_restoreGravity.store(false, std::memory_order_release);
                        }
                    });
                }
            }
        }
    }
}

namespace CMS2::MoonController
{
    void InitializeFromLoadedGameSettings()
    {
        auto& settings = CMS2::Settings::GetSingleton();
        auto snapshot = settings.GetSnapshot();
        g_moonTargetScale.store(snapshot.config.scaleAmount, std::memory_order_release);
        g_sunTargetScale.store(snapshot.config.scaleAmount, std::memory_order_release);
        g_moonDuration.store(snapshot.config.duration, std::memory_order_release);
        g_sunDuration.store(snapshot.config.duration, std::memory_order_release);
        g_shakeIntensity.store(snapshot.config.shakeIntensity, std::memory_order_release);
        g_gravity.store(snapshot.config.gravity, std::memory_order_release);
        g_gravityRadius.store(snapshot.config.gravityRadius, std::memory_order_release);
        logger::info("Power of Gravity initialized");
    }

    void RequestApplyConfigured()
    {
        auto& settings = CMS2::Settings::GetSingleton();
        auto snapshot = settings.GetSnapshot();
        g_moonTargetScale.store(snapshot.config.scaleAmount, std::memory_order_release);
        g_sunTargetScale.store(snapshot.config.scaleAmount, std::memory_order_release);
        g_moonDuration.store(snapshot.config.duration, std::memory_order_release);
        g_sunDuration.store(snapshot.config.duration, std::memory_order_release);
        g_shakeIntensity.store(snapshot.config.shakeIntensity, std::memory_order_release);
        g_gravity.store(snapshot.config.gravity, std::memory_order_release);
        g_gravityRadius.store(snapshot.config.gravityRadius, std::memory_order_release);
    }

    void RequestReapplyActive()
    {
        // no-op
    }

    void PullMoon()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player) {
            auto* cell = player->GetParentCell();
            if (cell && cell->IsInteriorCell()) {
                logger::info("Pull Moon: player is indoors, effect disabled");
                return;
            }
        }
        g_moonElapsed.store(0.0F, std::memory_order_release);
        auto* cal = RE::Calendar::GetSingleton();
        if (cal) {
            float hour = cal->GetHour();
            if (hour > 6.0F && hour < 20.0F) {
                logger::info("Pull Moon: sun is out (hour={}), effect disabled", hour);
                return;
            }
        }
        g_moonActive.store(true, std::memory_order_release);
        RE::PlaySound("NPCDragonRoar");
        logger::info("Pull Moon activated (scale={})", g_moonTargetScale.load());
    }

    void PullSun()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (player) {
            auto* cell = player->GetParentCell();
            if (cell && cell->IsInteriorCell()) {
                logger::info("Pull Sun: player is indoors, effect disabled");
                return;
            }
        }
        g_sunElapsed.store(0.0F, std::memory_order_release);
        auto* cal = RE::Calendar::GetSingleton();
        if (cal) {
            float hour = cal->GetHour();
            if (hour < 6.0F || hour > 20.0F) {
                logger::info("Pull Sun: moon is out (hour={}), effect disabled", hour);
                return;
            }
        }
        g_sunActive.store(true, std::memory_order_release);
        RE::PlaySound("NPCDragonRoar");
        logger::info("Pull Sun activated (scale={})", g_sunTargetScale.load());
    }

    void StartBackgroundThread()
    {
        if (!g_running.exchange(true, std::memory_order_acq_rel)) {
            g_backgroundThread = std::thread(BackgroundLoop);
        }
    }

    void StopBackgroundThread()
    {
        g_running.store(false, std::memory_order_release);
        if (g_backgroundThread.joinable()) {
            g_backgroundThread.join();
        }
    }
}