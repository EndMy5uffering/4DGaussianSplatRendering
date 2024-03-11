#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include "glm/glm.hpp"

namespace Utils 
{

	void Mat3Print(const glm::mat3& m);
	void Mat3Print(const std::string name, const glm::mat3& m);
	void Mat4Print(const glm::mat4& m);
	void Mat4Print(const std::string name, const glm::mat4& m);
	void Vec3Print(const glm::vec3& m);
	void Vec3Print(const std::string name, const glm::vec3& m);
	void Vec4Print(const glm::vec4& m);
	void Vec4Print(const std::string name, const glm::vec4& m);
	std::string Mat4ToStr(const std::string name, glm::mat4 m);
	std::string Mat3ToStr(const std::string name, glm::mat3 m);
	std::string Mat2ToStr(const std::string name, glm::mat2 m);
	std::string V4ToStr(const std::string name, glm::vec4 v);
	std::string V3ToStr(const std::string name, glm::vec3 v);
	std::string V2ToStr(const std::string name, glm::vec2 v);


	//Stupid utils
	float minf(float a, float b);
	float maxf(float a, float b);
	float lerp(float a, float b, float t);

	/**
		Mapps a value x that falls in the interval a and b to the interval c, d
		@param x Value to be mapped
		@param a Lower bound for the interval x falls into
		@param b Upper bound for the interval x falls into
		@param c Lower bound for the interval x gets mapped to
		@param d Upper bound for the interval x gets mapped to
		@return Mapped value of x now between c and d
	*/
	float mapf(float x, float a, float b, float c, float d);

}

