/*
    Codes for getting draw data to svg
    Thanks, ChatGPT!
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <imgui.h>

#include "drawsvg.h"

// Helper function to convert ImU32 to an SVG-compatible color string
std::string ConvertColorToSVG(ImU32 col) {
    int r = (col >> IM_COL32_R_SHIFT) & 0xFF;
    int g = (col >> IM_COL32_G_SHIFT) & 0xFF;
    int b = (col >> IM_COL32_B_SHIFT) & 0xFF;
    float a = ((col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
    std::ostringstream oss;
    oss << "rgba(" << r << "," << g << "," << b << "," << a << ")";
    return oss.str();
}

void ExportDrawDataToSVG(const char* filename, ImDrawData* draw_data) {
    if (!draw_data) return;

    // Canvas size from ImGui's display size
    float canvas_width = draw_data->DisplaySize.x;
    float canvas_height = draw_data->DisplaySize.y;

    std::ofstream svg_file(filename);
    if (!svg_file.is_open()) {
        printf("Failed to open SVG file: %s\n", filename);
        return;
    }

    // Start SVG header with correct canvas size
    svg_file << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
    svg_file << "width=\"" << canvas_width << "\" height=\"" << canvas_height << "\">\n";

    // Iterate over ImGui draw commands
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* draw_list = draw_data->CmdLists[n];
        for (int cmd_idx = 0; cmd_idx < draw_list->CmdBuffer.size(); cmd_idx++) {
            const ImDrawCmd& cmd = draw_list->CmdBuffer[cmd_idx];
            if (cmd.ElemCount == 0) continue;

            // Clip rect
            const ImVec4& clip_rect = cmd.ClipRect;
            svg_file << "<clipPath id=\"clip" << n << "_" << cmd_idx << "\">\n";
            svg_file << "<rect x=\"" << clip_rect.x << "\" y=\"" << clip_rect.y << "\" "
                << "width=\"" << (clip_rect.z - clip_rect.x) << "\" height=\"" << (clip_rect.w - clip_rect.y) << "\" />\n";
            svg_file << "</clipPath>\n";

            // Draw triangles
            const ImDrawIdx* idx_buffer = draw_list->IdxBuffer.Data + cmd.IdxOffset;
            const ImDrawVert* vtx_buffer = draw_list->VtxBuffer.Data + cmd.VtxOffset;

            for (int i = 0; i < cmd.ElemCount; i += 3) {
                const ImDrawVert& v0 = vtx_buffer[idx_buffer[i]];
                const ImDrawVert& v1 = vtx_buffer[idx_buffer[i + 1]];
                const ImDrawVert& v2 = vtx_buffer[idx_buffer[i + 2]];

                svg_file << "<polygon points=\""
                    << v0.pos.x << "," << v0.pos.y << " "
                    << v1.pos.x << "," << v1.pos.y << " "
                    << v2.pos.x << "," << v2.pos.y << "\" "
                    << "fill=\"" << ConvertColorToSVG(v0.col) << "\" "
                    << "clip-path=\"url(#clip" << n << "_" << cmd_idx << ")\" />\n";
            }
        }
    }

    svg_file << "</svg>\n";
    svg_file.close();
}