/**
  @file
  @brief Heap data structure

  Copyright (c) 2018, Alexander Lukichev <alexander.lukichev@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are
  those of the authors and should not be interpreted as representing official
  policies, either expressed or implied, of the FreeBSD Project.
*/

#ifndef HEAP_H
#define HEAP_H

#include <cstddef>
#include <functional>
#include <map>

#include <QtAlgorithms>
#include <QVector>

template <class T> class Heap;

template <class T>
inline void swap(Heap<T>& first, Heap<T>& second)
{
    first._d.swap(second._d);
    qSwap(first._less, second._less);
    qSwap(first._size, second._size);
}

/** Default template for size in memory of an object.
 *
 * This implementation returns a simple sizeof() value. For objects, size of
 * which may change (e.g., QByteArray), implement a template specialization to
 * more precisely compute its memory usage.
 */
template <class T>
inline size_t heapSizeOf(const T& ref)
{
    return sizeof(ref);
}

/** Default template for swapping two objects.
 *
 * This implementation uses qSwap() to swap objects. If other behavior is
 * required, a specialization of qSwap() or of this function may be
 * provided.
 */
template <class T>
inline void heapSwap(T& first, T& second)
{
    qSwap(first, second);
}

/** Heap data structure.
 *
 * Use this structure to hold an arbitrary number of elements which can be
 * totally ordered using operation "less than". After insertion (in
 * O(log N) time), the least element may then be determined (in constant time)
 * or extracted (in O(log N) time). The primary use case of this data structure
 * is when data elements (e.g., tasks or events) need to be prioritized by some
 * criterium (e.g., importance or timestamp).
 *
 * The type of data elements (if it is not a POD type) is required to:
 * - be default-constructible;
 * - be assignable;
 * - not disable copy constructor;
 * - have a destructor;
 * - provide an implementation of bool operator <(const T&, const T&).
 *
 * The default order of elements in this structure is using < operator (which
 * must be implemented by the data type). If some other order is needed then
 * a function with similar semantics (that is, "strictly less than", not "less
 * than or equal to") can be supplied to its constructor. E.g., it is possible
 * to have two heaps Heap<int>, one of which provides minimum and the other
 * provides maximum:
 *
 * @code
 * bool bt(const int& lhs, const int& rhs) { return rhs < lhs; }
 *
 * <...>
 * Heap<int> inc, dec(&bt);
 *
 * for (int i = 0; i < 10; ++i) {
 *      inc.insert(i);
 *      dec.insert(i);
 * }
 *
 * while (!inc.isEmpty())
 *      printf("%d ", inc.extractMin());
 *
 * while (!dec.isEmpty())
 *      printf("%d ", dec.extractMin());
 * @endcode
 *
 * The output of the example code will be a string:
 * "0 1 2 3 4 5 6 7 8 9 9 8 7 6 5 4 3 2 1 0 ".
 *
 * The heap is implicitly-shared, copy-and-swap data structure. It can be very
 * cheaply passed by value.
 *
 * Internally, heap uses QVector to store elements. It is resizable but if many
 * elements are already stored in it, further insertions may impose an O(N)
 * reallocation and movement cost. For non-POD types, every insertion
 * or extraction of a single element may imply calling copy constructors and
 * destructors for already inserted data. Also, QVector may allocate
 * more memory than it actually needs to hold elements, to minimize
 * reallocations.
 *
 * Internal QVector container for data may perform several reallocations when
 * many elements are inserted, that may be expensive. If it is known in advance
 * how many elements are approximately expected to be stored, it is better to
 * give this number to the heap constructor. In this case, the space needed for
 * data may be allocated only once in the constructor.
 *
 * This structure tracks and estimates as accurately as possible the amount of
 * memory used by all its data, including extra space for QVector needs. For
 * non-trivial data types that are resizable themselves (e.g., QByteArray) this
 * is impossible to do without a function that accounts for its
 * internal structure. Such function may be implemented as a template
 * specialization of heapSizeOf() function. To get the size of memory used by
 * all the data in a heap at a particular time, use size() method.
 */
template <class T>
class Heap
{
public:
    typedef bool (*Less)(const T& lhs, const T& rhs);

    static bool DefLess(const T& lhs, const T& rhs) { return lhs < rhs; }

    inline Heap(Less less = &DefLess, int reserve = 256);
    inline Heap(const Heap& other) : _d(other._d), _less(other._less), _size(other._size) {}
    inline ~Heap(void) {}

    friend void swap <>(Heap<T>& first, Heap<T>& second);
    inline Heap& operator =(Heap other) { swap(*this, other); return *this; }

    inline bool isEmpty(void) const { return _d.isEmpty(); }
    inline int count(void) const { return _d.size(); }
    inline size_t size(void) const { return _size; }
    inline const T& min(void) const { return isEmpty() ? _defValue : _d.at(0); }

    inline T extractMin(void) { return isEmpty() ? _defValue : _extract(0); }
    inline const T& insert(const T& v);

private:
    static const T _defValue;
    static const size_t _defSize;

    inline bool _le(const T& lhs, const T& rhs) const { return !_less(rhs, lhs); } // lhs <= rhs is !(lhs > rhs) is !(rhs < lhs)

    inline void _add(const T& v);
    inline T _extract(int i);

    QVector<T> _d;
    Less _less;
    size_t _size;
};

template <class T>
const T Heap<T>::_defValue = T();

template <class T>
const size_t Heap<T>::_defSize = heapSizeOf(T());

template <class T>
inline Heap<T>::Heap(Less less, int reserve) : _d(), _less(!less ? &DefLess : less)
{
    _d.reserve(reserve);
    _size = _d.capacity() * _defSize;
}

template <class T>
inline void Heap<T>::_add(const T& v)
{
    const bool will_realloc = _d.size() == _d.capacity();

    _d.append(v);

    if (_size)
        _size -= _defSize;

    _size += heapSizeOf(v);

    if (will_realloc)
        _size += (_d.capacity() - _d.size()) * _defSize;
}

template <class T>
inline T Heap<T>::_extract(int i)
{
    const T extracted = _d.at(i);

    // Replace the extracted with the last element
    _d[i] = _d.last();
    _d.pop_back();

    if (!_d.size())
        return extracted;

    // Bubble it down
    bool valid = false;
    const int dsize = _d.size();

    while (!valid) {
        const int left = 2 * (i + 1);
        const int right = left + 1;
        const bool lvd = left <= dsize, rvd = right <= dsize;
        const T& lchild = lvd ? _d.at(left - 1) : _defValue;
        const T& rchild = rvd ? _d.at(right - 1) : _defValue;

        valid = (!lvd || _le(_d.at(i), lchild)) && (!rvd || _le(_d.at(i), rchild));
        if (!valid) {
            int with;

            if (!rvd || (lvd && _le(lchild, rchild)))
                with = left - 1;
            else
                with = right - 1;

            heapSwap(_d[i], _d[with]);
            i = with;
        }
    }

    return extracted;
}

template <class T>
inline const T &Heap<T>::insert(const T& v)
{
    // Insert element
    _add(v);

    // Bubble it up
    int num = _d.size();
    bool valid = false;

    while (1 < num && !valid) {
        const int parent = num / 2;

        valid = _le(_d[parent - 1], v);
        if (!valid) {
            heapSwap(_d[num - 1], _d[parent - 1]);
            num = parent;
        }
    }

    return _d.at(num - 1);
}

#endif // HEAP_H
