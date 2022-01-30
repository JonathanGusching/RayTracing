#include "objects.hpp"

void Scene::AddObject(Object& object)
{
	objects.push_back(&object);
}
void Scene::Reset()
{
	objects.clear();
}

const void SceneManager::CreateScene(const char* name, std::vector<Object> objects)
{

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


		xml_file.close();
	}
}