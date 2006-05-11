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

    dependency_map[type].append(dependency, subject);
    reverse_map.append(subject,dependency);
}

void DynamicDomRestyler::removeDependency(ElementImpl* subject, ElementImpl* dependency, StructuralDependencyType type)
{
    dependency_map[type].remove(dependency, subject);
    // don't remove from reverse_map as there might be other depencies to the same element
}

void DynamicDomRestyler::removeDependencies(ElementImpl* subject, StructuralDependencyType type)
{
    KMultiMap<ElementImpl>::List* my_dependencies = reverse_map.find(subject);

    if (!my_dependencies) return;

    for (my_dependencies->first(); my_dependencies->current() ; my_dependencies->next())
    {
        ElementImpl* e = my_dependencies->current();
        dependency_map[type].remove(e,subject);
    }

    // don't remove from reverse_map as there might be other depencies to the same elements
}

void DynamicDomRestyler::resetDependencies(ElementImpl* subject)
{
    KMultiMap<ElementImpl>::List* my_dependencies = reverse_map.find(subject);

    if (!my_dependencies) return;

    for (my_dependencies->first(); my_dependencies->current() ; my_dependencies->next())
    {
        ElementImpl* e = my_dependencies->current();
        for (int type = 0; type < LastStructuralDependency; type++) {
            dependency_map[type].remove(e,subject);
        }
    }

    reverse_map.remove(subject);
}

void DynamicDomRestyler::restyleDepedent(ElementImpl* dependency, StructuralDependencyType type)
{
    assert(type < LastStructuralDependency);
    KMultiMap<ElementImpl>::List* dep = dependency_map[type].find(dependency);

    if (!dep) return;

    // take copy as the restyle will change the list
    KMultiMap<ElementImpl>::List dependent(*dep);

    for (dependent.first(); dependent.current() ; dependent.next())
    {
//         kdDebug() << "Restyling dependent" << endl;
        dependent.current()->setChanged(true);
    }
}

void DynamicDomRestyler::dumpStats() const
{
/*
    kdDebug() << "Keys in structural dependencies: " << dependency_map[StructuralDependency].size() << endl;
    kdDebug() << "Keys in attribute dependencies: " << dependency_map[AttributeDependency].size() << endl;

    kdDebug() << "Keys in reverse map: " << reverse_map.size() << endl;
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
