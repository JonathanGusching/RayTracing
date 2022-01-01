#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera:public Object
{
	public:
		// Object already contains the position
		glm::vec3 direction;
		glm::vec3 up;

		void Rotate()
		{

			
		}
		Camera(glm::vec3 thePos, glm::vec3 theDirection, glm::vec3 theUp):Object(thePos)
		{
			direction=theDirection;
			up=theUp;
		}

		Camera():Object()
		{
			direction=glm::vec3(0.0,0.0,1.0);
			up=glm::vec3(0.0,1.0,0.0);
			
		}

		~Camera()
		{
			
		}

};

#endif