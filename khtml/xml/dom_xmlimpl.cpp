/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2000 Peter Kelly (pmk@post.com)
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */


#include "dom/dom_text.h"
#include "dom_xmlimpl.h"
#include "dom_docimpl.h"
#include "dom_stringimpl.h"

using namespace DOM;

EntityImpl::EntityImpl(DocumentImpl *doc) : NodeImpl(doc)
{
    m_publicId = 0;
    m_systemId = 0;
    m_notationName = 0;
}

EntityImpl::EntityImpl(DocumentImpl *doc, DOMString _publicId, DOMString _systemId, DOMString _notationName) : NodeImpl(doc)
{
    m_publicId = _publicId.implementation();
    if (m_publicId)
	m_publicId->ref();
    m_systemId = _systemId.implementation();
    if (m_systemId)
	m_systemId->ref();
    m_notationName = _notationName.implementation();
    if (m_notationName)
	m_notationName->ref();
}


EntityImpl::~EntityImpl()
{
    if (m_publicId)
	m_publicId->deref();
    if (m_systemId)
	m_systemId->deref();
    if (m_notationName)
	m_notationName->deref();
}

const DOMString EntityImpl::nodeName() const
{
    // ### return entity name
    return 0;
}

unsigned short EntityImpl::nodeType() const
{
    return Node::ENTITY_NODE;
}

DOMString EntityImpl::publicId() const
{
    return m_publicId;
}

DOMString EntityImpl::systemId() const
{
    return m_systemId;
}

DOMString EntityImpl::notationName() const
{
    return m_notationName;
}

// -------------------------------------------------------------------------

EntityReferenceImpl::EntityReferenceImpl(DocumentImpl *doc) : NodeImpl(doc)
{
}

EntityReferenceImpl::~EntityReferenceImpl()
{
}

const DOMString EntityReferenceImpl::nodeName() const
{
    // ### return name of entity referenced
    return 0;
}

unsigned short EntityReferenceImpl::nodeType() const
{
    return Node::ENTITY_REFERENCE_NODE;
}

// -------------------------------------------------------------------------

NotationImpl::NotationImpl(DocumentImpl *doc) : NodeImpl(doc)
{
    m_publicId = 0;
    m_systemId = 0;
}

NotationImpl::NotationImpl(DocumentImpl *doc, DOMString _publicId, DOMString _systemId) : NodeImpl(doc)
{
    m_publicId = _publicId.implementation();
    if (m_publicId)
	m_publicId->ref();
    m_systemId = _systemId.implementation();
    if (m_systemId)
	m_systemId->ref();
}

NotationImpl::~NotationImpl()
{
    if (m_publicId)
	m_publicId->deref();
    if (m_systemId)
	m_systemId->deref();
}

const DOMString NotationImpl::nodeName() const
{
    // ### return notation name
    return 0;
}

unsigned short NotationImpl::nodeType() const
{
    return Node::NOTATION_NODE;
}

DOMString NotationImpl::publicId() const
{
    return m_publicId;
}

DOMString NotationImpl::systemId() const
{
    return m_systemId;
}

// -------------------------------------------------------------------------

ProcessingInstructionImpl::ProcessingInstructionImpl(DocumentImpl *doc) : NodeImpl(doc)
{
    m_target = 0;
    m_data = 0;
}

ProcessingInstructionImpl::ProcessingInstructionImpl(DocumentImpl *doc, DOMString _target, DOMString _data) : NodeImpl(doc)
{
    m_target = _target.implementation();
    if (m_target)
	m_target->ref();
    m_data = _data.implementation();
    if (m_data)
	m_data->ref();
}

ProcessingInstructionImpl::~ProcessingInstructionImpl()
{
    if (m_target)
	m_target->deref();
    if (m_data)
	m_data->deref();	
}

const DOMString ProcessingInstructionImpl::nodeName() const
{
    // ###
    return "target";
}

unsigned short ProcessingInstructionImpl::nodeType() const
{
    return Node::PROCESSING_INSTRUCTION_NODE;
}

DOMString ProcessingInstructionImpl::nodeValue() const
{
    // ### return entire content excluding the target
    return 0;
}

DOMString ProcessingInstructionImpl::target() const
{
    return m_target;
}

DOMString ProcessingInstructionImpl::data() const
{
    // ###
    return m_data;
}

void ProcessingInstructionImpl::setData( const DOMString &_data )
{
    if (m_data)
	m_data->deref();
    m_data = _data.implementation();
    if (m_data)
	m_data->ref();
}








