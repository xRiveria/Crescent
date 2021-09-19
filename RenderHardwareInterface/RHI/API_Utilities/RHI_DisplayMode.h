#pragma once
#include <memory>

namespace Aurora
{
    struct RHI_DisplayMode
    {
        RHI_DisplayMode() = default;
        RHI_DisplayMode(const uint32_t displayWidth, const uint32_t displayHeight, const uint32_t displayNumerator, const uint32_t displayDenominator)
        {
            this->m_Width = displayWidth;
            this->m_Height = displayHeight;
            this->m_Numerator = displayNumerator;
            this->m_Denominator = displayDenominator;
            this->m_Hertz = static_cast<double>(displayNumerator) / static_cast<double>(displayDenominator);
        }

        bool operator==(const RHI_DisplayMode& otherDisplayMode)
        {
            return m_Width == otherDisplayMode.m_Width && m_Height == otherDisplayMode.m_Height && m_Hertz == otherDisplayMode.m_Hertz;
        }

    private:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_Numerator = 0;
        uint32_t m_Denominator = 0;
        double m_Hertz = 0;

    private:
        friend class RHI_Display;
    };
}