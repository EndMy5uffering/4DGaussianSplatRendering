#pragma once

#include <math.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

class Camera
{
public:
	glm::vec3 position;
	glm::vec3 orientation = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);



	Camera(int width, int height);
	Camera(int width, int height, glm::vec3 startPos);
	Camera(int width, int height, glm::vec3 startPos, glm::vec3 orientation);
	Camera(int width, int height, glm::vec3 startPos, glm::vec3 orientation, glm::vec3 up);
	~Camera();

	glm::mat4 GetViewProjMatrix();
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjMatrix();
	glm::vec3 GetPosition();
	float GetFar();
	float GetNear();
	float GetFOV();
	float GetScreenWidth();
	float GetScreenHeight();

	void HandleInput(GLFWwindow *window, bool imguiActive = false);
	void SetWidth(int width) { this->mWidth = width; }
	void SetHeight(int height) { this->mHeight = height; }
	void SetNear(float near) { this->mNear = near; }
	void SetFar(float far) { this->mFar = far; }
	void SetFOV(float fov) { this->mFOV = fov; }
	void Resize(int width, int height);
	void HandleCamRotation(GLFWwindow* window);

private:
	bool mCaptureMouse = false;
	bool mFirstCapture = true;

	float mFOV = 60.0f;
	float mNear = 0.1f;
	float mFar = 256.0f;

	int mWidth;
	int mHeight;

	float mSensitivity = 100.0f;
	float mSpeed = 0.1f;
};

