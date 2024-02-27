#pragma once

class Scene
{
public:
    Scene() {}

    virtual ~Scene() {}

	virtual void init(Renderer& renderer, Camera& cam) = 0;
    virtual void unload() = 0;
	virtual void Render(Renderer &renderer, Camera& cam) = 0;
	virtual void Update(GLFWwindow *hwin) = 0;
};


