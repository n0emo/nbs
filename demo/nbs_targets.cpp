#include <exception>
#define NBS_IMPLEMENTATION
#include "../nbs.hpp"

using namespace std;
using namespace nbs;
using namespace os;
using namespace str;
using namespace target;

enum ConfType
{
    DEBUG,
    RELEASE
};

auto build(int argc, char **argv)
{
    log::info("Building");

    std::string conf_str;
    ConfType configuration;
    if (argc > 2)
        conf_str = argv[2];

    if (conf_str.empty() || conf_str == "debug")
        configuration = DEBUG;
    else if (conf_str == "release")
        configuration = RELEASE;
    else
    {
        log::error("Unknown configuration '" + conf_str + "'");
		std::terminate();
    }

    TargetMap targets;
    strvec sources{"App.cpp", "Csv.cpp", "CsvParser.cpp", "main.cpp", "sort.cpp"};
    c::CompileOptions options{.compiler = c::GXX,
                              .standard = "c++20",
                              .flags = {"-Wall", "-Wextra", "-pedantic"},
                              .include_paths = {path("include")}};

    path build_path("build");
    make_directory_if_not_exists(build_path);
    switch (configuration)
    {
    case DEBUG:
        build_path = build_path / "debug";
        options.flags.emplace_back("-g");
        break;
    case RELEASE:
        build_path = build_path / "relese";
        options.flags.emplace_back("-O3");
        break;
    }
    make_directory_if_not_exists(build_path);

    pathvec outputs;
    for (const auto &source : sources)
    {
        path output = build_path / change_extension(source, "o");
        outputs.emplace_back(output);
        path input = path("src") / source;
        Cmd cmd(options.obj_cmd(output, input));
        Target target(output, cmd, {input});
        targets.insert(target);
    }

    path exe = build_path / "lab1";
    Cmd exe_cmd = c::CompileOptions{.compiler = c::GXX}.exe_cmd(exe, outputs);
    Target exe_target(exe, exe_cmd, outputs);
    targets.insert(exe_target);

    return targets.build_if_needs(exe);
}

auto run()
{
    auto result = Cmd("./build/debug/lab1").run();
    if (result.is_err())
        log::error("Error running file");
    return result;
}

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);

    string subcommand = argc > 1 ? argv[1] : "";

    if (subcommand == "" || subcommand == "build")
    {
        return build(argc, argv).is_err();
    }
    else if (subcommand == "run")
    {
        auto result = build(argc, argv);
        if (result.is_err())
            return 1;
        return run().is_err();
    }
    else
    {
        log::error("Unknown subcommand '" + subcommand + "'");
        return 1;
    }

    return 0;
}
