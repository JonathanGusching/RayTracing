#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>


class Camera:public Object
{
	public:
		float last_x;
		float last_y;
		float cameraSpeed;
		float sensitivity;

		bool first_time;
	
		// Object already contains the position
		glm::vec3 direction;
		glm::vec3 left;

		float yaw;
		float pitch;

		Camera(glm::vec3 thePos, glm::vec3 theDirection, glm::vec3 theLeft, float theSensitivity):Object(thePos)
		{
			direction=theDirection;
			left=theLeft;
			pitch=0;
			yaw=0;
			cameraSpeed=7.0f;
			sensitivity=theSensitivity;
		}

		Camera():Object()
		{
			direction=glm::vec3(0.0,0.0,1.0);
			left=glm::vec3(-1.0,0.0,0.0);
			pitch=0;
			yaw=0;
			cameraSpeed=7.0f;
			sensitivity=0.07f;
		}


		~Camera()
		{
			
		}

};

#endif