/*
 * XPathExpressionImpl.cpp - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#include "XPathExpressionImpl.h"
#include "xml/dom_stringimpl.h"
#include "xml/dom_nodeimpl.h"
#include "XPathNSResolverImpl.h"
#include "XPathResultImpl.h"

#include "impl/expression.h"
#include "impl/util.h"

using namespace DOM;
using namespace khtml;

XPathExpressionImpl::XPathExpressionImpl( DOMStringImpl *expression, XPathNSResolverImpl *resolver )
{
	Expression::evaluationContext().resolver = resolver;
	m_statement.parse( expression->string() );
}

XPathResultImpl *XPathExpressionImpl::evaluate( NodeImpl *contextNode,
                                                unsigned short type,
                                                XPathResultImpl *result_,
                                                int &exceptioncode )
{
	if ( !isValidContextNode( contextNode ) ) {
		exceptioncode = NOT_SUPPORTED_ERR;
		return 0;
	}

	XPathResultImpl *result = result_ ? result_ : new XPathResultImpl;

	//*result = XPathResultImpl( m_statement.evaluate( contextNode ) );
	if ( type != ANY_TYPE ) {
		result->convertTo( type, exceptioncode );
		if( exceptioncode )
			return 0;
	}

	return result;
}

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
