#include "VulkanResource.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../../Vendor/tinyobjloader/tiny_obj_loader.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

namespace Crescent
{
	VulkanResource::VulkanResource(const std::string& modelFilePath) : m_FilePath(modelFilePath)
	{
		LoadModel();
	}

	void VulkanResource::LoadModel()
	{
		/*
			A model is loaded into the library's data structures by calling the tinyObj::LoadObj function. An OBJ file consists of positions, normal, texture coordinates and faces. 
			Faces consist of an arbitrary amount of vertices, where each vertex refer to a position, normal and/or texture coordinate by index. This makes it possible to not 
			just reuse entire vertices, but also individual attributes.

			The tinyobj::attrib_t container holds all of the positions, normals and texture coordinates in its attrib_t.vertices. The shape_t container contains all of the 
			seperate objects and their faces. Each face consists of an array of vertices, and each vertex contains the indices of the position, normal and texture coordinate 
			attributes. OBJ models can also define a material and texture per face, but we will be ignoring this.

			The error string contains errors and the warn string contains warnings that occur while loading the file, like a miss material definition. Loading only really 
			fail if the LoadObj function returns false. As mentioned above, faces in OBJ files actually contain an arbitrary number of vertices, whereas our application can 
			only render triangles. Luckily, the LoadObj has an optional parameter to automatically triangulate such faceds, which is enabled by default.
		*/

		tinyobj::attrib_t modelAttributes;
		std::vector<tinyobj::shape_t> modelShapes;
		std::vector<tinyobj::material_t> modelMaterials;
		std::string warning, error;

		if (!tinyobj::LoadObj(&modelAttributes, &modelShapes, &modelMaterials, &warning, &error, m_FilePath.c_str()))
		{
			throw std::runtime_error(warning + error);
		}
		else
		{
			std::cout << "Successfully loaded Model at path: " + m_FilePath + ".\n";
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		//We're going to combine all of the faces into the file into a single mode, so just iterate over all of the shapes.
		for (const tinyobj::shape_t& shape : modelShapes)
		{
			//The triangulation feature has already made sure that there are 3 vertices per face, so we can directly iterate over the vertices and dump them straight into our vertices vector.
			for (const tinyobj::index_t& index : shape.mesh.indices)
			{
				Vertex vertex{};
				/*
					For simplicity, we will assume that every vertex is unique for now, hence the simple auto-increment indices. The index variable is of type tinyobj::index_t,
					which contains the vertex_index, normal_index and texcoord_index members. We need to use these indices to look up the actual vertex attributes in the attrib arrays.

					Unfortunately, the modelAttributes.vertices array is an array of float values instead of something like glm::vec3, so you need to multiply the index of 3. Similarly,
					there are two texture coordinate components per entry. The offsets of 0, 1, and 2 are used to access the X, Y and Z components, or the U and V components in the case of texture coordinates.
				*/
				vertex.m_Position =
				{
					modelAttributes.vertices[3 * index.vertex_index + 0], //Vertex X
					modelAttributes.vertices[3 * index.vertex_index + 1], //Vertex Y
					modelAttributes.vertices[3 * index.vertex_index + 2]  //Vertex Z
				};

				/*
					Great, the geometry looks correct, but the texture might look wrong. The OBJ format assumes a coordinate system where a vertical coordinate of 0 means the 
					bottom of the image. However, we've uploaded our image into Vulkan in a top to bottom orientation where 0 means the top of the image. Solve this by flipping 
					the vertical component of the texture coordinates.
				*/
				vertex.m_TexCoord =
				{
					modelAttributes.texcoords[2 * index.texcoord_index + 0],		//U
					1.0 - modelAttributes.texcoords[2 * index.texcoord_index + 1]   //V
				};

				/*
					Unfortunately, we're not really taking advantage of the index buffer yet. The vertices vector contains a lot of duplicated vertex data, because many vertices 
					are included in multiple traingles. We should only keep the unique vertices and use the index buffer to reuse them whever they come up.

					A straightforward way to implement this is to use a map or unordered map to keep track of the unique vertices and respective indices.

					This means that every time we read vertices from the OBJ file, we check if we have already seen a vertex with the same exact position and texture coordinates
					before. If not, we add it to m_Vertices and stores its index in the uniqueVertices container. After that, we add the index of the new vertex to m_Indices. 

					The program will fail to compile as using a user-defined type like our Vertex struct as a key in a hash table requires us to implement 2 functions: equality test 
					and a hash calculation. The former is easy to do so by overriding the == operator in the Vertex struct. Once done, you will see that we successfully shrink the
					program from 1,500,000 vertices to 265,645.
				*/

				vertex.m_Color = { 1.0f, 1.0f, 1.0f };

				/*
					Searches the container for elements whose key is the parameter and returns the number of elements found. Because unordered_map containers do not allow for 
					duplicated keys, this means that the function actually returns 1 if an element with that key exists, and zero otherwise.
				*/
				if (uniqueVertices.count(vertex) == 0) //If the vertex does not yet exist in our map, aka it is unique.
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
					m_Vertices.push_back(vertex);
				}

				//Remember that we use index buffers to reuse vertices that already exist. Thus, if it already exists, we simply save its index in our indices array.
				m_Indices.push_back(uniqueVertices[vertex]); //We store the index of the vertices in m_Indices.
			}
		}
	}
}