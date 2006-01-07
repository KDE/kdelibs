// -*- mode: c++; c-basic-offset: 4 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2005 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef KXMLCORE_FAST_MALLOC_H
#define KXMLCORE_FAST_MALLOC_H

#include <stdlib.h>
#include <new>

namespace KXMLCore {

    inline void *fastMalloc(size_t n) {
        return malloc(n);
    }

    inline void *fastCalloc(size_t n_elements, size_t element_size) {
        return calloc(n_elements, element_size);
    }

    inline void fastFree(void* p) {
        free(p);
    }

    inline void *fastRealloc(void* p, size_t n) {
        return realloc(p, n);
    }
    
} // namespace KXMLCore

using KXMLCore::fastMalloc;
using KXMLCore::fastCalloc;
using KXMLCore::fastRealloc;
using KXMLCore::fastFree;


#endif /* KXMLCORE_FAST_MALLOC_H */
