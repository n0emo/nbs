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

namespace ab
{
typedef std::vector<std::string> strvec;

struct Cmd
{
    strvec items;
    void append(std::string item);
    void append_many(strvec items);
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
    GCC,
    CLANG,
};

class ObjectTarget
{
  private:
    Compiler compiler = CC;
    strvec flags;
    strvec include_paths;
    std::string source;
    strvec defines;

  public:
    Cmd cmd();
    ObjectTarget(std::string source);
    ObjectTarget(std::string source, Compiler compiler, strvec flags, strvec include_paths, strvec defines);
    void set_compiler(Compiler compiler);
    void set_flags(strvec flags);
    void set_include_paths(strvec paths); // TODO: consider path class
    void set_defines(strvec defines);
};

ABUILDAPI std::string comp_str(Compiler comp);
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

Cmd ObjectTarget::cmd()
{
    Cmd cmd;

    cmd.append(comp_str(compiler));
    cmd.append_many(flags);
    for (const auto &path : include_paths)
    {
        cmd.append(std::string("-I") + path);
    }
    cmd.append("-c");
    cmd.append(source);
    for (const auto &path : defines)
    {
        cmd.append(std::string("-D") + path);
    }

    return cmd;
}

ObjectTarget::ObjectTarget(std::string source)
{
    this->source = source;
}

ObjectTarget::ObjectTarget(std::string source, Compiler compiler, strvec flags, strvec include_paths, strvec defines)
{
    this->source = source;
    this->compiler = compiler;
    this->flags = flags;
    this->include_paths = include_paths;
    this->defines = defines;
}

void ObjectTarget::set_compiler(Compiler compiler)
{
    this->compiler = compiler;
}

void ObjectTarget::set_flags(strvec flags)
{
    this->flags = flags;
}

void ObjectTarget::set_include_paths(strvec paths)
{
    this->include_paths = paths;
}

void ObjectTarget::set_defines(strvec defines)
{
    this->defines = defines;
}

ABUILDAPI std::string comp_str(Compiler comp)
{
    switch (comp)
    {
    case CC:
        return "cc";
    case GCC:
        return "gcc";
    case CLANG:
        return "clang";
    default:
        return "UNKNOWN COMPILER";
    }
}
} // namespace ab::c

#endif // ABUILD_IMPLEMENTATION
#endif // ABUILD_HPP
