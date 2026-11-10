#pragma once

#include <atomic>

namespace CMS2::MoonController
{
    void InitializeFromLoadedGameSettings();
    void RequestApplyConfigured();
    void RequestReapplyActive();

    void PullMoon();
    void PullSun();

    // Background thread control
    void StartBackgroundThread();
    void StopBackgroundThread();
}