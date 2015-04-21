#ifndef MOUSE_STRING_H_
#define MOUSE_STRING_H_

#include <cstddef>
#include <cstring>
#include <utility>
#include <iostream>

namespace Mouse {
    class string {
        friend std::istream &operator>>(std::istream &is, string &str);
        friend std::ostream &operator<<(std::ostream &os, const string &str);

        friend bool operator==(const string &lhs, const string &rhs);
        friend bool operator!=(const string &lhs, const string &rhs);
        friend bool operator<(const string &lhs, const string &rhs);
        friend bool operator>(const string &lhs, const string &rhs);

        friend string operator+(const string &lhs, const string &rhs);

        public:
            typedef size_t size_type;

        public:
            string() : data_(new char[1]), size_(0)
            {
                *data_ = '\0';
            }

            string(const char *rhs) : data_(new char[strlen(rhs) + 1]), size_(strlen(rhs))
            {
                strcpy(data_, rhs);
            }

            string(const string &rhs) : data_(new char[rhs.size() + 1]), size_(rhs.size())
            {
                strcpy(data_, rhs.c_str());
            }

            string &operator=(const string &rhs)
            {
                string tmp(rhs);
                swap(tmp);
                return *this;
            }

            string &operator+=(const string &rhs)
            {
                string tmp(*this);
                tmp = tmp + rhs;
                swap(tmp);
                return *this;
            }

            ~string() noexcept
            {
                delete [] data_;
            }

        public:
            char *c_str() const
            {
                return data_;
            }

            size_type size() const
            {
                return size_;
            }

            void swap(string &rhs)
            {
                std::swap(data_, rhs.data_);
                std::swap(size_, rhs.size_);
            }

        public:
            char &operator[](size_type n)
            {
                return data_[n];
            }

            const char &operator[](size_type n) const
            {
                return data_[n];
            }

        private:
            char *data_;
            size_type size_;
    };

    std::istream &operator>>(std::istream &is, string &s);
    std::ostream &operator<<(std::ostream &os, const string &s);

    string operator+(const string &lhs, const string &rhs);

    bool operator==(const string &lhs, const string &rhs);
    bool operator!=(const string &lhs, const string &rhs);
    bool operator<(const string &lhs, const string &rhs);
    bool operator>(const string &lhs, const string &rhs);
}

#endif
