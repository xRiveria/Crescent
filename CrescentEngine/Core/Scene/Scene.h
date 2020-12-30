#pragma once
#include "SceneNode.h"
#include "../Models/Mesh.h"
#include "../Rendering/Material.h"

namespace CrescentEngine
{
	/*
		This is our global scene object. It will always hold a core root node to which other scene nodes can be attached. 
		There will always be one global scene object, which can be cleared and configured at will.
	*/

	class Scene
	{
	public:
		static void ClearAllSceneNodes(); //Completely resets the scene, deleting all nodes and recreating the root node.
		
		//Static helper function that directly builds an empty scene node. Other sub-engines can directly add childrens to this empty scene node (with an identity matrix as transform).
		static SceneNode* CreateSceneNode();

		//Similar to the CreateSceneNode function, but directly builds a node with an attached mesh and material.
		static SceneNode* CreateSceneNode(Mesh* mesh, Material* material);

		//Copy scene node. Manually copies each node due to how our scene is arranged.
		static SceneNode* MakeSceneNode(SceneNode* node);

		//Deletes a scene node from the global scene hierarchy (together with its children).
		static void DeleteSceneNode(SceneNode* node);

	public:
		static SceneNode* m_RootNode;
		static unsigned int m_CounterID;
	};
}