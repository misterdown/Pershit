#include "wirender.hpp"
#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <sgjk_head.hpp>
#include <ps_window.hpp>

using namespace wirender;

const static sgjk::linear::vec2 models[][9] = {
    {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.08f, 0.0f},
        {},
        {},
        {},
        {},
        {},
        {},
    },
    {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.07f, -0.025f},
        {-0.05f, 0.05f},
        {0.07f, 0.025f},
        {0.07f, -0.025f},
        {},
        {},
        {},
    },
    {
        {-0.1f, 0.1f},
        {-0.1f, -0.1f},
        {0.05f, -0.07f},
        {-0.1f, 0.1f},
        {0.05f, -0.07f},
        {0.05f, 0.07f},
        {0.05f, -0.07f},
        {0.05f, 0.07f},
        {0.15f, 0.0f},

    },
    {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.025f, -0.035f},
        {-0.05f, 0.05f},
        {0.025f, -0.035f},
        {0.025f, 0.035f},
        {0.025f, -0.035f},
        {0.025f, 0.035f},
        {0.1f, 0.0f},

    },
};
const static sgjk::polygon_collider_2d collidersForModels[] {
    sgjk::polygon_collider_2d( {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.08f, 0.0f},
    }),
    sgjk::polygon_collider_2d( {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.07f, 0.025f},
        {0.07f, -0.025f},
    }),
    sgjk::polygon_collider_2d( {
        {-0.1f, 0.1f},
        {-0.1f, -0.1f},
        {0.05f, -0.07f},
        {0.05f, 0.07f},
        {0.15f, 0.0f},
    }),
    sgjk::polygon_collider_2d( {
        {-0.05f, 0.05f},
        {-0.05f, -0.05f},
        {0.025f, -0.035f},
        {0.025f, 0.035f},
        {0.1f, 0.0f},
    }),
};

struct input_state {
    private:
    static constexpr uint32_t keysMax = UINT8_MAX + 1;
    bool prevPresseButtons[keysMax] {};
    bool currentPresseButtons[keysMax] {};

    public:
    void update() noexcept {
        for (uint32_t i = 0; i < keysMax; ++i)
            prevPresseButtons[i] = currentPresseButtons[i];
    }
    void key_up(uint32_t key) noexcept {
        currentPresseButtons[key] = false;
    }
    void key_down(uint32_t key) noexcept {
        currentPresseButtons[key] = true;
    }

    public:
    [[nodiscard]] bool is_pressed(uint32_t key) const {
        return currentPresseButtons[key];
    }
    [[nodiscard]] bool is_just_pressed(uint32_t key) const {
        return currentPresseButtons[key] && !prevPresseButtons[key];
    }
    [[nodiscard]] bool is_just_up(uint32_t key) const {
        return !currentPresseButtons[key] && prevPresseButtons[key];
    }
};

static input_state globalInput;
static sgjk::linear::vec2* globalVerts;
static ps_window::deafult_window globalWindow("---", ps_window::CREATE_WINDOW_FLAGS_BITS_MENU, 0, 0, 1200, 1200);

constexpr float FRICTION = 1.0f;
constexpr float PLAYER_SPEED = 3.0f;
constexpr float MARK_SPEED = 2.25f;
constexpr float MARK_DASH_STRENGTH = 2.25f;
constexpr float HENRY_SPEED = 4.5f;
constexpr float JOHN_DASH_STRENGTH = 4.5f;

void keyDownCallback(ps_window::deafult_window* window, int k) {
    globalInput.key_down(k);
}
void keyUpCallback(ps_window::deafult_window* window, int k) {
    globalInput.key_up(k);
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
/*uint32_t initialize_circle(sgjk::linear::vec2* verts, const uint32_t faceCount, const sgjk::linear::vec2& pos, float radius) {
    float a = 0.0f;
    const uint32_t triagleCount = faceCount - 2;
    const uint32_t vertsCount = triagleCount * 3;
    const float ratDelta = 6.2839f / (float)faceCount;
    verts[0] = sgjk::linear::vec2(cos(a), sin(a)) * radius + pos;
    a += ratDelta;
    for (uint32_t i = 0; i < vertsCount; i += 3) {
        verts[i] = verts[0];
        verts[i + 1] = sgjk::linear::vec2(cos(a), sin(a)) * radius + pos;
        a += ratDelta;
        verts[i + 2] = sgjk::linear::vec2(cos(a), sin(a)) * radius  + pos;
    }
    return vertsCount;
}*/
enum object_type {
    OBJECT_TYPE_PLAYER,
    OBJECT_TYPE_MARK,
    OBJECT_TYPE_HENRY,
    OBJECT_TYPE_JOHN,
};
struct object {
    object_type type;
    uint32_t modelID;
    uint32_t vertsCount;
    sgjk::linear::vec2* vertsPointer;
    sgjk::transform2d transform;
    sgjk::linear::vec2 velocity;
    sgjk::linear::vec2 penetration;
    double timer;
};

uint32_t push_mark(object* objects, uint32_t& objectCount, sgjk::linear::vec2* verts, const sgjk::linear::vec2& position) {
    const uint32_t vertsCount = sizeof(models[0]) / sizeof(*verts);
    for (uint32_t i = 0; i < vertsCount; ++i)
        verts[i] += position;
    objects[objectCount] = object{ OBJECT_TYPE_MARK, 1, vertsCount, verts, sgjk::transform2d(position, 0.0f), {}, {}, 0.0};
    ++objectCount;
    return vertsCount;
}
uint32_t push_henry(object* objects, uint32_t& objectCount, sgjk::linear::vec2* verts, const sgjk::linear::vec2& position) {
    const uint32_t vertsCount = sizeof(models[0]) / sizeof(*verts);
    for (uint32_t i = 0; i < vertsCount; ++i)
        verts[i] += position;
    objects[objectCount] = object{ OBJECT_TYPE_HENRY, 0, vertsCount, verts, sgjk::transform2d(position, 0.0f), {}, {}, 0.0};
    ++objectCount;
    return vertsCount;
}
uint32_t push_john(object* objects, uint32_t& objectCount, sgjk::linear::vec2* verts, const sgjk::linear::vec2& position) {
    const uint32_t vertsCount = sizeof(models[0]) / sizeof(*verts);
    for (uint32_t i = 0; i < vertsCount; ++i)
        verts[i] += position;
    objects[objectCount] = object{ OBJECT_TYPE_JOHN, 2, vertsCount, verts, sgjk::transform2d(position, 0.0f), {}, {}, 0.0};
    ++objectCount;
    return vertsCount;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    srand(time(0));

    //recompile shaders with:
    //  glslc -x glsl -fshader-stage=vert ./assets/shaders/vertexDefault.glsl -o ./assets/shaders/vertexDefault.spirv
    //  glslc -x glsl -fshader-stage=frag ./assets/shaders/fragmentDefault.glsl -o ./assets/shaders/fragmentDefault.spirv

    const auto vertCode = readBinary("./assets/shaders/vertexDefault.spirv");
    const auto fragCode = readBinary("./assets/shaders/fragmentDefault.spirv");
    const uint32_t maxVerteces = 1 << 16;

    render_manager m({globalWindow.get_handles().hwnd, globalWindow.get_handles().hInstance});
    shader_builder shaderBuilder =
        shader_builder(&m).
        //set_line_width(2.0f).
        //set_polygon_mode(VK_POLYGON_MODE_LINE).
        add_vertex_input_attribute(VkVertexInputAttributeDescription{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0}).
        add_stage(shader_stage{.codeSize = static_cast<uint32_t>(vertCode.size()), .code = (uint32_t*)vertCode.data(), .stage = VK_SHADER_STAGE_VERTEX_BIT}).
        add_stage(shader_stage{.codeSize = static_cast<uint32_t>(fragCode.size()), .code = (uint32_t*)fragCode.data(), .stage = VK_SHADER_STAGE_FRAGMENT_BIT}).
        pop_dynamic_state(). // pop scissor and viewport
        pop_dynamic_state();

    shader defaultShader = shaderBuilder.build();

    buffer_host_mapped_memory b(&m, buffer_create_info{.size = sizeof(sgjk::linear::vec2) * maxVerteces, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT });
    b.map_memory((void**)&globalVerts);

    MSG msg;
    double sTime = 0.0f;
    
    double maxDelta = 0;
    m.  bind_buffer(b.get_state()).
        start_record().

        set_shader(defaultShader.get_state()).
        record_start_render().
        record_draw_verteces(maxVerteces, 0, 1).
        record_end_render().

        end_record();

    long long ticks = 0;
    double delta = 0;

    uint32_t objectCount = 0;
    object objects[128];
    sgjk::linear::vec2* curVerts = globalVerts;
    curVerts = globalVerts;
    objects[objectCount] = { OBJECT_TYPE_PLAYER, 3, sizeof(models[0]) / sizeof(*curVerts), curVerts, {}, {}, {}, 0.0};
    curVerts += objects[0].vertsCount;
    ++objectCount;

    curVerts += push_mark(objects, objectCount, curVerts, {1.0f, 1.0f});
    curVerts += push_henry(objects, objectCount, curVerts, {1.0f, -1.0f});
    curVerts += push_mark(objects, objectCount, curVerts, {-1.0f, -1.0f});
    curVerts += push_john(objects, objectCount, curVerts, {-1.0f, 1.0f});

    object* player = nullptr;
    sgjk::linear::vec2* playerPositionForUniform = (sgjk::linear::vec2*)defaultShader.get_uniform_buffer_memory_on_binding(0);
    bool pause = false;

    globalWindow.userKeyDownCallback = keyDownCallback;
    globalWindow.userKeyUpCallback = keyUpCallback;
    globalWindow.show();

    while (globalWindow.is_open()) {
        auto start = std::chrono::steady_clock::now();
        globalInput.update();
        globalWindow.pool_events();

        if (!pause) {
            if (player != nullptr) {
                if ((abs(player->transform.get_position().x) > 1.0f) || (abs(player->transform.get_position().y) > 1.0f))
                    pause = true;
            }
            sgjk::collision_detecter_2d detecter;
            for (uint32_t oi = 0; oi < objectCount; ++oi)
                objects[oi].penetration = 0.0f;
            for (uint32_t oi = 0; oi < objectCount; ++oi) {
                for (uint32_t oii = oi + 1; oii < objectCount; ++oii) {
                    if (detecter.is_collide(
                        collidersForModels[objects[oi].modelID],
                        collidersForModels[objects[oii].modelID],
                        16, objects[oi].transform, objects[oii].transform)) {

                        const auto penetration = detecter.get_penetration_vector_unsafe(
                            collidersForModels[objects[oi].modelID],
                            collidersForModels[objects[oii].modelID],
                            1.0f / 1024.0f, 16, objects[oi].transform, objects[oii].transform);
                        objects[oi].penetration += -penetration;
                        objects[oii].penetration += penetration;
                    }
                }   
            }
            for (uint32_t oi = 0; oi < objectCount; ++oi) {
                if (objects[oi].type == OBJECT_TYPE_PLAYER) {
                    sgjk::linear::vec2* playerVerts = objects[oi].vertsPointer;
                    if (objects[oi].penetration != 0.0f) {
                        for (uint32_t d = 0; d < objects[oi].vertsCount; ++d) {
                            const float a =  ((((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f) * 6.2831f;
                            playerVerts[d] = sgjk::linear::vec2(cos(a), sin(a)) * 0.075f + objects[oi].transform.get_position();
                        }
                        pause = true;
                        break;
                    }

                    const sgjk::linear::vec2 inputDir = sgjk::linear::normalized(sgjk::linear::vec2(float((int)globalInput.is_pressed(68) - (int)globalInput.is_pressed(65)), float((int)globalInput.is_pressed(65 + 18) - (int)globalInput.is_pressed(65 + 22))));
                    objects[oi].velocity += inputDir * PLAYER_SPEED * (float)delta;
                    objects[oi].velocity /= 1.0f + ((float)delta);
                    *playerPositionForUniform = objects[oi].transform.get_position();

                    player = &objects[oi];
                } else if (objects[oi].type == OBJECT_TYPE_MARK) {
                    if (player == nullptr)
                        continue;
                    objects[oi].timer += delta;
                    const sgjk::linear::vec2 posDelta = player->transform.get_position() - objects[oi].transform.get_position();
                    
                    const sgjk::linear::vec2 direction = sgjk::linear::normalized(posDelta);
                    objects[oi].velocity += direction * MARK_SPEED * (float)delta;
                    objects[oi].velocity /= 1.0f + ((float)delta * FRICTION);
                    if ((objects[oi].timer > 0.5f) &&
                        (((float)rand() / (float)RAND_MAX) < (float)delta)) {
                        objects[oi].timer = 0;
                        objects[oi].velocity += direction * MARK_DASH_STRENGTH;
                    }
                } else if (objects[oi].type == OBJECT_TYPE_HENRY) {
                    if (player == nullptr)
                        continue;
                    const sgjk::linear::vec2 posDelta = player->transform.get_position() - objects[oi].transform.get_position();
                    
                    const sgjk::linear::vec2 direction = sgjk::linear::normalized(posDelta);
                    objects[oi].velocity += direction * (float)delta * HENRY_SPEED;
                    objects[oi].velocity /= 1.0f + ((float)delta * FRICTION);
                } else if (objects[oi].type == OBJECT_TYPE_JOHN) {
                     if (player == nullptr)
                        continue;
                    objects[oi].timer += delta;
                    const sgjk::linear::vec2 posDelta = player->transform.get_position() - objects[oi].transform.get_position();
                    
                    const sgjk::linear::vec2 direction = sgjk::linear::normalized(posDelta);
                    objects[oi].velocity /= 1.0f + ((float)delta * FRICTION);
                    if ((objects[oi].timer > 1.0f)) {
                        objects[oi].timer = 0;
                        objects[oi].velocity += direction * JOHN_DASH_STRENGTH;
                    }
                }
                const sgjk::linear::vec2 applied = objects[oi].velocity * (float)delta + objects[oi].penetration;
                sgjk::linear::vec2* verts = objects[oi].vertsPointer;

                objects[oi].transform.move(applied);
                objects[oi].transform.get_rotation() = atan2(objects[oi].velocity.y, objects[oi].velocity.x);
                for (uint32_t i = 0; i < objects[oi].vertsCount; ++i)
                    verts[i] = objects[oi].transform.transformed(models[objects[oi].modelID][i]);
            }
        } else {
            Sleep(100);
            objectCount = 0;
            curVerts = globalVerts;
            objects[objectCount] = { OBJECT_TYPE_PLAYER, 3, sizeof(models[0]) / sizeof(*curVerts), curVerts, {}, {}, {}, 0.0};
            curVerts += objects[0].vertsCount;
            ++objectCount;

            curVerts += push_mark(objects, objectCount, curVerts, {1.0f, 1.0f});
            curVerts += push_henry(objects, objectCount, curVerts,{1.0f, -1.0f});
            curVerts += push_mark(objects, objectCount, curVerts, {-1.0f, -1.0f});
            curVerts += push_john(objects, objectCount, curVerts, {-1.0f, 1.0f});

            curVerts += objects[objectCount].vertsCount;
            pause = false;
        }
        
        m.execute();
        Sleep(5);
        delta = (double)(std::chrono::steady_clock::now() - start).count() / (double)std::chrono::steady_clock::period::den;
        if (maxDelta < delta)
            maxDelta = delta;
        sTime += delta;
        if (sTime > 1) {
            char buff[64]{ 'f', 'p', 's', ':', ' ' , '\0'};

            itoa(ticks, buff + 5, 10);
            globalWindow.set_window_name_str(buff);
            //std::cout << "score: " << score << " max delta s: " << maxDelta << " real frame/second: " <<  ticks << "    \r";
            sTime = 0;
            ticks = 0;
            maxDelta = 0;
        }
        ++ticks;
    }
    b.unmap_memory();
    return 0;
}