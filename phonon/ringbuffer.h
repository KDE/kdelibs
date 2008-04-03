/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QtCore/QAtomicInt>
#include <QtCore/QList>

class QString;
struct RingBufferBasePrivate;

/**
 * \internal
 */
struct RingBufferBase
{
    RingBufferBasePrivate *d;
    int _writePosition();
    void _incSize();
    int _readPosition();
    bool _canRead();
    int _size() const;

    RingBufferBase();
    ~RingBufferBase();
};

namespace
{
    template<unsigned char x> struct _SizeCheck { enum { Value = _SizeCheck<x - 1>::Value + 1 }; };
    template<> struct _SizeCheck<0> { enum { Value = 0 }; };
    template<> struct _SizeCheck<21> {};

    template<unsigned int x> struct _NextPowerOfTwoHelper
    { enum { Value = _NextPowerOfTwoHelper<x >> 1>::Value + 1 }; };

    template<> struct _NextPowerOfTwoHelper<0> { enum { Value = 0 }; };

    template<unsigned int x> struct _NextPowerOfTwo
    { enum { Value = (x > 1 ? _NextPowerOfTwoHelper<(x - 1) >> 1>::Value + 1 : 0) }; };
} // anonymous namespace

/** \class RingBuffer lockfreeringbuffer.h RingBuffer
 * \short Threadsafe, lock free, generic, and variably sized ring-buffer.
 *
 * \param T Type to contain as entries in the ring-buffer. The type needs to provide a default
 *          constructor and an assignment operator.
 * \param Size Determines the size of the ring-buffer. The actual size is 2^Size. Defaults to 6,
 *             which means it can hold 2^6 == 64 elements. The largest allowed Size is 20, meaning
 *             2^20 == 1 millon entries.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
template<class T, unsigned char Size = 6>
class RingBufferExp : private RingBufferBase
{
    private:
        static const int DATA_SIZE = 1 << _SizeCheck<Size>::Value;
        static const int POSITION_MASK = DATA_SIZE - 1;

        // small helper class to mask the int member for all operations with POSITION_MASK
        class Position
        {
            public:
                inline Position(int p) : k(p & POSITION_MASK) {}
                inline Position(const QAtomicInt &rhs) : k(static_cast<int>(rhs) & POSITION_MASK) {}
                inline Position(const Position &rhs) : k(rhs.k) {}
                inline Position &operator=(int rhs) { k = (rhs & POSITION_MASK); return *this; }
                inline Position &operator=(const QAtomicInt &rhs) { k = (static_cast<int>(rhs) & POSITION_MASK); return *this; }
                inline Position &operator=(const Position &rhs) { k = rhs.k; return *this; }
                inline Position &operator++() { k = (k + 1) & POSITION_MASK; return *this; }
                inline Position operator++(int) { const Position tmp(*this); k = (k + 1) & POSITION_MASK; return tmp; }
                inline Position &operator--() { k = (k - 1) & POSITION_MASK; return *this; }
                inline Position operator--(int) { const Position tmp(*this); k = (k - 1) & POSITION_MASK; return tmp; }
                inline Position &operator+=(int x) { k = (k + x) & POSITION_MASK; return *this; }
                inline Position &operator-=(int x) { k = (k - x) & POSITION_MASK; return *this; }
                inline Position operator+(int x) const { Position r(*this); return (r += x); }
                inline Position operator-(int x) const { Position r(*this); return (r -= x); }
                inline operator const int() const { return k; }
            private:
                int k;
        };

        // simple wrapper for a C-style array in order to enforce the [] argument to be of type
        // Position and with that to always be in range
        class DataArray
        {
            public:
                inline T &operator[](const Position &pos) { return data[pos]; }
            private:
                T data[DATA_SIZE];
        };

        DataArray m_data;

    public:
        inline void write(const T &data)
        {
            m_data[_writePosition()] = data;
            _incSize();
        }

        inline RingBufferExp<T, Size> &operator<<(const T &data)
        {
            write(data);
            return *this;
        }

        QList<T> read(int count = 1)
        {
            Q_ASSERT(count > 0);
            QList<T> r;
            while (r.size() < count && _canRead()) {
                r << m_data[_readPosition()];
            }
            return r;
        }

        inline RingBufferExp<T, Size> &operator>>(T &data)
        {
            if (_canRead()) {
                data = m_data[_readPosition()];
            }
            return *this;
        }

        inline int size() const { return _size(); }
        inline bool isEmpty() const { return 0 == _size(); }
        inline bool isFull() const { return DATA_SIZE <= _size(); }
};

/** \class RingBuffer lockfreeringbuffer.h RingBuffer
 * \short Threadsafe, lock free, generic, and variably sized ring-buffer.
 *
 * \param T Type to contain as entries in the ring-buffer. The type needs to provide a default
 *          constructor and an assignment operator.
 * \param Size Requests a minimal size for the ring-buffer. The actual size will be the next biggest
 *             power of two (determined at compile time, so there is no run-time performance loss if
 *             you do not specify a power of two value). Defaults to 64.
 *             The largest allowed Size is 2^20.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
template<class T, unsigned int Size = 64>
class RingBuffer : public RingBufferExp<T, _NextPowerOfTwo<Size>::Value>
{
};

struct SharedMemoryRingBufferBasePrivate;

struct SharedMemoryRingBufferBase : public RingBufferBase
{
    SharedMemoryRingBufferBase(int sizeOfArray, const QString &key);
    ~SharedMemoryRingBufferBase();
    SharedMemoryRingBufferBasePrivate *const d2;
    void *_dataMemory() const;
};

template<class T, unsigned char Size = 6>
class SharedMemoryRingBufferExp : private SharedMemoryRingBufferBase
{
    private:
        static const int DATA_SIZE = 1 << _SizeCheck<Size>::Value;
        static const int POSITION_MASK = DATA_SIZE - 1;

        // small helper class to mask the int member for all operations with POSITION_MASK
        class Position
        {
            public:
                inline Position(int p) : k(p & POSITION_MASK) {}
                inline Position(const QAtomicInt &rhs) : k(static_cast<int>(rhs) & POSITION_MASK) {}
                inline Position(const Position &rhs) : k(rhs.k) {}
                inline Position &operator=(int rhs) { k = (rhs & POSITION_MASK); return *this; }
                inline Position &operator=(const QAtomicInt &rhs) { k = (static_cast<int>(rhs) & POSITION_MASK); return *this; }
                inline Position &operator=(const Position &rhs) { k = rhs.k; return *this; }
                inline Position &operator++() { k = (k + 1) & POSITION_MASK; return *this; }
                inline Position operator++(int) { const Position tmp(*this); k = (k + 1) & POSITION_MASK; return tmp; }
                inline Position &operator--() { k = (k - 1) & POSITION_MASK; return *this; }
                inline Position operator--(int) { const Position tmp(*this); k = (k - 1) & POSITION_MASK; return tmp; }
                inline Position &operator+=(int x) { k = (k + x) & POSITION_MASK; return *this; }
                inline Position &operator-=(int x) { k = (k - x) & POSITION_MASK; return *this; }
                inline Position operator+(int x) const { Position r(*this); return (r += x); }
                inline Position operator-(int x) const { Position r(*this); return (r -= x); }
                inline operator const int() const { return k; }
            private:
                int k;
        };

        // simple wrapper for a C-style array in order to enforce the [] argument to be of type
        // Position and with that to always be in range
        class DataArray
        {
            public:
                inline DataArray(void *memory) : data(reinterpret_cast<T *>(memory)) {}
                inline T &operator[](const Position &pos) { return data[pos]; }
            private:
                T *data;
        };

        DataArray m_data;

    public:
        SharedMemoryRingBufferExp(const QString &key)
            : SharedMemoryRingBufferBase(sizeof(T) * DATA_SIZE, key),
            m_data(_dataMemory())
        {
        }

        inline void write(const T &data)
        {
            m_data[_writePosition()] = data;
            _incSize();
        }

        inline SharedMemoryRingBufferExp<T, Size> &operator<<(const T &data)
        {
            write(data);
            return *this;
        }

        QList<T> read(int count = 1)
        {
            Q_ASSERT(count > 0);
            QList<T> r;
            while (r.size() < count && _canRead()) {
                r << m_data[_readPosition()];
            }
            return r;
        }

        inline SharedMemoryRingBufferExp<T, Size> &operator>>(T &data)
        {
            if (_canRead()) {
                data = m_data[_readPosition()];
            }
            return *this;
        }

        inline int size() const { return _size(); }
        inline bool isEmpty() const { return 0 == _size(); }
        inline bool isFull() const { return DATA_SIZE <= _size(); }
};

template<class T, unsigned int Size = 64>
class SharedMemoryRingBuffer : public SharedMemoryRingBufferExp<T, _NextPowerOfTwo<Size>::Value>
{
    public:
        inline SharedMemoryRingBuffer(const QString &key) : SharedMemoryRingBufferExp<T, _NextPowerOfTwo<Size>::Value>(key) {}
};

#endif // RINGBUFFER_H
