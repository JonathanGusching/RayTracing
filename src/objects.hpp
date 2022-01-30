#ifndef OBJECTS_HPP
#define OBJECTS_HPP
#include <iostream>
#include <fstream>
#include <vector>

#include "material.hpp"

#include <glm/glm.hpp>
#include <GL/gl.h>

#include <xml/parser>
#include <xml/serializer>

class Object;
class Cube;
class Sphere;

#include <string>
#define MAX_NAME_SIZE 32

class Scene
{
public:
	std::string name;
	std::vector<Object*> objects;

	void Reset();
	void AddObject(Object &object);
	
	Scene(){
		name="default.xml\0";
	}
	~Scene(){
	}
};

class SceneManager
{
	public:
		
		Scene currentScene;

		const void CreateScene(const char* name, std::vector<Object> objects);
		void ImportScene(const char* file);
		const void ExportScene(const char* file);
		SceneManager(){}
		~SceneManager(){}
};

class Object
{
	public:
		glm::vec3 centerPos;
		Material mat;

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
		
		Object(glm::vec3 pos, Material& material):centerPos(pos), mat(material){}
		Object(glm::vec3 pos):centerPos(pos){}
		Object(){centerPos=glm::vec3(0.0f,0.0f,0.0f);}

		virtual const char* Classname() { return "object";}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element(Classname());
			
			WritePositionToXML(s);
			
			s.end_element();
		}
	protected:
		static void MaterialToXML(xml::serializer& s, Material& mat)
		{
			s.start_element("material");
			s.attribute("shininess", mat.shininess);
			s.attribute("diffuse", mat.diffuse);
			s.attribute("transparency", mat.transparency);
			s.attribute("n", mat.n);
			Vec3ToXML(s, mat.color, "color");
			s.end_element();
		}
		static void Vec3ToXML(xml::serializer& s, glm::vec3& vec, const char* name)
		{
			s.start_element(name);
			s.attribute("x", vec.x);
			s.attribute("y", vec.y);
			s.attribute("z", vec.z);
			s.end_element();
		}
		void WritePositionToXML(xml::serializer& s)
		{
			Vec3ToXML(s, centerPos, "position");
		}

};

class Cube:public Object
{
	public:
		glm::vec3 low;
		glm::vec3 up;

		virtual const char* Classname() { return "cube";}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element(Classname());
			
			WritePositionToXML(s);

			Vec3ToXML(s, low, "low");
			Vec3ToXML(s, up, "up");
			MaterialToXML(s, mat);
			
			s.end_element();
		}
		Cube(glm::vec3 the_low, glm::vec3 the_up):Object((the_low + the_up)*0.5f)
		{
			low=the_low;
			up=the_up;
		}
		Cube(glm::vec3 the_low, glm::vec3 the_up, Material& mat):Object((the_low + the_up)*0.5f,mat)
		{
			low=the_low;
			up=the_up;
		}
		~Cube(){}
};

class Triangle:public Object
{
	public:
		glm::vec3 vert1;
		glm::vec3 vert2;
		glm::vec3 vert3;

		virtual const char* Classname() { return "triangle";}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element(Classname());
			WritePositionToXML(s);

			Vec3ToXML(s, vert1, "v1");
			Vec3ToXML(s, vert2, "v2");
			Vec3ToXML(s, vert3, "v3");

			MaterialToXML(s, mat);
		}
		Triangle():Object()
		{

		}

		Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, Material& mat):Object((v1 + v2 + v3)/3.0f, mat)
		{
			vert1 = v1;
			vert2 = v2;
			vert3 = v3;

		}
};

class Rectangle:public Object
{
	public:
		virtual const char* Classname() { return "rectangle";}
		Rectangle():Object()
		{

		}
};
class Sphere:public Object
{
	public:
		GLfloat radius;

		virtual const char* Classname() { return "sphere";}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element(Classname());
			WritePositionToXML(s);

			s.start_element("radius");
			s.attribute("radius", radius);
			s.end_element();
			
			MaterialToXML(s, mat);			
		}
		Sphere(GLfloat theRadius, glm::vec3 position):Object(position)
		{
			radius=theRadius;
		}
		Sphere(GLfloat theRadius, glm::vec3 position, Material& mat):Object(position, mat)
		{
			radius=theRadius;
		}

};


#endif