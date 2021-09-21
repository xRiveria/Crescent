#pragma once

namespace Aurora
{
    // A viewport is a rectangle that defines the area of the framebuffer you're rendering to.
    class RHI_Viewport
    {
    public:
        RHI_Viewport(const float x = 0.0f, const float y = 0.0f, const float width = 0.0f, const float height = 0.0f, const float depthMinimum = 0.0f, const float depthMaximum = 1.0f)
        {
            this->m_X = x;
            this->m_Y = y;
            this->m_Width = width;
            this->m_Height = height;
            this->m_DepthMinimum = depthMinimum;
            this->m_DepthMaximum = depthMaximum;
        }

        RHI_Viewport(const RHI_Viewport& viewport)
        {
            this->m_X = viewport.m_X;
            this->m_Y = viewport.m_Y;
            this->m_Width = viewport.m_Width;
            this->m_Height = viewport.m_Height;
            this->m_DepthMinimum = viewport.m_DepthMinimum;
            this->m_DepthMaximum = viewport.m_DepthMaximum;
        }

        ~RHI_Viewport() = default;

        bool operator== (const RHI_Viewport & otherViewport) const
        {
            return m_X == otherViewport.m_X && m_Y == otherViewport.m_Y &&
                   m_Width == otherViewport.m_Width && m_Height == otherViewport.m_Height &&
                   m_DepthMinimum == otherViewport.m_DepthMinimum && m_DepthMaximum == otherViewport.m_DepthMaximum;
        }

        bool operator!=(const RHI_Viewport& otherViewport) const
        {
            return !(*this == otherViewport);
        }

        bool IsDefined() const
        {
            return m_X            != 0.0f ||
                   m_Y            != 0.0f ||
                   m_Width        != 0.0f ||
                   m_Height       != 0.0f ||
                   m_DepthMinimum != 0.0f ||
                   m_DepthMaximum != 0.0f;
        }

        float GetAspectRatio() const
        {
            return m_Width / m_Height;
        }

    private:
        float m_X = 0.0f;
        float m_Y = 0.0f;
        float m_Width = 0.0f;
        float m_Height = 0.0f;
        float m_DepthMinimum = 0.0f;
        float m_DepthMaximum = 0.0f;

        static const RHI_Viewport Undefined;
    };
}