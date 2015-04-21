#ifndef _BIGINT_H_
#define _BIGINT_H_

#include <vector>

namespace Mouse {
    class bigint {
        friend std::ostream &operator<<(std::ostream &, const bigint &);
        friend std::istream &operator>>(std::istream &, bigint &);
        friend bool operator<(const bigint &lhs, const bigint &rhs);
        friend bool operator>(const bigint &lhs, const bigint &rhs);
        friend bool operator==(const bigint &lhs, const bigint &rhs);
        friend bool operator!=(const bigint &lhs, const bigint &rhs);

        friend const bigint operator+(const bigint &lhs, const bigint &rhs);
        friend const bigint operator-(const bigint &lhs, const bigint &rhs);
        friend const bigint operator*(const bigint &lhs, const bigint &rhs);

        public:
            bigint() : sign_(0), num_(1, 0) {}
            bigint(int sign, std::vector<short> n) : sign_(sign), num_(n) {}
            bigint(const bigint &rhs) : sign_(rhs.sign_), num_(rhs.num_) {}

            bigint(int rhs);
            bigint(const std::string &rhs);

            //条款10
            bigint &operator=(const bigint &rhs);
            bigint &operator+=(const bigint &rhs);

        public:
            bigint operator-() const
            {
                int sign = sign_;
                if (*this != 0)
                    sign = 1 - sign_;
                return bigint(sign, num_);
            }

        private:
            /* enum sign {plus, minus}; */
            int sign_; //0-plus, 1-minus
            std::vector<short> num_;
    };

    std::ostream &operator<<(std::ostream &, const bigint &);
    std::istream &operator>>(std::istream &, bigint &);

    bool operator==(const bigint &lhs, const bigint &rhs);
    bool operator!=(const bigint &lhs, const bigint &rhs);
    bool operator<(const bigint &lhs, const bigint &rhs);
    bool operator>(const bigint &lhs, const bigint &rhs);

    const bigint operator+(const bigint &lhs, const bigint &rhs);
    const bigint operator-(const bigint &lhs, const bigint &rhs);
    const bigint operator*(const bigint &lhs, const bigint &rhs);
    const bigint operator/(const bigint &lhs, const bigint &rhs);
}

#endif
