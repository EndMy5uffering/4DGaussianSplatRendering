#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "Texture.h"

typedef unsigned int ShaderType;

struct ShaderSource {
	unsigned int type;
	std::string path;
	std::string source;
};

class Shader
{
public:
	Shader();
	~Shader();

	void Bind() const;
	void Unbind() const;

	void AddShaderSource(const std::string &path, ShaderType type);
	void BuildShader();
	void RebuildShader();

	static bool TryCompile(std::string source, ShaderType type);

	//Set uniforms

	inline unsigned int GetUniformLocation(const std::string& name);

	/* Sets 1 float value */
	void SetUniform1f(const std::string& name, float v0);

	/* Sets a vec2 with 2 float values */
	void SetUniform2f(const std::string& name, float v0, float v1);

	/* Sets a vec2 with 2 float values */
	void SetUniform2f(const std::string& name, glm::vec2 v);

	/* Sets a vec3 with 3 float values */
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);

	/* Sets a vec3 with 3 float values */
	void SetUniform3f(const std::string& name, glm::vec3 v);
	
	/* Sets a vec4 with 4 float values */
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

	/* Sets a vec4 with 4 float values */
	void SetUniform4f(const std::string& name, glm::vec4 v);

	/* Sets a 4x4 Matrix */
	void SetUniformMat4f(const std::string& name, glm::mat4 mat);

	/* Sets a 3x3 Matrix */
	void SetUniformMat3f(const std::string& name, glm::mat3 mat);

	/* Sets a 2x2 Matrix */
	void SetUniformMat2f(const std::string& name, glm::mat2 mat);

	/* Sets a texture unit slot for a given name*/
	void SetUniform1i(const std::string& name, int unit);

	//For comput shaders

	void DispatchCompute(unsigned int width, unsigned int height, unsigned int depth);
	void DispatchCompute(unsigned int width, unsigned int height);
	void DispatchCompute(Texture &tex, unsigned int depth);
	void DispatchCompute(Texture &tex);

	std::string GetLoadedShaderSource(const char* path);
	unsigned int GetRenderID();
	size_t GetShaderID();
	std::vector<ShaderSource> GetShaderSources();
	ShaderSource* GetShourceByIdx(int idx);
	size_t GetShaderSourceSize();

	int CompileShader(const ShaderSource& shader);
	std::string LoadShaderSource(const std::string& filePath);

private:
	size_t mShaderID = 0;
	std::vector< ShaderSource > m_ShaderSources;
	unsigned int m_RendererID{0};
	std::unordered_map<std::string, int> uniformCash; //chashing for uniforms

};

