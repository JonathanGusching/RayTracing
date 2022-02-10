#include "objects.hpp"

void Scene::AddObject(Object& object)
{
	objects.push_back(&object);
}
void Scene::Reset()
{
	objects.clear();
}

void SceneManager::ImportScene(const char* file)
{
	currentScene.Reset();
	std::fstream xml_file;
	xml_file.open(file, std::ios::in);
	if(!xml_file)
	{
		std::cerr << "Could not open XML file" << std::endl;
	}
	else
	{
		std::cout << file << " XML File opened successfully" << std::endl;
		
		xml::parser p (xml_file, file);
		p.next_expect (xml::parser::start_element,"scene",xml::content::complex); // xml::content::complex allows to ignore new lines, etc...

		std::string name=p.attribute("name");
		currentScene.name=name;
		do
		{
			try
			{
				p.next_expect(xml::parser::start_element, "Object",xml::content::complex);
				std::string objectType=p.attribute("type");

				// Objects need to be dynamic (pointers necessary, they must not be local)
				if(objectType=="Object")
				{
					Object* object = new Object();
					object->FromXML(p);
					currentScene.AddObject(*object);
				}
				else if(objectType=="Cube")
				{
					Cube* cube = new Cube();
					cube->FromXML(p);
					currentScene.AddObject(*cube);
				}
				else if(objectType=="Sphere")
				{
					Sphere* sphere= new Sphere();
					sphere->FromXML(p);
					currentScene.AddObject(*sphere);
				}
				else if(objectType=="Triangle")
				{
					Triangle* triangle= new Triangle();
					triangle->FromXML(p);
					currentScene.AddObject(*triangle);
				}
				else if(objectType=="Cylinder")
				{
					Cylinder* cylinder= new Cylinder();
					cylinder->FromXML(p);
					currentScene.AddObject(*cylinder);
				}
				p.next_expect(xml::parser::end_element);
			}
			catch(const xml::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}while(p.peek() != xml::parser::eof);
	}

	xml_file.close();
}

const void SceneManager::ExportScene(const char* file)
{
	std::fstream xml_file;
	xml_file.open(file, std::ios::out);
	if(!xml_file)
	{
		std::cerr << "Error : Could not create XML file" << std::endl;
	}
	else
	{
		std::cout << file << " XML File created successfully" << std::endl;
		xml::serializer s(xml_file, file);
		s.start_element("scene");
		s.attribute("name", currentScene.name);
		for(Object* obj:currentScene.objects)
		{
			obj->ToXML(s);
		}
		s.end_element();
	}
	xml_file.close();
}