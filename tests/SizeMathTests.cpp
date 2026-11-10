#include "SizeMath.h"
#include "TestHarness.h"

#include <iostream>

int main()
{
    using namespace CMS2::SizeMath;

    // Relative sizing and rounding.
    CMS2_CHECK(TargetFromPercent(90, 100) == 90);
    CMS2_CHECK(TargetFromPercent(40, 150) == 60);
    CMS2_CHECK(TargetFromPercent(3, 150) == 5);
    CMS2_CHECK(TargetFromPercent(1, 1) == 1);
    CMS2_CHECK(TargetFromPercent(90, 1000) == 900);
    CMS2_CHECK(TargetFromPercent(kMaximumEffectiveSize, kMaximumPercent) == kMaximumEffectiveSize);
    CMS2_CHECK(TargetFromPercent(90, 50) == 45);
    CMS2_CHECK(TargetFromPercent(40, 200) == 80);

    // Reverse conversion.
    CMS2_CHECK(PercentFromTarget(90, 90) == 100);
    CMS2_CHECK(PercentFromTarget(40, 60) == 150);
    CMS2_CHECK(PercentFromTarget(90, 1) == 1);
    CMS2_CHECK(PercentFromTarget(0, 1) == 100);
    CMS2_CHECK(PercentFromTarget(90, 45) == 50);
    CMS2_CHECK(PercentFromTarget(40, 80) == 200);

    // Linked relative mode means the same percentage; linked absolute preserves ratio.
    CMS2_CHECK(TargetFromPercent(90, 150) == 135);
    CMS2_CHECK(TargetFromPercent(40, 150) == 60);

    // Linked absolute mode preserves the source-to-baseline ratio.
    CMS2_CHECK(LinkedAbsoluteTarget(180, 90, 40) == 80);
    CMS2_CHECK(LinkedAbsoluteTarget(20, 40, 90) == 45);
    CMS2_CHECK(LinkedAbsoluteTarget(135, 90, 40) == 60);
    CMS2_CHECK(LinkedAbsoluteTarget(kMaximumEffectiveSize, 1, 1) == kMaximumAbsolute);

    // Mode conversion helpers preserve effective size as closely as rounding permits.
    {
        const auto masserBaseline = 90;
        const auto secundaBaseline = 40;
        const auto masserPercent = 150;
        const auto secundaPercent = 75;
        const auto masserTarget = TargetFromPercent(masserBaseline, masserPercent);
        const auto secundaTarget = TargetFromPercent(secundaBaseline, secundaPercent);
        CMS2_CHECK(PercentFromTarget(masserBaseline, masserTarget) == masserPercent);
        CMS2_CHECK(PercentFromTarget(secundaBaseline, secundaTarget) == secundaPercent);
    }

    // Zero/invalid baseline handling.
    CMS2_CHECK(ClampBaseline(0) == kMinimumSize);
    CMS2_CHECK(ClampBaseline(-10) == kMinimumSize);
    CMS2_CHECK(TargetFromPercent(0, 100) == kMinimumSize);
    CMS2_CHECK(PercentFromTarget(0, 100) == kMaximumPercent);

    // Defensive bounds.
    CMS2_CHECK(ClampBaseline(50000) == kMaximumEffectiveSize);
    CMS2_CHECK(ClampPercent(-100) == kMinimumPercent);
    CMS2_CHECK(ClampPercent(5000) == kMaximumPercent);
    CMS2_CHECK(ClampAbsolute(0) == kMinimumSize);
    CMS2_CHECK(ClampAbsolute(-5) == kMinimumSize);
    CMS2_CHECK(ClampAbsolute(5000) == kMaximumAbsolute);
    CMS2_CHECK(kNormalMinimumPercent > kMinimumPercent);
    CMS2_CHECK(kNormalMaximumPercent < kMaximumPercent);
    CMS2_CHECK(kNormalMaximumAbsolute < kMaximumAbsolute);

    std::cout << "Moon Scaler NG size-math tests passed.\n";
    return 0;
}
