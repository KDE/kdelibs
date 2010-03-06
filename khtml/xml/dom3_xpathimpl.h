/*
 * dom3_xpathimpl.h - Copyright 2005 Frerich Raabe <raabe@kde.org>
 *                      Copyright 2010 Maksim Orlovich <maksim@kde.org>
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
#ifndef XPATHRESULTIMPL_H
#define XPATHRESULTIMPL_H

#include "misc/shared.h"
#include "xpath/expression.h"
#include "xpath/parsedstatement.h"


namespace DOM {
    class DOMStringImpl;
    class NodeImpl;
}

namespace khtml {

class XPathResultImpl : public Shared<XPathResultImpl>
{
	public:
		XPathResultImpl();
		XPathResultImpl( const Value &value );

		void convertTo( unsigned short type, int &exceptioncode );

		unsigned short resultType() const;

		double numberValue( int &exceptioncode ) const;
		DOMStringImpl *stringValue( int &exceptioncode ) const;
		bool booleanValue( int &exceptioncode ) const;
		NodeImpl *singleNodeValue( int &exceptioncode ) const;

		bool invalidIteratorState() const;
		unsigned long snapshotLength( int &exceptioncode ) const;
		NodeImpl *iterateNext( int &exceptioncode );
		NodeImpl *snapshotItem( unsigned long index, int &exceptioncode );

	private:
		Value m_value;
		DomNodeList::Iterator m_nodeIterator;
		unsigned short m_resultType;
};

class XPathExpressionImpl : public Shared<XPathExpressionImpl>
{
	public:
		XPathExpressionImpl( DOMStringImpl *expression,
		                     XPathNSResolverImpl *resolver );

		XPathResultImpl *evaluate( NodeImpl *contextNode,
		                           unsigned short type,
		                           XPathResultImpl *result,
		                           int &exceptioncode );


	private:
		XPath::ParsedStatement m_statement;
};

// This is the base class for resolver interfaces
class XPathNSResolverImpl : public khtml::Shared<XPathNSResolverImpl>
{
	public:
		enum Type {
			Default,
			JS,
			CPP
		}

		virtual DOMString lookupNamespaceURI( const DOM::DOMString& prefix ) = 0;
		virtual Type      type() = 0;
};

// This is the default implementation, used by createNSResolver
class DefaultXPathNSResolverImpl : public XPathNSResolverImpl
{
	public:
		DefaultXPathNSResolverImpl( NodeImpl *node );

		virtual DOMString lookupNamespaceURI( const DOM::DOMString& prefix );
		virtual Type      type() { return Default; }

	private:
		NodeImpl *m_node;
};


class XPathEvaluatorImpl
{
	public:
		static XPathExpressionImpl *createExpression( DOMStringImpl *expression,
		                                              XPathNSResolverImpl *resolver,
		                                              int &exceptioncode );
		static XPathNSResolverImpl *createNSResolver( NodeImpl *nodeResolver );
		static XPathResultImpl *evaluate( DOMStringImpl *expression,
		                                   NodeImpl *contextNode,
		                                   XPathNSResolverImpl *resolver,
		                                   unsigned short type,
		                                   XPathResultImpl *result,
		                                   int &exceptioncode );
};




} // namespace khtml

#endif // XPATHRESULTIMPL_H

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
