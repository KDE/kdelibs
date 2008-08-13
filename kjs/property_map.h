// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2004, 2005, 2006, 2007 Apple Computer, Inc.
 *  Copyright (C) 2007 Christopher E. Hyde <C.Hyde@parableuk.force9.co.uk>
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

#ifndef KJS_PROPERTY_MAP_H_
#define KJS_PROPERTY_MAP_H_

#include "identifier.h"
#include <wtf/OwnArrayPtr.h>

namespace KJS {

    class PropertyNameArray;
    class JSValue;

    class SavedProperty;

    struct PropertyMapHashTable;

/*
* Saved Properties
*/
    class SavedProperties {
    friend class PropertyMap;
    public:
        SavedProperties();
        ~SavedProperties();

    private:
        int _count;
        OwnArrayPtr<SavedProperty> _properties;
    };

/*
* A hashtable entry for the @ref PropertyMap.
*/
    struct PropertyMapHashTableEntry
    {
        PropertyMapHashTableEntry() : key(0) { }
        UString::Rep *key;
        JSValue *value;
        int attributes;
        int index;
    };

/**
* Javascript Property Map.
*/
    class KJS_EXPORT PropertyMap {
        friend class JSObject;
    public:
        PropertyMap();
        ~PropertyMap();

        void clear();

        void put(const Identifier &name, JSValue *value, int attributes, bool roCheck = false);
        void remove(const Identifier &name);
        JSValue *get(const Identifier &name) const;
        JSValue *get(const Identifier &name, unsigned &attributes) const;
        JSValue **getLocation(const Identifier &name);

        // Returns a location where this property can be set, if it
        // exists, is writeable, and not a setter.
        // Warning: this pointer may become invalid after any further modifications
        JSValue **getWriteLocation(const Identifier &name);

        void mark() const;
        void getEnumerablePropertyNames(PropertyNameArray&) const;

        void save(SavedProperties &) const;
        void restore(const SavedProperties &p);

        bool isEmpty() const;

        bool hasGetterSetterProperties() const { return m_getterSetterFlag; }
        void setHasGetterSetterProperties(bool f) { m_getterSetterFlag = f; }

        // This /computes/ whether the table has getters or setters, while the above is
        // used to cache the result. In other words, one usually does
        // setHasGetterSetterProperties(containsGettersOrSetters()) whenver
        // there is a reason to believe that the result has changed
        bool containsGettersOrSetters() const;
    private:
        static bool keysMatch(const UString::Rep *, const UString::Rep *);
        void expand();
        void rehash();
        void rehash(int newTableSize);
        void createTable();

        void insert(UString::Rep *, JSValue *value, int attributes, int index);

        void checkConsistency();

        typedef PropertyMapHashTableEntry Entry;
        typedef PropertyMapHashTable Table;

        UString::Rep* m_singleEntryKey;
        union {
          JSValue* singleEntryValue;
          Table* table;
        } m_u;


        short m_singleEntryAttributes;

        bool m_getterSetterFlag : 1;
        bool m_usingTable       : 1;

        // We also stick some of the object's
        // bitflags here. Kind of ugly, but saves memory...
        bool m_objLocalInjected : 1;
    };

    inline PropertyMap::PropertyMap() :
          m_singleEntryKey(0),
          m_getterSetterFlag(false),
          m_usingTable(false),
          m_objLocalInjected(false)
    {}
} // namespace

#endif // _KJS_PROPERTY_MAP_H_
