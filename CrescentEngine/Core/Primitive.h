#pragma once
#include <GL/glew.h>
#include "../Rendering/Shader.h"
#include "../Rendering/Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Models/Mesh.h"

namespace Crescent
{
	enum PrimitiveShape
	{
		PlanePrimitive,
		CubePrimitive,
		QuadPrimitive
	};
	
	class Primitive
	{
	public:
		Primitive() {}
		Primitive(const PrimitiveShape& primitiveShape);

		void SetupPrimitiveBuffers(const PrimitiveShape& primitiveShape);
		void BindPrimitiveVertexArray() const;
		void DrawPrimitive(Shader& shader);
		void DrawEditorSettings();

	private:
		std::string ConvertUUIDToChar() const;
		std::string ConvertPrimitiveEnumToString() const;

	private:
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
		unsigned int m_PrimitiveObjectID = 0; //Temporary until UUIDs are implemented.
		PrimitiveShape m_PrimitiveShape; 

		glm::vec3 m_PrimitivePosition = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_PrimitiveRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_PrimitiveScale = { 1.0f, 1.0f, 1.0f };
	};

	class TransparentQuad
	{
	public:
		TransparentQuad() {}
		void SetupTransparentQuadBuffers();
		void BindTransparentQuadVertexArray() const { glBindVertexArray(m_VertexArrayID); }

		void DrawTransparentQuad(Shader& shader, glm::mat4& modelMatrix);
		void DrawTransparentQuad(Shader& shader, glm::mat4& modelMatrix, Texture2D& texture);	

	private:
		unsigned int m_VertexArrayID = 0;
		unsigned int m_VertexBufferID = 0;
	};
}