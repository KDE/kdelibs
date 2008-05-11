/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2008 Maksim Orlovich (maksim@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef _DOM_IDSTRING_h_
#define _DOM_IDSTRING_h_

#include "misc/shared.h"
#include "dom/dom_string.h"
#include "xml/dom_stringimpl.h"
#include "wtf/Vector.h"
#include <QHash>

using DOM::DOMString;

namespace khtml {

/**
 An IDString is used to manage an identifier namespace that has some predefined constants,
 but can be extended with new ones at runtime

 The TableFactory template parameter must point to a class
 that provides an idTable method returning a singleton IDTable<TableFactory>
*/
template<typename TableFactory>
class IDString {
private:
    unsigned short m_id;

public:
    // id 0xFFFF is handled specially in derefId so it's our default..
    IDString(): m_id(0xFFFF) {}

    IDString<TableFactory>& operator=(const IDString& other) {
        TableFactory::idTable()->refId  (other.m_id);
        TableFactory::idTable()->derefId(m_id);
        m_id = other.m_id;
        return *this;
    }

    IDString(const IDString& other) {
        m_id = other.m_id;
        TableFactory::idTable()->refId(m_id);
    }

    ~IDString() {
        TableFactory::idTable()->derefId(m_id);
    }

    unsigned id() const {
        return m_id;
    }

    DOMString toString() const {
        return TableFactory::idTable()->idToString(m_id);
    }

    static IDString<TableFactory> fromId(unsigned short id) {
        IDString<TableFactory> nw;
        nw.m_id = id;
        TableFactory::idTable()->refId(id);
        return nw;
    }

    static IDString<TableFactory> fromString(const DOMString& string) {
        IDString<TableFactory> nw;
        nw.m_id = TableFactory::idTable()->grabId(string); // Refs it already.
        return nw;
    }

    bool operator==(const IDString<TableFactory>& other) const {
        return m_id == other.m_id;
    }
};

class IDTableBase {
    struct Mapping {
        unsigned   refCount; // # of references, 0 if not in use.
        DOMString  name;

        Mapping(): refCount(0)
        {}

        Mapping(const DOMString& _name): refCount(0), name(_name)
        {}
    };
protected:
    void refId(unsigned id) {
        ++m_mappings[id].refCount;
    }

    void derefId(unsigned id) {
        if (id == 0xFFFF)
            return;
        --m_mappings[id].refCount;
        if (m_mappings[id].refCount == 0)
            releaseId(id);
    }

    const DOMString& idToString(unsigned id) {
        return m_mappings[id].name;
    }

    unsigned short grabId(const DOMString& string);

public:
    // Registers a compile-type known ID constant with the name.
    // This must be called before any other operations
    void addStaticMapping(unsigned id, const DOMString& string);

    // Registers a hidden ID constant --- it has a name, but it
    // can not be looked up by it.
    void addHiddenMapping(unsigned id, const DOMString& string);
private:
    void releaseId(unsigned id);

    WTF::Vector <unsigned> m_idFreeList;
    WTF::Vector <Mapping>  m_mappings;
    QHash       <DOMString, unsigned short> m_mappingLookup;
};

template<typename TableFactory>
class IDTable: public IDTableBase {
public:
    // Export methods to our version of IDString
    friend class IDString<TableFactory>;
    using IDTableBase::refId;
    using IDTableBase::derefId;
    IDTable() {}
};

}

/**
 Now these are the various ID types we used.. They are here to avoid circular
 dependenies in headers
*/
namespace DOM {
    class EventImpl;
    typedef khtml::IDString<EventImpl> EventName;
}


#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
