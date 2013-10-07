/*
 * Copyright (C) 2006, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
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
 */

#include "wtf/Platform.h"

#if ENABLE(SVG)
#include "JSSVGPathSegList.h"

#include "Document.h"
#include "Frame.h"
#include "JSSVGPathSeg.h"
#include "SVGDocumentExtensions.h"
#include "SVGElement.h"
#include "SVGPathSegList.h"

#include <wtf/Assertions.h>

using namespace KJS;
using namespace DOM;

namespace khtml {

KJS::JSValue* JSSVGPathSegList::clear(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());
    imp->clear(ec);

    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());
    return jsUndefined();
}

KJS::JSValue* JSSVGPathSegList::initialize(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;
    SVGPathSeg* newItem = toSVGPathSeg(args[0]);

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());

    SVGPathSeg* obj = WTF::getPtr(imp->initialize(newItem, ec));

    KJS::JSValue* result = toJS(exec, obj, m_context.get());
    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());    
    return result;
}

KJS::JSValue* JSSVGPathSegList::getItem(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;

    bool indexOk;
    unsigned index = args[0]->toInt32(exec, indexOk);
    if (!indexOk) {
        setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
        return jsUndefined();
    }

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());
    SVGPathSeg* obj = WTF::getPtr(imp->getItem(index, ec));

    KJS::JSValue* result = toJS(exec, obj, m_context.get());
    setDOMException(exec, ec);
    return result;
}

KJS::JSValue* JSSVGPathSegList::insertItemBefore(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;
    SVGPathSeg* newItem = toSVGPathSeg(args[0]);

    bool indexOk;
    unsigned index = args[1]->toInt32(exec, indexOk);
    if (!indexOk) {
        setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
        return jsUndefined();
    }

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());

    KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->insertItemBefore(newItem, index, ec)), m_context.get());
    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());    
    return result;
}

KJS::JSValue* JSSVGPathSegList::replaceItem(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;
    SVGPathSeg* newItem = toSVGPathSeg(args[0]);
    
    bool indexOk;
    unsigned index = args[1]->toInt32(exec, indexOk);
    if (!indexOk) {
        setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
        return jsUndefined();
    }

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());

    KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->replaceItem(newItem, index, ec)), m_context.get());
    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());    
    return result;
}

KJS::JSValue* JSSVGPathSegList::removeItem(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;
    
    bool indexOk;
    unsigned index = args[0]->toInt32(exec, indexOk);
    if (!indexOk) {
        setDOMException(exec, DOMException::TYPE_MISMATCH_ERR);
        return jsUndefined();
    }

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());

    RefPtr<SVGPathSeg> obj(imp->removeItem(index, ec));

    KJS::JSValue* result = toJS(exec, obj.get(), m_context.get());
    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());    
    return result;
}

KJS::JSValue* JSSVGPathSegList::appendItem(ExecState* exec, const List& args)
{
    ExceptionCode ec = 0;
    SVGPathSeg* newItem = toSVGPathSeg(args[0]);

    SVGPathSegList* imp = static_cast<SVGPathSegList*>(impl());

    KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->appendItem(newItem, ec)), m_context.get());
    setDOMException(exec, ec);

    m_context->svgAttributeChanged(imp->associatedAttributeName());    
    return result;
}

}

#endif // ENABLE(SVG)
