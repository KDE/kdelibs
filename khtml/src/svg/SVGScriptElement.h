/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005, 2007 Rob Buis <buis@kde.org>

    This file is part of the KDE project

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

#ifndef SVGScriptElement_h
#define SVGScriptElement_h
#if ENABLE(SVG)

#include "SVGElement.h"
#include "SVGURIReference.h"
#include "SVGExternalResourcesRequired.h"

namespace WebCore
{
    class SVGScriptElement : public SVGElement,
                             public SVGURIReference,
                             public SVGExternalResourcesRequired
    {
    public:
        SVGScriptElement(const QualifiedName&, Document*);
        virtual ~SVGScriptElement();

        // 'SVGScriptElement' functions
        String type() const;
        void setType(const String&);

        // Internal
        virtual void parseMappedAttribute(MappedAttribute *attr);

        virtual void getSubresourceAttributeStrings(Vector<String>&) const;

        // KHTML ElementImpl pure virtual method
        virtual quint32 id() const { return SVGNames::scriptTag.id(); }
    protected:
        virtual const SVGElement* contextElement() const { return this; }

    private:
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGURIReference, String, Href, href)
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGExternalResourcesRequired, bool, ExternalResourcesRequired, externalResourcesRequired)

        String m_type;
    };

} // namespace WebCore

#endif // ENABLE(SVG)
#endif

// vim:ts=4:noet
