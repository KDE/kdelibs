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

#include "dom_restyler.h"
#include "dom_elementimpl.h"

namespace khtml {

DynamicDomRestyler::DynamicDomRestyler()
{
}

void DynamicDomRestyler::addDependency(ElementImpl* subject, ElementImpl* dependency, StructuralDependencyType type)
{
    assert(type < LastStructuralDependency);
    if (subject == dependency && type == HoverDependency) {
        subject->setHasHoverDependency(true);
        return;
    }

    dependency_map[type].insert(dependency, subject);
    reverse_map.insert(subject,dependency);
}

void DynamicDomRestyler::removeDependency(ElementImpl* subject, ElementImpl* dependency, StructuralDependencyType type)
{
    if (subject == dependency && type == HoverDependency) {
        subject->setHasHoverDependency(false);
        return;
    }
    dependency_map[type].remove(dependency, subject);
    // don't remove from reverse_map as there might be other dependencies to the same element
}

void DynamicDomRestyler::removeDependencies(ElementImpl* subject, StructuralDependencyType type)
{
    if (type == HoverDependency)
        subject->setHasHoverDependency(false);

    QSet<ElementImpl*>* my_dependencies = reverse_map.find(subject);

    if (!my_dependencies) return;

    QSet<ElementImpl*>::iterator it = my_dependencies->begin();
    while ( it != my_dependencies->end() )
    {
        ElementImpl* e = *it;
        dependency_map[type].remove(e,subject);

        ++it;
    }

    // don't remove from reverse_map as there might be other dependencies to the same elements
}

void DynamicDomRestyler::resetDependencies(ElementImpl* subject)
{
    subject->setHasHoverDependency(false);

    QSet<ElementImpl*>* my_dependencies = reverse_map.find(subject);

    if (!my_dependencies) return;

    QSet<ElementImpl*>::iterator it = my_dependencies->begin();
    while ( it != my_dependencies->end() )
    {
        ElementImpl* e = *it;
        for (int type = 0; type < LastStructuralDependency; type++) {
            dependency_map[type].remove(e,subject);
        }
        ++it;
    }

    reverse_map.remove(subject);
}

void DynamicDomRestyler::restyleDependent(ElementImpl* dependency, StructuralDependencyType type)
{
    assert(type < LastStructuralDependency);
    if (type == HoverDependency && dependency->hasHoverDependency())
        dependency->setChanged(true);

    QSet<ElementImpl*>* dep = dependency_map[type].find(dependency);

    if (!dep) return;

    // take copy as the restyle will change the list
    QSet<ElementImpl*> dependent(*dep);

    QSet<ElementImpl*>::iterator it = dependent.begin();
    while ( it != dependent.end() )
    {
//         kDebug() << "Restyling dependent";
        (*it)->setChanged(true);
        ++it;
    }
}

void DynamicDomRestyler::dumpStats() const
{
/*
    kDebug() << "Keys in structural dependencies: " << dependency_map[StructuralDependency].size();
    kDebug() << "Keys in attribute dependencies: " << dependency_map[AttributeDependency].size();

    kDebug() << "Keys in reverse map: " << reverse_map.size();
    */
}

void DynamicDomRestyler::addDependency(uint attrID, AttributeDependencyType type)
{
    assert(type < LastAttributeDependency);

    unsigned int hash = (attrID * 257) % 512;
    attribute_map[type][hash] = true;
}

bool DynamicDomRestyler::checkDependency(uint attrID, AttributeDependencyType type)
{
    assert(type < LastAttributeDependency);

    unsigned int hash = (attrID * 257) % 512;
    // ### gives false positives, but that's okay.
    return attribute_map[type][hash];
}

} // namespace
