/*
    Copyright (C) 2006 Apple Computer, Inc.
                  2006 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the WebKit project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SVGDocumentExtensions_h
#define SVGDocumentExtensions_h

#if ENABLE(SVG)

#include <memory>
#include <wtf/Forward.h>
#include <wtf/HashSet.h>
#include <wtf/HashMap.h>

#include "FloatRect.h"
#include "StringHash.h"
//#include "StringImpl.h"
#include "AtomicString.h"
#include "xml/Document.h"

namespace DOM {
    class EventListener;
}

namespace WebCore {

//class AtomicString;
//class Document;
//class EventListener;
//class Node;
//class String;
class SVGElement;
class SVGElementInstance;
class SVGStyledElement;
class SVGSVGElement;
class TimeScheduler;

class DOMStringHash
{
    public:
        static unsigned hash(DOMString key) { return qHash(key.implementation()); }
        static bool equal(DOMString a, DOMString b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = false;
};

class SVGDocumentExtensions {
public:
    SVGDocumentExtensions(Document*);
    ~SVGDocumentExtensions();
    
    DOM::EventListener* createSVGEventListener(const DOMString& functionName, const DOMString& code, DOM::NodeImpl*);
    
    void addTimeContainer(SVGSVGElement*);
    void removeTimeContainer(SVGSVGElement*);
    
    void startAnimations();
    void pauseAnimations();
    void unpauseAnimations();

    void reportWarning(const String&);
    void reportError(const String&);

private:
    Document* m_doc; // weak reference
    HashSet<SVGSVGElement*> m_timeContainers; // For SVG 1.2 support this will need to be made more general.
    //HashMap<String, HashSet<SVGStyledElement*>*, DOMStringHash> m_pendingResources;
    HashMap<SVGElement*, HashSet<SVGElementInstance*>*> m_elementInstances;

    SVGDocumentExtensions(const SVGDocumentExtensions&);
    SVGDocumentExtensions& operator=(const SVGDocumentExtensions&);

    template<typename ValueType>
    HashMap<const SVGElement*, HashMap<StringImpl*, ValueType>*>* baseValueMap() const
    {
        static HashMap<const SVGElement*, HashMap<StringImpl*, ValueType>*>* s_baseValueMap = new HashMap<const SVGElement*, HashMap<StringImpl*, ValueType>*>();
        return s_baseValueMap;
    }

public:
    // This HashMap contains a list of pending resources. Pending resources, are such
    // which are referenced by any object in the SVG document, but do NOT exist yet.
    // For instance, dynamically build gradients / patterns / clippers...
    void addPendingResource(const AtomicString& id, SVGStyledElement*);
    bool isPendingResource(const AtomicString& id) const;
    std::auto_ptr<HashSet<SVGStyledElement*> > removePendingResource(const AtomicString& id);

    // This HashMap maps elements to their instances, when they are used by <use> elements.
    // This is needed to synchronize the original element with the internally cloned one.
    void mapInstanceToElement(SVGElementInstance*, SVGElement*);
    void removeInstanceMapping(SVGElementInstance*, SVGElement*);
    HashSet<SVGElementInstance*>* instancesForElement(SVGElement*) const;

    // Used by the ANIMATED_PROPERTY_* macros
    template<typename ValueType>
    ValueType baseValue(const SVGElement* element, const AtomicString& propertyName) const
    {
        HashMap<StringImpl*, ValueType>* propertyMap = baseValueMap<ValueType>()->get(element);
        if (propertyMap)
            return propertyMap->get(propertyName.impl());

        return 0;
    }

    template<typename ValueType>
    void setBaseValue(const SVGElement* element, const AtomicString& propertyName, ValueType newValue)
    {
        HashMap<StringImpl*, ValueType>* propertyMap = baseValueMap<ValueType>()->get(element);
        if (!propertyMap) {
            propertyMap = new HashMap<StringImpl*, ValueType>();
            baseValueMap<ValueType>()->set(element, propertyMap);
        }

        propertyMap->set(propertyName.impl(), newValue);
    }

    template<typename ValueType>
    void removeBaseValue(const SVGElement* element, const AtomicString& propertyName)
    {
        HashMap<StringImpl*, ValueType>* propertyMap = baseValueMap<ValueType>()->get(element);
        if (!propertyMap)
            return;

        propertyMap->remove(propertyName.impl());
    }

    template<typename ValueType>
    bool hasBaseValue(const SVGElement* element, const AtomicString& propertyName) const
    {
        HashMap<StringImpl*, ValueType>* propertyMap = baseValueMap<ValueType>()->get(element);
        if (propertyMap)
            return propertyMap->contains(propertyName.impl());

        return false;
    }
};

// Special handling for WebCore::String
template<>
inline String SVGDocumentExtensions::baseValue<String>(const SVGElement* element, const AtomicString& propertyName) const
{
    HashMap<StringImpl*, String>* propertyMap = baseValueMap<String>()->get(element);
    if (propertyMap)
        return propertyMap->get(propertyName.impl());

    return String();
}

// Special handling for WebCore::FloatRect
template<>
inline FloatRect SVGDocumentExtensions::baseValue<FloatRect>(const SVGElement* element, const AtomicString& propertyName) const
{
    HashMap<StringImpl*, FloatRect>* propertyMap = baseValueMap<FloatRect>()->get(element);
    if (propertyMap)
        return propertyMap->get(propertyName.impl());

    return FloatRect();
}

// Special handling for booleans
template<>
inline bool SVGDocumentExtensions::baseValue<bool>(const SVGElement* element, const AtomicString& propertyName) const
{
    HashMap<StringImpl*, bool>* propertyMap = baseValueMap<bool>()->get(element);
    if (propertyMap)
        return propertyMap->get(propertyName.impl());

    return false;
}

// Special handling for doubles
template<>
inline double SVGDocumentExtensions::baseValue<double>(const SVGElement* element, const AtomicString& propertyName) const
{
    HashMap<StringImpl*, double>* propertyMap = baseValueMap<double>()->get(element);
    if (propertyMap)
        return propertyMap->get(propertyName.impl());

    return 0.0;
}

}

#endif // ENABLE(SVG)

#endif
