#define NBS_IMPLEMENTATION
#include "nbs.hpp"

using namespace nbs;
using namespace nbs::err;

enum GetError
{
    LESS_THAN_ZERO
};

Result<std::string, GetError> get_smth(int a)
{
    if (a > 0)
        return Ok("The value was " + std::to_string(a));
    else
        return Err(LESS_THAN_ZERO);
}

int main()
{
    auto result = get_smth(-23);
    if (result.is_ok())
    {
        std::cout << result.value() << std::endl;
    }
    else
    {
        std::cout << result.error() << std::endl;
    }
}
