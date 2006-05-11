/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2006 Allan Sandfeld Jensen (kde@carewolf.com)
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

#ifndef _DOM_restyler_h_
#define _DOM_restyler_h_

#include "misc/multimap.h"
#include <bitset>

namespace DOM {
    class ElementImpl;
}

// Restyle dependency tracker for dynamic DOM

namespace khtml {

using DOM::ElementImpl;

// These types are different types of dependencies, and serves to identify which element should be
// restyled when a change of that kind triggers on the element
enum StructuralDependencyType {
        // Style relies on the children of the element (unaffected by append & close)
        StructuralDependency = 0,
        // Style relies on the last children of the element (affected by append & close)
        BackwardsStructuralDependency = 1,
        // Style relies on the element having hover
        HoverDependency = 2,
        // Style relies on the element being active
        ActiveDependency = 3,
        // Style relies on another state of element (focus, disabled, checked, etc.)
        // (focus is special cased though since elements always depend on their own focus)
        OtherStateDependency = 4,
        LastStructuralDependency
};

// Attribute dependencies are much coarser than structural, for memory reasons rather than performance
// This tracks global depencies of various kinds.
// The groups are separated into where possible depending elements might be:
enum AttributeDependencyType {
        // Style of the changed element depend on this attribute
        PersonalDependency = 0,
        // Style of the elements children depend on this attribute
        AncestorDependency = 1,
        // Style of the elements later siblings or their children depend on this attribute
        PredecessorDependency = 2,
        LastAttributeDependency
};


/**
 * @internal
 */
class DynamicDomRestyler {
public:
    DynamicDomRestyler();

    // Structural dependencies are tracked from element to subject
    void addDependency(ElementImpl* subject, ElementImpl* dependency, StructuralDependencyType type);
    void resetDependencies(ElementImpl* subject);
    void removeDependency(ElementImpl* subject, ElementImpl* dependency, StructuralDependencyType type);
    void removeDependencies(ElementImpl* subject, StructuralDependencyType type);
    void restyleDepedent(ElementImpl* dependency, StructuralDependencyType type);

    // Attribute dependencies are traced on attribute alone
    void addDependency(uint attrID, AttributeDependencyType type);
    bool checkDependency(uint attrID, AttributeDependencyType type);

    void dumpStats() const;
private:
     // Map of dependencies.
     KMultiMap<ElementImpl> dependency_map[LastStructuralDependency];
     // Map of reverse dependencies. For fast reset
     KMultiMap<ElementImpl> reverse_map;

     // Map of the various attribute dependencies
     std::bitset<512> attribute_map[LastAttributeDependency];
};

}

#endif

