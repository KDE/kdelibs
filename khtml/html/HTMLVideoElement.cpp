/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "HTMLVideoElement.h"

#include "HTMLDocument.h"
#include <misc/htmlhashes.h>

namespace khtml {

HTMLVideoElement::HTMLVideoElement(Document* doc)
    : HTMLMediaElement(doc)
{
}

DOM::NodeImpl::Id HTMLVideoElement::id() const
{
    return ID_VIDEO;
}

int HTMLVideoElement::width() const
{
    bool ok;
    int w = getAttribute(ATTR_WIDTH).toInt(&ok);
    return ok ? w : 0;
}

void HTMLVideoElement::setWidth(int value)
{
    setAttribute(ATTR_WIDTH, QString::number(value));
}
    
int HTMLVideoElement::height() const
{
    bool ok;
    int h = getAttribute(ATTR_HEIGHT).toInt(&ok);
    return ok ? h : 0;
}
    
void HTMLVideoElement::setHeight(int value)
{
    setAttribute(ATTR_HEIGHT, QString::number(value));
}

DOMString HTMLVideoElement::poster() const
{
    return getDocument()->completeURL(getAttribute(ATTR_POSTER).string());
}

void HTMLVideoElement::setPoster(const DOMString& value)
{
    setAttribute(ATTR_POSTER, value);
}

}
