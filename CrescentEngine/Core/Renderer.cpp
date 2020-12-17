#include "CrescentPCH.h"
#include "Renderer.h"

namespace CrescentEngine
{
	void Renderer::InitializeSelectedRenderer(API selectedAPI)
	{
		Renderer::API selectedRendererAPI = Renderer::API::OpenGL;
		switch (selectedRendererAPI)
		{
			case API::OpenGL:
			{
				std::cout << "Initialized OpenGL" << "\n";
				break;
			}

			case API::Vulcan:
			{
				std::cout << "Initialized Vulcan" << "\n";
				break;
			}
		}
	}
}