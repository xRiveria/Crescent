#include "SceneNode.h"
#include "Scene.h"
#include "../Rendering/TextureCube.h"

namespace CrescentEngine
{
	/*
		A skybox represents as a scene node for easy scene management. The skybox scene node is setup in a way that when passed to the renderer, it will
		automatically render behind all visible geometry with no camera parallax.
	*/

	class Skybox : public SceneNode
	{
	public:
		Skybox();
		~Skybox();

		void SetCubeMap(TextureCube* cubeMap);

	private:
		TextureCube* m_CubeMap;
		Shader* m_Shader;
	};
}