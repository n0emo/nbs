#define ABUILD_IMPLEMENTATION
#include "abuild.hpp"

using namespace ab;
using namespace log;

strvec flags{"-Wall", "-Wextra", "-pedantic"};
strvec include_paths{"."};
c::Compiler compiler = c::GXX;

void compile_object_file(std::string file, strvec defines = {})
{
    c::CompileOptions options{.compiler = compiler, .flags = flags, .include_paths = include_paths, .defines = defines};
    options.obj_cmd(file).run_or_die("Error during compilation");
}

void compile_executable(std::string file)
{
    c::CompileOptions options{.compiler = compiler};
    options.exe_cmd("hello", {"hello.o"}).run_or_die("Error during compilation");
}

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);

    info("Hello from Abuild");

    compile_object_file("hello.cpp");
    compile_object_file("abuild.hpp", {"ABUILD_IMPLEMENTATION"});
    compile_executable("hello.o");

    info("Compilation successful");

    return 0;
}
