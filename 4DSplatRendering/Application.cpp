#define DEBUG_OUTPUT

#include<GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stb_image.h>
#include <algorithm>

#include "Camera.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"

#include "Geometry.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

#include "Splat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "DebugMenus.h"

#include "Utils.h"


int SCREEN_WIDTH = 1000;
int SCREEN_HEIGHT = 1000;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, {0.0, 0.0, 5.0});

float frand(float from, float to)
{
    return from + ((to - from) * ((float)std::rand() / (float)RAND_MAX));
}

glm::vec2 getVecFromAngle(float ang) 
{
    float _ang = ang * PI / 180.0f;
    return glm::vec2{ cos(_ang), sin(_ang) };
}

std::string getTextInfo(Camera cam) 
{
    glm::mat4 vmat = cam.GetViewMatrix();
    glm::mat4 projmat = cam.GetProjMatrix();
    std::string vmats = Utils::Mat4ToStr("View", vmat);
    std::string projMats = Utils::Mat4ToStr("Proj", projmat);
    return vmats + "\n" + projMats;
}


void updateScreenSize(GLFWwindow* window, int width, int height) 
{
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    cam.Resize(SCREEN_WIDTH, SCREEN_HEIGHT);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_MAXIMIZED, 1);
    //glfwWindowHint(GLFW_SAMPLES, 16);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "4D Gaussian Splats", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) 
    {
        std::cout << "[Error]: Glew init" << std::endl;
    }
    std::cout << glGetString(GL_VERSION) << std::endl;

    glfwSetWindowSizeCallback(window, updateScreenSize);
    //IMGUI

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    //IMGUI
    
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));
    
    Texture texture0("../Textures/Kenney/PNG/Dark/texture_01.png", GL_RGB, GL_RGB);
    texture0.Bind(1);
    Texture skyTexture("../Textures/Kenney/PNG/Orange/texture_08.png", GL_RGB, GL_RGB);
    skyTexture.Bind(2);
    
    Shader shader;
    shader.AddShaderSource("../Shader/TemplateFragmentShader.GLSL", GL_FRAGMENT_SHADER);
    shader.AddShaderSource("../Shader/TemplateVertexShader.GLSL", GL_VERTEX_SHADER);
    shader.BuildShader();

    Renderer renderer;
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    
    /* Loop until the user closes the window */
    glClearColor(0.34901960784313724f, 0.3843137254901961f, 0.4588235294117647f,1.0f);
    cam.SetFar(5000.0f);

    Geometry::Box background(glm::vec3(0.0f), glm::vec3(5000.0f));

    Shader SplatRenderShader;
    SplatRenderShader.AddShaderSource("../Shader/Splat2DFragShader.GLSL", GL_FRAGMENT_SHADER);
    SplatRenderShader.AddShaderSource("../Shader/Splat2DVertexShader.GLSL", GL_VERTEX_SHADER);
    SplatRenderShader.BuildShader();

    Shader SplatRenderShader3D;
    SplatRenderShader3D.AddShaderSource("../Shader/Splat3DFragShader.GLSL", GL_FRAGMENT_SHADER);
    SplatRenderShader3D.AddShaderSource("../Shader/Splat3DVertexShader.GLSL", GL_VERTEX_SHADER);
    SplatRenderShader3D.BuildShader();

    Splat3D s3d(
        glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
        glm::quatLookAt(glm::normalize(glm::vec3{ 0.0f, 0.0f, -1.0f }), glm::vec3{0.0f, 1.0f, 0.0f}),
        glm::vec3{1.0f, 1.0f, 1.0f},
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}
    );

    Splat4D s4d{
        glm::vec4{2.0f, 0.0f, 0.0f, 0.0f},
        glm::normalize(glm::quatLookAt(glm::vec3{1.0, 1.0, 0.0}, glm::vec3{0.0, 1.0, 0.0})),
        glm::normalize(glm::quatLookAt(glm::vec3{1.0, -1.0, 0.0}, glm::vec3{0.0, 1.0, 0.0})),
        glm::vec4{1.0, 1.0, 1.0, 1.0},
        glm::vec4{1.0, 0.0, 0.0, 1.0}
    };

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DebugMenus::Menu02Data menu2Data;
    menu2Data.buffer = "This could be your shader :D";
    menu2Data.shaders.push_back(&SplatRenderShader);

    DebugMenus::Splat3DMenuData splat3DMenuData;
    splat3DMenuData.rot = glm::vec3{ 1.0f, 1.0f, 1.0f };
    splat3DMenuData.pos = s3d.GetPosition();
    splat3DMenuData.lambdas = s3d.GetScale();
    splat3DMenuData.color = s3d.GetColor();

    DebugMenus::BlendOpt blendOpt;
    blendOpt.selected0 = GL_SRC_ALPHA;
    blendOpt.selected1 = GL_ONE_MINUS_SRC_ALPHA;

    DebugMenus::MenueStripData menueStripData;
    menueStripData.cam = &cam;

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        cam.HandleInput(window, ImGui::IsAnyItemActive());

        shader.Bind();
        shader.SetUniformMat4f("u_cam", cam.GetViewProjMatrix());
        
        shader.SetUniform1i("u_tex0", 1);
        shader.SetUniformMat4f("u_modle", background.GetTransform());
        background.Render(renderer);
        

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);

        s3d.SetPosition(splat3DMenuData.pos);
        s3d.SetLambas(splat3DMenuData.lambdas.x, splat3DMenuData.lambdas.y, splat3DMenuData.lambdas.z);
        s3d.SetQuaternion(glm::quatLookAt(glm::normalize(splat3DMenuData.rot), glm::vec3{0.0f, 1.0f, 0.0f}));
        s3d.SetColor(splat3DMenuData.color);

        //s3d.Draw(renderer, SplatRenderShader3D, cam);

        s4d.Draw(renderer, SplatRenderShader3D, cam);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glm::vec3 lv0{0, 0, 0};
        glm::vec3 lv1{10000, 0, 0};
        glm::vec3 lv2{0, 10000, 0};
        glm::vec3 lv3{0, 0, 10000};
        renderer.DrawLine(lv0, lv1, glm::vec4{1.0, 0.0, 0.0, 1.0}, cam);
        renderer.DrawLine(lv0, lv2, glm::vec4{0.0, 1.0, 0.0, 1.0}, cam);
        renderer.DrawLine(lv0, lv3, glm::vec4{0.0, 0.0, 1.0, 1.0}, cam);

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        std::string val = getTextInfo(cam);
        DebugMenus::CamInfo(io, val, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData);

        DebugMenus::Splat3DMenu(&splat3DMenuData, &menueStripData.show_3DSplat);
        DebugMenus::Splat4DMenu(&s4d, &cam, &menueStripData.show_4DSplat);
        //IMGUI

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}