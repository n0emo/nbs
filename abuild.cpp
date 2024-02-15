#define ABUILD_IMPLEMENTATION
#include "abuild.hpp"

using namespace ab;
using namespace log;

strvec flags{"-Wall", "-Wextra", "-pedantic"};
strvec include_paths{"."};
c::Compiler compiler = c::GCC;

void compile_object_file(std::string file, strvec defines = {})
{
    c::ObjectTarget target(file, compiler, flags, include_paths, defines);
    target.cmd().run_or_die("Error during compilation");
}

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);

    info("Hello from Abuild");

    compile_object_file("hello.cpp");
    compile_object_file("abuild.hpp", {"ABUILD_IMPLEMENTATION"});

    info("Compilation successful");

    return 0;
}
