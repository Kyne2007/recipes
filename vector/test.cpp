#include <iostream>
#include "vector.h"

int main(int argc, char *argv[])
{
    Mouse::vector<int> v;

    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    for (auto it = v.begin(); it != v.end(); ++it)
        std::cout << *it << std::endl;

    return 0;
}
