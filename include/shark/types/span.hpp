#pragma once

#include <iostream>

namespace shark
{
    /// shark::span: a hybrid between std::span and std::vector
    /// allows you to be both memory managed (like std::vector) and unmanaged (like std::span)
    /// depending on how you construct the span
    template <typename T>
    class span
    {
        static int allocs;
        T *_data;
        size_t _size;
        bool managed;
    public:

        span(T *_data, size_t _size) : _data(_data), _size(_size), managed(false) 
        {
            // if  (std::is_integral<T>::value)
            //     std::cout << "span(T *_data, size_t _size)" << std::endl;
        }
        span(size_t _size) : _data(new T[_size]), _size(_size), managed(true)
        {
            // if  (std::is_integral<T>::value)
            //     std::cout << "span(size_t _size)" << std::endl;
            allocs += 1;
            // if  (std::is_integral<T>::value)
            //     std::cerr << "allocs cons: " << allocs << std::endl;
        }
        span(const span &other) : _data(new T[other.size()]), _size(other.size()), managed(true) 
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "span(const span &other)" << std::endl;
            allocs += 1;
            // if  (std::is_integral<T>::value)
            //     std::cerr << "allocs copy cons: " << allocs << std::endl;

            for (size_t i = 0; i < _size; i++)
                _data[i] = other[i]; // cannot use memcpy here as the type T may not be trivially copyable
        }
        span() : _data(nullptr), _size(0), managed(false) 
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "span()" << std::endl;
        }

        // move constructor
        span(span &&other) : _data(other._data), _size(other._size), managed(other.managed)
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "span(span &&other)" << std::endl;
            other._data = nullptr;
            other._size = 0;
            other.managed = false;
        }

        void operator=(const span<T>& C) 
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "operator=(const span<T>& C)" << std::endl;
            if (managed) {
                delete[] _data;
                allocs -= 1;
                // if  (std::is_integral<T>::value)
                //     std::cerr << "allocs eq dest: " << allocs << std::endl;
            }
            _size = C.size();
            _data = new T[_size];
            managed = true;
            for (size_t i = 0; i < _size; i++)
                _data[i] = C[i];
            allocs += 1;
            // if  (std::is_integral<T>::value)
            //     std::cerr << "allocs eq cons: " << allocs << std::endl;
        }

        // move assigment operator
        void operator=(span<T>&& C) 
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "operator=(span<T>&& C)" << std::endl;
            if (managed) {
                delete[] _data;
                allocs -= 1;
                // if  (std::is_integral<T>::value)
                //     std::cerr << "allocs eq dest: " << allocs << std::endl;
            }
            _size = C.size();
            _data = C._data;
            managed = C.managed;
            C._data = nullptr;
            C._size = 0;
            C.managed = false;
        }

        ~span()
        {
            // if (std::is_integral<T>::value)
            //     std::cout << "~span()" << std::endl;
            // std::cerr << "allocs: " << allocs << std::endl;
                
            if (managed) {
                allocs -= 1;
                delete[] _data;
                // if  (std::is_integral<T>::value)
                //     std::cerr << "allocs dest: " << allocs << std::endl;
            }
        }

        T &operator[](size_t i) { return _data[i]; }
        const T &operator[](size_t i) const { return _data[i]; }

        T *begin() { return _data; }
        T *end() { return _data + _size; }

        T *data() { return _data; }
        T *data() const { return _data; }
        size_t size() { return _size; }
        size_t size() const { return _size; }

        static int get_allocs() { return allocs; }
    };
}

template <typename T>
int shark::span<T>::allocs = 0;

template <typename T>
std::ostream& operator<<(std::ostream& os, const shark::span<T>& s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        os << s[i] << std::endl;
    }
    return os;
}
