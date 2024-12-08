/*
    Codes for getting coordinates for drawing things by image
    Useful for 16SEG or IIDX CAB drawing
 */

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <windows.h>
#include <gl/GL.h>
#include <imgui.h>
#include "stb_image.h"
#include <vector>

static GLuint img;
static int img_w, img_h;
static bool is_load = false, is_loaded;

static bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height);

void RenderImageWithClick(void) {
    if (!is_load) {
        is_loaded = LoadTextureFromFile("image.png", &img, &img_w, &img_h);
        printf("Load('%s') == %s\n", "image.png", is_loaded ? "T" : "F");
        is_load = true;
    }

    if (!is_loaded) {
        return;
    }

    ImGui::Begin("Image Click Example");

    // Display the image
    ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Top-left of the image
    ImGui::Image((ImTextureID)(intptr_t)img, ImVec2((float)img_w, (float)img_h));

    // Check for mouse click within the image bounds
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 relativePos = { mousePos.x - cursorPos.x, mousePos.y - cursorPos.y };

        // Normalize coordinates (if needed)
        // float normalizedX = relativePos.x / img_w;
        float normalizedX = (relativePos.x - (img_w / 2.f)) / img_h;
        float normalizedY = relativePos.y / img_h;

        printf("ImVec2(%ff, %ff),\n", normalizedX, normalizedY);
    }

    ImGui::End();
}

// Simple helper function to load an image into a OpenGL texture with common settings
static bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
static bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}