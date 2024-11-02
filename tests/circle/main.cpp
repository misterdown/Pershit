#include "render.hpp"
#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <sgjk_head.hpp>

//using namespace render::platform; LOL
using namespace render;
static HWND w;
LRESULT WINAPI mysypurproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (Msg == WM_DESTROY)
        w = 0;
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

std::vector<unsigned char> readBinary(const std::string& path) {
    std::fstream file(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    assert(file.good());
    const size_t fileSize = file.seekg(0, std::ios_base::seekdir::_S_end).tellg();
    file.seekg(std::ios_base::seekdir::_S_beg);

    
    std::vector<unsigned char> result(fileSize);
    file.read((char*)result.data(), fileSize);

    return result;
}

int main() {
    srand(time(0));
    WNDCLASSA wc{};
    wc.lpfnWndProc = (WNDPROC)mysypurproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandleA(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName =  "MainMenu";
    wc.lpszClassName = "MainWindowClass";
    assert(RegisterClassA(&wc));
    w = CreateWindowA("MainWindowClass", "hame", WS_EX_OVERLAPPEDWINDOW | WS_SYSMENU /*| WS_SIZEBOX | WS_MINIMIZEBOX*/, CW_USEDEFAULT, CW_USEDEFAULT, 1600, 1600, 0, 0, 0, 0);


    const auto vertCode = readBinary("F:/programming/hame/assets/shaders/vertexDefault.spirv");
    const auto fragCode = readBinary("F:/programming/hame/assets/shaders/fragmentDefault.spirv");
    const uint32_t forCircle = 40;
    const uint32_t toDraw = forCircle * 3;

    render_manager m({w, GetModuleHandleA(0)});
    shader_builder shaderBuilder =
        shader_builder(&m).
        //set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST).
        //set_line_width(2.0f).
        //set_polygon_mode(VK_POLYGON_MODE_LINE).
        add_stage(shader_stage{.codeSize = static_cast<uint32_t>(vertCode.size()), .code = (uint32_t*)vertCode.data(), .stage = VK_SHADER_STAGE_VERTEX_BIT}).
        add_stage(shader_stage{.codeSize = static_cast<uint32_t>(fragCode.size()), .code = (uint32_t*)fragCode.data(), .stage = VK_SHADER_STAGE_FRAGMENT_BIT}).
        pop_dynamic_state().
        pop_dynamic_state();

    shader defaultShader =  shaderBuilder.build();

    buffer_host_mapped_memory b(&m, buffer_create_info{.size = sizeof(sgjk::linear::vec2) * toDraw, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT });
    sgjk::linear::vec2* verts;
    b.map_memory((void**)&verts);
    float a = -3.1415;
    for (uint32_t i = 0; i < toDraw; i += 3) {
        verts[i] = sgjk::linear::vec2(cos(a), sin(a)) * 0.9f;
        a += 6.2839f / (float)forCircle;
        verts[i + 1] = sgjk::linear::vec2(cos(a), sin(a)) * 0.9f;
        verts[i + 2] = 0.0f;
    }
    b.unmap_memory();

    MSG msg;
    double sTime = 0.0f;
    
    double maxDelta = 0;
    m.  resize().
        clear_command_list().
        bind_buffer(b.get_state()).
        start_record().

        set_shader(defaultShader.get_state()).
        record_start_render().
        record_draw_verteces(toDraw, 0, 1).
        record_end_render().

        end_record();

    long long ticks = 0;
    double delta = 0;
    ShowWindow(w, SW_NORMAL);
    while (true) {
        auto start = std::chrono::steady_clock::now();
        PeekMessageA(&msg, w, 0, 0, PM_REMOVE);
        DispatchMessageA(&msg);
        TranslateMessage(&msg);
        if (w == 0)
            break;
        
        m.execute();
        Sleep(1);
        
        delta = (double)(std::chrono::steady_clock::now() - start).count() / (double)std::chrono::steady_clock::period::den;
        if (maxDelta < delta)
            maxDelta = delta;
        sTime += delta;
        if (sTime > 1) {
            std::cout << "max delta s: " << maxDelta << " real frame/second: " <<  ticks << '\n';
            sTime = 0;
            ticks = 0;
            maxDelta = 0;
        }
        ++ticks;
    }
    std::cout << "success\n";
    return 0;
}