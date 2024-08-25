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

namespace driver {
    namespace codes {
        // used to setup the driver
        constexpr ULONG attach =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

        // read process memory from um application 
        constexpr ULONG read =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

        // write process memory from um application
        constexpr ULONG write =
            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    } // namespace codes

    // shared between user mode and kernel mode
    struct Request {
        HANDLE process_id;

        PVOID target;
        PVOID buffer;

        SIZE_T size;
        SIZE_T return_size;
    };

    inline bool attach_to_process(HANDLE driver_handle, const DWORD pid) {
        Request r;
        r.process_id = reinterpret_cast<HANDLE>(pid);

        return DeviceIoControl(driver_handle, codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
    }

    template <class T>
    T read_memory(HANDLE driver_handle, const std::uintptr_t addr) {
        T temp = {};

        Request r;
        r.target = reinterpret_cast<PVOID>(addr);
        r.buffer = &temp;
        r.size = sizeof(T);

        DeviceIoControl(driver_handle, codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

        return temp;
    }

    template <class T>
    void write_memory(HANDLE driver_handle, const std::uintptr_t addr, const T& value) {
        Request r;
        r.target = reinterpret_cast<PVOID>(addr);
        r.buffer = (PVOID)&value;
        r.size = sizeof(T);

        DeviceIoControl(driver_handle, codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
    }
} // namespace driver