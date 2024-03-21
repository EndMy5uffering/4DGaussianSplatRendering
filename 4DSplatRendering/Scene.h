#pragma once

/*Base class for the scenes*/

class Scene
{
private:
	Renderer& m_renderer;
	Camera& m_camera;

public:
	Scene(Renderer& renderer, Camera& cam) : m_renderer{ renderer }, m_camera{ cam } {}

    virtual ~Scene() {}

	virtual void init() = 0;
    virtual void unload() = 0;
	virtual void Render() = 0;
	virtual void Update(GLFWwindow *hwin) = 0;
	virtual void GUI() = 0;

	Renderer& GetRenderer() { return m_renderer; }
	Camera& GetCamera() { return m_camera; }
};


