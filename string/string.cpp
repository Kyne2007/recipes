#include "string.h"
#include <algorithm>
#include <cstdio>
#include <cctype>

namespace Mouse {
    std::istream &operator>>(std::istream &is, string &s)
    {
        char *str = new char[10];
        char c;

        while (is.get(c)) {
            if (!isspace(c))
                break;
        }

        string::size_type i, size;
        str[0] = c;

        for (i = 1, size = 10; is.get(c); ++i) {
            if (isspace(c))
                break;

            if (i == size) {
                char *tmp = new char[size + 10];
                std::copy_n(str, size, tmp);
                delete [] str;
                str = tmp;
                size += 10;
            }

            str[i] = c;
        }

        s.size_ = i;
        s.data_ = new char[s.size_ + 1];
        std::copy_n(str, s.size_, s.data_);
        s.data_[s.size_] = '\0';

        delete [] str;

        return is;
    }

    std::ostream &operator<<(std::ostream &os, const string &s)
    {
        os << s.data_;
        return os;
    }

    string operator+(const string &lhs, const string &rhs)
    {
        string ret;

        ret.size_ = lhs.size() + rhs.size();
        delete [] ret.data_;
        ret.data_ = new char[ret.size_ + 1];
        std::copy_n(lhs.c_str(), lhs.size(), ret.c_str());
        std::copy_n(rhs.c_str(), rhs.size(), ret.data_ + lhs.size());
        ret.data_[ret.size_] = '\0';

        return ret;
    }

    bool operator==(const string &lhs, const string &rhs)
    {
        return !strcmp(lhs.data_, rhs.data_);
    }

    bool operator!=(const string &lhs, const string &rhs)
    {
        return (lhs == rhs);
    }

    bool operator<(const string &lhs, const string &rhs)
    {
        return (strcmp(lhs.data_, rhs.data_) < 0);
    }

    bool operator>(const string &lhs, const string &rhs)
    {
        return (strcmp(lhs.data_, rhs.data_) > 0);
    }
}

