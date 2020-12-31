#include "CrescentPCH.h"
#include "Skybox.h"
#include "../Memory/Resources.h"
#include "../Rendering/Material.h"
#include "../Core/Primitive.h"

namespace CrescentEngine
{
	Skybox::Skybox() : SceneNode(Scene::m_CounterID++)
	{
		Scene::m_RootNode->AddChildNode(this);

		m_Shader = Resources::LoadShader("Skybox", "Resources/Shaders/Skybox.vs", "Resources/Shaders/Background.fs");
		m_Material = new Material(m_Shader);
		m_Mesh = new Cube;
		m_BoundingBoxMinimum = glm::vec3(-99999.0f);
		m_BoundingBoxMaximum = glm::vec3(99999.9f);

		//Default material configuration.
		m_Material->SetShaderFloat("Exposure", 1.0f);
		m_Material->m_DepthTestComparisonFactor = GL_LEQUAL; //Discard elements if depth value is lesser or equal to the stored depth value.
		m_Material->m_FaceCullingEnabled = false;
		m_Material->m_ShadowCasting = false;
		m_Material->m_ShadowReceiving = false;
	}

	Skybox::~Skybox()
	{

	}

	void Skybox::SetCubeMap(TextureCube* cubeMap)
	{
		m_CubeMap = cubeMap;
		m_Material->SetShaderTextureCube("background", m_CubeMap, 0);
	}
}