#ifndef ABUILD_HPP
#define ABUILD_HPP

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define ABUILDAPI static inline
#define TODO(thing) assert(0 && thing "is not implemented.")

namespace ab
{
typedef std::vector<std::string> strvec;

struct Cmd
{
    strvec items;
    void append(std::string item);
    void append_many(strvec items);
    void append_many_prefixed(std::string prefix, strvec items);
    std::string to_string();
    int run();       // TODO: make better than system from cstdlib
    int run_async(); // TODO
    void run_or_die(std::string message);
};

// GO_REBUILD_YOURSELF TECHNOLOGY (TM Amista Azozin)
ABUILDAPI void self_update(int argc, char **argv, std::string source);
ABUILDAPI long compare_last_mod_time(std::string path1, std::string path2);
}; // namespace ab

namespace ab::log
{
enum LogLevel
{
    Info = 0,
    Warning = 1,
    Error = 2,
};

ABUILDAPI std::string log_level_str(LogLevel level);
ABUILDAPI void log(LogLevel level, std::string message);
ABUILDAPI void info(std::string message);
ABUILDAPI void warning(std::string message);
ABUILDAPI void error(std::string message);
} // namespace ab::log

namespace ab::c
{
enum Compiler
{
    CC,
    CXX,
    GCC,
    GXX,
    CLANG,
    MSVC,
};

ABUILDAPI constexpr std::string comp_str(Compiler comp);
ABUILDAPI constexpr Compiler current_compiler();
ABUILDAPI Compiler get_default_compiler();
ABUILDAPI void set_default_compiler(Compiler compiler);

struct CompileOptions
{
    Compiler compiler = get_default_compiler();
    std::string standard;
    strvec flags = {};
    strvec include_paths = {};
    strvec libs = {};
    strvec lib_paths = {};
    strvec defines = {};
    strvec other_flags = {};

    Cmd cmd(strvec sources, strvec additional_flags = {});
    Cmd exe_cmd(std::string output, strvec sources);
    Cmd obj_cmd(std::string source);
    Cmd static_lib_cmd(strvec sources);
    Cmd dynamic_lib_cmd(strvec sources);
};

} // namespace ab::c

// -------------------------------
//
//        Implementation
//
// -------------------------------
#ifdef ABUILD_IMPLEMENTATION

namespace ab
{
void Cmd::append(std::string item)
{
    // TODO: escape item
    items.emplace_back(item);
}

void Cmd::append_many(strvec items)
{
    for (auto item : items)
    {
        append(item);
    }
}

void Cmd::append_many_prefixed(std::string prefix, strvec items)
{
    for (auto item : items)
    {
        append(prefix + item);
    }
}

std::string Cmd::to_string()
{
    if (items.empty())
        return "";

    std::stringstream ss;
    for (size_t i = 0; i < items.size() - 1; i++)
    {
        ss << items[i] << " ";
    }
    ss << items[items.size() - 1];

    return ss.str();
}

int Cmd::run()
{
    std::string cmd = to_string();
    std::cout << cmd << '\n';
    return system(cmd.c_str());
}

void Cmd::run_or_die(std::string message)
{
    int result = run();
    if (result != 0)
    {
        ab::log::error(message);
        exit(result);
    }
}

ABUILDAPI void self_update(int argc, char **argv, std::string source)
{
    log::info("Updating");
    assert(argc > 0);
    std::string exe(argv[0]);

    if (compare_last_mod_time(source, exe) < 0)
        return;

    std::filesystem::rename(exe, exe + ".old");
    {
        Cmd cmd;
        cmd.append_many({"c++", source, "-o", exe});
        cmd.run_or_die("Error during self_update!!!");
    }
    Cmd cmd;
    cmd.append(exe);
    cmd.run();

    exit(0);
}
ABUILDAPI long compare_last_mod_time(std::string path1, std::string path2)
{
    auto time1 = std::filesystem::last_write_time(path1);
    auto time2 = std::filesystem::last_write_time(path2);
    auto diff = time1 - time2;
    return diff.count();
}

} // namespace ab

namespace ab::log
{
static LogLevel minimal_level = Info;

ABUILDAPI void log(LogLevel level, std::string message)
{
    if (level < minimal_level)
        return;

    (level == Info ? std::cout : std::cerr) << '[' << log_level_str(level) << "] " << message << '\n';
}

ABUILDAPI std::string log_level_str(LogLevel level)
{
    switch (level)
    {
    case Info:
        return "INFO";
    case Warning:
        return "WARNING";
    case Error:
        return "ERROR";
    default:
        return "UNKNOWN LOG LEVEL";
    }
}

ABUILDAPI void info(std::string message)
{
    log(Info, message);
}

ABUILDAPI void warning(std::string message)
{
    log(Warning, message);
}

ABUILDAPI void error(std::string message)
{
    log(Error, message);
}

} // namespace ab::log

namespace ab::c
{
Compiler default_compiler = current_compiler();

ABUILDAPI constexpr std::string comp_str(Compiler comp)
{
    switch (comp)
    {
    case CC:
        return "cc";
    case CXX:
        return "c++";
    case GCC:
        return "gcc";
    case GXX:
        return "g++";
    case CLANG:
        return "clang";
    case MSVC:
        TODO("msvc");
    default:
        return "UNKNOWN COMPILER";
    }
}

ABUILDAPI constexpr Compiler current_compiler()
{
//  Clang C++ emulates GCC, so it has to appear early.
#if defined __clang__ && !defined(__ibmxl__) && !defined(__CODEGEARC__)
    return CLANG;

//  GNU C++:
#elif defined(__GNUC__) && !defined(__ibmxl__)
    return GXX;

//  Microsoft Visual C++
#elif defined _MSC_VER
    return MSVC;
#endif

    TODO("Unknown compiler");
}

ABUILDAPI Compiler get_default_compiler()
{
    return default_compiler;
}

ABUILDAPI void set_default_compiler(Compiler compiler);

Cmd CompileOptions::cmd(strvec sources, strvec additional_flags)
{
    Cmd cmd;

    cmd.append(comp_str(compiler));
    if (!standard.empty())
        cmd.append("-std=" + standard);

    cmd.append_many(flags);
    cmd.append_many_prefixed("-I", include_paths);
    cmd.append_many_prefixed("-l", libs);
    cmd.append_many_prefixed("-L", lib_paths);
    cmd.append_many_prefixed("-D", defines);
    cmd.append_many(other_flags);

    cmd.append_many(additional_flags);
    cmd.append_many(sources);

    return cmd;
}

Cmd CompileOptions::exe_cmd(std::string output, strvec sources)
{
    return this->cmd(sources, {"-o", output});
}

Cmd CompileOptions::obj_cmd(std::string source)
{
    return this->cmd({source}, {"-c"});
}

Cmd static_lib_cmd(strvec sources)
{
    (void)sources;
    TODO("static_lib_cmd");
}

Cmd dynamic_lib_cmd(strvec sources)
{
    (void)sources;
    TODO("static_lib_cmd");
}

} // namespace ab::c

#endif // ABUILD_IMPLEMENTATION
#endif // ABUILD_HPP
