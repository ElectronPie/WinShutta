#include <Windows.h>
#include <WinUser.h>
#include <lmcons.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <future>

namespace chr = std::chrono;
using namespace std::chrono_literals;

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Initialize GLFW and declare it's resources
    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit())
    {
        std::cerr << "GLFW initialization failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Initialize variables vital for program
    int secondsOnlineDuration = 0;
    std::string password;
    bool is_init = false;
    const std::unordered_map<std::string, std::string> user_passwords = {
        #include "pw_config.inc"
    };

    // Enter window reopen loop >:)
    for(;;)
    {
        // Setup some window hints
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

        // Create window with graphics context
        GLFWwindow* window = glfwCreateWindow(1920, 1080, "WinShutta", glfwGetPrimaryMonitor(), NULL);
        if(window == nullptr)
        {
            std::cerr << "Faled to initialize GLFW window!" << std::endl;
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        /* Load OpenGL with GLAD */
        if(!gladLoadGL())
        {
            std::cerr << "Can't load GLAD!" << std::endl;
            exit(EXIT_FAILURE);
        }

        /* Output OpenGL version */
        std::cout << glGetString(GL_VERSION) << std::endl;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        //Main Render loop
        while(!glfwWindowShouldClose(window))
        {
            // Poll and handle events
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // You don't have such privilege >:)
            //ImGui::ShowDemoWindow();

            // Show our ImGui window
            {
                ImGui::Begin("Input Requested");
                ImGui::InputInt("Seconds before shutdown", &secondsOnlineDuration);
                ImGui::InputText("Password", &password);
                if(ImGui::Button("Submit"))
                {
                    if(secondsOnlineDuration < 0)
                    {
                        //auto temp = std::async(std::launch::async, []{
                        //    MessageBox(
                        //        NULL,
                        //        "There must be 0 or more seconds before shutdown",
                        //        "Error!",
                        //        MB_OK | MB_ICONERROR
                        //    );
                        //});
                        goto EndImGuiWindow;
                    }

                    char username[UNLEN + 1];
                    DWORD username_len = UNLEN + 1;
                    GetUserName(username, &username_len);
                    try
                    {
                        if(user_passwords.at(username) != password)
                        {
                            //auto temp = std::async(std::launch::async, []{
                            //    MessageBox(
                            //        NULL,
                            //        "The passwords don't match!",
                            //        "Error!",
                            //        MB_OK | MB_ICONERROR
                            //    );
                            //});
                            goto EndImGuiWindow;
                        }
                    }
                    catch(const std::out_of_range&)
                    {}

                    is_init = true;
                }
                EndImGuiWindow:
                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);

            if(is_init)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }

        // Now everything must be destroyed
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);

        // Only if is_init is on, we can finally be set free
        if(is_init) break;
    }

    // Now we can set the wait timepoint
    auto now = chr::steady_clock::now();
    auto shutdown_timepoint = now + chr::seconds(secondsOnlineDuration);
    auto warning_timepoint  = shutdown_timepoint - 10min;

    if(warning_timepoint > now)
    {
        std::this_thread::sleep_until(warning_timepoint);
        auto temp = std::async(std::launch::async, []{
            MessageBox(
                NULL,
                "Your time is up in 10 minutes!",
                "Warning!",
                MB_OK | MB_ICONWARNING
            );
        });
    }

    std::this_thread::sleep_until(shutdown_timepoint);
    system("shutdown /s /t 0");

    return 0;
}
