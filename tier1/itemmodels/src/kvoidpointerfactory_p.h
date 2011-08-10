/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KVOIDPOINTERFACTORY_P_H
#define KVOIDPOINTERFACTORY_P_H

#include <vector>
#include <cstdlib>
#include <cstddef>

#define DEFAULT_BLOCK_SIZE 256

/**
 * @brief A Class for managing void pointers for use in QModelIndexes.
 *
 * This class creates void pointers pointing to individual blocks of memory.
 * The pointed-to memory contains zeros.
 *
 * Memory is allocated in blocks of size @p blockSize times sizeof(void*) at a time. The used
 * memory is automatically freed and can be cleared manually.
 *
 * The void pointers should not be dereferenced, but only used as a unique
 * identifier suitable for use with createIndex() and for comparison with other void pointers.
 */
template<std::size_t blockSize = DEFAULT_BLOCK_SIZE>
class KVoidPointerFactory
{
  // a class with size 1.
  class Bit { bool bit; };

public:
  KVoidPointerFactory()
    : m_previousPointer(0),
      m_finalPointer(0)
  {
  }

  KVoidPointerFactory(const KVoidPointerFactory<blockSize> &other)
  {
    *this = other;
  }

  KVoidPointerFactory<blockSize>& operator=(const KVoidPointerFactory<blockSize> &other)
  {
    m_previousPointer = other.m_previousPointer;
    m_finalPointer = other.m_finalPointer;
    m_blocks = other.m_blocks;
    return *this;
  }

  ~KVoidPointerFactory()
  {
    clear();
  }

  void clear()
  {
    typename std::vector<Bit*>::const_iterator it = m_blocks.begin();
    const typename std::vector<Bit*>::const_iterator end = m_blocks.end();
    for ( ; it != end; ++it)
    {
      free(*it);
    }
    m_blocks.clear();
    m_finalPointer = 0;
    m_previousPointer = 0;
  }

  void* createPointer() const
  {
    if (m_previousPointer == m_finalPointer)
    {
      static const std::size_t pointer_size = sizeof(Bit*);
      Bit * const bit = static_cast<Bit*>(calloc(blockSize, pointer_size));
      m_blocks.push_back(bit);
      m_finalPointer = bit + (blockSize * pointer_size) - 1;
      m_previousPointer = bit;
      return bit;
    }
    return ++m_previousPointer;
  }

private:
  mutable std::vector<Bit *> m_blocks;
  mutable Bit *m_previousPointer;
  mutable Bit *m_finalPointer;
};

// Disable factory with 0 blockSize
template<>
class KVoidPointerFactory<0>
{
public:
  KVoidPointerFactory();

  void clear();
  void* createPointer();
};


#endif
