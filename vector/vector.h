#ifndef MOUSE_VECTOR_H_
#define MOUSE_VECTOR_H_

#include <cstddef>
#include <memory>
#include <initializer_list>

namespace Mouse {
    template<typename T> class vector
    {
        public:
            typedef size_t size_type;

        public:
            vector() : elements(nullptr), first_free(nullptr), cap(nullptr) {};
            vector(const vector<T> &);
            vector<T> &operator=(const vector<T> &);
            vector<T> &operator=(std::initializer_list<T>);

            ~vector() noexcept;

        public:
            void clear();
            void push_back(const T&);
            void pop_back();

            size_type size() const { return first_free - elements; }
            size_type capacity() const { return cap - elements; }

            T *begin() const { return elements; }
            T *end() const { return first_free; }

            T &front() { return *elements; }
            const T &front() const { return *elements; }
            T &back() { return *(first_free - 1); }
            const T &back() const { return *(first_free - 1); }
            T &operator[](size_type n) { return *(elements + n); }
            const T &operator[](size_type n) const { return *(elements + n); }

        private:
            static std::allocator<T> alloc;

            void check_n_alloc() { if (size() == capacity()) reallocate(); }

            std::pair<T *, T *> alloc_n_copy(const T *, const T *);
            void reallocate();

        private:
            T *elements;
            T *first_free;
            T *cap;
    };

    template<typename T> inline vector<T>::vector(const vector<T> &rhs)
    {
        auto data = alloc_n_copy(rhs.begin(), rhs.end());
        elements = data.first;
        first_free = cap = data.second;
    }

    template<typename T> inline vector<T> &vector<T>::operator=(const vector<T> &rhs)
    {
        auto data = alloc_n_copy(rhs.begin(), rhs.end());
        clear();
        elements = data.first;
        first_free = cap = data.second;
        return *this;
    }

    template<typename T> inline vector<T> &vector<T>::operator=(std::initializer_list<T> il)
    {
        auto data = alloc_n_copy(il.begin(), il.end());
        clear();
        elements = data.first;
        first_free = cap = data.second;
        return *this;
    }

    template<typename T> inline vector<T>::~vector() noexcept { clear(); }

    template<typename T> inline void vector<T>::push_back(const T& rhs)
    {
        check_n_alloc();
        alloc.construct(first_free++, rhs);
    }

    template<typename T> inline void vector<T>::pop_back()
    {
        if (elements != first_free) {
            alloc.destroy(--first_free);
        }
    }

    template<typename T> inline std::pair<T *, T *> vector<T>::alloc_n_copy(const T *b, const T *e)
    {
        auto data = alloc.allocate(e - b);
        return {data, uninitialized_copy(b, e, data)};
    }

    template<typename T> inline void vector<T>::clear()
    {
        if (elements == nullptr)
            return ;

        for (auto p = first_free; p != elements; )
            alloc.destroy(--p);

        alloc.deallocate(elements, cap - elements);
    }

    template<typename T> void vector<T>::reallocate()
    {
        auto newcapacity = size() ? size() * 2 : 1;

        auto newdata = alloc.allocate(newcapacity);

        auto dest = newdata;
        auto elem = elements;
        for (size_type i = 0; i < size(); ++i)
            alloc.construct(dest++, std::move(*elem++));

        clear();
        elements = newdata;
        first_free = dest;
        cap = elements + newcapacity;
    }

    template<typename T> std::allocator<T> vector<T>::alloc;
}

#endif
