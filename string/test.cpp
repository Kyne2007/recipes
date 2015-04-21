#include "string.h"
#include <iostream>

int main(int argc, char *argv[])
{
    Mouse::string s;
    while (std::cin >> s) {
        std::cout << s << std::endl;
    }

    return 0;
}
