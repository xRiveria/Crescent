#pragma once
#include "GL/glew.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "../OpenGL/GShader.h"

struct GraphicalInformation
{
    const char* rendererInformation = "";
    const char* versionInformation = "";
    const char* vendorInformation = "";
};

#define ASSERT(x) if ((x == false)) __debugbreak();  //__ means Compiler Intrisic. This will only work in MSVS.
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();  //Clears all errors.
bool GLLogCall(const char* function, const char* file, int line);

class OpenGLRenderer
{
public:
    OpenGLRenderer();
    inline GraphicalInformation RetrieveGraphicalInformation() const { return systemInformation; }
    void Clear() const;
    void Draw(const VertexArray& vertexArray, const IndexBuffer& indexBuffer, const GShader& shader);
private:
    static GraphicalInformation systemInformation;
};




