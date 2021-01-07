#include "CrescentPCH.h"
#include "RendererSettingsPanel.h"
#include "Renderer.h"
#include "GLStateCache.h"
#include <imgui/imgui.h>

namespace Crescent
{
	RendererSettingsPanel::RendererSettingsPanel(Renderer* renderer)
	{
		m_RendererContext = renderer;
	}

	void RendererSettingsPanel::RenderRendererEditorUI()
	{
		ImGui::Begin("Settings");

		if (ImGui::Checkbox("Enable Wireframes", &m_RendererContext->RetrieveGLStateCache()->m_WireframesEnabled))
		{
			if (m_RendererContext->RetrieveGLStateCache()->m_WireframesEnabled)
			{
				m_RendererContext->RetrieveGLStateCache()->SetPolygonMode(GL_LINE);
			}
			else
			{
				m_RendererContext->RetrieveGLStateCache()->SetPolygonMode(GL_FILL);
			}
		}

		ImGui::End();

		RenderDeviceInformationUI();
	}

	void RendererSettingsPanel::RenderDeviceInformationUI()
	{
		ImGui::Begin("Renderer");

		ImGui::Text("Renderer: %s", m_RendererContext->RetrieveDeviceRendererInformation());
		ImGui::Text("Version: %s", m_RendererContext->RetrieveDeviceVersionInformation());
		ImGui::Text("Vendor: %s", m_RendererContext->RetrieveDeviceVendorInformation());

		ImGui::NewLine();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}
}