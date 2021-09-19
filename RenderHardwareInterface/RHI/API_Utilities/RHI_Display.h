#pragma once
#include <vector>
#include "RHI_DisplayMode.h"

namespace Aurora
{
    class RHI_Display
    {
    public:
        RHI_Display() = default;

        // Display Modes
        static void RegisterDisplayMode(RHI_DisplayMode&& newDisplayMode, const bool updateFPSLimitToHighestHertz);
        static void SetActiveDisplayMode(const RHI_DisplayMode& displayMode) { m_DisplayModeActive = displayMode; }
        static const RHI_DisplayMode& GetActiveDisplayMode() { return m_DisplayModeActive; }
        static const std::vector<RHI_DisplayMode>& GetDisplayModes() { return m_DisplayModes; }

        // Display Dimensions
        static uint32_t GetWidthPrimary();
        static uint32_t GetHeightPrimary();

        // Virtual Display Dimensions (Multiple Displays)
        static uint32_t GetWidthVirtual();
        static uint32_t GetHeightVirtual();

    private:
        static std::vector<RHI_DisplayMode> m_DisplayModes;
        static RHI_DisplayMode m_DisplayModeActive;
    };
}