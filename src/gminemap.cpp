#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <GL/gl3w.h>            // Initialize with gl3wInit()

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include "tinyfiledialogs.h"
#include <Magick++.h>

#include "common.h"
#include "Map.h"
#include "VersionSpec.h"
#include "ColorMap.h"
#include "constants.h"

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static char* OpenImageFileDialog()
{
    return tinyfd_openFileDialog("Select an image file", NULL, 0, NULL, NULL, 0);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static Magick::Image LoadPaletteImage(const int mc_ver) {
    Magick::Image palette_img;
    Minemap::VersionSpec mc_version = static_cast<Minemap::VersionSpec>(mc_ver + 1);
    std::string palette_path = Minemap::verSpecToPalettePath(mc_version);
    try {
        palette_img.read(palette_path);
    }
    catch (Magick::Exception &e) {
        // If we can't find the pre-configured one, try looking in the program directory
        palette_path = Minemap::VerSpecToFallbackPalettePath(mc_version);
        palette_img.read(palette_path);
    }
    return palette_img;
}

static Magick::Image LoadImage(const char* filename) {
    Magick::Image input_img;
    input_img.read(filename);
    return input_img;
}

bool ConvertToTexture(Magick::Quantum* pixels, int height, int width, GLuint* out_texture) {
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);
    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Convert pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, pixels);
    fprintf(stderr, "Texture converted: %d\n", glGetError());

    *out_texture = image_texture;
    return true;
}


int main(int, char** argv)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GMinemap", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    io.Fonts->AddFontDefault();

    // Init Magick Core
    Magick::InitializeMagick(*argv);
    // Loading Image
    Magick::Image palette_img;
    Magick::Image input_img;
    Magick::Image output_img;

    GLuint texture_id = 0;

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    bool preview_controls = false;
    bool export_controls = false;
    int dither_mode = 0;
    int game_version = 0;
    const Magick::DitherMethod dither_modes[] = {
        Magick::DitherMethod::NoDitherMethod,
        Magick::DitherMethod::RiemersmaDitherMethod,
        Magick::DitherMethod::FloydSteinbergDitherMethod
    };
    const char* game_version_strings[] = {"1.8", "1.12"};
    char* status_text = "Ready.";
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // pop up state
    char* popup_title = "_";
    char* popup_message = "";
    bool popup_message_free = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("GMinemap");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text(status_text);               // Display some text (you can use a format strings too)

            if (ImGui::Button("Load Image")) {
                char* filename = OpenImageFileDialog();
                if (filename) {
                    status_text = "Loading...";
                    if (!palette_img.isValid()) {
                        try {
                            palette_img = LoadPaletteImage(game_version);
                        } catch (Magick::Exception &e) {
                            popup_title = "Error loading palette image";
                            popup_message = strdup(e.what());
                            popup_message_free = true;
                            ImGui::OpenPopup(popup_title);
                        }
                    }
                    // re-check, since the previous load could fail
                    if (palette_img.isValid()) {
                        try {
                            input_img = LoadImage(filename);
                        } catch (Magick::Exception &e) {
                            popup_title = "Error loading image";
                            popup_message = strdup(e.what());
                            popup_message_free = true;
                            ImGui::OpenPopup(popup_title);
                        }
                    }
                    // re-check input image
                    if (input_img.isValid()) {
                        Magick::DitherMethod dithering = dither_modes[dither_mode];
                        MagickCore::Quantum* pixels;
                        output_img = input_img;
                        output_img.modifyImage();
                        // Scale input to 128x128
                        output_img.resize("128x128!");
                        // Execute remap
                        output_img.quantizeDitherMethod(dithering);
                        output_img.map(palette_img, dithering != Magick::DitherMethod::NoDitherMethod);
                        pixels = output_img.getPixels(0, 0, 128, 128);
                        ConvertToTexture(pixels, 128, 128, &texture_id);
                        preview_controls = true;
                    }
                }
            }

            ImGui::Text("Dithering algorithm:");
            ImGui::RadioButton("None", &dither_mode, 0); ImGui::SameLine();
            ImGui::RadioButton("Riemersma", &dither_mode, 1); ImGui::SameLine();
            ImGui::RadioButton("FloydSteinberg", &dither_mode, 2);

            ImGui::Text("Game version:");
            ImGui::Combo("", &game_version, game_version_strings, IM_ARRAYSIZE(game_version_strings));
            ImGui::SameLine(); HelpMarker("1.12 for game version [1.12, +oo)\n1.8  for game version [1.8,  1.12)");

            // pop-up window
            if (ImGui::BeginPopupModal(popup_title, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text(popup_message);
                ImGui::Separator();
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    if (popup_message_free && popup_message) free(popup_message);
                    popup_message_free = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::EndPopup();
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // show preview window
        if (preview_controls)
        {
            ImGui::Begin("GMinemap - Preview", &preview_controls);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            if (texture_id) {
                ImGui::Image((void*)(intptr_t)texture_id, ImVec2(128, 128));
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
