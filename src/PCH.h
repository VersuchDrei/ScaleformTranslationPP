#pragma once

#include <clocale>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <locale>
#include <queue>
#include <regex>
#include <stack>
#include <string>
#include <typeinfo>

#pragma warning(push)
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <REL/Relocation.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#pragma warning(pop)

#include <Psapi.h>
#include <ShlObj_core.h>
#include <Windows.h>
#undef cdecl  // Workaround for Clang 14 CMake configure error.

// Compatible declarations with other sample projects.
#define DLLEXPORT __declspec(dllexport)

using namespace std::literals;
using namespace REL::literals;

namespace logger = SKSE::log;

namespace util {
	using SKSE::stl::memzero;
    using SKSE::stl::report_and_fail;
}
