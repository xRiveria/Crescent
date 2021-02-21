#include "CrescentPCH.h"
#include "RendererSettingsPanel.h"
#include "Renderer.h"
#include "GLStateCache.h"
#include "PostProcessor.h"
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

		ImGui::Checkbox("Enable Wireframes", &m_RendererContext->m_WireframesEnabled);
		ImGui::Checkbox("Enable Lighting", &m_RendererContext->m_LightsEnabled);
		ImGui::Checkbox("Enable Shadows", &m_RendererContext->m_ShadowsEnabled);
		ImGui::Checkbox("Enable Lighting Volumes", &m_RendererContext->m_ShowDebugLightVolumes);
		ImGui::Spacing();
		ImGui::Text("Effects");
		ImGui::Checkbox("Enable Inversion", &m_RendererContext->m_PostProcessor->m_InversionEnabled);
		ImGui::Checkbox("Enable Greyscale", &m_RendererContext->m_PostProcessor->m_GreyscaleEnabled);

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