#define NBS_IMPLEMENTATION
#include "nbs.hpp"

int main(int argc, char **argv)
{
    nbs::self_update(argc, argv, __FILE__);
    nbs::Cmd({"g++", "hello.cpp", "-o", "hello"})
        .run_or_die("Error during compilation");
    return 0;
}
