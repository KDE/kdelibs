/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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


#ifndef DOM_H
#define DOM_H

class QDomNode;
class QDomDocument;
class QDomElement;
class QDomAttr;
class QDomDocumentType;
class QDomNodeList;
class QDomNamedNodeMap;
class QDomText;

#include "value_binding.h"
#include "static_binding.h"

namespace KJSEmbed
{
    class DomNodeBinding : public ValueBinding
    {
        public:
            DomNodeBinding( KJS::ExecState *exec, const QDomNode &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomDocumentBinding : public ValueBinding
    {
        public:
            DomDocumentBinding( KJS::ExecState *exec, const QDomDocument &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomElementBinding : public ValueBinding
    {
        public:
            DomElementBinding( KJS::ExecState *exec, const QDomElement &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomAttrBinding : public ValueBinding
    {
        public:
            DomAttrBinding( KJS::ExecState *exec, const QDomAttr &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomDocumentTypeBinding : public ValueBinding
    {
        public:
            DomDocumentTypeBinding( KJS::ExecState *exec, const QDomDocumentType &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomNodeListBinding : public ValueBinding
    {
        public:
            DomNodeListBinding( KJS::ExecState *exec, const QDomNodeList &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomNamedNodeMapBinding : public ValueBinding
    {
        public:
            DomNamedNodeMapBinding( KJS::ExecState *exec, const QDomNamedNodeMap &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    class DomTextBinding : public ValueBinding
    {
        public:
            DomTextBinding( KJS::ExecState *exec, const QDomText &value );
        private:
            virtual const KJS::ClassInfo* classInfo() const { return &info; }
            static const KJS::ClassInfo info;
    };

    KJS_BINDING( DomNode )
    KJS_BINDING( DomDocument ) // Done
    KJS_BINDING( DomElement ) // Done
    KJS_BINDING( DomAttr )	// Done
    KJS_BINDING( DomDocumentType ) // Done
    KJS_BINDING( DomNodeList ) // Done
    KJS_BINDING( DomNamedNodeMap )
    KJS_BINDING( DomText )

}
#endif
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
