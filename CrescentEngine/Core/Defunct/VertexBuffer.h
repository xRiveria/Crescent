#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

private:
	//We know that OpenGL needs an unsigned integer to keep track of every object we create in OpenGL such as Textures, Shaders etc.
	//Each one gets a unique ID to identify said object. We are calling this a RendererID. Note that this is done similarly in other rendering APIs. 
	unsigned int m_RendererID;
};