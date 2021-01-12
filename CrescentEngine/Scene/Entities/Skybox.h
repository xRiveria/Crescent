#pragma once
#include "../SceneEntity.h"

namespace Crescent
{
	class TextureCube;
	class Material;
	class Shader;
	class Renderer;
	class Cube;

	/*
		A SkyBox represented as a scene entity for easy scene management. This is set up in a way that when passed to the renderer, it will automatically
		render behind all visible geometry (with no camera parallax).
	*/

	class Skybox : public SceneEntity
	{
	public:
		Skybox();
		~Skybox();

		void SetCubeMap(TextureCube* cubeMap);

	private:
		TextureCube* m_CubeMap = nullptr;
		Shader* m_CubeMapShader = nullptr;
	};
}