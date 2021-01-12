#include "CrescentPCH.h"
#include "Skybox.h"
#include "../Scene.h"
#include "../Models/DefaultPrimitives.h"
#include "../Shading/Material.h"
#include "../Rendering/Resources.h"

namespace Crescent
{
	Skybox::Skybox() : SceneEntity("Skybox", Scene::m_SceneEntityCounterID++)
	{
		m_CubeMapShader = Resources::LoadShader("Background", "Resources/Shaders/SkyboxVertex.shader", "Resources/Shaders/SkyboxFragment.shader");
		m_Material = new Material(m_CubeMapShader);
		m_Mesh = new Cube();

		//Default Material Configuration
		m_Material->SetShaderFloat("Exposure", 1.0f);
		m_Material->m_DepthTestFunction = GL_LEQUAL;
		m_Material->m_FaceCullingEnabled = false;
		m_Material->m_ShadowCasting = false;
		m_Material->m_ShadowReceiving = false;
	}

	Skybox::~Skybox()
	{
		delete m_Material;
		delete m_Mesh;
	}

	void Skybox::SetCubeMap(TextureCube* cubeMap)
	{
		m_CubeMap = cubeMap;
		m_Material->SetShaderTextureCube("background", m_CubeMap, 0);
	}
}