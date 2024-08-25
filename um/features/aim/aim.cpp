#include "../../globals.hpp"
#include <WinUser.h>
#include <wtypes.h>
#include "../../classes/funcs.h"
#include <mutex>
#include "../../classes/player.h"
#include "aim.hpp"
/**/

std::atomic<bool> loop_control(true);
std::atomic<bool> loop_running(true);
std::atomic<bool> stop_aimbot(false);
std::chrono::time_point<std::chrono::steady_clock> last_iteration_time;
std::mutex loop_mutex;
std::thread aimbot_thread;

float calc_dist(vector2_t a, vector2_t b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

float calc_dist_vec3(vector3_t a, vector3_t b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

static void run(instance_t player, vector3_t resolved_velocity,vector3_t Characterpos) {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(globals::window_handle, &cursor_point);

   vector3_t part_pos_3d;

    //auto& players = playerList;

    auto dimensions = globals::visualengine.get_dimensions();
    auto view_matrix = globals::visualengine.get_view_matrix();
    auto local_player = globals::LocalPlayer;
    float smoothness = ((static_cast<float>(101) - static_cast<float>(1)) / 100);
    instance_t workspace = globals::workspace;
    instance_t camera = workspace.get_camera();

    vector3_t camera_position = camera.get_camera_pos();
    matrix3_t camera_rotation = camera.get_camera_rotation();

    /*
    if (globals::prediction) {
        chaos::roblox::vector3_t velocity = globals::resolver ? resolved_velocity : player.get_part_velocity();
        chaos::roblox::vector3_t target_pos = player.get_part_pos();

        float prediction_x = globals::separate_predictions ? globals::prediction_x : globals::main_prediction;
        float prediction_y = globals::separate_predictions ? globals::prediction_y : globals::main_prediction;
        float prediction_z = globals::separate_predictions ? globals::prediction_x : globals::main_prediction;


        if (globals::autopred) {
            chaos::roblox::instance_t ping_data = globals::datamodel.find_first_child("Stats").find_first_child("PerformanceStats").find_first_child("Ping");
            float ping = ping_data.get_ping();
            float new_prediction = 0.1f + (ping / 2000) + ((ping / 1000) * (ping / 1500) * 1.025f);

            prediction_x = prediction_y = prediction_z = new_prediction;
        }

        chaos::roblox::vector3_t velocity_vector;

        if (globals::prediction_method == 0)
            velocity_vector = { velocity.x / prediction_x, velocity.y / prediction_y, velocity.z / prediction_z };
        else
            velocity_vector = { velocity.x * prediction_x, velocity.y * prediction_y, velocity.z * prediction_z };

        part_pos_3d = target_pos + velocity_vector;
    }
    else {*/
        part_pos_3d = player.get_part_pos();
    //}

    auto part_pos = function::world_to_screen(part_pos_3d, dimensions, view_matrix);

    if (part_pos.x == -1)
        return;

    float dist = calc_dist(part_pos, { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) });

    vector2_t relative = { 0, 0 };
    /*
    if (globals::shake) {
        if (globals::shaketype == 0) {
            int shake_range_x = static_cast<int>(globals::shake_x * 10);
            int shake_range_y = static_cast<int>(globals::shake_y * 10);

            auto shake_x = (rand() & shake_range_x - (shake_range_x / 2));
            auto shake_y = (rand() & shake_range_y - (shake_range_y / 2));

            relative.x = (part_pos.x - cursor_point.x + shake_x) * globals::sensitivity / globals::smoothness_x;
            relative.y = (part_pos.y - cursor_point.y + shake_y) * globals::sensitivity / globals::smoothness_y;
        }
        else if (globals::shaketype == 0) {
            int shake_range = static_cast<int>(globals::shake_value * 10);
            auto shake = (rand() & shake_range - (shake_range / 2));

            relative.x = (part_pos.x - cursor_point.x + shake_range) * globals::sensitivity / globals::smoothness_x;
            relative.y = (part_pos.y - cursor_point.y + shake_range) * globals::sensitivity / globals::smoothness_y;
        }
    }
    else {*/
        relative.x = (part_pos.x - cursor_point.x) * 1.5 / 1; // * globals::sensitivity / globals::smoothness_x;
        relative.y = (part_pos.y - cursor_point.y) * 1.5 / 1; // * globals::sensitivity / globals::smoothness_y;
    //}

    if (relative.x == -1 || relative.y == -1)
        return;

    float dist_to_deadzone = calc_dist({ static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) },
        { static_cast<float>(cursor_point.x) + relative.x,
          static_cast<float>(cursor_point.y) + relative.y });
    
    /*
    if (globals::deadzone && dist_to_deadzone <= globals::deadzone_value)
        return;
        */
    int aimtype = 0;

    if (/*!globals::disable_outside_fov ||*/ dist <= 100) {

        switch (aimtype)
        {
        case 0: {
            static float current_dx = 0.0f;
            static float current_dy = 0.0f;

            current_dx += (relative.x - current_dx) * smoothness;
            current_dy += (relative.y - current_dy) * smoothness;

            INPUT input{};
            input.mi.time = 0;
            input.type = INPUT_MOUSE;
            input.mi.mouseData = 0;
            input.mi.dx = static_cast<LONG>(current_dx);
            input.mi.dy = static_cast<LONG>(current_dy);
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(input));
            break;
        }
        }
    }
}

vector3_t calc_vel(instance_t body_part) {
    auto start = std::chrono::high_resolution_clock::now();

    vector3_t old_pos = body_part.get_part_pos();
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    vector3_t current_pos = body_part.get_part_pos();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed_seconds = end - start;

    vector3_t velocity = (current_pos - old_pos) / elapsed_seconds.count();

    return velocity;
}

float calculate_distance(vector2_t a, vector2_t b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

std::tuple<instance_t, instance_t> get_closest_player_to_cursor() {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(FindWindowA(0, ("Roblox")), &cursor_point);

    vector2_t cursor = {
        static_cast<float>(cursor_point.x),
        static_cast<float>(cursor_point.y)
    };

    ////////////////////////////////////////////////////////////
    auto& players = playerList;

    instance_t closest_player;
    instance_t _body_part;
    float min_dist = 9e9;

    auto dimensions = globals::visualengine.get_dimensions();
    auto view_matrix = globals::visualengine.get_view_matrix();
    auto local_player = globals::LocalPlayer;

    for (auto& player : players) {
        const std::string& player_name = player.playerName;

        //if (player.playerInstance.self == local_player.self || globals::aimwhitelistedPlayers.find(player_name) != globals::aimwhitelistedPlayers.end())
        //	continue;

        auto character = player.modelinstance;
        if (!character.self)
            continue;
        if (player.isLocalPlayer)
            continue;

        //auto humanoid = character.find_first_child("Humanoid");
        //if (!humanoid.self)
        //	continue;

        //auto team = player.playerInstance.get_team();
        //if (globals::team_check && (team.self == local_player.get_team().self))
        //	continue;

        //if (globals::knock_check && humanoid.get_health() <= 3) //replace these into the thread check, phantom forces dead check 
        //	continue;

        instance_t body_part;

        switch (/*globals::aimpart*/0)
        {
        case 0:
            body_part = player.headInstance;
            break;
        case 1:
            body_part = character.find_first_child("UpperTorso");
            break;
        case 2:
            body_part = player.hrpInstance;
            break;
        case 3:
            body_part = character.find_first_child("LowerTorso");
            break;
        case 4:
            body_part = character.find_first_child("Torso");
            break;
        default:
            body_part = player.hrpInstance;
            break;
        }

        if (!body_part.self)
            continue;

        auto part_pos_screen = function::world_to_screen(body_part.get_part_pos(), dimensions, view_matrix);
        float dist = calculate_distance(part_pos_screen, cursor);


        if ((/*!globals::disable_outside_fov ||*/ dist < /*globals::fov*/100) && (min_dist > dist)) {
            closest_player = player.playerInstance;
            min_dist = dist;
            _body_part = body_part;
        }
    }

    return { _body_part, closest_player };
}

void hook_aimbot() {
    instance_t _saved_target;
    instance_t _target;
    instance_t _body_part;
    bool focus = false;
    bool _locked = false;
    vector3_t velocity = {};

    loop_running = true;
    std::thread velocity_calculation([&]() {

        while (!stop_aimbot) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));


            if (/*!globals::aimbot || !globals::aimbotkey.enabled || !globals::resolver ||*/ _target.self == 0)
                continue;

            instance_t _character = _target.get_model_instance();

            if (_character.self == 0)
                continue;

            instance_t _root_part = _character.find_first_child("HumanoidRootPart");

            if (_root_part.self == 0)
                continue;

            velocity = calc_vel(_root_part);
        }
        });


    while (!stop_aimbot) {
        while (loop_control) {
            {
                std::lock_guard<std::mutex> guard(loop_mutex);
                last_iteration_time = std::chrono::steady_clock::now();
            }

            if (/*globals::aimbot*/true) {
                if (globals::is_focused) {
                    vector3_t lastpos = { 3, 3, 3 };

                    if (/*globals::aimbotkey.enabled*/true) {
                        if (!focus && true && _locked && _saved_target.self != 0 && globals::saved_player.self != 0) {
                            _body_part = _saved_target;
                            _target = globals::saved_player;
                        }
                        else {
                            if (!focus) {
                                focus = true;
                                auto [body_part, closest_player] = get_closest_player_to_cursor();
                                _target = closest_player;
                                _body_part = body_part;
                            }
                        }

                        if (_body_part.self != 0 && _body_part.get_part_pos().y > 0.1f) {
                            /*if (globals::autoresolve) {
                                chaos::roblox::vector3_t current_vel = _body_part.get_part_velocity();
                                float velocity_magnitude = std::sqrt((current_vel.x * current_vel.x) + (current_vel.y * current_vel.y) + (current_vel.z * current_vel.z));

                                bool velocity_exceeds_threshold = std::abs(current_vel.x) > globals::velocity_threshold ||
                                    std::abs(current_vel.y) > globals::velocity_threshold ||
                                    std::abs(current_vel.z) > globals::velocity_threshold;

                                globals::resolver = velocity_exceeds_threshold;
                            }*/

                            /*if (globals::saved_player.self != 0 && globals::healthcheck) {
                                auto character = globals::saved_player;
                                auto humanoid = character.find_first_child("Humanoid");
                            }

                            if (globals::saved_player.self != 0 && globals::knock_check) {
                                auto character = globals::saved_player.get_model_instance();
                                auto humanoid = character.find_first_child("Humanoid");
                            }*/

                            if (focus) {
                                run(_body_part, velocity, lastpos);
                            }

                            _saved_target = _body_part;
                            globals::saved_player = _target;
                            _locked = true;
                        }
                        else {
                            globals::saved_player = {};
                            _target = {};
                            _body_part = {};
                            _locked = false;
                            focus = false;
                        }
                    }
                    else {
                        globals::saved_player = {};
                        _target = {};
                        _body_part = {};
                        _locked = false;
                        focus = false;
                    }
                }
                else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                /*
                if (globals::aimtype == 1 && globals::smoothcamlock)
                {

                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }*/

            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
        }
        loop_control = true;
    }

    loop_running = false;
}


void aim::watchdog() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto now = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> guard(loop_mutex);
            if (loop_running && std::chrono::duration_cast<std::chrono::seconds>(now - last_iteration_time).count() > 2) {
                std::cout << "AIMBOTS VARESTARTEBT" << std::endl;

                stop_aimbot = true;

                loop_control = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                loop_control = true;
                last_iteration_time = now;

                if (aimbot_thread.joinable()) {
                    aimbot_thread.join();
                }

                stop_aimbot = false;
                aimbot_thread = std::thread([]() {
                    hook_aimbot();
                    });

                std::cout << "aimboti darestartda" << std::endl;
            }
        }
    }
}

