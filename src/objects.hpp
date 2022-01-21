#ifndef OBJECTS_HPP
#define OBJECTS_HPP
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <GL/gl.h>

class Object;
class Cube;
class Sphere;

class Scene
{
public:
	char* name;
	std::vector<Object> objects;

	void Reset();
	Scene(){}
	~Scene(){}
};

class SceneManager
{
	public:
		
		Scene currentScene;

		void AddObject(Object &object);
		void OpenScene(const char* file);
		void CreateScene(const char* file, std::vector<Object> objects);

		SceneManager(){}
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