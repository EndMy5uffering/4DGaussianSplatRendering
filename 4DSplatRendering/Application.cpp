#define DEBUG_OUTPUT

#include<GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stb_image.h>

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

#include "Splat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "DebugMenu.h"

float frand(float from, float to)
{
    return from + ((to - from) * ((float)std::rand() / (float)RAND_MAX));
}

int SCREEN_WIDTH = 980;
int SCREEN_HEIGHT = 680;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, {0.0, 0.0, 5.0});


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
    //ImGui_ImplOpenGL3_Init(glsl_version);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    //IMGUI
    
    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));
    
    /*Texture texture0("../Textures/Kenney/PNG/Dark/texture_01.png", GL_RGB, GL_RGB);
    texture0.Bind(1);
    Texture skyTexture("../Textures/Kenney/PNG/Orange/texture_08.png", GL_RGB, GL_RGB);
    skyTexture.Bind(2);
    */
    Shader shader;

    shader.AddShaderSource("../Shader/TemplateFragmentShader.GLSL", GL_FRAGMENT_SHADER);
    shader.AddShaderSource("../Shader/TemplateVertexShader.GLSL", GL_VERTEX_SHADER);
    shader.BuildShader();

    Renderer renderer;
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Loop until the user closes the window */
    glClearColor(0.34901960784313724f, 0.3843137254901961f, 0.4588235294117647f,1.0f);
    cam.SetFar(5000.0f);

    Geometry::Box background(glm::vec3(0.0f), glm::vec3(2500.0f));

    Shader SplatComputeShader;
    SplatComputeShader.AddShaderSource("../Shader/Splat2DComputeShader.GLSL", GL_COMPUTE_SHADER);
    SplatComputeShader.BuildShader();
    Shader SplatRenderShader;
    SplatRenderShader.AddShaderSource("../Shader/Splat2DFragShader.GLSL", GL_FRAGMENT_SHADER);
    SplatRenderShader.AddShaderSource("../Shader/Splat2DVertexShader.GLSL", GL_VERTEX_SHADER);
    SplatRenderShader.BuildShader();

    Splat2D s2d({ 0.0f, 0.0f, 0.0f }, { 1.0f , 0.0f }, 4.0f, 2.0f, SplatRenderShader, {0.0f, 0.0f, 0.0f});

    size_t numOfSplats = 1000;
    std::vector<Splat2D*> splats;
    splats.reserve(numOfSplats);

    float from = -20;
    float to = 20;
    for(size_t i = 0; i < numOfSplats; ++i)
    {
        Splat2D* s = new Splat2D({ frand(from, to), frand(from, to) , 0.0f },
            { frand(-1.0f, 1.0f), frand(-1.0f, 1.0f) },
            frand(-8.0f, 8.0f),
            frand(-8.0f, 8.0f),
            SplatRenderShader,
            { frand(0.0f, 1.0f), frand(0.0f, 1.0f) , frand(0.0f, 1.0f) });
        splats.push_back(s);
    }


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DebugMenu::Menu01Data menuData;
    menuData.v0 = 1.0f;
    menuData.v1 = 0.0f;
    menuData.l0 = 4.0f;
    menuData.l1 = 2.0f;

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_BLEND);
        /* Render here */
        renderer.Clear();

        /*shader.Bind();
        shader.SetUniformMat4f("u_cam", cam.GetViewProjMatrix());
        
        shader.SetUniform1i("u_tex0", 1);
        shader.SetUniformMat4f("u_modle", background.GetTransform());
        background.Render(renderer);
        */
        s2d.SetLambas(menuData.l0, menuData.l1);
        s2d.SetVectors({ menuData.v0, menuData.v1 });
        s2d.SetColor({ menuData.color[0], menuData.color[1], menuData.color[2] });
        s2d.Draw(renderer, cam);


        /*for (size_t i = 0; i < splats.size(); ++i)
        {
            splats[i]->Draw(renderer, cam, 0.0f);
        }*/

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenu::Menu_01(io, &menuData);

        //IMGUI

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
        cam.HandleInput(window);
        glDisable(GL_BLEND);

    }

    for (size_t i = 0; i < splats.size(); ++i) 
    {
        delete splats[i];
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}