#include "objects.hpp"

void SceneManager::AddObject(Object& object)
{
	currentScene.objects.push_back(object);
}