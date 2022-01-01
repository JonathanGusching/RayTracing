#include "objects.hpp"

void ObjectManager::AddObject(GLenum type,Object& object)
{
	// type=GL_STATIC_DRAW or GL_DYNAMIC_DRAW
	if(type==GL_STATIC_DRAW)
	{
		//staticVertices.insert(staticVertices.end(), object.vertex_array.begin(), object.vertex_array.end());
		//staticIndices.insert(staticIndices.end(), object.index_array.begin(), object.index_array.end());
		staticObjects.push_back(object);

		numberOfStaticObjects++;	
		std::cout << "Taille Vertices:" << staticVertices.size()<< std::endl;
		std::cout << "Taille Indices:" << staticIndices.size()<< std::endl;
	}
	else if(type==GL_DYNAMIC_DRAW)
	{
		//dynamicVertices.insert(dynamicVertices.end(),object.vertex_array.begin(),object.vertex_array.end());
		//dynamicIndices.insert(dynamicIndices.end(),object.index_array.begin(),object.index_array.end());
		dynamicObjects.push_back(object);
		numberOfDynamicObjects++;
	}
	else
	{
		std::cerr << "Error: could not add object to ObjectManager - Incorrect type" << std::endl;
		return;
	}
}