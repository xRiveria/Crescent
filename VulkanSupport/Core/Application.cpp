#include "Application.h"

namespace Crescent
{
	Application::Application(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion)
		: m_ApplicationName(applicationName), m_EngineName(engineName), m_ApplicationMainVersion(applicationMainVersion), m_ApplicationSubVersion(applicationSubVersion)
	{
		m_Renderer = std::make_shared<VulkanRenderer>(m_ApplicationName, m_EngineName, m_ApplicationMainVersion, m_ApplicationSubVersion, true);
	}

	void Application::OnUpdate()
	{
		m_Renderer->DrawFrames();
	}
}