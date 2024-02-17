#ifndef ABUILD_HPP
#define ABUILD_HPP

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define ABUILDAPI static inline
#define TODO(thing) assert(0 && thing "is not implemented.")

namespace ab
{
typedef std::vector<std::string> strvec;

const std::string os_path_sep =
#ifdef _WIN32
    "\\";
#else
    "/";
#endif

struct Cmd
{
    strvec items;

    Cmd();
    Cmd(const std::string &cmd);
    Cmd(const strvec &cmd);

    void append(const std::string &item);
    void append_many(const strvec &items);
    void append_many_prefixed(const std::string &prefix, const strvec &items);

    std::string to_string() const;
    int run() const;       // TODO: make better than system from cstdlib
    int run_async() const; // TODO
    void run_or_die(const std::string &message) const;
};

ABUILDAPI void self_update(int argc, char **argv, std::string source);
ABUILDAPI long compare_last_mod_time(const std::string &path1, const std::string &path2);
ABUILDAPI std::string string_join(const std::string &sep, const strvec &strings);
ABUILDAPI std::string path(const strvec &path);
ABUILDAPI bool make_directory_if_not_exists(const std::string &path);
ABUILDAPI std::string trim_to(const std::string &str, const std::string &chars = "\n\r ");
ABUILDAPI std::string trim_right_to(const std::string &str, const std::string &chars = "\n\r ");
ABUILDAPI std::string trim_left_to(const std::string &str, const std::string &chars = "\n\r ");
ABUILDAPI strvec split(const std::string &str, const std::string &delim);
ABUILDAPI std::string change_extension(const std::string &file, const std::string &new_extension);

struct Defaults
{
    std::string build_path = "";
};
ABUILDAPI Defaults *get_defaults();

struct Target
{
    std::string output;
    strvec dependencies;
    std::vector<Cmd> cmds;

    Target(const std::string &output, const Cmd &cmd, const strvec &dependencies = {});
    Target(const std::string &output, const std::vector<Cmd> &cmds, const strvec &dependencies = {});

    int build();
};

struct TargetMap
{
    std::unordered_map<std::string, Target> targets;

    TargetMap() = default;

    void insert(Target &target);
    bool remove(const std::string &target);
    int build(const std::string &output);
    int build_if_needs(const std::string &output);
    bool needs_rebuild(const std::string &output);
};

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

ABUILDAPI std::string comp_str(Compiler comp);
ABUILDAPI Compiler current_compiler();

struct CDefaults
{
    Compiler compiler = CXX;
    std::string standard;
    strvec flags;
    strvec include_paths;
    strvec libs;
    strvec lib_paths;
    strvec defines;
    strvec other_flags;
};

ABUILDAPI CDefaults *get_cdefaults();

struct CompileOptions
{
    Compiler compiler = get_cdefaults()->compiler;
    std::string standard = get_cdefaults()->standard;
    strvec flags = get_cdefaults()->flags;
    strvec include_paths = get_cdefaults()->include_paths;
    strvec libs = get_cdefaults()->libs;
    strvec lib_paths = get_cdefaults()->lib_paths;
    strvec defines = get_cdefaults()->defines;
    strvec other_flags = get_cdefaults()->other_flags;

    Cmd cmd(strvec sources, strvec additional_flags = {});
    Cmd exe_cmd(std::string output, strvec sources);
    Cmd obj_cmd(std::string output, std::string source);
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
static Defaults defaults;
ABUILDAPI Defaults *get_defaults()
{
    return &defaults;
}

Cmd::Cmd()
{
}

Cmd::Cmd(const std::string &cmd)
{
    append(cmd);
}

Cmd::Cmd(const strvec &cmd)
{
    append_many(cmd);
}

void Cmd::append(const std::string &item)
{
    items.emplace_back(item);
}

void Cmd::append_many(const strvec &items)
{
    for (auto item : items)
    {
        append(item);
    }
}

void Cmd::append_many_prefixed(const std::string &prefix, const strvec &items)
{
    for (auto item : items)
    {
        append(prefix + item);
    }
}

std::string Cmd::to_string() const
{
    // TODO: escape item
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

int Cmd::run() const
{
    std::string cmd = to_string();
    std::cout << cmd << '\n';
    return system(cmd.c_str());
}

void Cmd::run_or_die(const std::string &message) const
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
    assert(argc > 0);
    std::string exe(argv[0]);

    if (compare_last_mod_time(source, exe) < 0)
        return;

    log::info("Updating");
    std::filesystem::rename(exe, exe + ".old");
    {
        Cmd cmd;
        cmd.append_many({c::comp_str(c::current_compiler()), source, "-o", exe});
        cmd.run_or_die("Error during self_update!!!");
    }
    Cmd cmd;
    cmd.append(exe);
    for (size_t i = 1; i < argc; i++)
    {
        cmd.append(argv[i]);
    }
    cmd.run();

    exit(0);
}
ABUILDAPI long compare_last_mod_time(const std::string &path1, const std::string &path2)
{
    auto time1 = std::filesystem::last_write_time(path1);
    auto time2 = std::filesystem::last_write_time(path2);
    auto diff = time1 - time2;
    return diff.count();
}

ABUILDAPI std::string string_join(const std::string &sep, const strvec &strings)
{
    if (strings.empty())
        return "";

    if (strings.size() == 1)
        return strings[0];

    std::stringstream ss;
    for (size_t i = 0; i < strings.size() - 1; i++)
    {
        ss << strings[i];
        ss << sep;
    }
    ss << strings.back();

    return ss.str();
}

ABUILDAPI std::string path(const strvec &path)
{
#ifdef __WIN32
    return string_join("\\", path);
#else
    return string_join("/", path);
#endif
}

ABUILDAPI bool make_directory_if_not_exists(const std::string &path)
{
    return std::filesystem::create_directory(path);
}

ABUILDAPI std::string trim_to(const std::string &str, const std::string &chars)
{
    return trim_right_to(trim_left_to(str));
}

ABUILDAPI std::string trim_right_to(const std::string &str, const std::string &chars)
{
    size_t count = str.size();
    while (count > 0 && chars.find(str[count - 1]) == std::string::npos)
    {
        count--;
    }
    return str.substr(0, count);
}

ABUILDAPI std::string trim_left_to(const std::string &str, const std::string &chars)
{
    size_t index = 0;
    while (index < str.size() && chars.find(str[index]) == std::string::npos)
    {
        index++;
    }
    return str.substr(index);
}

ABUILDAPI strvec split(const std::string &str, const std::string &delim)
{
    if (str.empty())
        return {""};

    strvec result;
    size_t index = 0;
    size_t count = 0;

    for (size_t i = 0; i < str.size(); i++)
    {
        char c = str[i];
        if (delim.find(c) == std::string::npos)
        {
            count++;
        }
        else
        {
            result.emplace_back(str.substr(index, count));
            index = i + 1;
            count = 0;
        }
    }
    result.emplace_back(str.substr(index, count));

    return result;
}

ABUILDAPI std::string change_extension(const std::string &file, const std::string &new_extension)
{
    return trim_right_to(file, ".") + new_extension;
}

Target::Target(const std::string &output, const Cmd &cmd, const strvec &dependencies)
    : output(output), cmds({cmd}), dependencies(dependencies)
{
}
Target::Target(const std::string &output, const std::vector<Cmd> &cmds, const strvec &dependencies)
    : output(output), cmds(cmds), dependencies(dependencies)
{
}

int Target::build()
{
    for (const auto cmd : cmds)
    {
        int result = cmd.run();
        if (result != 0)
            return result;
    }
    return 0;
}

void TargetMap::insert(Target &target)
{
    targets.insert({target.output, target});
}
bool TargetMap::remove(const std::string &target_output)
{
    return targets.erase(target_output) > 0;
}
int TargetMap::build(const std::string &output)
{
    auto target_it = targets.find(output);
    if (target_it == targets.end())
        return 1;
    auto target = target_it->second;

    for (const auto &dep : target.dependencies)
    {
        if (targets.find(dep) != targets.end())
        {
            int result = build(dep);
            if (result != 0)
                return result;
        }
        else if (!std::filesystem::exists(dep))
        {
            return 1;
        }
    }

    for (const auto &cmd : target.cmds)
    {
        int result = cmd.run();
        if (result != 0)
            return result;
    }
    return 0;
}
int TargetMap::build_if_needs(const std::string &output)
{
    if (!needs_rebuild(output))
        return 0;

    auto target = targets.find(output)->second;

    for (const auto &dep : target.dependencies)
    {
        if (targets.find(dep) != targets.end())
        {
            int result = build_if_needs(dep);
            if (result != 0)
                return result;
        }
        else if (!std::filesystem::exists(dep))
        {
            return 1;
        }
    }

    for (const auto &cmd : target.cmds)
    {
        int result = cmd.run();
        if (result != 0)
            return result;
    }
    return 0;
}
bool TargetMap::needs_rebuild(const std::string &output)
{
    if (!std::filesystem::exists(output))
        return true;

    auto target_it = targets.find(output);
    if (target_it == targets.end())
        TODO("needs_rebuild error handling");
    auto target = target_it->second;

    for (const auto &dep : target.dependencies)
    {
        if ((targets.find(dep) != targets.end() && needs_rebuild(dep)) ||
            (targets.find(dep) != targets.end() && !std::filesystem::exists(dep) ||
             (std::filesystem::exists(dep) && compare_last_mod_time(output, dep) < 0)))
        {
            return true;
        }
    }
    return false;
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
static CDefaults cdefaults;

ABUILDAPI std::string comp_str(Compiler comp)
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

ABUILDAPI Compiler current_compiler()
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

ABUILDAPI CDefaults *get_cdefaults()
{
    return &cdefaults;
}

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

Cmd CompileOptions::obj_cmd(std::string output, std::string source)
{
    return this->cmd({source}, {"-c", "-o", output});
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
