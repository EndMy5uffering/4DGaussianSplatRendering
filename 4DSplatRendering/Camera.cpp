#include "Camera.h"

#include <iostream>

Camera::Camera(int width, int height) :
	width{ width },
	height{ height },
	orientation(glm::vec3(0.0f, 0.0f, -1.0f)), 
	position(glm::vec3(0.0f, 0.0f, 0.0f)), 
	up(glm::vec3(0.0f, 1.0f, 0.0f))
{
}

Camera::Camera(int width, int height, glm::vec3 startPos) :
	width{ width },
	height{ height },
	orientation(glm::vec3(0.0f, 0.0f, -1.0f)),
	position(startPos),
	up(glm::vec3(0.0f, 1.0f, 0.0f))
{
}

Camera::Camera(int width, int height, glm::vec3 startPos, glm::vec3 orientation) :
	width{ width },
	height{ height },
	orientation(orientation),
	position(startPos),
	up(glm::vec3(0.0f, 1.0f, 0.0f))
{
}

Camera::Camera(int width, int height, glm::vec3 startPos, glm::vec3 orientation, glm::vec3 up) :
	width{ width },
	height{ height },
	orientation(orientation), 
	position(startPos), up(up)
{
}

Camera::~Camera()
{
}

glm::mat4 Camera::GetViewProjMatrix()
{
	return glm::perspective(glm::radians(mFOV), ((float)width / (float)height), mNear, mFar) * 
		glm::lookAt(this->position, this->position + this->orientation, this->up);
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(this->position, this->position + this->orientation, this->up);
}

glm::mat4 Camera::GetProjMatrix()
{
	return glm::perspective(glm::radians(mFOV), ((float)width / (float)height), mNear, mFar);
}

void Camera::HandleInput(GLFWwindow* window, bool imguiActive)
{

	if (imguiActive) return;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
	{
		position += orientation * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
	{
		position += orientation * -speed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position += -speed * glm::normalize(glm::cross(orientation, up));
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position += speed * glm::normalize(glm::cross(orientation, up));
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		up = glm::rotate(up, glm::radians(1.0f), orientation);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		up = glm::rotate(up, glm::radians(-1.0f), orientation);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		speed = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		speed = 0.1f;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		position += speed * up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		position += -speed * up;
	}

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !mCaptureMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);
		mFirstCapture = false;
		mCaptureMouse = true;
	}
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		mCaptureMouse = false;
	}
	
	if (mCaptureMouse) 
	{
		HandleCamRotation(window);
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) 
	{
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		double hw = (double)width / 2.0, hh = (double)height / 2.0;
		std::cout << "MOUSE: { " << mx << "; " << my << " }\nSCREEN: { " 
			<< width << "; " << height << "}\nHSCREEN: { "
			<< hw << "; " << hh << " }\n";
	}
}

void Camera::Resize(int width, int height)
{
	this->width = width;
	this->height = height;
}

void Camera::HandleCamRotation(GLFWwindow* window)
{
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);
	glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);

	double rotX = -sensitivity * (mouseY - int((double)height / 2.0)) / (double)(height);
	double rotY = -sensitivity * (mouseX - int((double)width / 2.0)) / (double)(width);

	orientation = glm::rotate(orientation, (float)glm::radians(rotX), glm::normalize(glm::cross(orientation, up)));

	glm::vec3 side = glm::normalize(glm::cross(up, orientation));
	up = glm::normalize(glm::cross(orientation, side));

	orientation = glm::rotate(orientation, (float)glm::radians(rotY), up);
}
