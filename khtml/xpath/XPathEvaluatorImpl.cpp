/*
 * XPathEvaluatorImpl.cpp - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#include "XPathEvaluatorImpl.h"
#include "XPathExceptionImpl.h"
#include "XPathExpressionImpl.h"
#include "XPathNSResolverImpl.h"
#include "XPathResultImpl.h"

#include "impl/util.h"

using namespace DOM;
using namespace DOM::XPath;
using namespace khtml;

XPathExpressionImpl *XPathEvaluatorImpl::createExpression( DOMStringImpl *expression,
                                                           XPathNSResolverImpl *resolver,
                                                           int & )
{
	return new XPathExpressionImpl( expression, resolver );
}

XPathNSResolverImpl *XPathEvaluatorImpl::createNSResolver( NodeImpl *nodeResolver )
{
	return new XPathNSResolverImpl( nodeResolver );
}

XPathResultImpl *XPathEvaluatorImpl::evaluate( DOMStringImpl *expression,
                                               NodeImpl *contextNode,
                                               XPathNSResolverImpl *resolver,
                                               unsigned short type,
                                               XPathResultImpl *result,
                                               int &exceptioncode )
{
	if ( !isValidContextNode( contextNode ) ) {
		exceptioncode = NOT_SUPPORTED_ERR;
		return 0;
	}

	XPathExpressionImpl *expr = createExpression( expression, resolver, exceptioncode );
	if ( exceptioncode )
		return 0;

	XPathResultImpl *res = expr->evaluate( contextNode, type, result, exceptioncode );
	if ( exceptioncode )
		return 0;

	delete expr;
	if ( result ) {
		result = res;
		result->ref();
	}
	return res;
}

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
