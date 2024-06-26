/*
    Basic setup was copied from the glfw documentation
    https://www.glfw.org/documentation.html
    Small additions were made to add functionality for scenes and the splats in general
*/

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
#include <chrono>
#include <functional>

#include "Camera.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"

#include "Geometry.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/common.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "Splat.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Utils.h"

#include "radix_sort.hpp"

#include "BSPTree.h"

#include "ShareStorageBuffer.h"

#include "VDataParser.h"

#include "Scene.h"
#include "Scenes.h"
#include "DebugMenus.h"

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 800;
Camera cam(SCREEN_WIDTH, SCREEN_HEIGHT, { 70, 110, 110 }, glm::normalize(glm::vec3{0, -1.0, -1.0}));

/* Callback function to react to screen resize event */
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
    //glfwWindowHint(GLFW_SAMPLES, 8);
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    //glfwSwapInterval(0); //Disable vsync

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

    //IMGUI Setup
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

    Renderer renderer;
    
    /* Loop until the user closes the window */
    //glClearColor(0.34901960784313724f, 0.3843137254901961f, 0.4588235294117647f, 1.0f);
    glClearColor(0.1843137254901961, 0.20784313725490197, 0.25882352941176473,1.0f);
    cam.SetFar(5000.0f);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    DebugMenus::Menu02Data menu2Data;
    menu2Data.buffer = "This could be your shader :D";

    DebugMenus::BlendOpt blendOpt;
    blendOpt.selected0 = GL_SRC_ALPHA;
    blendOpt.selected1 = GL_ONE_MINUS_SRC_ALPHA;

    DebugMenus::MenueStripData menueStripData;
    menueStripData.cam = &cam;
    menueStripData.u_scene_ptr = std::make_unique<Scenes::Empty>(renderer, cam);
    menueStripData.u_scene_ptr->init();

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();
        cam.HandleInput(window, ImGui::IsAnyItemActive());
        glBlendFunc(blendOpt.selected0, blendOpt.selected1);


        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        if (menueStripData.u_scene_ptr)
        {
            menueStripData.u_scene_ptr->Update(window);
            menueStripData.u_scene_ptr->Render();
        }
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        //IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DebugMenus::ShaderEditor(&menu2Data, &menueStripData.show_ShaderEditor);
        DebugMenus::BlendOptMenu(&blendOpt, &menueStripData.show_BlendOpt);
        DebugMenus::CamInfo(io, cam, &menueStripData.show_CamInfo);
        DebugMenus::MainMenuStrip(&menueStripData, renderer, cam);
        //IMGUI

        if (menueStripData.u_scene_ptr) 
        {
            menueStripData.u_scene_ptr->GUI();
        }

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