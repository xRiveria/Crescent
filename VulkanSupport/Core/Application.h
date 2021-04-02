#pragma once
#include <string>
#include <memory>
#include "../Source/VulkanRenderer.h"

namespace Crescent
{
	class Application
	{
	public:
		Application(const std::string& applicationName, const std::string& engineName, const int& applicationMainVersion, const int& applicationSubVersion);
		~Application();

		void OnUpdate();

	private:
		void InitializeGLFWLibrary();

	private:
		std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;

		//Application Information
		std::string m_ApplicationName = "Application Name";
		std::string m_EngineName = "Engine Name";
		int m_ApplicationMainVersion = 0;
		int m_ApplicationSubVersion = 0;
	};

	//To be defined in client application.
	Application* CreateApplication();
}