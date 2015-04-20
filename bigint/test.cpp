#include "bigint.h"
#include <iostream>

int main(int argc, char *argv[])
{
    Mouse::bigint a;
    Mouse::bigint b;

    while (std::cin >> a >> b) {
        std::cout << (a * b) << std::endl;
    }

    return 0;
}

