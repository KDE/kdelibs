/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2003 Apple Computer, Inc.
 *           (C) 2006 Allan Sandfeld Jensen(kde@carewolf.com)
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

#ifndef Document_h
#define Document_h

#include "xml/dom_docimpl.h"
#include "Frame.h"
#include "xml/dom_nodeimpl.h"
#include "misc/htmlnames.h"
//#include "wtf/PassRefPtr.h"
#include "dom/QualifiedName.h"
#include "css/css_valueimpl.h"
#include "css/cssproperties.h"
//#include "css/cssstyleselector.h"
#include "css/css_stylesheetimpl.h"
#include "dom/ExceptionCode.h"
#include "xml/dom2_eventsimpl.h"

#define CSSPropertyWidth CSS_PROP_WIDTH
#define CSSPropertyHeight CSS_PROP_HEIGHT

namespace WebCore
{
    using namespace khtml;
    using namespace DOM;
    typedef DocumentImpl Document;
    typedef DOMImplementationImpl DOMImplementation;
    typedef ElementImpl Element;
    typedef ElementImpl StyledElement;
    typedef DOMString String;
    typedef DOMStringImpl StringImpl;
    typedef NodeImpl Node;
    typedef AttributeImpl Attribute;
    typedef EventImpl Event;
    typedef RegisteredListenerList RegisteredEventListenerList;
    typedef CSSStyleDeclarationImpl CSSStyleDeclaration;
    typedef QChar UChar;
    typedef AttributeImpl MappedAttribute;
    //typedef RenderCanvas RenderView;
    typedef StyleSheetImpl StyleSheet;
    //typedef QColor Color;
    typedef NodeImpl EventTargetNode;
}

#endif
