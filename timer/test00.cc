#include "timer.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    Mouse::Timer time;
    for (int i = 0; i < 10000000; i++);
    cout << time.elapsed_micro() << endl;
    return 0;
}
