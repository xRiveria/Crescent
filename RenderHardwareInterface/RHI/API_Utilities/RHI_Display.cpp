#include "RHI_PCH.h"
#include "RHI_Display.h"
#include <windows.h>

namespace Aurora
{
    std::vector<RHI_DisplayMode> RHI_Display::m_DisplayModes;
    RHI_DisplayMode RHI_Display::m_DisplayModeActive;

    void RHI_Display::RegisterDisplayMode(RHI_DisplayMode&& newDisplayMode, const bool updateFPSLimitToHighestHertz)
    {
        // Early exit if the display is already registered.
        for (const RHI_DisplayMode& displayModeExisting : m_DisplayModes)
        {
            if (newDisplayMode == displayModeExisting)
            {
                return;
            }
        }

        RHI_DisplayMode& displayMode = m_DisplayModes.emplace_back(std::move(newDisplayMode));
        std::cout << "Registered Display Mode of " << displayMode.m_Width << ", " << displayMode.m_Height << ", " << displayMode.m_Hertz << "\n";

        // Keep display modes sorted based on refresh rate (from highest to lowest).
        std::sort(m_DisplayModes.begin(), m_DisplayModes.end(), [](const RHI_DisplayMode& firstDisplayMode, const RHI_DisplayMode& secondDisplayMode)
        {
            return firstDisplayMode.m_Hertz > secondDisplayMode.m_Hertz;
        });

        // Find preferred display mode.
        for (const RHI_DisplayMode& displayModeExisting : m_DisplayModes)
        {
            // Attempt to use higher resolution.
            if (displayModeExisting.m_Width > m_DisplayModeActive.m_Width || displayModeExisting.m_Height > m_DisplayModeActive.m_Height)
            {
                // But not lower hertz...
                if (displayModeExisting.m_Hertz >= m_DisplayModeActive.m_Hertz)
                {
                    SetActiveDisplayMode(displayModeExisting);
                }
            }
        }

        // Update FPS Limit
        if (updateFPSLimitToHighestHertz)
        {
            double currentHertz = m_DisplayModes.front().m_Hertz;
            /// Set FPS limit for your application timer.
        }
    }

    uint32_t RHI_Display::GetWidthPrimary()
    {
        return static_cast<uint32_t>(GetSystemMetrics(SM_CXSCREEN));
    }

    uint32_t RHI_Display::GetHeightPrimary()
    {
        return static_cast<uint32_t>(GetSystemMetrics(SM_CYSCREEN));
    }

    uint32_t RHI_Display::GetWidthVirtual()
    {
        return static_cast<uint32_t>(GetSystemMetrics(SM_CXVIRTUALSCREEN));
    }

    uint32_t RHI_Display::GetHeightVirtual()
    {
        return static_cast<uint32_t>(GetSystemMetrics(SM_CYVIRTUALSCREEN));
    }
}