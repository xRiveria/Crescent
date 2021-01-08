#pragma once
#include "../Shading/Texture.h"
#include "../Shading/Shader.h"
#include "RenderTarget.h"
#include <vector>
#include <map>

namespace Crescent
{
	class Material;
	class RenderTarget;
	/*
		Manages and maintains a list of common render materials, either for storing a template list of common materials or for storing internal renderer materials.	
	*/

	class MaterialLibrary
	{
		friend Renderer;

	public:
		MaterialLibrary(RenderTarget* gBuffer);
		~MaterialLibrary(); 

		//Either create a deferred default material (based on default sets of materials avaliable (like glass), or a custom material (with custom you have to supply your own shader).
		Material* CreateMaterial(std::string& base);				//These don't have the custom flag set (a default material has default state and uses checkboard textures as albedo (and black metallic, half roughness, purple normal, white AO).
		//Material* CreateCustomMaterial(Shader* shader);				//These have the custom flag set (will be rnedered in the forward pass).
		//Material* CreatePostProcessingMaterial(Shader* shader);		//These have the post-processing flags set (will be rendered after deferred/forward pass).

	private:
		//Generate all default template materials.
		void GenerateDefaultMaterials();
		//Generate all internal materials used by the renderer/
		void GenerateInternalMaterials(RenderTarget* gBuffer);

	private:
		//Internal Render-Specific Materials
		//Material* m_DefaultBlitMaterial;

		//Shader* m_DeferredIrradianceShader;
		//Shader* m_DeferredAmbientLightShader;
		//Shader* m_DeferredDirectionalLightShader;
		//Shader* m_DeferredPointLightShader;

		Shader* m_DirectionalShadowShader;

		//Material* m_DebugLightMaterial;

		//Holds a list of default material templates that other materials can derive from.
		std::map<unsigned int, Material*> m_DefaultMaterials;

		//Stores all generated/copied materials.
		std::vector<Material*> m_Materials;
	};
}
