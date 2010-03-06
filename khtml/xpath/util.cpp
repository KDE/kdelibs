/*
 * util.cc - Copyright 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "util.h"
#include "xml/dom_nodeimpl.h" 
// #include "DOMStringImpl.h"

#include "../kdomxpath.h"

using namespace DOM;

bool isRootDomNode( NodeImpl *node )
{
	return node && !node->parentNode();
}

DomString stringValue( NodeImpl *node )
{
	switch ( node->nodeType() ) {
		case Node::ATTRIBUTE_NODE:
		case Node::PROCESSING_INSTRUCTION_NODE:
		case Node::COMMENT_NODE:
		case Node::TEXT_NODE:
			return node->nodeValue().string();
		default:
			if ( isRootDomNode( node )
			     || node->nodeType() == Node::ELEMENT_NODE ) {
				QString str;
				DomNodeList nodes = getChildrenRecursively( node );
				foreach ( NodeImpl *node, nodes ) {
					str.append( stringValue( node ) );
				}
				return str;
			}
	}
	return DomString();
}

DomNodeList getChildrenRecursively( NodeImpl *node )
{
	DomNodeList nodes;
	NodeImpl *n = node->firstChild();
	while ( n ) {
		nodes.append( n );
		nodes += getChildrenRecursively( n );
		n = n->nextSibling();
	}
	return nodes;
}

bool isValidContextNode( NodeImpl *node )
{
	return node && ( 
	       node->nodeType() == Node::ELEMENT_NODE ||
	       node->nodeType() == Node::ATTRIBUTE_NODE ||
	       node->nodeType() == Node::TEXT_NODE ||
	       node->nodeType() == Node::CDATA_SECTION_NODE ||
	       node->nodeType() == Node::PROCESSING_INSTRUCTION_NODE ||
	       node->nodeType() == Node::COMMENT_NODE ||
	       node->nodeType() == Node::DOCUMENT_NODE ||
	       node->nodeType() == XPath::XPATH_NAMESPACE_NODE );
}

