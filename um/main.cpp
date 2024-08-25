#include <iostream>
#include <Windows.h>
#include <thread>
#include <TlHelp32.h>
#include <vector>
#include <Psapi.h>
#include <string>
#include <tchar.h>
#include <ShlObj.h>
#include <fstream>
#include <cstdio>
#include <string>
//imgui stuff
#include <dwmapi.h>
#include <d3d11.h>
#include <windowsx.h>
#include <filesystem>
#include <xstring>
#include <regex>
#include "globals.hpp"
#include <mutex>
#include "classes/player.h"

#include "offsets.hpp"
#include "features/aim/aim.hpp"

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0L;
    }

    switch (message)
    {
    case WM_NCHITTEST:
    {
        const LONG borderWidth = GetSystemMetrics(SM_CXSIZEFRAME);
        const LONG titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
        POINT cursorPos = { GET_X_LPARAM(w_param), GET_Y_LPARAM(l_param) };
        RECT windowRect;
        GetWindowRect(window, &windowRect);

        if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + titleBarHeight)
            return HTCAPTION;
        
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(window, message, w_param, l_param);
}

int GetProcessIdByName(const wchar_t* processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
            if (_wcsicmp(entry.szExeFile, processName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return 0;
}

DWORD_PTR GetModuleBaseAddress(DWORD dwPid, const wchar_t* moduleName) {
    MODULEENTRY32 moduleEntry = { sizeof(MODULEENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPid);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            if (!_wcsicmp(moduleEntry.szModule, moduleName)) {
                CloseHandle(hSnapshot);
                return (DWORD_PTR)moduleEntry.modBaseAddr;
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }
    CloseHandle(hSnapshot);
    return 0;
}

static std::wstring appdata_path()
{
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
    {
        return std::wstring(path);
    }
    return L"";
}

std::vector<std::filesystem::path> get_roblox_file_logs()
{
    std::vector<std::filesystem::path> roblox_log;
    std::wstring app_data_path = appdata_path();
    std::wstring roblox_log_path = app_data_path + L"\\Roblox\\logs";

    for (const auto& entry : std::filesystem::directory_iterator(roblox_log_path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".log" && entry.path().filename().string().find("Player") != std::string::npos)
            roblox_log.push_back(entry.path());
    }

    return roblox_log;
}

std::filesystem::path get_latest_log()
{
    auto logs = get_roblox_file_logs();

    std::sort(logs.begin(), logs.end(), [](const std::wstring& a, const std::wstring& b) {
        return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
        });

    return logs[0];
}

std::uint64_t get_render_view()
{
    auto latest_log = get_latest_log();

    std::ifstream rbx_log(latest_log);
    std::string rbx_log_line;
    std::regex view_regex(R"(\[FLog::SurfaceController\] SurfaceController\[_:1\]::initialize view\(([0-9A-Fa-f]+)\))");
    std::smatch match;

    while (std::getline(rbx_log, rbx_log_line))
    {
        if (std::regex_search(rbx_log_line, match, view_regex))
        {
            std::string hex_value = match[1];
            std::uint64_t renderview = std::strtoull(hex_value.c_str(), nullptr, 16);
            return renderview;
        }
    }
}



HANDLE loaded_driver;

std::string readstring(std::uint64_t address)
{
    std::string string;
    char character = 0;
    int char_size = sizeof(character);
    int offset = 0;

    string.reserve(204);

    while (offset < 200)
    {
        character = driver::read_memory<char>(loaded_driver, address + offset);

        if (character == 0)
            break;

        offset += char_size;
        string.push_back(character);
    }

    return string;
}

std::string readstring2(std::uint64_t string)
{
    const auto length = driver::read_memory<int>(loaded_driver, string + 0x18);

    if (length >= 16u)
    {
        const auto New = driver::read_memory<std::uint64_t>(loaded_driver,string);
        return readstring(New);
    }
    else
    {
        const auto Name = readstring(string);
        return Name;
    }
}

std::string instance_t::class_name()
{
    auto Numba1 = driver::read_memory<std::uint64_t>(loaded_driver, this->self + 0x18);
    auto Numba2 = driver::read_memory<std::uint64_t>(loaded_driver, Numba1 + 0x8);

    if (Numba2)
        return readstring2(Numba2);

    return "???_classname";
}



std::vector<instance_t> instance_t::children()
{
    std::vector<instance_t> container;

    auto start = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x50);

    auto end = driver::read_memory<std::uint64_t>(globals::driverhandle, start + 0x8);

    for (auto instances = driver::read_memory<std::uint64_t>(globals::driverhandle, start); instances != end; instances += 16) {

        if (instances > 1099511627776 && instances < 3298534883328) {

            container.emplace_back(driver::read_memory<instance_t>(globals::driverhandle, instances));

        }
    }
    return container;
}

instance_t instance_t::find_first_child_of_class(const std::string& classname)
{
    std::vector<instance_t> children = this->children();

    for (auto& object : children)
    {
        if (object.class_name() == classname)
        {
            return object;
        }
    }

    return instance_t();
}

union convertion
{
    std::uint64_t hex;
    float f;
} conv;

float instance_t::get_health() {
    float last_valid_health = -1.0f;
    float health = -1.0f;

    while (true) {
        auto one = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x184);
        if (!one) {
            return last_valid_health;
        }

        auto two = driver::read_memory<std::uint64_t>(globals::driverhandle, driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x184));

        conv.hex = one ^ two;
        health = conv.f;


        if (health >= 0.1f && health <= 10000.0f) {
            last_valid_health = health;
            break;
        }
        else {
            health = last_valid_health;
        }
    }

    return last_valid_health;
}

instance_t instance_t::get_local_player()
{
    auto local_player = driver::read_memory<instance_t>(globals::driverhandle, this->self + 0x100);
    return local_player;
}

instance_t instance_t::get_team()
{
    if (!this->self) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    auto getteam = driver::read_memory<instance_t>(globals::driverhandle, this->self + 0x1D0);
    return getteam;
}


instance_t instance_t::get_model_instance()
{
    auto character = driver::read_memory<instance_t>(globals::driverhandle, this->self + 0x298);
    return character;

}

std::string instance_t::name()
{
    const auto ptr = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x48);

    if (ptr)
        return readstring2(ptr);

    return "???";
}

instance_t instance_t::find_first_child(const std::string& child)
{
    std::vector<instance_t> children = this->children();

    for (auto& object : children)
    {
        if (object.name() == child)
        {
            return object;
        }
    }

    return instance_t();
}

std::atomic<bool> heartbeat_received(false);
std::mutex heartbeat_mutex;
std::atomic<bool> thread_running;

void updateThreadUniversal() {
    auto players = globals::players;
    auto localplayer = globals::LocalPlayer;
    std::mutex playerListMutex;

    thread_running = true;
    std::vector<Player> newPlayerList;

    while (thread_running) {

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto lpTeam = globals::LocalPlayer.get_team();

        {
            std::lock_guard<std::mutex> lock(heartbeat_mutex);
            heartbeat_received = true;
        }

        newPlayerList.clear();

        for (auto& player : players.children()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            bool isLocalPlayer = (player.self == localplayer.self);

            Player plr;
            plr.playerInstance = player;
            plr.isLocalPlayer = isLocalPlayer;

            auto modelInstance = player.get_model_instance();
            plr.modelinstance = modelInstance;
            plr.humanoid = modelInstance.find_first_child("Humanoid");

            

            plr.headInstance = modelInstance.find_first_child("Head");
            plr.hrpInstance = modelInstance.find_first_child("HumanoidRootPart");

            // jump power ahh shi
            /*
            if (isLocalPlayer)
                driver::write_memory(globals::driverhandle, plr.humanoid.self + Offsets::JumpPower, (float)120.0);
                driver::write_memory(globals::driverhandle, plr.humanoid.self + Offsets::WalkSpeedA, (float)120.0);
                driver::write_memory(globals::driverhandle, plr.humanoid.self + Offsets::WalkSpeedB, (float)120.0);
                */
            plr.playerName = modelInstance.name();
            //std::cout << plr.playerName << "-s Sicocxle => " << driver::read_memory<float>(globals::driverhandle, plr.humanoid.self + 0x184) << std::endl;
            newPlayerList.push_back(plr);
        }

        {
            std::lock_guard<std::mutex> lock(playerListMutex);
            playerList = std::move(newPlayerList);
        }


    }
}

void heartbeatThread() {
    while (thread_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(heartbeat_mutex);
        heartbeat_received = true;
    }
}

void positionUpdateThreadUniversal() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        {
            for (auto& player : playerList) {
                if (player.hrpInstance.self) {
                    auto hrp = player.hrpInstance;
                    player.hrpPos3d = hrp.get_part_pos();
                    player.velocity = hrp.get_part_velocity();
                    player.hrpRot = hrp.get_part_rotation();
                }
            }
        }
    }
}

void threadLoop() {
        std::cout << "didebuli tredi gashvebulia\n";
        std::thread(positionUpdateThreadUniversal).detach();
        std::thread(heartbeatThread).detach();
        updateThreadUniversal();
}

void monitorThread(std::thread& funcThread, HANDLE& threadHandle) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        bool heartbeat_ok = false;
        {
            std::lock_guard<std::mutex> lock(heartbeat_mutex);
            heartbeat_ok = heartbeat_received;
            heartbeat_received = false;
            threadcrash = false;
        }

        if (!heartbeat_ok) {
            std::cout << "vcdilobt tredis amushavebas" << std::endl;
            heartbeat_ok = true;
            threadcrash = true;
            if (funcThread.joinable()) {
                CloseHandle(threadHandle);
                TerminateThread(threadHandle, 0);
                funcThread.join();
            }


            funcThread = std::thread(threadLoop);
            threadHandle = funcThread.native_handle();
            funcThread.detach();
        }
    }
}

uint64_t instance_t::get_gameid()
{
    auto gameid = driver::read_memory<std::uint64_t>(globals::driverhandle, this->self + 0x160);
    return gameid;
}

//....
instance_t find_local_humanoid() {
    if (!playerList.empty()) {
        return playerList.front().humanoid;
    }
    return instance_t{};
}

float get_humanoid_health()
{
    auto humanoid_instance = find_local_humanoid();

    return driver::read_memory<float>(globals::driverhandle, humanoid_instance.self + 0x184);
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
    const wchar_t* targetModuleName = L"RobloxPlayerBeta.exe";

    int pid = GetProcessIdByName(targetModuleName);
    if (pid == 0) {
        MessageBoxW(NULL, L"procesi ver moidzebna", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    const HANDLE driver = CreateFile(L"\\\\.\\km", GENERIC_READ, 0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driver == INVALID_HANDLE_VALUE) {
        MessageBoxW(NULL, L"draiveri ver gavchitet", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    loaded_driver = driver;
    globals::driverhandle = driver;

    if (driver::attach_to_process(driver, pid) == true) {
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
        
        std::cout << R"(                                    ZS       
                                 ZSZ        
                              ZSZSZ  Z      
                             ZSZSZSZS       
                           ZSZSZSZSZ        
                            ZSZSZSZ         
                          ZSZSZSZSZ         
                       ZSZSZSZSZSZ          
                    ZSZSZSZSZSZSZS          
              ZSZSZSZSZSZSZSZSZSZ           
           ZSZSZSZSZSZSZSZSZSZSZS           
         ZSZSZSZSZSZSZSZSZSZSZSZS           
 Z      ZSZSZSZSZS    ZSZSZSZSZSZ           
 ZS    ZSZSZSZSZS           ZS Z            
 ZSZSZS ZSZSZSZSZ            Z Z            
         ZSZSZS              Z Z            
          ZSZSZ              Z Z            
          ZSZ ZS             Z Z            
          ZS   ZS                   
         ZS     Z        
          ZS     ZS  
           ZS  

Mogesalmebit ZeruaS Roblox External Kernel Chetshi, Sasiamovno Cheterobas Gisurvebt

E010 5254 F322 DE38 8C86 611B A037 B75B 2E4D 8461
)" << "\n";

        std::cout << std::hex << "Datamodel Napovnia Rogorc 0x" << get_render_view() << std::endl;

        auto game_ptr = driver::read_memory<std::uint64_t>(driver, get_render_view() + 0x118);
        auto game = static_cast<instance_t>(driver::read_memory<std::uint64_t>(driver, game_ptr + 0x198));

        //auto visualengine = static_cast<instance_t>(get_visualengine_address());

        auto visualengine = static_cast<instance_t>(driver::read_memory<std::uint64_t>(driver, get_render_view() + 0x10));

        auto players =
            game.find_first_child_of_class("Players");
        globals::players = players;
        std::cout << players.class_name() << " => " << players.self << std::endl;

        globals::LocalPlayer = players.get_local_player();
        std::cout << globals::LocalPlayer.class_name() << " => " << globals::LocalPlayer.self << std::endl;

        globals::datamodel = game;
        std::cout << game.class_name() << " => " << game.self << std::endl;

        globals::visualengine = visualengine;
        std::cout << "VisualEngine" << " => " << visualengine.self << std::endl;

        auto Workspace = game.find_first_child_of_class("Workspace");   
        globals::workspace = Workspace;


        std::thread(aim::watchdog).detach();

        thread_running = true;

        std::thread funcThread(threadLoop);
        HANDLE threadHandle = funcThread.native_handle();
        funcThread.detach();

        std::thread monitorThreadInstance(monitorThread, std::ref(funcThread), std::ref(threadHandle));
        monitorThreadInstance.detach();

        globals::window_handle = FindWindowA(NULL, "Roblox");
        std::cout << "Windows Handle => " << globals::window_handle << std::endl;


  

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            globals::is_focused = GetForegroundWindow() == globals::window_handle;

            if (FindWindowA(0, "Roblox") == NULL) {
                break;
            }
        }
    }

    CloseHandle(driver);

    return 0;
}
