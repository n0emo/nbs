#include <unordered_map>
#include <unordered_set>
#define NBS_IMPLEMENTATION
#include "nbs.hpp"

using namespace nbs;
using namespace log;
using namespace os;


void test_levels()
{
    std::unordered_map<std::string, std::unordered_set<std::string>> graph;
    graph.insert({"1", {"2", "3", "4"}});
    graph.insert({"2", {"7"}});
    graph.insert({"3", {"7"}});
    graph.insert({"4", {}});
    graph.insert({"5", {}});

    auto roots = nbs::graph::find_roots(graph);
    auto result = nbs::graph::topological_levels<std::string>(graph, {"1"});
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

	std::vector<std::string> strs {"xd", "lol", "kekwait"};
	auto res = fp::map([](auto s){ return s.size();}, strs);
	for(auto n : res)
	{
		std::cout << n << std::endl;
	}
    return 0;
}
