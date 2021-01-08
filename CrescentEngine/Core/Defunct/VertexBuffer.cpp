#include "CrescentPCH.h"
#include "VertexBuffer.h"
#include "GL/glew.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    glGenBuffers(1, &m_RendererID);          //We would like to generate 1 empty buffer and store it in the memory address of "buffer".
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);  //OpenGL will always select whatever is bound to the buffer and do your commands with it.
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);  //OpenGL will always select whatever is bound to the buffer and do your commands with it.
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);  //OpenGL will always select whatever is bound to the buffer and do your commands with it.
}
