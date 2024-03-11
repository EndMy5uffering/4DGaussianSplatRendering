#include "Utils.h"

void Utils::Mat3Print(const glm::mat3& m)
{
    std::cout << "M: [\n"
        << "\t[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << " ],\n"
        << "\t[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << " ],\n"
        << "\t[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << " ]\n"
        << "]\n";
}

void Utils::Mat3Print(const std::string name, const glm::mat3& m)
{
    std::cout << name << ": [\n"
        << "\t[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << " ],\n"
        << "\t[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << " ],\n"
        << "\t[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << " ]\n"
        << "]\n";
}

void Utils::Mat4Print(const glm::mat4& m)
{
    std::cout << "M: [\n"
        << "\t[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << " ],\n"
        << "\t[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << " ],\n"
        << "\t[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << " ],\n"
        << "\t[ " << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << " ]\n"
        << "]\n";
}

void Utils::Mat4Print(const std::string name, const glm::mat4& m)
{
    std::cout << name << ": [\n"
        << "\t[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << " ],\n"
        << "\t[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << " ],\n"
        << "\t[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << " ],\n"
        << "\t[ " << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << " ]\n"
        << "]\n";
}

void Utils::Vec3Print(const glm::vec3& m)
{
    std::cout << "V: [ " << m[0] << ", " << m[1] << ", " << m[2] << " ]\n";
}

void Utils::Vec3Print(const std::string name, const glm::vec3& m)
{
    std::cout << name << ": [ " << m[0] << ", " << m[1] << ", " << m[2] << " ]\n";
}

void Utils::Vec4Print(const glm::vec4& m)
{
    std::cout << ": [ " << m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << " ]\n";
}

void Utils::Vec4Print(const std::string name, const glm::vec4& m)
{
    std::cout << name << ": [ " << m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << " ]\n";
}

std::string Utils::Mat4ToStr(const std::string name, glm::mat4 m)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << " ]\n";
    ss << "[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << " ]\n";
    ss << "[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << " ]\n";
    ss << "[ " << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << " ]\n";
    return ss.str();
}

std::string Utils::Mat3ToStr(const std::string name, glm::mat3 m)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << " ]\n";
    ss << "[ " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << " ]\n";
    ss << "[ " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << " ]\n";
    return ss.str();
}

std::string Utils::Mat2ToStr(const std::string name, glm::mat2 m)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << m[0][0] << ", " << m[0][1] << " ]\n";
    ss << "[ " << m[1][0] << ", " << m[1][1] << " ]\n";
    return ss.str();
}

std::string Utils::V4ToStr(const std::string name, glm::vec4 v)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << " ]\n";
    return ss.str();
}

std::string Utils::V3ToStr(const std::string name, glm::vec3 v)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << v[0] << ", " << v[1] << ", " << v[2] << " ]\n";
    return ss.str();
}

std::string Utils::V2ToStr(const std::string name, glm::vec2 v)
{
    std::stringstream ss;
    ss << name << ":\n";
    ss << "[ " << v[0] << ", " << v[1] << " ]\n";
    return ss.str();
}

float Utils::minf(float a, float b)
{
    return a < b ? a : b;
}

float Utils::maxf(float a, float b)
{
    return a > b ? a : b;
}

float Utils::lerp(float a, float b, float t)
{
    return a + ((b - a) * t);
}

float Utils::mapf(float x, float a, float b, float c, float d)
{
    return c + ((x / (b - a)) * (d - c));
}
