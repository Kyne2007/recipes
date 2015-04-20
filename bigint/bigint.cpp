#include "bigint.h"
#include <stdexcept>
#include <string>
#include <iostream>

namespace Mouse {
    bigint::bigint(int rhs)
    {
        if (rhs < 0) {
            sign_ = 1;
            rhs = -rhs;
        } else {
            sign_ = 0;
        }

        do {
            num_.push_back(rhs % 10);
            rhs /= 10;
        } while (rhs);
    }

    bigint::bigint(const std::string &rhs)
    {
        if (rhs.size() == 0)
            return ;

        sign_ = 0;
        for (auto it = rhs.rbegin(); it != rhs.rend(); ++it) {
            if (!isdigit(*it)) {
                if (*it == '-' && ++it == rhs.rend()) {
                    sign_ = 1;
                    break;
                } else {
                    throw std::invalid_argument("construct");
                }
            }

            num_.push_back(*it - '0');
        }
    }

    bigint &bigint::operator=(const bigint &rhs)
    {
        sign_ = rhs.sign_;
        num_ = rhs.num_;

        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const bigint &num)
    {
        if (num.sign_ == 1)
            os << "-";

        for (auto it = num.num_.rbegin(); it != num.num_.rend(); ++it)
            os << *it ;

        return os;
    }

    std::istream &operator>>(std::istream &is, bigint &num)
    {
        std::string s;

        is >> s;
        num = s;

        return is;
    }

    bool operator==(const bigint &lhs, const bigint &rhs)
    {
        if (lhs.sign_ != rhs.sign_ || lhs.num_ != rhs.num_)
            return false;

        return true;
    }

    bool operator!=(const bigint &lhs, const bigint &rhs)
    {
        return !(lhs == rhs);
    }

    bool operator<(const bigint &lhs, const bigint &rhs)
    {
        if (lhs.sign_ == 0 && rhs.sign_ == 1) {
            return false;
        } else if (lhs.sign_ == 1 && rhs.sign_ == 0) {
            return true;
        }

        auto llen = lhs.num_.size();
        auto rlen = rhs.num_.size();

        bool rnt;
        if (llen > rlen) {
            rnt = false;
        } else if (llen < rlen) {
            rnt = true;
        } else {
            rnt = false;
            for (int i = llen - 1; i >= 0; ++i) {
                if (lhs.num_[i] == rhs.num_[i])
                    continue;

                if (lhs.num_[i] < rhs.num_[i]) {
                    rnt = true;
                    break;
                } else {
                    rnt = false;
                    break;
                }
            }
        }

        if (lhs.sign_ == 1 && rhs.sign_ == 1)
            return !rnt;
        else
            return rnt;
    }

    bool operator>(const bigint &lhs, const bigint &rhs)
    {
        return !(lhs < rhs);
    }

    const bigint operator+(const bigint &lhs, const bigint &rhs)
    {
        if (lhs.sign_ == 0 && rhs.sign_ == 1)
            return lhs - (-rhs);

        if (rhs.sign_ == 0 && lhs.sign_ == 1)
            return rhs - (-lhs);

        bigint sum;
        sum.num_.clear();

        std::vector<short>::const_iterator itl, itr;
        for (itl = lhs.num_.begin(), itr = rhs.num_.begin(); 
                itl != lhs.num_.end() && itr != rhs.num_.end(); ++itl, ++itr) {
            sum.num_.push_back(*itl + *itr);
        }

        for (; itl != lhs.num_.end(); ++itl)
            sum.num_.push_back(*itl);
        for (; itr != rhs.num_.end(); ++itr)
            sum.num_.push_back(*itr);

        int carry = 0;
        for (auto it = sum.num_.begin(); it != sum.num_.end(); ++it) {
            *it += carry;
            carry = *it / 10;
            *it %= 10;
        }
        if (carry > 0)
            sum.num_.push_back(carry);

        if (lhs.sign_ == 1 && rhs.sign_ == 1)
            return -sum;
        else
            return sum;
    }

    const bigint operator-(const bigint &lhs, const bigint &rhs)
    {
        if (lhs.sign_ == 0 && rhs.sign_ == 1)
            return lhs + (-rhs);

        if (rhs.sign_ == 0 && lhs.sign_ == 1)
            return -(rhs + (-lhs));

        if (lhs.sign_ == 0 && lhs.sign_ == 0 && lhs < rhs)
            return -(rhs - lhs);

        if (lhs.sign_ == 1 && lhs.sign_ == 1 && rhs < lhs)
            return -(rhs - lhs);

        bigint diff(lhs);

        std::vector<short>::iterator itl;
        std::vector<short>::const_iterator itr; 
        for (itl = diff.num_.begin(), itr = rhs.num_.begin(); 
                itr != rhs.num_.end(); ++itl, ++itr) {
            *itl -= *itr;
        }

        short borrow = 0;
        std::vector<short>::iterator it;
        for (it = diff.num_.begin(); it != diff.num_.end(); ++it) {
            *it -= borrow;
            if (*it < 0) {
                *it += 10;
                borrow = 1;
            }
        }

        for (it = diff.num_.end() - 1; it != diff.num_.begin(); --it)
            if (*it != 0)
                break;

        diff.num_.erase(it + 1, diff.num_.end());

        if (lhs.sign_ == 1 && rhs.sign_ == 1)
            return -diff;
        else
            return diff;
    }

    const bigint operator*(const bigint &lhs, const bigint &rhs)
    {
        bigint product;

        if (lhs == 0 || rhs == 0)
            return product;

        std::vector<short>::size_type size;
        size = lhs.num_.size() + rhs.num_.size() - 1;
        
        product.num_.clear();
        product.num_.resize(size, 0);

        std::vector<short>::const_iterator itl, itr;
        std::vector<short>::iterator it;
        for (itl = lhs.num_.begin(); itl != lhs.num_.end(); ++itl) {
            it = product.num_.begin() + (itl - lhs.num_.begin());
            for (itr = rhs.num_.begin(); itr != rhs.num_.end(); ++itr, ++it) {
                *it += (*itl) * (*itr);
            }
        }

        short carry = 0;
        for (it = product.num_.begin(); it != product.num_.end(); ++it) {
            *it += carry;
            carry = *it / 10;
            *it %= 10;
        }

        if (carry > 0)
            product.num_.push_back(carry);

        if ((lhs.sign_ == 1 && rhs.sign_ == 0) || (lhs.sign_== 0 && rhs.sign_ == 1))
            product.sign_ = 1;
        else
            product.sign_ = 0;

        return product;
    }
}

