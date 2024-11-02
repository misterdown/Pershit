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
static bool keydown = false;
LRESULT WINAPI mysypurproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (Msg == WM_DESTROY)
        w = 0;
    if (Msg == WM_KEYDOWN)
        keydown = true;
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
    w = CreateWindowA("MainWindowClass", "hame", WS_EX_OVERLAPPEDWINDOW | WS_SYSMENU /*| WS_SIZEBOX | WS_MINIMIZEBOX*/, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0);


    const auto vertCode = readBinary("F:/programming/hame/assets/shaders/vertexDefault.spirv");
    const auto fragCode = readBinary("F:/programming/hame/assets/shaders/fragmentDefault.spirv");
    render_manager m({w, GetModuleHandleA(0)});
    shader s =  shader_builder(&m).
                add_stage(shader_stage{.codeSize = static_cast<uint32_t>(vertCode.size()), .code = (uint32_t*)vertCode.data(), .stage = VK_SHADER_STAGE_VERTEX_BIT}).
                add_stage(shader_stage{.codeSize = static_cast<uint32_t>(fragCode.size()), .code = (uint32_t*)fragCode.data(), .stage = VK_SHADER_STAGE_FRAGMENT_BIT}).
                pop_dynamic_state().
                pop_dynamic_state().
                build();
    buffer_host_mapped_memory b(&m, buffer_create_info{.size = sizeof(sgjk::linear::vec2) * 16, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT });
    sgjk::linear::vec2* verts;
    b.map_memory((void**)&verts);

    m.set_shader(s.get_state());
    m.bind_buffer(b.get_state());
    MSG msg;
    double sTime = 0.0f;
    
    double maxDelta = 0;
    m.  resize().
        clear_command_list().
        start_record().
        record_start_render().
        record_draw_verteces(9, 0, 1).
        record_end_render().
        end_record();

    for (uint32_t d = 0; d < 6; ++d) {
        verts[d].x = (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f;
        verts[d].y = (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f;
    }

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
        if (keydown) {
            for (uint32_t d = 0; d < 6; ++d) {
                verts[d].x = (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f;
                verts[d].y = (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f;
            }
        }
        keydown = false;
        for (uint32_t d = 0; d < 6; ++d) {
            verts[d].y += delta / 1.0f;
        }
        sgjk::polygon_collider_2d polygon1({verts, verts + 3});
        sgjk::polygon_collider_2d polygon2({verts + 3, verts + 6});
        const sgjk::polygon_collider_2d pol({{-2.0f, 1.0f}, {2.0f, 1.0f}, {0.5f, 2.0f}});
        sgjk::collision_detecter_2d detector;
        verts[6] = 0.0f;
        verts[7] = 0.0f;
        verts[8] = 0.0f;
        if (detector.is_collide(polygon1, polygon2)) {
           const auto p = detector.get_penetration_vector_unsafe(polygon1, polygon2, 0.00001f, 64);
           for (auto& i : polygon1.get_vertices())
               i -= p / 2.0f;
           for (auto& i : polygon2.get_vertices())
               i += p / 2.0f;
            verts[6] = polygon2.get_furthest_point(-p);
            verts[7] = polygon1.get_furthest_point(p);
            verts[8] = 0.0f;
        }
        if (detector.is_collide(polygon1, pol)) {
           const auto p = detector.get_penetration_vector_unsafe(polygon1, pol);
           for (auto& i : polygon1.get_vertices())
               i -= p;
        }
        if (detector.is_collide(polygon2, pol)) {
           const auto p = detector.get_penetration_vector_unsafe(polygon2, pol);
           for (auto& i : polygon2.get_vertices())
               i -= p;
        }
        verts[0] = polygon1.get_vertices()[0];
        verts[1] = polygon1.get_vertices()[1];
        verts[2] = polygon1.get_vertices()[2];
        verts[3] = polygon2.get_vertices()[0];
        verts[4] = polygon2.get_vertices()[1];
        verts[5] = polygon2.get_vertices()[2];
        
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
    b.unmap_memory();
    std::cout << "success\n";
    return 0;
}