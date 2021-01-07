#pragma once

namespace Crescent
{
	class Renderer;

	class RendererSettingsPanel
	{
	public:
		RendererSettingsPanel(Renderer* renderer);
		void RenderRendererEditorUI();

	private:
		void RenderDeviceInformationUI();

	private:
		Renderer* m_RendererContext;
	};
}
