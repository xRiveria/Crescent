#include "CrescentPCH.h"
#include "IndexBuffer.h"
#include "GL/glew.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_Count(count)
{
    glGenBuffers(1, &m_RendererID);         
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);  
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);  //OpenGL will always select whatever is bound to the buffer and do your commands with it.
}

void IndexBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  //OpenGL will always select whatever is bound to the buffer and do your commands with it.
}
