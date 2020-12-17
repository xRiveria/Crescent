#pragma once

namespace CrescentEngine
{
	class Renderer
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			Vulcan = 2
		};

		static void InitializeSelectedRenderer(API selectedAPI);
		inline static API GetAPI() { return selectedRendererAPI; }

	private:
		static API selectedRendererAPI;
	};
}