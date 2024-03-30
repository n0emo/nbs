/*
 * A simple build system to build in C++
 */

/*
 * MIT License
 *
 * Copyright (c) 2024 Albert Shefner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NBS_HPP
#define NBS_HPP

#include <algorithm>
#include <cassert>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#define NBSAPI static inline
#define TODO(thing) assert(0 && thing "is not implemented.")

// TODO: multiprocessed target building
// TODO: generate targets in nbs::c
// TODO: determine user toolchain (compiler, etc.)
// TODO: support for MSVC syntax
// TODO: CLI class for easy CLI app building
// TODO: maybe nbs::unit for unit testing
// TODO: nbs::config

namespace nbs
{
typedef std::vector<std::string> strvec;

NBSAPI void self_update(int argc, char **argv, const std::string &source);

namespace err
{
template <typename T>
struct Ok
{
    T value;

    Ok() = default;
    ~Ok() = default;
    Ok(const Ok &) = delete;
    Ok(Ok &&) = delete;

    Ok(const T &value);
};

template <>
struct Ok<void>
{
    Ok() = default;
    ~Ok() = default;
    Ok(const Ok &) = delete;
    Ok(Ok &&) = delete;
};

Ok() -> Ok<void>;

template <typename T>
struct Err
{
    T value;

    Err() = default;
    ~Err() = default;
    Err(const Err &) = delete;
    Err(Err &&) = delete;

    Err(const T &value);
};

template <typename V, typename E>
class Result
{
    std::variant<V, E> variant;

  public:
    constexpr Result();
    constexpr Result(const Result &other);
    constexpr Result(const std::variant<V, E> &variant);
    constexpr Result(const Ok<V> &ok);
    constexpr Result(const Err<E> &err);
    constexpr Result &operator=(const Result &other);

    constexpr bool is_ok() const;
    constexpr bool is_err() const;
    constexpr V value() const;
    constexpr V value_or(const V &default_value) const;
    constexpr E error() const;
    constexpr V &operator*();
    constexpr V *operator->();

    template <typename NewV>
    Result<NewV, E> bind(std::function<Result<NewV, E>(V)> func);
};

template <typename E>
class Result<void, E>
{
    std::optional<E> m_error;

  public:
    constexpr Result() = default; // TODO: consider delete
    constexpr Result(const Result &other);
    constexpr Result(const Ok<void> &);
    constexpr Result(const Err<E> &err);
    constexpr Result &operator=(const Result &other);

    constexpr bool is_ok() const;
    constexpr bool is_err() const;
    constexpr E error() const;
};

class BadResultException : public std::exception
{
  public:
    BadResultException() = default;
    const char *what() const noexcept;
};
} // namespace err

namespace os
{
using path = std::filesystem::path;

enum ProcessError
{
    PROCESS_WAIT_ERROR,
    PROCESS_EXIT_STATUS_ERROR,
    PROCESS_EMPTY_CMD_ERROR,
    PROCESS_CREATE_ERROR,
#ifdef _WIN32
    PROCESS_GET_EXIT_CODE_ERROR,
    PROCESS_GET_HANDLE_ERROR,
#else
    PROCESS_SIGNAL_ERROR,
    PROCESS_EXEC_ERROR,
#endif
};

struct Process
{
#ifdef _WIN32
    HANDLE handle;
    Process(HANDLE handle);
#else
    int pid;
    Process(int pid);
#endif
    err::Result<void, ProcessError> await() const;
};

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
    err::Result<void, ProcessError> run() const;
    err::Result<Process, ProcessError> run_async() const;
    void run_or_die(const std::string &message) const;
    std::unique_ptr<char *[]> to_c_argv() const;
};

NBSAPI err::Result<void, ProcessError> await_processes(const std::vector<Process> &processes);

#ifdef _WIN32
std::string windows_error_code_to_str(DWORD error);
std::string windows_last_error_str();
#endif

typedef std::vector<path> pathvec;

NBSAPI strvec paths_to_strs(const pathvec &paths);
NBSAPI pathvec strs_to_paths(const strvec &paths);
NBSAPI long compare_last_mod_time(const path &path1, const path &path2);
NBSAPI bool make_directory_if_not_exists(const path &path);
NBSAPI bool exists(const path &path);
} // namespace os

namespace str
{
NBSAPI std::string join(const std::string &sep, const strvec &strings);
NBSAPI std::string trim_to(const std::string &str, const std::string &chars = "\n\r ");
NBSAPI std::string trim_right_to(const std::string &str, const std::string &chars = "\n\r ");
NBSAPI std::string trim_left_to(const std::string &str, const std::string &chars = "\n\r ");
NBSAPI strvec split(const std::string &str, const std::string &delim);
NBSAPI std::string change_extension(const std::string &file, const std::string &new_extension);
} // namespace str

namespace log
{
enum LogLevel
{
    Info = 0,
    Warning = 1,
    Error = 2,
};

NBSAPI std::string log_level_str(LogLevel level);
NBSAPI void log(LogLevel level, const std::string &message);
NBSAPI void info(const std::string &message);
NBSAPI void warning(const std::string &message);
NBSAPI void error(const std::string &message);
} // namespace log

namespace graph
{
template <typename T>
using Edges = std::unordered_set<T>;

template <typename T>
using Graph = std::unordered_map<T, Edges<T>>;

enum GraphError
{
    CycleDependency,
    VertexNotFound
};

template <typename T>
NBSAPI std::unordered_set<T> find_roots(const Graph<T> &graph);

template <typename T>
NBSAPI err::Result<std::vector<std::vector<T>>, GraphError> topological_levels(
    const Graph<T> &graph, const Edges<T> &roots);
} // namespace graph

namespace target
{
enum BuildError
{
    BUILD_CMD_ERROR,
    BUILD_NO_RULE_FOR_TARGET_ERROR,
    BUILD_CYCLE_DEPENDENCY_ERROR,
};

struct Target
{
    os::path output;
    std::vector<os::Cmd> cmds;
    os::pathvec dependencies;

    Target(const os::path &output, const os::Cmd &cmd, const os::pathvec &dependencies = {});
    Target(const os::path &output, const std::vector<os::Cmd> &cmds, const os::pathvec &dependencies = {});

    err::Result<void, os::ProcessError> build() const;
};

struct TargetMap
{
    std::unordered_map<std::string, Target> targets;

    TargetMap() = default;

    void insert(Target &target);
    bool remove(const std::string &target);
    err::Result<void, BuildError> build(const std::string &output) const;
    err::Result<void, BuildError> build_if_needs(const std::string &output) const;
    bool needs_rebuild(const std::string &output) const;
};
} // namespace target

namespace c
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

struct CDefaults
{
    Compiler compiler = CXX;
    std::string standard;
    strvec flags;
    os::pathvec include_paths;
    os::pathvec libs;
    os::pathvec lib_paths;
    strvec defines;
    strvec other_flags;
};

NBSAPI CDefaults *get_cdefaults();
struct CompileOptions
{
    Compiler compiler = get_cdefaults()->compiler;
    std::string standard = get_cdefaults()->standard;
    strvec flags = get_cdefaults()->flags;
    os::pathvec include_paths = get_cdefaults()->include_paths;
    os::pathvec libs = get_cdefaults()->libs;
    os::pathvec lib_paths = get_cdefaults()->lib_paths;
    strvec defines = get_cdefaults()->defines;
    strvec other_flags = get_cdefaults()->other_flags;

    os::Cmd cmd(const os::pathvec &sources, const strvec &additional_flags = {}) const;
    os::Cmd exe_cmd(const os::path &output, const os::pathvec &sources) const;
    os::Cmd obj_cmd(const os::path &output, const os::path &source) const;
    os::Cmd static_lib_cmd(const os::pathvec &sources) const;  // TODO
    os::Cmd dynamic_lib_cmd(const os::pathvec &sources) const; // TODO
};

NBSAPI std::string comp_str(Compiler comp);
NBSAPI Compiler current_compiler();

} // namespace c
}; // namespace nbs

// -------------------------------
//
//        Implementation
//
// -------------------------------

#ifdef NBS_IMPLEMENTATION

namespace nbs
{
namespace err
{
template <typename T>
Ok<T>::Ok(const T &value)
    : value(value)
{
}

template <typename T>
Err<T>::Err(const T &value)
    : value(value)
{
}

template <typename V, typename E>
constexpr Result<V, E>::Result()
    : variant()
{
}

template <typename V, typename E>
constexpr Result<V, E>::Result(const Result &other)
    : variant(other.variant)
{
}

template <typename V, typename E>
constexpr Result<V, E>::Result(const std::variant<V, E> &variant)
    : variant(variant)
{
}

template <typename V, typename E>
constexpr Result<V, E>::Result(const Ok<V> &ok)
    : variant{std::in_place_index<0>, ok.value}
{
}

template <typename V, typename E>
constexpr Result<V, E>::Result(const Err<E> &err)
    : variant{std::in_place_index<1>, err.value}
{
}

template <typename V, typename E>
Result<V, E> constexpr &Result<V, E>::operator=(const Result<V, E> &other)
{
    variant = other.variant;
    return *this;
}

template <typename V, typename E>
bool constexpr Result<V, E>::is_ok() const
{
    return variant.index() == 0;
}

template <typename V, typename E>
bool constexpr Result<V, E>::is_err() const
{
    return variant.index() == 1;
}

template <typename V, typename E>
V constexpr Result<V, E>::value() const
{
    if (!is_ok())
        throw BadResultException();
    return std::get<0>(variant);
}

template <typename V, typename E>
V constexpr Result<V, E>::value_or(const V &default_value) const
{
    if (auto *v = std::get_if<0>(variant))
    {
        return *v;
    }
    else
    {
        return default_value;
    }
}

template <typename V, typename E>
E constexpr Result<V, E>::error() const
{
    if (!is_err())
        throw BadResultException();
    return std::get<1>(variant);
}

template <typename V, typename E>
V constexpr &Result<V, E>::operator*()
{
    return *std::get_if<0>(&variant);
}

template <typename V, typename E>
V constexpr *Result<V, E>::operator->()
{
    return std::get_if<0>(&variant);
}

template <typename V, typename E>
template <typename NewV>
Result<NewV, E> Result<V, E>::bind(std::function<Result<NewV, E>(V)> func)
{
    if (auto value = std::get_if<0>(&variant))
    {
        return func(*value);
    }
    else
    {
        return err::Err(std::get<1>(variant));
    }
}

template <typename E>
constexpr Result<void, E>::Result(const Result &other)
    : m_error(other.m_error)
{
}

template <typename E>
constexpr Result<void, E>::Result(const Ok<void> &)
    : m_error(std::nullopt)
{
}

template <typename E>
constexpr Result<void, E>::Result(const Err<E> &err)
    : m_error(err.value)
{
}

template <typename E>
Result<void, E> constexpr &Result<void, E>::operator=(const Result &other)
{
    m_error = other.m_error;
    return *this;
}

template <typename E>
bool constexpr Result<void, E>::is_ok() const
{
    return !m_error.has_value();
}

template <typename E>
bool constexpr Result<void, E>::is_err() const
{
    return m_error.has_value();
}

template <typename E>
E constexpr Result<void, E>::error() const
{
    if (!is_err())
        throw BadResultException();
    return *m_error;
}

const char *BadResultException::what() const noexcept
{
    return "Attempt to access bad result";
}
} // namespace err
NBSAPI void self_update(int argc, char **argv, const std::string &source)
{
    assert(argc > 0);
    std::string exe(argv[0]);

    if (os::compare_last_mod_time(source, exe) < 0)
        return;

    log::info("Updating");
    std::filesystem::rename(exe, exe + ".old");

    os::Cmd compile_cmd;
    compile_cmd.append_many({c::comp_str(c::current_compiler()), source});
#ifdef _MSC_VER
    cmd.append_many({"-std:c++20", "-Fe:" + exe, "-FC", "-EHsc", "-nologo"});
#else
    compile_cmd.append_many({"-o", exe});
#endif
    compile_cmd.run_or_die("Error during self_update!!!");

    os::Cmd exe_cmd(exe);
    for (int i = 1; i < argc; i++)
    {
        exe_cmd.append(argv[i]);
    }
    exe_cmd.run();

    exit(0);
}

namespace os
{
#ifdef _WIN32
Process::Process(HANDLE handle)
    : handle(handle)
{
}
#else
Process::Process(int pid)
    : pid(pid)
{
}
#endif

err::Result<void, ProcessError> Process::await() const
{
#ifdef _WIN32
    DWORD result = WaitForSingleObject(handle, INFINITE);

    if (result == WAIT_FAILED)
    {
        log::error(os::windows_last_error_str());
        return err::Err(PROCESS_WAIT_ERROR);
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(handle, &exit_status))
        return err::Err(PROCESS_GET_EXIT_CODE_ERROR);

    if (exit_status != 0)
        return err::Err(PROCESS_EXIT_STATUS_ERROR);

    CloseHandle(handle);

    return err::Ok();
#else
    while (true)
    {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0)
        {
            return err::Err(PROCESS_WAIT_ERROR);
        }

        if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 0)
                return err::Ok();
            else
                return err::Err(PROCESS_EXIT_STATUS_ERROR);
        }

        if (WIFSIGNALED(status))
        {
            return err::Err(PROCESS_WAIT_ERROR);
        }
    }
#endif
}

NBSAPI err::Result<void, ProcessError> await_processes(const std::vector<Process> &processes)
{
    err::Result<void, ProcessError> result = err::Ok();

    for (const auto &proc : processes)
    {
        result = proc.await();
        if (result.is_err())
        {
            return result;
        }
    }
    return result;
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

err::Result<void, ProcessError> Cmd::run() const
{
    return run_async().bind<void>(
        [](auto p) { return p.await(); });
}

err::Result<Process, ProcessError> Cmd::run_async() const
{
    if (items.empty())
        return err::Err(PROCESS_EMPTY_CMD_ERROR);

    std::string args_str = to_string();
    std::cout << args_str << '\n';

#ifdef _WIN32
    char *args = (char *)args_str.c_str(); // TODO: Proper Cmd.to_string
    STARTUPINFO startupinfo;
    ZeroMemory(&startupinfo, sizeof(startupinfo));
    startupinfo.cb = sizeof(startupinfo);

    startupinfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (startupinfo.hStdInput == INVALID_HANDLE_VALUE)
    {
        return err::Err(PROCESS_GET_HANDLE_ERROR);
    }

    startupinfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (startupinfo.hStdOutput == INVALID_HANDLE_VALUE)
    {
        return err::Err(PROCESS_GET_HANDLE_ERROR);
    }

    startupinfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (startupinfo.hStdError == INVALID_HANDLE_VALUE)
    {
        return err::Err(PROCESS_GET_HANDLE_ERROR);
    }

    startupinfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));

    BOOL success = CreateProcessA(NULL, args, NULL, NULL, TRUE, 0, NULL, NULL, &startupinfo, &process_info);
    if (!success)
    {
        return err::Err(PROCESS_CREATE_ERROR);
    }

    CloseHandle(process_info.hThread);

    return process_info.hProcess;
#else
    int p = fork();
    if (p < 0)
    {
        return err::Err(PROCESS_CREATE_ERROR);
    }
    else if (p == 0)
    {
        auto args = to_c_argv();
        execvp(args[0], args.get());
        return err::Err(PROCESS_EXEC_ERROR);
    }

    return err::Ok(Process(p));
#endif
}

void Cmd::run_or_die(const std::string &message) const
{
    if (run().is_err())
    {
        nbs::log::error(message);
        exit(1);
    }
}

std::unique_ptr<char *[]> Cmd::to_c_argv() const
{
    auto result = std::make_unique<char *[]>(items.size() + 1);
    for (size_t i = 0; i < items.size(); i++)
    {
        result[i] = (char *)items[i].c_str();
    }
    result[items.size()] = NULL;
    return result;
}

#ifdef _WIN32
std::string windows_error_code_to_str(DWORD error)
{
    // https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror

    LPSTR messageBuffer = nullptr;

    DWORD size = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

    return messageBuffer;
}

std::string windows_last_error_str()
{
    DWORD error = GetLastError();
    return windows_error_code_to_str(error);
}
#endif

NBSAPI strvec paths_to_strs(const pathvec &paths)
{
    strvec result;
    for (const path &path : paths)
    {
        result.push_back(path);
    }
    return result;
}

NBSAPI pathvec strs_to_paths(const strvec &paths)
{
    size_t size = paths.size();
    pathvec result(size);
    std::copy_n(paths.begin(), size, result.begin());
    return result;
}

NBSAPI long compare_last_mod_time(const path &path1, const path &path2)
{
    auto time1 = std::filesystem::last_write_time(path1);
    auto time2 = std::filesystem::last_write_time(path2);
    auto diff = time1 - time2;
    return diff.count();
}

NBSAPI bool make_directory_if_not_exists(const path &path)
{
    return std::filesystem::create_directory(path);
}

NBSAPI bool exists(const path &path)
{
    return std::filesystem::exists(path);
}
} // namespace os

namespace str
{
NBSAPI std::string join(const std::string &sep, const strvec &strings)
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

NBSAPI std::string trim_to(const std::string &str, const std::string &chars)
{
    return trim_right_to(trim_left_to(str), chars);
}

NBSAPI std::string trim_right_to(const std::string &str, const std::string &chars)
{
    size_t count = str.size();
    while (count > 0 && chars.find(str[count - 1]) == std::string::npos)
    {
        count--;
    }
    return str.substr(0, count);
}

NBSAPI std::string trim_left_to(const std::string &str, const std::string &chars)
{
    size_t index = 0;
    while (index < str.size() && chars.find(str[index]) == std::string::npos)
    {
        index++;
    }
    return str.substr(index);
}

NBSAPI strvec split(const std::string &str, const std::string &delim)
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

NBSAPI std::string change_extension(const std::string &file, const std::string &new_extension)
{
    return trim_right_to(file, ".") + new_extension;
}
} // namespace str

namespace log
{
static LogLevel minimal_level = Info;

NBSAPI std::string log_level_str(LogLevel level)
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

NBSAPI void log(LogLevel level, const std::string &message)
{
    if (level < minimal_level)
        return;

    (level == Info ? std::cout : std::cerr) << '[' << log_level_str(level) << "] " << message << '\n';
}

NBSAPI void info(const std::string &message)
{
    log(Info, message);
}

NBSAPI void warning(const std::string &message)
{
    log(Warning, message);
}

NBSAPI void error(const std::string &message)
{
    log(Error, message);
}

} // namespace log

namespace graph
{
template <typename T>
NBSAPI std::unordered_set<T> find_roots(const Graph<T> &graph)
{
    std::unordered_set<T> result;

    for (const auto &pair : graph)
    {
        result.insert(pair.first);
    }

    for (const auto &pair : graph)
    {
        for (const auto &vertex : pair.second)
        {
            result.erase(vertex);
        }
    }

    return result;
}

template <typename T>
NBSAPI err::Result<std::vector<std::vector<T>>, GraphError> topological_levels(
    const Graph<T> &graph, const Edges<T> &roots)
{
    class CycleException : public std::exception
    {
    };
    class VertexNotFoundException : public std::exception
    {
    };

    struct Vertex
    {
        const T &name;
        const std::unordered_set<T> &edges;
        std::optional<ssize_t> level;
        bool traversing;

        Vertex(const T &name, const Edges<T> &edges)
            : name(name), edges(edges), level(std::nullopt), traversing(false)
        {
        }
        Vertex(const T &name, const Edges<T> &edges, ssize_t level)
            : name(name), edges(edges), level(level), traversing(false)
        {
        }
    };

    std::unordered_map<T, Vertex> vertices;
    for (const auto &pair : graph)
    {
        vertices.insert({pair.first, Vertex(pair.first, pair.second)});
    }

    ssize_t max_level = 0;

    std::function<void(Vertex &, ssize_t)> do_search =
        [&](Vertex &vertex, ssize_t level) {
            if (vertex.traversing)
                throw CycleException();

            vertex.traversing = true;

            if (vertex.level.value_or(-1) <= level)
                vertex.level = level;

            for (const T &edge : vertex.edges)
            {
                auto vertex_search = vertices.find(edge);
                if (vertex_search == vertices.end())
                    throw VertexNotFoundException();
                Vertex &v = vertex_search->second;

                do_search(v, level + 1);
            }

            vertex.traversing = false;

            if (level > max_level)
                max_level = level;
        };

    Vertex root("", roots);

    try
    {
        do_search(root, -1);
    }
    catch (CycleException *)
    {
        return err::Err(CycleDependency);
    }
    catch (VertexNotFoundException *)
    {
        return err::Err(VertexNotFound);
    }

    std::vector<std::vector<T>> result(max_level + 1);

    for (const auto &v : vertices)
    {
        if (!v.second.level.has_value())
            continue;

        if (*v.second.level < 0)
            continue;

        result[*v.second.level].emplace_back(v.first);
    }

    return err::Ok(result);
}
} // namespace graph

namespace target
{
Target::Target(const os::path &output, const os::Cmd &cmd, const os::pathvec &dependencies)
    : output(output), cmds({cmd}), dependencies(dependencies)
{
}
Target::Target(const os::path &output, const std::vector<os::Cmd> &cmds, const os::pathvec &dependencies)
    : output(output), cmds(cmds), dependencies(dependencies)
{
}

err::Result<void, os::ProcessError> Target::build() const
{
    err::Result<void, os::ProcessError> result;
    for (const auto &cmd : cmds)
    {
        result = cmd.run();
        if (result.is_err())
            return result;
    }
    return result;
}

void TargetMap::insert(Target &target)
{
    targets.insert({target.output, target});
}

bool TargetMap::remove(const std::string &target_output)
{
    return targets.erase(target_output) > 0;
}

err::Result<void, BuildError> TargetMap::build(const std::string &output) const
{
    auto target_it = targets.find(output);
    if (target_it == targets.end())
        return err::Err(BUILD_NO_RULE_FOR_TARGET_ERROR);
    auto target = target_it->second;

    for (const auto &dep : target.dependencies)
    {
        if (os::exists(dep))
            continue;

        auto result = build(dep);
        if (result.is_err())
            return result;
    }

    auto build_result = target.build();
    if (build_result.error())
        return err::Err(BUILD_CMD_ERROR);

    return err::Ok();
}

err::Result<void, BuildError> TargetMap::build_if_needs(const std::string &output) const
{
    if (!needs_rebuild(output))
        return err::Ok();

    auto target = targets.find(output)->second; // TODO: error handling
    graph::Graph<std::string> graph;
    for (const auto &p : targets)
    {
        graph::Edges<std::string> edges;
        for (const auto &e : p.second.dependencies)
        {
            edges.insert(e);
            graph[e];
        }
        graph[p.first] = edges;
    }
    auto levels_result = graph::topological_levels(graph, {target.output});

    if (levels_result.is_err())
    {
        switch (levels_result.error())
        {
        case graph::CycleDependency:
            return err::Err(BUILD_CYCLE_DEPENDENCY_ERROR);
        case graph::VertexNotFound:
            return err::Err(BUILD_NO_RULE_FOR_TARGET_ERROR);
        default:
            assert(0 && "Unreachable");
        }
    }

    auto levels = *levels_result;

    for (ssize_t i = levels_result->size() - 1; i >= 0; i--)
    {
        std::vector<os::Process> processes;
        for (const auto &t_name : levels[i])
        {
            if (auto t_search = targets.find(t_name); t_search != targets.end())
            {
                if (!needs_rebuild(t_name))
                    continue;
                auto t = t_search->second;
                auto p_result = t.cmds[0].run_async();
                if (p_result.is_err())
                    return err::Err(BUILD_CMD_ERROR);
                processes.emplace_back(p_result.value()); // TODO: multiple commands
            }
            else if (!os::exists(t_name))
            {
                return err::Err(BUILD_NO_RULE_FOR_TARGET_ERROR);
            }
        }
        auto await_result = await_processes(processes);
        if (await_result.is_err())
            return err::Err(BUILD_CMD_ERROR);
    }
    return err::Ok();
}

bool TargetMap::needs_rebuild(const std::string &output) const
{
    if (!std::filesystem::exists(output))
        return true;

    auto target_it = targets.find(output);
    if (target_it == targets.end())
        TODO("needs_rebuild error handling");
    auto target = target_it->second;

    for (const auto &dep : target.dependencies)
    {
        if (((targets.find(dep) != targets.end()) && needs_rebuild(dep)) ||
            (((targets.find(dep) != targets.end()) && !std::filesystem::exists(dep)) ||
             (std::filesystem::exists(dep) && (os::compare_last_mod_time(output, dep) < 0))))
        {
            return true;
        }
    }
    return false;
}
} // namespace target

namespace c
{
static CDefaults cdefaults;

NBSAPI std::string comp_str(Compiler comp)
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
        return "cl.exe";
    default:
        return "UNKNOWN COMPILER";
    }
}

NBSAPI Compiler current_compiler()
{
//  Clang C++ emulates GCC, so it has to appear early.
#if defined __clang__ && !defined(__ibmxl__) && !defined(__CODEGEARC__)
    return CLANG;

// MINGW32
#elif defined(__MINGW32__)
    TODO("mingw")

//  GNU C++:
#elif defined(__GNUC__) && !defined(__ibmxl__)
    return GXX;

//  Microsoft Visual C++
#elif defined _MSC_VER
    return MSVC;
#endif

    TODO("Unknown compiler");
}

NBSAPI CDefaults *get_cdefaults()
{
    return &cdefaults;
}

os::Cmd CompileOptions::cmd(const os::pathvec &sources, const strvec &additional_flags) const
{
    // TODO: fucking windows pain
    os::Cmd cmd;

    cmd.append(comp_str(compiler));
    if (!standard.empty())
        cmd.append("-std=" + standard);

    cmd.append_many(os::paths_to_strs(sources));
    cmd.append_many(additional_flags);

    cmd.append_many(flags);
    cmd.append_many_prefixed("-I", os::paths_to_strs(include_paths));
    cmd.append_many_prefixed("-D", defines);
    cmd.append_many(other_flags);

    cmd.append_many_prefixed("-L", os::paths_to_strs(lib_paths));
    cmd.append_many_prefixed("-l", os::paths_to_strs(libs));

    return cmd;
}

os::Cmd CompileOptions::exe_cmd(const os::path &output, const os::pathvec &sources) const
{
    strvec additional_flags;
    if (compiler == MSVC)
    {
        additional_flags.emplace_back("-Fe:" + output.string());
    }
    else
    {
        additional_flags.emplace_back("-o");
        additional_flags.emplace_back(output);
    }

    return this->cmd(sources, additional_flags);
}

os::Cmd CompileOptions::obj_cmd(const os::path &output, const os::path &source) const
{
    strvec additional_flags;
    additional_flags.emplace_back("-c");
    if (compiler == MSVC)
    {
        additional_flags.emplace_back("-Fo:" + output.string());
    }
    else
    {
        additional_flags.emplace_back("-o");
        additional_flags.emplace_back(output);
    }
    return this->cmd({source}, additional_flags);
}
} // namespace c
} // namespace nbs
#endif // NBS_IMPLEMENTATION
#endif // NBS_HPP
