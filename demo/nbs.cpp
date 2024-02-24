#define NBS_IMPLEMENTATION
#include "../nbs.hpp"

using namespace std;
using namespace nbs;
using namespace nbs::os;
using namespace nbs::str;

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);

    make_directory_if_not_exists("build");

    strvec sources{"App.cpp", "Csv.cpp", "CsvParser.cpp", "main.cpp", "sort.cpp"};
    c::CompileOptions options{.compiler = c::GXX,
                              .standard = "c++20",
                              .flags = {"-Wall", "-Wextra", "-pedantic", "-O3"},
                              .include_paths = {Path("include")}};
    std::vector<Process> processes;
    pathvec objects;
    for (const auto &source : sources)
    {
        Path input = Path({"src", source});
        Path output = Path({"build", change_extension(source, "o")});
        objects.emplace_back(output);
        processes.emplace_back(options.obj_cmd(output, input).run_async());
    }

    await_processes(processes);
    Path exe("build/lab1");
    c::CompileOptions{.compiler = c::GXX}.exe_cmd(exe, objects).run();
}
