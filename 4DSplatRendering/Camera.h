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
	glm::vec3 orientation{1.0f, 0.0f, 0.0f};
	glm::vec3 up{0.0f, 1.0f, 0.0f};

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
	glm::vec2 GetViewport();
	glm::vec2 GetFocal();
	float GetSpeed();
	bool IsViewFixed();
	bool IsPositionFixed();

	void HandleInput(GLFWwindow *window, bool imguiActive = false);
	void SetWidth(int width) { this->mWidth = width; }
	void SetHeight(int height) { this->mHeight = height; }
	void SetNear(float near) { this->mNear = near; }
	void SetFar(float far) { this->mFar = far; }
	void SetFOV(float fov) { this->mFOV = fov; }
	void Resize(int width, int height);
	void HandleCamRotation(GLFWwindow* window);
	void SetPosition(glm::vec3 npos) { this->position = npos; }
	void SetOrientation(glm::vec3 norientation) { this->orientation = norientation; }
	void SetUp(glm::vec3 nup) { this->up = nup; }
	void SetIsViewFixedOnPoint(bool fixed, glm::vec4 point);
	void SetIsViewFixedOnPoint(bool fixed);
	void SetIsPositionFixed(bool fixed);
	void SetSpeed(float speed);

	inline bool IsLockX() { return this->mLockX; }
	inline bool IsLockY() { return this->mLockY; }
	void SetLockX(bool lock) { this->mLockX = lock; }
	void SetLockY(bool lock) { this->mLockY = lock; }

private:
	bool mCaptureMouse = false;
	bool mFirstCapture = true;
	bool mFixViewPoint = false;
	bool mFixPostion = false;

	float mFOV = 60.0f;
	float mNear = 0.1f;
	float mFar = 256.0f;

	int mWidth;
	int mHeight;

	float mSensitivity = 100.0f;
	float mSpeed = 0.5f;
	float mFastSpeed = 2.0f;

	bool mLockY = false;
	bool mLockX = false;
};

