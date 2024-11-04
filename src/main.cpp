#include "wirender.hpp"
#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <stack>
#include <optional>
#include <chrono>
#include <ps_window.hpp>
#include <sparse_vector.hpp>
#include "sgjk_glm.hpp"

// 3 triangles? enough.

#define MAX_MODEL_SIZE 9
#define MAX_MODEL_DRAW_COUNT 256
#define MAX_VERTEX_COUNT (MAX_MODEL_SIZE * MAX_MODEL_DRAW_COUNT)
#define PLAYER_SPEED 2.0f
#define DEFAULT_FRITCTION 1.0f
#define SPAWNED_OBJECTS_LIMIT 129
#define TICK_TIME_STEP 0.015625f
#define DRAW_TIME_STEP 0.0078125f

#define MODEL_ID_PLAYER 0
#define MODEL_PLAYER_VERT_COUNT 9

#define MODEL_ID_ARROW 3
#define MODEL_ARROW_VERT_COUNT 6

#define MODEL_ID_PLAYER_BULLET 2
#define MODEL_PLAYER_BULLET_VERT_COUNT 3

#define MODEL_ID_PLAYER_DEATH_WALL 7
#define MODEL_PLAYER_DEATH_WALL_VERT_COUNT 6

#define MODEL_ID_BASHI_ENEMY 1
#define MODEL_BASHI_ENEMY_VERT_COUNT 9

#define MODEL_ID_DASHI_ENEMY 6
#define MODEL_DASHI_ENEMY_VERT_COUNT 9

#define MODEL_ID_START_BUTTON 5
#define MODEL_START_BUTTON_VERT_COUNT 3

#define PERSHIT_FPI 3.141592654f

using namespace wirender;
using namespace ps_window;
using namespace sgjk;
using namespace glm;
using glm::sin;
using glm::cos;
using glm::abs;
using sv::sparse_vector;
using std::stack;
using std::vector;
using ps_window::key_codes;

// probably, we has to написать поясняющие комментарии

// HARDCODE BABYYY
// Я писал эту ХУЙНЮ 30 минут, тестируя каждый пердёж

const static vec2 tinyFontModels[][MAX_MODEL_SIZE] = {
    { // zero
        { -0.5f, -1.0f },
        { 0.45f, -1.0f },
        { -0.5f, 0.9f },
        { 0.5f, 1.0f },
        { -0.45f, 1.0f },
        { 0.5f, -0.9f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
    },
    { // one
        { -0.25f, -1.0f },
        { 0.5f, -1.0f },
        { -0.25f, 1.0f },
        { 0.5f, 1.0f },
        { -0.25f, 1.0f },
        { 0.5f, -1.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
    },
    { // two
        { 0.5f, 1.0f },
        { -0.5f, 1.0f },
        { -0.5f, 0.5f },
        { 0.5f, -1.0f },
        { -0.5f, -1.0f },
        { 0.5f, -0.5f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
    },
    { // three
        { 0.5f, 1.0f },
        { -0.5f, 1.0f },
        { 0.5f, 0.5f },
        { 0.5f, 0.3f },
        { -0.5f, 0.05f },
        { 0.5f, -0.2f },
        { 0.5f, -0.5f },
        { -0.5f, -1.0f },
        { 0.5f, -1.0f },
    },
    { // four
        { -0.5f, -1.0f },
        { 0.0f, -0.55f },
        { -0.5f, -0.05f },
        { 0.5f, 0.0f },
        { -0.45f, 0.0f },
        { 0.5f, -1.0f },
        { 0.5f, 0.0f },
        { 0.0f, 1.0f },
        { 0.5f, 1.0f },
    },
    { // five
        { -0.5f, 0.0f },
        { 0.5f, 0.0f },
        { 0.5f, 0.9f },
        { -0.5f, 1.0f },
        { 0.0f, 0.55f },
        { 0.5f, 1.0f },
        { -0.5f, 0.0f },
        { 0.5f, -1.0f },
        { -0.5f, -1.0f },
    },
    { // six
        { -0.5f, 0.0f },
        { 0.45f, 0.0f },
        { -0.5f, 0.95f },
        { 0.5f, 1.0f },
        { -0.45f, 1.0f },
        { 0.5f, 0.05f },
        { -0.5f, 0.0f },
        { 0.5f, -1.0f },
        { -0.5f, -1.0f },
    },
    { // seven
        { 0.0f, -1.0f },
        { 0.5f, -1.0f },
        { 0.0f, 1.0f },
        { 0.5f, 1.0f },
        { 0.0f, 1.0f },
        { 0.5f, -1.0f },
        { -0.05f, -0.5f },
        { -0.5f, -1.0f },
        { -0.05f, -1.0f },
    },
    { // eight
        { -0.5f, 0.0f },
        { 0.45f, 0.0f },
        { -0.5f, 0.95f },
        { 0.5f, 1.0f },
        { -0.45f, 1.0f },
        { 0.5f, 0.05f },
        { 0.5f, -0.05f },
        { -0.5f, -0.05f },
        { -0.0f, -1.0f },
    },
    { // nine
        { -0.5f, 0.0f },
        { 0.45f, 0.0f },
        { -0.5f, -0.95f },
        { 0.5f, -1.0f },
        { -0.45f, -1.0f },
        { 0.5f, -0.05f },
        { -0.5f, -0.0f },
        { 0.5f, 1.0f },
        { -0.5f, 1.0f },
    },
};

// baked objectModels
const static vec2 objectModels[][MAX_MODEL_SIZE] = {
    {
        { -0.05f, 0.05f },
        { -0.05f, -0.05f },
        { 0.025f, -0.035f },
        { -0.05f, 0.05f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.1f, 0.0f },

    },
    {
        { -0.04f, 0.03f },
        { -0.04f, -0.03f },
        { 0.025f, -0.035f },
        { -0.04f, 0.03f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.1f, 0.0f },

    },
    {
        { -0.05f, 0.025f },
        { -0.05f, -0.025f },
        { 0.075f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },

    },
    {
        { 0.1f, -0.05f },
        { 0.15f, 0.0f },
        { 0.6f, 0.0f },
        { 0.1f, 0.05f },
        { 0.15f, 0.0f },
        { 0.6f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },

    },
    {
        { -0.05f, 0.12f },
        { -0.05f, -0.12f },
        { 0.025f, -0.1f },
        { -0.05f, 0.12f },
        { 0.025f, -0.1f },
        { 0.025f, 0.1f },
        { 0.025f, -0.1f },
        { 0.025f, 0.1f },
        { 0.125f, 0.0f },

    },
    {
        { -0.5f, 0.5f },
        { -0.5f, -0.5f },
        { 0.5f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },

    },
    {
        { -0.04f, 0.03f },
        { -0.04f, -0.03f },
        { 0.03f, -0.035f },
        { -0.04f, 0.03f },
        { 0.03f, -0.035f },
        { 0.03f, 0.035f },
        { 0.03f, -0.035f },
        { 0.03f, 0.035f },
        { 0.15f, 0.0f },
    },
    {
        { -0.04f, 0.7f },
        { -0.04f, -0.7f },
        { 0.04f, -0.7f },
        { -0.04f, 0.7f },
        { 0.04f, 0.7f },
        { 0.04f, -0.7f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
    },
};
// baked colliders for the objectModels
const static polygon_collider_2d collidersForModels[] {
    polygon_collider_2d( {
        { -0.05f, 0.05f },
        { -0.05f, -0.05f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.1f, 0.0f },
    }),
    polygon_collider_2d( {
        { -0.04f, 0.03f },
        { -0.04f, -0.03f },
        { 0.025f, -0.035f },
        { 0.025f, 0.035f },
        { 0.1f, 0.0f },
    }),
    polygon_collider_2d( {
        { -0.05f, 0.025f },
        { -0.05f, -0.025f },
        { 0.075f, 0.0f },
    }),
    polygon_collider_2d(),
    polygon_collider_2d( {
        { -0.05f, 0.12f },
        { -0.05f, -0.12f },
        { 0.025f, -0.1f },
        { 0.025f, 0.1f },
        { 0.125f, 0.0f },
    }),
    polygon_collider_2d( {
        { -0.5f, 0.5f },
        { -0.5f, -0.5f },
        { 0.5f, 0.0f },
    }),
    polygon_collider_2d( {
        { -0.04f, 0.03f },
        { -0.04f, -0.03f },
        { 0.03f, -0.035f },
        { 0.03f, 0.035f },
        { 0.15f, 0.0f },
    }),
    polygon_collider_2d( {
        { -0.04f, 0.7f },
        { -0.04f, -0.7f },
        { 0.04f, -0.7f },
        { 0.04f, 0.7f },
    }),
};


// very simple implementation of (virtual)input handler 
struct input_state {
    private:
    static constexpr uint32_t keysMax = UINT8_MAX + 1;
    bool prevPresseButtons[keysMax] {};
    bool currentPresseButtons[keysMax] {};
    bool rmbButton[2] {};
    bool lmbButton[2] {};

    public:
    void update() noexcept {
        for (uint32_t i = 0; i < keysMax; ++i)
            prevPresseButtons[i] = currentPresseButtons[i];
        rmbButton[0] = rmbButton[1];
        lmbButton[0] = lmbButton[1];
    }
    void key_up(uint32_t key) noexcept {
        currentPresseButtons[key] = false;
    }
    void key_down(uint32_t key) noexcept {
        currentPresseButtons[key] = true;
    }
    void rigth_mb_down() {
        rmbButton[1] = true;
    }
    void left_mb_down() {
        lmbButton[1] = true;
    }
    void rigth_mb_up() {
        rmbButton[1] = false;
    }
    void left_mb_up() {
        lmbButton[1] = false;
    }

    public:
    [[nodiscard]] bool is_pressed(uint32_t key) const {
        return currentPresseButtons[key];
    }
    [[nodiscard]] bool is_just_pressed(uint32_t key) const {
        return currentPresseButtons[key] && !prevPresseButtons[key];
    }
    [[nodiscard]] bool is_just_released(uint32_t key) const {
        return !currentPresseButtons[key] && prevPresseButtons[key];
    }
    [[nodiscard]] bool is_lmb_pressed() const {
        return lmbButton[1];
    }
    [[nodiscard]] bool is_lmb_just_pressed() const {
        return lmbButton[1] && !lmbButton[0];
    }
    [[nodiscard]] bool is_lmb_just_released() const {
        return !lmbButton[1] && lmbButton[0];
    }
    [[nodiscard]] bool is_rmb_pressed() const {
        return rmbButton[1];
    }
    [[nodiscard]] bool is_rmb_just_pressed() const {
        return rmbButton[1] && !rmbButton[0];
    }
    [[nodiscard]] bool is_rmb_just_released() const {
        return !rmbButton[1] && rmbButton[0];
    }

};

vector<unsigned char> readBinary(const std::string& path) {
    std::fstream file(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    assert(file.good());
    const size_t fileSize = file.seekg(0, std::ios_base::seekdir::_S_end).tellg();
    file.seekg(std::ios_base::seekdir::_S_beg);

    vector<unsigned char> result(fileSize);
    file.read((char*)result.data(), fileSize);

    return result;
}
/*uint32_t initialize_circle(vec2* verts, const uint32_t faceCount, const vec2& pos, float radius) {
    float a = 0.0f;
    const uint32_t triagleCount = faceCount - 2;
    const uint32_t vertsCount = triagleCount * 3;
    const float ratDelta = 6.2839f / (float)faceCount;
    verts[0] = vec2(cos(a), sin(a)) * radius + pos;
    a += ratDelta;
    for (uint32_t i = 0; i < vertsCount; i += 3) {
        verts[i] = verts[0];
        verts[i + 1] = vec2(cos(a), sin(a)) * radius + pos;
        a += ratDelta;
        verts[i + 2] = vec2(cos(a), sin(a)) * radius  + pos;
    }
    return vertsCount;
}*/
enum object_type {
    OBJECT_TYPE_MENU_BUTTON_START,
    OBJECT_TYPE_PLAYER,
    OBJECT_TYPE_ENEMY_SPAWNER,
    OBJECT_TYPE_PLAYER_BULLET,
    OBJECT_TYPE_PLAYER_DEATH_WALL,
    OBJECT_TYPE_BASHI_ENEMY,
    OBJECT_TYPE_DASHI_ENEMY,
};
uint32_t get_model_id(const object_type type1) {
    switch (type1) {
        case OBJECT_TYPE_PLAYER:            return MODEL_ID_PLAYER;
        case OBJECT_TYPE_PLAYER_BULLET:     return MODEL_ID_PLAYER_BULLET;
        case OBJECT_TYPE_PLAYER_DEATH_WALL: return MODEL_ID_PLAYER_DEATH_WALL;
        case OBJECT_TYPE_BASHI_ENEMY:       return MODEL_ID_BASHI_ENEMY;
        case OBJECT_TYPE_DASHI_ENEMY:       return MODEL_ID_DASHI_ENEMY;
        case OBJECT_TYPE_MENU_BUTTON_START: return MODEL_ID_START_BUTTON;
        default:                            return ~0u;
    }
}
uint32_t get_model_verteces_count(const object_type type1) {
    switch (type1) {
        case OBJECT_TYPE_PLAYER:            return MODEL_PLAYER_VERT_COUNT;
        case OBJECT_TYPE_PLAYER_BULLET:     return MODEL_PLAYER_BULLET_VERT_COUNT;
        case OBJECT_TYPE_PLAYER_DEATH_WALL: return MODEL_PLAYER_DEATH_WALL_VERT_COUNT;
        case OBJECT_TYPE_BASHI_ENEMY:       return MODEL_BASHI_ENEMY_VERT_COUNT;
        case OBJECT_TYPE_DASHI_ENEMY:       return MODEL_DASHI_ENEMY_VERT_COUNT;
        case OBJECT_TYPE_MENU_BUTTON_START: return MODEL_START_BUTTON_VERT_COUNT;
        default:                            return 0;
    }
}
struct object {
    size_t index;
    object_type type;
    vec2* vertsPointer;
    uint32_t vertecesInUse;
    transform2d transform;
    vec2 velocity;
    vec2 penetration;
    float timer1;
    float timerDur1;

    public:
    object() {

    }
    object(object_type type, const transform2d& transform =  transform2d(), const vec2& velocity = vec2()) :
        index(0),
        type(type),
        vertecesInUse(0),
        transform(transform),
        velocity(velocity),
        penetration(0.0f),
        timer1(0.0f),
        timerDur1(0.0f) {

    }
};

float radians_differens(const float radian1, const float radian2) {
    const float diff = fmod(radian1 - radian2, 2.0f * PERSHIT_FPI);
    if (diff < -PERSHIT_FPI) {
        return diff + (2.0f * PERSHIT_FPI);
    } else if (diff > PERSHIT_FPI) {
        return diff - (2.0f * PERSHIT_FPI);
    }
    return diff;
}

struct Game {
    private:
    enum long_player_action {
        PLAYER_ACTION_WAIT,
    };
    struct player_action_state {
        long_player_action actionType;
        vec2 arrowDirection;
        float arrowRadians;
    };
    enum scene_type {
        SCENE_TYPE_MENU,
        SCENE_TYPE_BATTLE,
    };
    struct camera_info {
        vec2 position;
        vec2 scale;
        camera_info() : position(), scale(1, 1) {

        }
        camera_info(const vec2& position, const vec2& scale) : position(position), scale(scale) {

        }
    };
    struct scene_state {
        scene_type type;
        player_action_state playerActionState;
        sparse_vector<object> gameObjects;
        camera_info cameraInfo;
        float turnDuration;
        float totalInTurnElapsed;
    };
    

    private:
    deafult_window window;
    input_state input;
    render_manager render;
    vector<unsigned char> objectsVertexCode;
    vector<unsigned char> objectsFragmentCode;
    vector<unsigned char> cellBoardVertexCode;
    vector<unsigned char> cellBoardFragmentCode;
    shader objectsShader;
    shader cellBoardShader;
    buffer_host_mapped_memory objectsBuffer;
    buffer_host_mapped_memory cellBoardBuffer;
    vec2* bufferVerteces;
    uint32_t bufferVertexCount;
    scene_state sceneState;

    public:
    Game() :
        window("game", CREATE_WINDOW_FLAGS_BITS_MENU, 0, 0, 1200, 1200),
        render({window.get_handles().hwnd, window.get_handles().hInstance}, true),
        objectsVertexCode(readBinary("./assets/shaders/vertexDefault.spirv")),
        objectsFragmentCode(readBinary("./assets/shaders/fragmentDefault.spirv")),
        cellBoardVertexCode(readBinary("./assets/shaders/vertexCellBoard.spirv")),
        cellBoardFragmentCode(readBinary("./assets/shaders/fragmentCellBoard.spirv")),
        objectsShader(
            shader_builder(&render).
            //set_line_width(2.0f).
            //set_polygon_mode(VK_POLYGON_MODE_LINE).
            add_vertex_input_attribute(VkVertexInputAttributeDescription{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0}).
            add_stage(shader_stage{.codeSize = static_cast<uint32_t>(objectsVertexCode.size()), .code = (uint32_t*)objectsVertexCode.data(), .stage = VK_SHADER_STAGE_VERTEX_BIT}).
            add_stage(shader_stage{.codeSize = static_cast<uint32_t>(objectsFragmentCode.size()), .code = (uint32_t*)objectsFragmentCode.data(), .stage = VK_SHADER_STAGE_FRAGMENT_BIT}).
            set_clear_screen(false).
            pop_dynamic_state(). // pop scissor and viewport
            pop_dynamic_state().
            build()),
        cellBoardShader(
            shader_builder(&render).
            //set_line_width(2.0f).
            //set_polygon_mode(VK_POLYGON_MODE_LINE).
            add_vertex_input_attribute(VkVertexInputAttributeDescription{.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0}).
            add_stage(shader_stage{.codeSize = static_cast<uint32_t>(cellBoardVertexCode.size()), .code = (uint32_t*)cellBoardVertexCode.data(), .stage = VK_SHADER_STAGE_VERTEX_BIT}).
            add_stage(shader_stage{.codeSize = static_cast<uint32_t>(cellBoardFragmentCode.size()), .code = (uint32_t*)cellBoardFragmentCode.data(), .stage = VK_SHADER_STAGE_FRAGMENT_BIT}).
            set_clear_screen(true).
            pop_dynamic_state(). // pop scissor and viewport
            pop_dynamic_state().
            build()),
        objectsBuffer(&render, buffer_create_info{.size = sizeof(vec2) * MAX_VERTEX_COUNT, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT }),
        cellBoardBuffer(&render, buffer_create_info{.size = sizeof(vec2) * 6, .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT }),
        bufferVerteces(reinterpret_cast<vec2*>(objectsBuffer.map_memory())),
        bufferVertexCount(0),
        sceneState{} {

        window.userPointer = this;
        window.userKeyDownCallback = &keyDownCallback;
        window.userKeyUpCallback = &keyUpCallback;
        window.userLmbDownCallback = &lmbDown;
        window.userLmbUpCallback = &lmbUp;

        render. start_record().

                set_shader(cellBoardShader.get_state()).
                bind_buffer(cellBoardBuffer.get_state()).
                record_start_render().
                record_draw_verteces(6, 0, 1).
                record_end_render().

                bind_buffer(objectsBuffer.get_state()).
                set_shader(objectsShader.get_state()).
                record_start_render().
                record_draw_verteces(MAX_VERTEX_COUNT, 0, 1).
                record_end_render().

                end_record();
        objectsVertexCode.clear();
        objectsVertexCode.shrink_to_fit();
        objectsFragmentCode.clear();
        objectsFragmentCode.shrink_to_fit();

        cellBoardVertexCode.clear();
        cellBoardVertexCode.shrink_to_fit();
        cellBoardFragmentCode.clear();
        cellBoardFragmentCode.shrink_to_fit();

        {
            vec2* mappedCellBoard = reinterpret_cast<vec2*>(cellBoardBuffer.map_memory());
            mappedCellBoard[0] = vec2(-1.0f, -1.0f);
            mappedCellBoard[1] = vec2(1.0f, -1.0f);
            mappedCellBoard[2] = vec2(1.0f, 1.0f);
            mappedCellBoard[3] = vec2(1.0f, 1.0f);
            mappedCellBoard[4] = vec2(-1.0f, 1.0f);
            mappedCellBoard[5] = vec2(-1.0f, -1.0f);
            cellBoardBuffer.unmap_memory();
        }

    }
    ~Game() {
        objectsBuffer.unmap_memory();
    }

    public:
    void clear_scene() {
        sceneState.gameObjects.clear();
        sceneState.playerActionState = player_action_state{};
        sceneState.cameraInfo = camera_info();
        sceneState.turnDuration = 0.0f;
        sceneState.totalInTurnElapsed = 0.0f;
    }
    void initialize_scene_state_battle() {
        sceneState.type = SCENE_TYPE_BATTLE;
        sceneState.cameraInfo = camera_info();
        add_object(object(OBJECT_TYPE_PLAYER));

        {
            object spawner(OBJECT_TYPE_ENEMY_SPAWNER);
            spawner.timerDur1 = 1.0f;
            add_object(std::move(spawner));
        }
    }
    void initialize_scene_state_menu() {
        sceneState.type = SCENE_TYPE_MENU;
        sceneState.cameraInfo = camera_info();
        add_object(object(OBJECT_TYPE_MENU_BUTTON_START));
    }
    void run() {
        window.show();
        auto tickTimerStart = std::chrono::steady_clock::now();
        auto drawTimerStart = std::chrono::steady_clock::now();

        auto sStart = std::chrono::steady_clock::now();

        initialize_scene_state_menu();

        double max = 0.0f;
        while (window.is_open()) {
            const auto fStart = std::chrono::steady_clock::now();

            const double tickElapsed = (double)(std::chrono::steady_clock::now() - tickTimerStart).count() / (double)std::chrono::steady_clock::period::den;
            const double drawElapsed = (double)(std::chrono::steady_clock::now() - drawTimerStart).count() / (double)std::chrono::steady_clock::period::den;
            const double sElapsed = (double)(std::chrono::steady_clock::now() - sStart).count() / (double)std::chrono::steady_clock::period::den;
            if (tickElapsed > TICK_TIME_STEP) {
                tickTimerStart = std::chrono::steady_clock::now();
                input.update();
                window.pool_events();
                
                tick(TICK_TIME_STEP);
                
            }
            if (drawElapsed > DRAW_TIME_STEP) {
                drawTimerStart = std::chrono::steady_clock::now();
                draw_objects(DRAW_TIME_STEP);
                render.execute();
            }
            if (sElapsed > 1.0) {
                sStart = std::chrono::steady_clock::now();
                // std::cout << "worst sdelta: " << max << " / worst fps: " << 1.0 / max << "\n";
                max = 0.0;
            }
            const double delta = (double)((std::chrono::steady_clock::now() - fStart).count() / (double)std::chrono::steady_clock::period::den);
            if (max < delta)
                max = delta;
        }
    }

    private:
    void tick(float ts) {
        if (sceneState.type == SCENE_TYPE_BATTLE) {
            float& turnDuration = sceneState.turnDuration;
            float& totalInTurnElapsed = sceneState.totalInTurnElapsed;
            if (turnDuration <= 0.0) {
                turnDuration = try_wait_player_turn();
            } else {
                proccess_actions(ts);
                turnDuration -= ts;
                totalInTurnElapsed += ts;
            }
        } else if (sceneState.type == SCENE_TYPE_MENU) {
            const auto& gameObjects = sceneState.gameObjects;
            if (input.is_lmb_just_pressed()) {
                const transform2d clickTransform(get_cursor_position_in_game_world(), 0.0f, vec2(1.0f / 50.0f));
                collision_detecter_2d detecter;

                for (const auto& i : gameObjects) {
                    if (i.type == OBJECT_TYPE_MENU_BUTTON_START) {
                        if (detecter.is_collide(collidersForModels[MODEL_ID_START_BUTTON], collidersForModels[MODEL_ID_START_BUTTON], 16, clickTransform, i.transform)) {
                            clear_scene();
                            initialize_scene_state_battle();
                        }

                    }
                }
            }
        }
    }
    // draw objects without interpolation, but still need a time step for camera moves
    void draw_objects(float ts) {
        clear_bacth();

        camera_info& cameraInfo = sceneState.cameraInfo;

        (*reinterpret_cast<camera_info*>(objectsShader.get_uniform_buffer_memory_on_binding(0))) = cameraInfo;
        (*reinterpret_cast<camera_info*>(cellBoardShader.get_uniform_buffer_memory_on_binding(0))) = cameraInfo;
        (*reinterpret_cast<vec2*>(objectsShader.get_uniform_buffer_memory_on_binding(1))) = get_cursor_position_in_game_world();

        const auto& gameObjects = sceneState.gameObjects;

        if (sceneState.type == SCENE_TYPE_BATTLE) {
            cameraInfo.scale -= ts * (float)window.get_mouse_wheel_scroll_delta(); 
            cameraInfo.scale = clamp(cameraInfo.scale, vec2(0.5f), vec2(2.0f));
            print_number_in_world((int)(sceneState.totalInTurnElapsed * 1000), vec2(-0.95f, -0.85f) * cameraInfo.scale + cameraInfo.position, vec2(0.05f) * cameraInfo.scale);
        }

        for (const auto& i : gameObjects) {
            const transform2d& currentTransform = i.transform;
            const object_type objType = i.type;

            if (objType == OBJECT_TYPE_PLAYER) {
                const vec2 posDiff = currentTransform.get_position() - cameraInfo.position;
                const float distance = length(posDiff);
                if (distance > 0.1414) {
                    if (distance < 1.0) {
                        cameraInfo.position += normalized(posDiff) * ts * 0.5f;
                    } else {
                        cameraInfo.position += normalized(posDiff) * distance * ts * 0.5f;
                    }
                }

                if ((sceneState.type == SCENE_TYPE_BATTLE) && (sceneState.turnDuration <= 0.0f)) {
                    const player_action_state& playerActionState = sceneState.playerActionState;
                    transform2d arrowTransform(currentTransform.get_position(), playerActionState.arrowRadians);
                    put_on_batch(MODEL_ID_ARROW, MODEL_ARROW_VERT_COUNT, arrowTransform);
                }
            }
            put_on_batch(get_model_id(objType), get_model_verteces_count(objType), currentTransform);
        }


    }
    vec2 murmuration(const object& currentObject) const {
        constexpr float neighborDistance = 0.2f;

        vec2 alignment(0.0f);
        vec2 cohesion(0.0f);
        vec2 separation(0.0f);

        auto& gameObjects = sceneState.gameObjects;

        int bashiCount = 0;
        for (const auto& i : gameObjects) {
            if ((i.type != OBJECT_TYPE_BASHI_ENEMY) || (&i == &currentObject))
                continue;
            const vec2 diff = currentObject.transform.get_position() - i.transform.get_position();
            if (length(diff) > neighborDistance)
                continue;
            alignment += i.velocity;
            cohesion += i.transform.get_position();
            separation += diff;
            ++bashiCount;
        }
        if (bashiCount == 0)
            return vec2(0.0f);

        alignment /= (float)bashiCount;
        cohesion /= (float)bashiCount;
        separation /= (float)bashiCount * 8.0f;

        alignment = (alignment - currentObject.velocity) / 8.0f;
        cohesion = (cohesion - currentObject.transform.get_position()) / 25.0f;

        return alignment + cohesion + separation;
    }
    // returns the time in seconds required for the players action
    float try_wait_player_turn() {
        /*
            Медленно, нечитаемо, контринтуитивно - просто ужасно
            Но пойдёт
        */
        auto& gameObjects = sceneState.gameObjects;
        player_action_state& playerActionState = sceneState.playerActionState;

        for (auto& i : gameObjects) {
            if (i.type == OBJECT_TYPE_PLAYER) {
                transform2d& playerTrasform = i.transform;

                const vec2 cursorInWorld = get_cursor_position_in_game_world();
                const vec2 cursorRelPlayer = cursorInWorld - playerTrasform.get_position();
                const vec2 cursorRelPlayerDirection = normalized(cursorRelPlayer);
                const float arrowRadians = atan2(cursorRelPlayer.y, cursorRelPlayer.x);

                playerActionState.arrowDirection = cursorRelPlayerDirection;
                playerActionState.arrowRadians = arrowRadians;
                
                if (input.is_just_pressed(KEY_CODES_SPACE)) {
                    return 0.5f;

                } else if (input.is_just_pressed(KEY_CODES_1)) {
                    playerActionState.actionType = PLAYER_ACTION_WAIT;
                    playerTrasform.get_rotation() = arrowRadians;
                    i.velocity = cursorRelPlayerDirection * 1.75f;
                    return 0.75f;

                } else if (input.is_just_pressed(KEY_CODES_2)) {
                    playerTrasform.get_rotation() = arrowRadians;

                    playerActionState.actionType = PLAYER_ACTION_WAIT;
                    transform2d bulletTransform = playerTrasform;
                    bulletTransform.get_rotation() = arrowRadians;

                    const vec2 recoil = cursorRelPlayerDirection * 0.25f;
                    i.velocity -= recoil;

                    add_object(object(OBJECT_TYPE_PLAYER_BULLET, std::move(bulletTransform), cursorRelPlayerDirection * 2.0f)); // костыли не нужны, если объекты будут перераспределены, то это, тут конкретно, ни на что не повлияет 
                    return 0.5f;

                } else if (input.is_just_pressed(KEY_CODES_3)) {
                    playerTrasform.get_rotation() = arrowRadians;

                    playerActionState.actionType = PLAYER_ACTION_WAIT;
                    transform2d bulletTransform = playerTrasform;
                    bulletTransform.get_rotation() = arrowRadians;

                    const vec2 recoil = cursorRelPlayerDirection * 1.25f;
                    i.velocity -= recoil;

                    add_object(object(OBJECT_TYPE_PLAYER_DEATH_WALL, std::move(bulletTransform), cursorRelPlayerDirection * 2.0f)); // костыли не нужны, если объекты будут перераспределены, то это, тут конкретно, ни на что не повлияет 
                    return 1.25f;

                } else if (input.is_just_pressed(KEY_CODES_4)) {
                    playerActionState.actionType = PLAYER_ACTION_WAIT;
                    playerTrasform.get_position() = cursorInWorld;
                    return length(cursorRelPlayer) * 0.5f + 0.5f;

                }
                return 0.0f;
            }
        }
        return 0.0f;
    }
    // update every object
    void proccess_actions(float ts) {
        auto& gameObjects = sceneState.gameObjects;

        {
            collision_detecter_2d detector;
            for (size_t i = 0; i < gameObjects.size(); ++i) {
                if (!gameObjects.exist_at(i))
                    continue;
                for (size_t j = i + 1; j < gameObjects.size(); ++j) {
                    if ((!gameObjects.exist_at(i)) || (!gameObjects.exist_at(j)))
                        continue;
                        
                    collide_check(gameObjects[i], gameObjects[j]);

                }
            }   
        }

        size_t playerIndex = ~0ull;
        for (auto& i : gameObjects) {
            if (i.type == OBJECT_TYPE_PLAYER) {
                playerIndex = i.index;
                break;
            }
        }

        if (playerIndex == ~0ull) {
            clear_scene();
            initialize_scene_state_menu();
            return;
        }

        player_action_state& playerActionState = sceneState.playerActionState;
        // БЕЗОПАСНО так делать, понял, да?
        for (size_t index = 0; index < gameObjects.size(); ++index) {
            if (!gameObjects.exist_at(index))
                continue;
            object* i = &gameObjects[index];

            transform2d& currentTransform = i->transform;

            if (i->type == OBJECT_TYPE_PLAYER) {
                i->velocity /= 1.0f + ts * 2.0f;

            } else if (i->type == OBJECT_TYPE_ENEMY_SPAWNER) {
                if ((gameObjects.size() - gameObjects.get_free_cells().size()) > SPAWNED_OBJECTS_LIMIT)
                    continue;

                while (i->timer1 > i->timerDur1) {
                    const float radians = ((float)rand() / (float)RAND_MAX) * (PERSHIT_FPI * 2.0f);
                    const vec2 position = vec2(cos(radians), sin(radians)) + gameObjects[playerIndex].transform.get_position();

                    int randResult = rand() % 3;
                    if (randResult == 0) {
                        object dashi(OBJECT_TYPE_DASHI_ENEMY, transform2d(position, 0.0f));
                        add_object(std::move(dashi));       // ДАЖЕ если вектор перераспределится, то благодаря
                        i = &gameObjects[index];            // этой строке, указатель на объект останется валидным
                    } else {
                        object bashi(OBJECT_TYPE_BASHI_ENEMY, transform2d(position, 0.0f));
                        bashi.velocity = normalized(position - gameObjects[playerIndex].transform.get_position());
                        add_object(std::move(bashi));
                        i = &gameObjects[index];            // указатель на объект останется валидным
                    }
                    i->timer1 -= i->timerDur1;
                    i->timerDur1 *= 0.95f;
                    i->timerDur1 = i->timerDur1 < 0.2f ? 0.2f : i->timerDur1;
                }
                i->timer1 += ts;

            } else if (i->type == OBJECT_TYPE_PLAYER_BULLET) {
                if (i->timer1 > 2.0f) {
                    destroy_object(*i);
                    continue;
                }
                i->velocity /= 1.0f + ts;
                i->timer1 += ts;

            } else if (i->type == OBJECT_TYPE_PLAYER_DEATH_WALL) {
                if (i->timer1 > 2.0f) {
                    destroy_object(*i);
                    continue;
                }
                i->timer1 += ts;

            } else if (i->type == OBJECT_TYPE_BASHI_ENEMY) {
                const vec2 murmur = murmuration(*i); // nya
                
                const vec2 direction = 
                (
                    normalized(gameObjects[playerIndex].transform.get_position() - currentTransform.get_position()) * 2.0f +
                    (length(murmur) == 0.0f ? vec2(0.0f) : normalized(murmur))
                ) * 0.33f;
                i->velocity += direction * 1.75f * ts;
                currentTransform.get_rotation() = atan2(i->velocity.y, i->velocity.x);
                i->velocity /= 1.0f + ts;
                i->timer1 += ts;

            } else if (i->type == OBJECT_TYPE_DASHI_ENEMY) {
                const vec2 direction = normalized(gameObjects[playerIndex].transform.get_position() - currentTransform.get_position());
                if (i->timer1 > 1.5f) {
                    i->velocity += direction * 3.0f;
                    i->timer1 = 0.0f;
                }
                currentTransform.get_rotation() = atan2(i->velocity.y, i->velocity.x);
                i->velocity /= 1.0f + ts;
                i->timer1 += ts;
            }
            currentTransform.move(i->velocity * ts);
        }
    }
    void collide_check(object& obj1, object& obj2) {
        collision_detecter_2d detector;
        
        const object_type type1 = obj1.type;
        const object_type type2 = obj2.type;

        const polygon_collider_2d& collider1 = collidersForModels[get_model_id(type1)];
        const polygon_collider_2d& collider2 = collidersForModels[get_model_id(type2)];

        // I CAN first check if the objects are colliding, and then look at their types,
        // BUT this will probably add a couple of thousand extra collision checks '.o.'
        if ((type1 == OBJECT_TYPE_PLAYER) && (type2 == OBJECT_TYPE_BASHI_ENEMY)) { // player - bashi collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_BASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        } else if ((type1 == OBJECT_TYPE_PLAYER) && (type2 == OBJECT_TYPE_DASHI_ENEMY)) { // player - dashi collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_DASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        } else if ((type1 == OBJECT_TYPE_BASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER_BULLET)) { // players bullet - bashi enemy collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_PLAYER_BULLET) && (type2 == OBJECT_TYPE_BASHI_ENEMY)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        } else if ((type1 == OBJECT_TYPE_DASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER_BULLET)) { // players bullet - dashi enemy collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_PLAYER_BULLET) && (type2 == OBJECT_TYPE_DASHI_ENEMY)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        } else if ((type1 == OBJECT_TYPE_BASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER_DEATH_WALL)) { // players death wall - bashi enemy collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_PLAYER_DEATH_WALL) && (type2 == OBJECT_TYPE_BASHI_ENEMY)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        } else if ((type1 == OBJECT_TYPE_DASHI_ENEMY) && (type2 == OBJECT_TYPE_PLAYER_DEATH_WALL)) { // players death wall - dashi enemy collision
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj1);
            }
        } else if ((type1 == OBJECT_TYPE_PLAYER_DEATH_WALL) && (type2 == OBJECT_TYPE_DASHI_ENEMY)) {
            if (detector.is_collide(collider1, collider2, 16, obj1.transform, obj2.transform)) {
                destroy_object(obj2);
            }
        }
    }

    public:
    vec2 get_cursor_position_in_game_world() const {
        return vec2((float)window.get_mouse_x() / (float)window.get_window_extent_x() - 0.5f, (float)window.get_mouse_y() / (float)window.get_window_extent_y() - 0.5f) * 2.0f * sceneState.cameraInfo.scale + sceneState.cameraInfo.position;
    }
    void add_object(object&& obj) {
        auto& gameObjects = sceneState.gameObjects;

        const size_t index = gameObjects.emplace_free(std::move(obj));
        gameObjects[index].index = index;
    }
    void add_object(const object& obj) {
        auto& gameObjects = sceneState.gameObjects;

        const size_t index = gameObjects.emplace_free(obj);
        gameObjects[index].index = index;
    }
    void destroy_object(object& object) {
        auto& gameObjects = sceneState.gameObjects;

        gameObjects.erase_at(object.index);
    }
    void put_on_batch(uint32_t modelID, uint32_t vertCount, const transform2d& transform) {
        const uint32_t nextBufferVertexCount = bufferVertexCount + vertCount;
        if (nextBufferVertexCount > MAX_VERTEX_COUNT)
            return;
        for (uint32_t i = 0; i < vertCount; ++i) {
            const uint32_t bufferIndex = bufferVertexCount + i;
            bufferVerteces[bufferIndex] = transform.transformed(objectModels[modelID][i]);
        }
        bufferVertexCount = nextBufferVertexCount;
    }
    void print_number_in_world(int num, vec2 firstDigitPosition, const vec2& scale) {
        num = abs(num);
        bool nonNullFound = false;
        for (uint32_t i = 0; i < std::numeric_limits<int>::digits10 + 1; ++i) {
            const int ost = num / (pow(10, std::numeric_limits<int>::digits10 - i));
            num -= ost * pow(10, std::numeric_limits<int>::digits10 - i);
            if (ost != 0) {
                nonNullFound = true;
            }

            if (nonNullFound) {
                firstDigitPosition.x += 1.5f * scale.x;
                put_digit_on_batch(ost, transform2d(firstDigitPosition, 0.0f, scale));
            }
        }
    }
    void put_digit_on_batch(int d, const transform2d& transform) {
        d %= 10;
        uint32_t vertCount;
        switch (d) {
            case 0:
            case 1:
            case 2:
                vertCount = 6;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                vertCount = 9;
                break;
            default:
                vertCount = 0;
        }
        const uint32_t nextBufferVertexCount = bufferVertexCount + vertCount;
        if (nextBufferVertexCount > MAX_VERTEX_COUNT)
            return;
        for (uint32_t i = 0; i < vertCount; ++i) {
            const uint32_t bufferIndex = bufferVertexCount + i;
            bufferVerteces[bufferIndex] = transform.transformed(tinyFontModels[d][i]);
        }
        bufferVertexCount = nextBufferVertexCount;
    }
    void clear_bacth() {
        for (uint32_t i = 0; i < bufferVertexCount; ++i)
            bufferVerteces[i] = vec2(0.0f, 0.0f);
        bufferVertexCount = 0;
    }

    private:
    static void lmbDown(deafult_window* window, int x, int y) {
        (void)x;
        (void)y;
        ((Game*)window->userPointer)->input.left_mb_down();
    }
    static void lmbUp(deafult_window* window, int x, int y) {
        (void)x;
        (void)y;
        ((Game*)window->userPointer)->input.left_mb_up();
    }
    static void keyDownCallback(deafult_window* window, int k) {
        ((Game*)window->userPointer)->input.key_down(k);
    }
    static void keyUpCallback(deafult_window* window, int k) {
        ((Game*)window->userPointer)->input.key_up(k);
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    srand(time(0));
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cout << e.what();
    }

    return 0;
}
/*
glslc -x glsl -fshader-stage=frag ./assets/shaders/fragmentDefault.glsl -o ./assets/shaders/fragmentDefault.spirv
glslc -x glsl -fshader-stage=vert ./assets/shaders/vertexDefault.glsl -o ./assets/shaders/vertexDefault.spirv
*/