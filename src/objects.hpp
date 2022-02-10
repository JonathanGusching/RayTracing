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
		//TODO TO DO
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

		virtual std::string Classname() { return "Object";}
		virtual void FromXML(xml::parser & p)
		{
			p.next_expect (xml::parser::start_element,"position", xml::content::complex);
			centerPos=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"material",xml::content::complex);
			mat=MaterialAttributesFromXML(p);
			p.next_expect(xml::parser::end_element);
		}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element("Object");
			s.attribute("type", "Object");
			
			WritePositionToXML(s);
			MaterialToXML(s, mat);
			
			s.end_element();
		}
	protected:
		static Material MaterialAttributesFromXML(xml::parser& p)
		{
			Material mat;
			mat.shininess = p.attribute<float>("shininess");
			mat.transparency = p.attribute<float>("transparency");
			mat.diffuse = p.attribute<float>("diffuse");
			mat.n = p.attribute<float>("n");
			p.next_expect(xml::parser::start_element, "color",xml::content::complex);
			mat.color=Vec3FromXML(p);
			p.next_expect(xml::parser::end_element);
			return mat;
		}
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
		static glm::vec3 Vec3FromXML(xml::parser& p)
		{
			float x = p.attribute<float>("x");
			float y = p.attribute<float>("y");
			float z = p.attribute<float>("z");
			return glm::vec3(x,y,z);
		}
		static void Vec3ToXML(xml::serializer& s, glm::vec3& vec, const char* name)
		{
			s.start_element(name);
			s.attribute("x", vec.x);
			s.attribute("y", vec.y);
			s.attribute("z", vec.z);
			s.end_element();
		}
		const void WritePositionToXML(xml::serializer& s)
		{
			Vec3ToXML(s, centerPos, "position");
		}

};

class Cube:public Object
{
	public:
		glm::vec3 low;
		glm::vec3 up;

		virtual std::string Classname() { return "Cube";}
		virtual void FromXML(xml::parser& p)
		{
			p.next_expect (xml::parser::start_element,"position", xml::content::complex);
			centerPos=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"low", xml::content::complex);
			low=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"up", xml::content::complex);
			up=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);
			
			p.next_expect (xml::parser::start_element,"material",xml::content::complex);
			mat=MaterialAttributesFromXML(p);
			p.next_expect (xml::parser::end_element);
		}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element("Object");
			s.attribute("type", Classname());
			WritePositionToXML(s);

			Vec3ToXML(s, low, "low");
			Vec3ToXML(s, up, "up");
			MaterialToXML(s, mat);
			
			s.end_element();
		}
		Cube():Object()
		{
			low=glm::vec3(0.0,0.0,0.0);
			up=glm::vec3(0.0,0.0,0.0);
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

		virtual std::string Classname() { return "Triangle";}
		virtual void FromXML(xml::parser& p)
		{
			p.next_expect (xml::parser::start_element,"position", xml::content::complex);
			centerPos=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"v1", xml::content::complex);
			vert1=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"v2", xml::content::complex);
			vert2=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);
			
			p.next_expect (xml::parser::start_element,"v3", xml::content::complex);
			vert3=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);
			
			p.next_expect (xml::parser::start_element,"material",xml::content::complex);
			mat=MaterialAttributesFromXML(p);
			p.next_expect (xml::parser::end_element);
		}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element("Object");
			s.attribute("type", Classname());
			WritePositionToXML(s);

			Vec3ToXML(s, vert1, "v1");
			Vec3ToXML(s, vert2, "v2");
			Vec3ToXML(s, vert3, "v3");

			MaterialToXML(s, mat);
			s.end_element();
		}
		Triangle():Object()
		{
			vert1 = glm::vec3(0.0,0.0,0.0);
			vert2 = glm::vec3(0.0,0.0,0.0);
			vert3 = glm::vec3(0.0,0.0,0.0);
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
		virtual std::string Classname() { return "Rectangle";}
		Rectangle():Object()
		{

		}
};

class Cylinder:public Object
{
	public:
		glm::vec3 up;
		float radius;
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element("Object");
			s.attribute("type", Classname());
			WritePositionToXML(s);
			Vec3ToXML(s, up, "up");
			s.start_element("radius");
			s.attribute("radius", radius);
			s.end_element();
			MaterialToXML(s, mat);
		}
		virtual std::string Classname(){ return "Cylinder"; }

		Cylinder(glm::vec3 position, glm::vec3 theUp, float rad):Object(position, mat), up(theUp), radius(rad){}
};

class Sphere:public Object
{
	public:
		GLfloat radius;

		virtual std::string Classname() { return "Sphere";}
		virtual void FromXML(xml::parser& p)
		{
			p.next_expect (xml::parser::start_element,"position", xml::content::complex);
			centerPos=Vec3FromXML(p);
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"radius", xml::content::complex);
			radius=p.attribute<float>("radius");
			p.next_expect (xml::parser::end_element);

			p.next_expect (xml::parser::start_element,"material",xml::content::complex);
			mat=MaterialAttributesFromXML(p);
			p.next_expect (xml::parser::end_element);
		}
		virtual const void ToXML(xml::serializer& s)
		{
			s.start_element("Object");
			s.attribute("type", Classname());
			WritePositionToXML(s);

			s.start_element("radius");
			s.attribute("radius", radius);
			s.end_element();
			
			MaterialToXML(s, mat);	
			s.end_element();		
		}
		Sphere()
		{
			radius=0.0f;
			centerPos=glm::vec3(0.0,0.0,0.0);
		}
		Sphere(GLfloat theRadius, glm::vec3 position):Object(position)
		{
			radius=theRadius;
		}
		Sphere(glm::vec3 position, float theRadius, Material& mat):Object(position, mat)
		{
			radius=theRadius;
		}

};


#endif