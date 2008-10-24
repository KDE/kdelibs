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

#include "idstring.h"
#include <assert.h>

namespace khtml {

void IDTableBase::releaseId(unsigned id)
{
    m_mappingLookup.remove(m_mappings[id].name);
    m_idFreeList.append(id);
}

unsigned short IDTableBase::grabId(const DOMString& name)
{
    unsigned short newId;

    // Check for existing one
    QHash<DOMString, unsigned short>::const_iterator i = m_mappingLookup.constFind(name);
    if (i != m_mappingLookup.constEnd()) {
        newId = *i;
        refId(newId);
        return newId;
    }

    // Nope. Allocate new ID.
    if (!m_idFreeList.isEmpty()) {
        // Grab from freelist..
        newId = m_idFreeList.last();
        m_idFreeList.removeLast();
        m_mappings[newId].name = name;
    } else {
        // Make a new one --- if we can (we keep one spot for "last resort" mapping)
        if (m_mappings.size() < 0xFFFE) {
            m_mappings.append(Mapping(name));
            newId = m_mappings.size() - 1;
        } else {
            // We ran out of resources. Did we add a fallback mapping yet?
            if (m_mappings.size() == 0xFFFE) {
                // Have an ID for "everything else" as last resource. This sucks
                m_mappings.append(Mapping("_khtml_fallback"));
                m_mappings[0xFFFF].refCount = 1; // pin it.
            }
            newId = 0xFFFF;
        }
    }

    m_mappingLookup[name] = newId;

    refId(newId);
    return newId;
}

void IDTableBase::addStaticMapping(unsigned id, const DOMString& name)
{
    assert(id == m_mappings.size());
    assert(!m_mappingLookup.contains(name));
    m_mappings.append(Mapping(name));
    m_mappings[m_mappings.size() - 1].refCount = 1; // Pin it.
    m_mappingLookup[name] = id;
}

void IDTableBase::addHiddenMapping(unsigned id, const DOMString& name)
{
    assert(id == m_mappings.size());
    m_mappings.append(Mapping(name));
    m_mappings[m_mappings.size() - 1].refCount = 1; // Pin it.    
}

}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
