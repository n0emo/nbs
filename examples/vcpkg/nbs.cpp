#define NBS_IMPLEMENTATION
#include "nbs.hpp"

using namespace nbs;

int main(int argc, char **argv) {
    self_update(argc, argv, __FILE__);

#ifdef _WIN32
    std::string triplet("x64-windows-static");
#else
    std::string triplet("x64-linux-static");
#endif
    vcpkg::Vcpkg v = vcpkg::Vcpkg().with_triplet(triplet);
    v.install();

    c::CompileOptions options;
    options.compiler = c::CLANG;
    auto ipaths = v.include_paths();
    options.include_paths.insert(options.include_paths.end(), ipaths.begin(), ipaths.end());

    auto lpaths = v.library_paths();
    options.lib_paths.insert(options.lib_paths.end(), lpaths.begin(), lpaths.end());

    options.libs.push_back("raylib");
    options.libs.push_back("glfw3");

#ifdef _WIN32
    options.libs.push_back("OpenGL32");
    options.libs.push_back("user32");
    options.libs.push_back("shell32");
    options.libs.push_back("windowsapp");
    options.libs.push_back("gdi32");
#else
    options.libs.push_back("gl");
#endif
    options.exe_cmd("app", {"src/main.c"}).run();

    return 0;
}
