#include <unordered_map>
#include <unordered_set>
#define NBS_IMPLEMENTATION
#include "nbs.hpp"

using namespace nbs;
using namespace log;
using namespace os;

void build()
{
    c::CompileOptions options{
        .compiler = c::current_compiler(), .flags = {"-Wall"},
        // .include_paths = {Path(".")},
    };

    auto obj_proc = options.obj_cmd("hello.o", "hello.cpp").run_async();
    if (obj_proc.bind<void>([](auto p) { return p.await(); }).is_err())
        return;

    auto exe_proc = options.exe_cmd("hello", {"hello.o"}).run_async();
    exe_proc.bind<void>([](auto p) { return p.await(); });
}

void test_levels()
{
    std::unordered_map<std::string, std::unordered_set<std::string>> graph;
    graph.insert({"1", {"2", "3", "4"}});
    graph.insert({"2", {"4"}});
    graph.insert({"3", {"4"}});
    graph.insert({"4", {}});
    graph.insert({"5", {}});

    auto roots = nbs::graph::find_roots(graph);
    auto result = nbs::graph::topological_levels<std::string>(graph, roots);
    if (result.is_err())
    {
        switch (result.error())
        {
        case nbs::graph::CycleDependency:
            std::cout << "Cycle dependency detected" << std::endl;
            break;
        case nbs::graph::VertexNotFound:
            std::cout << "Vertex not found" << std::endl;
            break;
        }

        return;
    }

    auto levels = result.value();

    for (const auto &pair : graph)
    {
        std::cout << pair.first << " ->";
        for (const auto &edge : pair.second)
        {
            std::cout << " " << edge;
        }
        std::cout << '\n';
    }

    for (size_t level = 0; level < levels.size(); level++)
    {
        std::cout << level << ":";
        for (const auto &vertex : levels[level])
        {
            std::cout << " " << vertex;
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    self_update(argc, argv, __FILE__);
    log::info("Testing levels");
    test_levels();
    return 0;

    os::path build_path("build");
    info("nbs" / build_path / "debug");
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
