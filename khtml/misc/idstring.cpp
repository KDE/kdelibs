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

CaseNormalizeMode IDTableBase::MappingKey::caseNormalizationMode;

bool IDTableBase::MappingKey::operator==(const MappingKey& other) const
{
    if (IDTableBase::MappingKey::caseNormalizationMode == IDS_CaseSensitive)
        return str == other.str;
    else
        return !strcasecmp(str, other.str);
}

static inline unsigned int qHash(const IDTableBase::MappingKey& key) {
    if (key.str.isNull() || key.caseNormalizationMode == IDS_CaseSensitive) {
        return qHash(key.str);
    } else if (key.caseNormalizationMode == IDS_NormalizeLower) {
        return key.str.implementation()->lowerHash();
    } else { // caseNormalizationMode == IDS_NormalizeUpper
        return key.str.implementation()->upperHash();
    }
}

void IDTableBase::releaseId(unsigned id)
{
    IDTableBase::MappingKey::caseNormalizationMode = IDS_CaseSensitive;

    m_mappingLookup.remove(m_mappings[id].name);
    m_idFreeList.append(id);
}

unsigned short IDTableBase::grabId(const DOMString& origName, CaseNormalizeMode cnm)
{
    unsigned short newId;

    // Check for existing one, ignoring case if needed
    IDTableBase::MappingKey::caseNormalizationMode = cnm;
    QHash<MappingKey, unsigned short>::const_iterator i = m_mappingLookup.constFind(origName);
    if (i != m_mappingLookup.constEnd()) {
        newId = *i;
        refId(newId);
        return newId;
    }

    // Nope. Allocate new ID. If there is normalization going on, we may now have to 
    // update our case so the canonical mapping is of the expected case.
    DOMString name;
    switch (cnm) {
    case IDS_CaseSensitive:
        name = origName;
        break;
    case IDS_NormalizeUpper:
        name = origName.upper();
        break;
    case IDS_NormalizeLower:
        name = origName.lower();
        break;
    }

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
    IDTableBase::MappingKey::caseNormalizationMode = IDS_CaseSensitive;

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
