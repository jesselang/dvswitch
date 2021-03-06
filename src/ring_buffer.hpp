// Copyright 2007, 2011 Ben Hutchings.
// See the file "COPYING" for licence details.

// Class template for ring buffers

#ifndef DVSWITCH_RING_BUFFER_HPP
#define DVSWITCH_RING_BUFFER_HPP

#include <algorithm>
#include <cassert>
#include <new>

#include <tr1/type_traits>

template<typename T>
class ring_buffer;

template<typename T>
void swap(ring_buffer<T> & left, ring_buffer<T> & right);

template<typename T>
class ring_buffer
{
public:
    explicit ring_buffer(std::size_t capacity)
	: capacity_(capacity), front_(0), back_(0),
	  buffer_(reinterpret_cast<T *>(new char[sizeof(T) * capacity_]))
    {}
    ring_buffer(const ring_buffer &);
    ~ring_buffer();
    ring_buffer & operator=(const ring_buffer &);

    void reset();

    std::size_t capacity() const { return capacity_; }
    std::size_t size() const { return back_ - front_; }
    bool empty() const { return front_ == back_; }
    bool full() const { return back_ - front_ == capacity_; }

    // Reader functions
    void pop();
    const T & front() const;

    // Writer functions
    void push(const T &);
    const T & back() const;

    friend void swap<T>(ring_buffer & left, ring_buffer & right);

private:
    std::size_t capacity_, front_, back_;
    T * buffer_;
};

template<typename T>
ring_buffer<T>::ring_buffer(const ring_buffer & other)
    : capacity_(other.capacity_), front_(0), back_(0),
      buffer_(reinterpret_cast<T *>(new char[sizeof(T) * capacity_]))
{
    try
    {
	for (std::size_t i = other.front_; i != other.back_; ++i)
	    push(other.buffer_[i % capacity_]);
    }
    catch (...)
    {
	reset();
	delete[] reinterpret_cast<char*>(buffer_);
    }
}

template<typename T>
ring_buffer<T>::~ring_buffer()
{
    reset();
    delete[] reinterpret_cast<char*>(buffer_);
}

template<typename T>
ring_buffer<T> & ring_buffer<T>::operator=(const ring_buffer & other)
{
    reset();

    for (std::size_t i = other.front_; i != other.back_; ++i)
	push(other.buffer_[i % capacity_]);

    return *this;
}

template<typename T>
void ring_buffer<T>::reset()
{
    while (!empty())
	pop();
}

template<typename T>
void ring_buffer<T>::pop()
{
    assert(!empty());
    buffer_[front_ % capacity_].~T();
    ++front_;
}

template<typename T>
const T & ring_buffer<T>::front() const
{
    assert(!empty());
    return buffer_[front_ % capacity_];
}

template<typename T>
void ring_buffer<T>::push(const T & value)
{
    assert(!full());
    new (&buffer_[back_ % capacity_]) T(value);
    ++back_;
}

template<typename T>
const T & ring_buffer<T>::back() const
{
    assert(!empty());
    return buffer_[(back_ - 1) % capacity_];
}

template<typename T>
void swap(ring_buffer<T> & left, ring_buffer<T> & right)
{
    using std::swap;
    swap(left.capacity_, right.capacity_);
    swap(left.front_, right.front_);
    swap(left.back_, right.back_);
    swap(left.buffer_, right.buffer_);
}

#endif // !defined(DVSWITCH_RING_BUFFER_HPP)
