/*
 * step.h - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#ifndef STEP_H
#define STEP_H

#include "predicate.h"
#include "util.h"

#include <QList>

#include <dom/dom_string.h>

namespace DOM {
	class NodeImpl;
}

namespace khtml {
namespace XPath {

class Step
{
	public:
		enum AxisType {
			AncestorAxis=1, AncestorOrSelfAxis, AttributeAxis,
			ChildAxis, DescendantAxis, DescendantOrSelfAxis,
			FollowingAxis, FollowingSiblingAxis, NamespaceAxis,
			ParentAxis, PrecedingAxis, PrecedingSiblingAxis,
			SelfAxis
		};

		static QString axisAsString( AxisType axis );

		Step();
		Step( AxisType axis,
		      const DomString &nodeTest,
		      const QList<Predicate *> &predicates = QList<Predicate *>() );
		~Step();

		DomNodeList evaluate( DOM::NodeImpl *context ) const;

		void optimize();
		QString dump() const;

	private:
		DomNodeList nodesInAxis( DOM::NodeImpl *context ) const;
		DomNodeList nodeTestMatches( const DomNodeList &nodes ) const;
		QString namespaceFromNodetest( const QString &nodeTest ) const;
		unsigned int primaryNodeType( AxisType axis ) const;

		AxisType m_axis;
		DOM::DOMString m_nodeTest;
		QList<Predicate *> m_predicates;
};

} // namespace XPath

} // namespace khtml


#endif // STEP_H

