/*
 * variablereference.cc - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#include "variablereference.h"
#include <dom_stringimpl.h>
using namespace khtml;
using namespace khtml::XPath;

VariableReference::VariableReference( const DOM::DOMString &name )
	: m_name( name )
{
}

bool VariableReference::isConstant() const
{
	return false;
}

QString VariableReference::dump() const
{
	return QString() + "<variablereference name=\"" + m_name.string() + "\"/>";
}

Value VariableReference::doEvaluate() const
{
	QHash<DOM::DOMString, DOM::DOMString>& bindings = evaluationContext().variableBindings;
	if ( !bindings.contains( m_name ) ) {
		// XXX What to do if an unknown variable is referenced?
		return Value( DOMString() );
	}
	return Value( bindings[ m_name ] );
}

