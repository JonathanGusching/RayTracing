#ifndef OBJECTS_HPP
#define OBJECTS_HPP
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <GL/gl.h>

class Object;
class Cube;
class Sphere;

class ObjectManager
{
	public:
		int numberOfStaticObjects;
		int numberOfDynamicObjects;
		
		std::vector<GLfloat> staticVertices; // for the objects that rarely change (static balls, cubes, etc...)
		std::vector<GLuint> staticIndices;
		std::vector<Object> staticObjects;

		std::vector<GLfloat> dynamicVertices; // for the ones whose shape can change, or that can be translated/rotated
		std::vector<GLuint> dynamicIndices;
		std::vector<Object> dynamicObjects;

		void AddObject(GLenum type,Object &object);

		ObjectManager():numberOfStaticObjects(0),numberOfDynamicObjects(0){}
};

class Object
{
	public:
		glm::vec3 centerPos;

		// To translate from the current pos to currentPos+translation
		void Translate(glm::vec3 translation)
		{
			centerPos=centerPos+translation;
		}

		// To "teleport" the said object to new_pos
		void Move(glm::vec3 new_pos)
		{
			centerPos=new_pos;
		}
		
		Object(glm::vec3 pos):centerPos(pos){}
		Object(){centerPos=glm::vec3(0.0f,0.0f,0.0f);}
};

class Cube:public Object
{
	public:
		GLfloat half_length;
		//GLfloat vertex_array[48]; // Contains the colour and coordinates, need to be put in a VBO to use VRAM
	public:
		Cube(GLfloat the_half_length, glm::vec3 pos):Object(pos)
		{
			half_length=the_half_length;
		}
};

class Triangle:public Object
{
public:
	Triangle():Object()
	{

	}

	Triangle(GLfloat x):Object()
	{

	}
};

class Rectangle:public Object
{
	public:
		Rectangle():Object()
		{

		}
};
class Sphere:public Object
{
	public:
		GLfloat radius;
		Sphere(GLfloat theRadius, glm::vec3 position):Object(position)
		{
			radius=theRadius;
		}

};


#endif