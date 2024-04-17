#include <exception>
#define NBS_IMPLEMENTATION
#include "../nbs.hpp"

using namespace std;
using namespace nbs;
using namespace nbs::os;
using namespace nbs::str;

int main(int argc, char **argv)
{
    self_update(argc, argv, __BASE_FILE__);

    make_directory_if_not_exists("build");

    strvec sources{"App.cpp", "Csv.cpp", "CsvParser.cpp", "main.cpp", "sort.cpp"};
    c::CompileOptions options{.compiler = c::GXX,
                              .standard = "c++20",
                              .flags = {"-Wall", "-Wextra", "-pedantic", "-g"},
                              .include_paths = {path("include")}};
    pathvec objects;
    for (const auto &source : sources)
    {
        path input = path("src") / source;
        path output = path("build") / change_extension(source, "o");
		auto run_result = options.obj_cmd(output, input).run();
		objects.emplace_back(output);
		if(run_result.is_err())
			std::terminate();
    }

    path exe("build/lab1");
    c::CompileOptions{.compiler = c::GXX}.exe_cmd(exe, objects).run();
}
