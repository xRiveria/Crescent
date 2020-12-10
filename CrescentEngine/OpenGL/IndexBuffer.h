#pragma once

//When we use size, it means bytes. Count means element count. 

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;
	inline unsigned int GetCount() const { return m_Count; }

private:
	//We know that OpenGL needs an unsigned integer to keep track of every time of object we create in OpenGL such as Textures, Shaders etc.
	//Each one gets a unique ID to identify said object. We are calling this a RendererID. Note that this is done similarly in other rendering APIs. 
	unsigned int m_RendererID;
	//To know how many indices/vertexes it has.
	unsigned int m_Count;
};