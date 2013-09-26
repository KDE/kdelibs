/*
    Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2008 Rob Buis <buis@kde.org>

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

#ifndef SVGTextElement_h
#define SVGTextElement_h

#if ENABLE(SVG)
#include "SVGTextPositioningElement.h"
#include "SVGTransformable.h"

#include <wtf/OwnPtr.h>

namespace WebCore {

    class SVGTextElement : public SVGTextPositioningElement,
                           public SVGTransformable {
    public:
        SVGTextElement(const QualifiedName&, Document*);
        virtual ~SVGTextElement();

        virtual void parseMappedAttribute(MappedAttribute*);

        virtual SVGElement* nearestViewportElement() const;
        virtual SVGElement* farthestViewportElement() const;

        virtual FloatRect getBBox() const;
        virtual AffineTransform getCTM() const;
        virtual AffineTransform getScreenCTM() const;
        virtual AffineTransform animatedLocalTransform() const;
        virtual AffineTransform* supplementalTransform();

        virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);
        virtual bool childShouldCreateRenderer(Node*) const;
                
        virtual void svgAttributeChanged(const QualifiedName&);

        // KHTML ElementImpl pure virtual method
        virtual quint32 id() const { return SVGNames::textTag.id(); }
    protected:
        virtual const SVGElement* contextElement() const { return this; }

    private:
        ANIMATED_PROPERTY_DECLARATIONS(SVGTextElement, SVGTransformList*, RefPtr<SVGTransformList>, Transform, transform)
       
       // Used by <animateMotion>
       OwnPtr<AffineTransform> m_supplementalTransform;
    };

} // namespace WebCore

#endif // ENABLE(SVG)
#endif
