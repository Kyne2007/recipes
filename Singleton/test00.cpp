#include "Singleton.h"
#include <iostream>
using namespace std;

class Foo
{
public:
    void set(int n)
    {
        a_ = n;
    }

    friend ostream& operator<<(ostream& os, Foo& foo);
    int a_;
};

ostream& operator<<(ostream& os, Foo& foo)
{
    os << foo.a_;
    return os;
}

int main(int argc, char *argv[])
{
    Foo &foo = Mouse::Singleton<Foo>::instance();
    cout << foo << endl;
    foo.set(4);

    Foo &f = Mouse::Singleton<Foo>::instance();
    cout << f <<endl;

    return 0;
}

