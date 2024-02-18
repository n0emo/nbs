#define ABUILD_IMPLEMENTATION
#include "../nbs.hpp"

using namespace std;
using namespace nbs;

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);

    make_directory_if_not_exists("build");

    strvec sources{"App.cpp", "Csv.cpp", "CsvParser.cpp", "main.cpp", "sort.cpp"};
    c::CompileOptions options{.compiler = c::GXX,
                              .standard = "c++20",
                              .flags = {"-Wall", "-Wextra", "-pedantic", "-O3"},
                              .include_paths = {"include"}};
    std::vector<Process> processes;
    strvec objects;
    for (const auto &source : sources)
    {
        std::string input = path({"src", source});
        std::string output = path({"build", change_extension(source, "o")});
        objects.emplace_back(output);
        processes.emplace_back(options.obj_cmd(output, input).run_async());
    }

    await_processes(processes);
    std::string exe = path({"build", "lab1"});
    c::CompileOptions{.compiler = c::GXX}.exe_cmd(exe, objects).run();
}
