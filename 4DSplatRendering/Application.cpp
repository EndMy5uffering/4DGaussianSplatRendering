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

#include "Gizmo.h"

#include "Utils.h"


int SCREEN_WIDTH = 980;
int SCREEN_HEIGHT = 680;
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
    glfwWindowHint(GLFW_MAXIMIZED, 1);

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

    Geometry::Box background(glm::vec3(0.0f), glm::vec3(500.0f));

    Shader SplatComputeShader;
    SplatComputeShader.AddShaderSource("../Shader/Splat2DComputeShader.GLSL", GL_COMPUTE_SHADER);
    SplatComputeShader.BuildShader();
    Shader SplatRenderShader;
    SplatRenderShader.AddShaderSource("../Shader/Splat2DFragShader.GLSL", GL_FRAGMENT_SHADER);
    SplatRenderShader.AddShaderSource("../Shader/Splat2DVertexShader.GLSL", GL_VERTEX_SHADER);
    SplatRenderShader.BuildShader();

    Shader SplatRenderShader3D;
    SplatRenderShader3D.AddShaderSource("../Shader/Splat3DFragShader.GLSL", GL_FRAGMENT_SHADER);
    SplatRenderShader3D.AddShaderSource("../Shader/Splat3DVertexShader.GLSL", GL_VERTEX_SHADER);
    SplatRenderShader3D.BuildShader();

    Shader lineShader;
    lineShader.AddShaderSource("../Shader/LineFrag.GLSL", GL_FRAGMENT_SHADER);
    lineShader.AddShaderSource("../Shader/LineVert.GLSL", GL_VERTEX_SHADER);
    lineShader.BuildShader();

    Splat2D s2d({ 0.0f, 0.0f, 0.0f }, { 1.0f , 0.0f }, 4.0f, 2.0f, SplatRenderShader, {0.0f, 0.0f, 0.0f, 1.0f});

    Splat3D s3d(
        glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
        glm::quatLookAt(glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{0.0f, 1.0f, 0.0f}), 
        5.0f, 1.0f, 5.0f, 
        SplatRenderShader3D,
        lineShader,
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
        cam
    );

    size_t numOfSplats = 100;
    std::vector<Splat2D*> splats;
    splats.reserve(numOfSplats);

    float from = -50;
    float to = 50;
    for(size_t i = 0; i < numOfSplats; ++i)
    {
        Splat2D* s = new Splat2D({ 100 + frand(from, to), frand(from, to), frand(from, to) },
            getVecFromAngle(frand(0, 360.0f)),
            frand(0.5f, 10.0f),
            frand(0.5f, 10.0f),
            SplatRenderShader,
            { frand(0.0f, 1.0f), frand(0.0f, 1.0f) , frand(0.0f, 1.0f), frand(0.1f, 1.0f) });
        splats.push_back(s);
    }


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DebugMenus::Splat2DMenuData menuData;
    menuData.angle = 0.0f;
    menuData.l0 = 4.0f;
    menuData.l1 = 2.0f;

    DebugMenus::Menu02Data menu2Data;
    menu2Data.buffer = "This could be your shader :D";
    menu2Data.shaders.push_back(&SplatRenderShader);

    DebugMenus::BlendOpt blendOpt;
    blendOpt.selected0 = GL_SRC_ALPHA;
    blendOpt.selected1 = GL_ONE_MINUS_SRC_ALPHA;

    DebugMenus::MenueStripData menueStripData;
    menueStripData.cam = &cam;

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();

        shader.Bind();
        shader.SetUniformMat4f("u_cam", cam.GetViewProjMatrix());
        
        shader.SetUniform1i("u_tex0", 1);
        shader.SetUniformMat4f("u_modle", background.GetTransform());
        background.Render(renderer);
        

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);
        s2d.SetLambas(menuData.l0, menuData.l1);
        float ang = menuData.angle * PI / 180.0f;
        s2d.SetVectors({ cos(ang), sin(ang) });
        s2d.SetColor({ menuData.color[0], menuData.color[1], menuData.color[2], menuData.color[3] });
        s2d.SetPosition(menuData.splatPos);
        //s2d.Draw(renderer, cam);

        s3d.Draw(renderer);

        /*lineShader.Bind();
        lineShader.SetUniformMat4f("uViewProj", cam.GetViewProjMatrix());
        glm::vec3 lv0{0.0, 0.0, 0.0};
        glm::vec3 lv1{100.0, 0.0, 0.0};
        renderer.DrawLine(lv0, lv1);*/


        std::sort(splats.begin(), splats.end(), [](Splat2D *a, Splat2D *b) {
            float aDist = glm::length((a->GetPosition() - cam.GetPosition()));
            float bDist = glm::length((b->GetPosition() - cam.GetPosition()));
            return aDist > bDist;
        });

        for (size_t i = 0; i < splats.size(); ++i)
        {
            splats[i]->Draw(renderer, cam);
        }
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::Splat2DMenu(&menuData, &menueStripData.show_2DSplat);
        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        std::string val = getTextInfo(cam);
        DebugMenus::CamInfo(io, val, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData);

        std::string splatInfo = s3d.GetSplatData();
        DebugMenus::TextScreen("Splat Info:", splatInfo);

        //DebugMenu::Splat3DMenu();
        //IMGUI

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
        cam.HandleInput(window, ImGui::IsAnyItemActive());

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