#define NBS_IMPLEMENTATION
#include "nbs.hpp"

using namespace nbs;
using namespace log;
using namespace os;

void build()
{
    c::CompileOptions options{
        .compiler = c::GXX,
        .flags = {"-Wall", "-Wextra", "-pedantic"},
        .include_paths = {Path(".")},
    };

    auto obj_proc = options.obj_cmd("hello.o", "hello.cpp").run_async();
    if (!obj_proc.await())
        return;
    auto exe_proc = options.exe_cmd("hello", {"hello.o"}).run_async();
    exe_proc.await();
}

int main(int argc, char **argv)
{
    os::Path build_path("build");
    info(("nbs" + build_path + "debug").str());
    self_update(argc, argv, __FILE__);
    info("Starting build");

    std::string subcommand;
    if (argc == 1 || (subcommand = argv[1]) == "build")
    {
        build();
    }
    else if (subcommand == "clean")
    {
        Cmd({"rm", "-f", "*.o", "hello"}).run_or_die("Error cleaning directory");
    }
    else if (subcommand == "run")
    {
        build();
        Cmd("./hello").run_or_die("Error executing hello");
    }

    return 0;
}
