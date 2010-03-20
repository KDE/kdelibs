/*
 * step.cc - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#include "step.h"

#include "xml/dom_docimpl.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_textimpl.h"

#include "../kdomxpath.h"
#include "../XPathExceptionImpl.h"
#include "../XPathNSResolverImpl.h"

#include <QtDebug>

using namespace DOM;
using namespace DOM::XPath;
using namespace khtml;
using namespace khtml::XPath;


static bool areAdjacentTextNodes( NodeImpl *n, NodeImpl *m )
{
	if ( !n || !m ) {
		return false;
	}

	if ( n->nodeType() != Node::TEXT_NODE && n->nodeType() != Node::CDATA_SECTION_NODE ) {
		return false;
	}
	if ( m->nodeType() != Node::TEXT_NODE && m->nodeType() != Node::CDATA_SECTION_NODE ) {
		return false;
	}

    //FIXME: are ids supposed to be used like this?
	return ( n->nextSibling() && ( n->nextSibling()->id() == m->id() ) ) ||
	       ( m->nextSibling() && ( m->nextSibling()->id() == n->id() ) );
}

static DomNodeList compressTextNodes( const DomNodeList &nodes )
{
	DomNodeList outNodes;
	DomNodeList::ConstIterator it = nodes.begin();
	DomNodeList::ConstIterator end = nodes.end();
	while ( it != end ) {
		DomNodeList::ConstIterator nextIt = it;
		++nextIt;
		if ( nextIt == end || !areAdjacentTextNodes( *it, *nextIt ) ) {
			outNodes.append( *it );
		} else if ( areAdjacentTextNodes( *it, *nextIt ) ) {
			QString s = ( *it )->nodeValue().string();
			it = nextIt++;
			while ( nextIt != end && areAdjacentTextNodes( *it, *nextIt ) ) {
				s += ( *it )->nodeValue().string();
			}
			outNodes.append( ( *it )->ownerDocument()->createTextNode( new DOMStringImpl( s.data(), s.length() ) ) );
		}
		it = nextIt;

	}
	return outNodes;
}

QString Step::axisAsString( AxisType axis )
{
	switch ( axis ) {
		case AncestorAxis: return "ancestor";
		case AncestorOrSelfAxis: return "ancestor-or-self";
		case AttributeAxis: return "attribute";
		case ChildAxis: return "child";
		case DescendantAxis: return "descendant";
		case DescendantOrSelfAxis: return "descendant-or-self";
		case FollowingAxis: return "following";
		case FollowingSiblingAxis: return "following-sibling";
		case NamespaceAxis: return "namespace";
		case ParentAxis: return "parent";
		case PrecedingAxis: return "preceding";
		case PrecedingSiblingAxis: return "preceding-sibling";
		case SelfAxis: return "self";
	}
	return QString();
}

Step::Step()
{
}

Step::Step( AxisType axis, const DomString &nodeTest,
            const QList<Predicate *> &predicates )
	: m_axis( axis ),
	m_nodeTest( nodeTest ),
	m_predicates( predicates )
{
}

Step::~Step()
{
	qDeleteAll( m_predicates );
}

DomNodeList Step::evaluate( NodeImpl *context ) const
{
	qDebug() << "Evaluating step, axis='" << axisAsString( m_axis ) <<"'"
	         << ", nodetest='" << m_nodeTest << "'"
	         << ", " << m_predicates.count() << " predicates";
	if ( context->nodeType() == Node::ELEMENT_NODE ) {
		qDebug() << "Context node is an element called " << context->nodeName().string();
	} else if ( context->nodeType() == Node::ATTRIBUTE_NODE ) {
		qDebug() << "Context node is an attribute called " << context->nodeName().string() << " with value " << context->nodeValue();
	} else {
		qDebug() << "Context node is of unknown type " << context->nodeType();
	}

	DomNodeList inNodes = nodesInAxis( context ), outNodes;
	qDebug() << "Axis " << axisAsString( m_axis ) << " matches " << inNodes.count() << " nodes.";
	inNodes = nodeTestMatches( inNodes );
	qDebug() << "\tNodetest " << m_nodeTest << " trims this number to " << inNodes.count();

	outNodes = inNodes;

	foreach( Predicate *predicate, m_predicates ) {
		outNodes.clear();
		Expression::evaluationContext().size = inNodes.count();
		Expression::evaluationContext().position = 1;
		foreach( NodeImpl *node, inNodes ) {
			Expression::evaluationContext().node = node;
			EvaluationContext backupCtx = Expression::evaluationContext();
			if ( predicate->evaluate() ) {
				outNodes.append( node );
			}
			Expression::evaluationContext() = backupCtx;
			++Expression::evaluationContext().position;
		}
		qDebug() << "\tPredicate trims this number to " << outNodes.count();
		inNodes = outNodes;
	}

	return outNodes;
}

DomNodeList Step::nodesInAxis( NodeImpl *context ) const
{
	DomNodeList nodes;
	switch ( m_axis ) {
		case ChildAxis: {
			NodeImpl *n = context->firstChild();
			while ( n ) {
				nodes.append( n );
				n = n->nextSibling();
			}
			return nodes;
		}
		case DescendantAxis:
			return getChildrenRecursively( context );
		case ParentAxis:
			nodes.append( context->parentNode() );
			return nodes;
		case AncestorAxis: {
			NodeImpl *n = context->parentNode();
			while ( n ) {
				nodes.append( n );
				n = n->parentNode();
			}
			return nodes;
		}
		case FollowingSiblingAxis: {
			if ( context->nodeType() == Node::ATTRIBUTE_NODE ||
			     context->nodeType() == XPath::XPATH_NAMESPACE_NODE ) {
				return DomNodeList();
			}

			NodeImpl *n = context->nextSibling();
			while ( n ) {
				nodes.append( n );
				n = n->nextSibling();
			}
			return nodes;
		}
		case PrecedingSiblingAxis: {
			if ( context->nodeType() == Node::ATTRIBUTE_NODE ||
			     context->nodeType() == XPath::XPATH_NAMESPACE_NODE ) {
				return DomNodeList();
			}

			NodeImpl *n = context->previousSibling();
			while ( n ) {
				nodes.append( n );
				n = n->previousSibling();
			}
			return nodes;
		}
		case FollowingAxis: {
			NodeImpl *p = context;
			while ( !isRootDomNode( p ) ) {
				NodeImpl *n = p->nextSibling();
				while ( n ) {
					nodes.append( n );
					nodes += getChildrenRecursively( n );
					n = n->nextSibling();
				}
				p = p->parentNode();
			}
			return nodes;
		}
		case PrecedingAxis: {
			NodeImpl *p = context;
			while ( !isRootDomNode( p ) ) {
				NodeImpl *n = p->previousSibling();
				while ( n ) {
					nodes.append( n );
					nodes += getChildrenRecursively( n );
					n = n->previousSibling();
				}
				p = p->parentNode();
			}
			return nodes;
		}
		case AttributeAxis: {
			if ( context->nodeType() != Node::ELEMENT_NODE ) {
				return DomNodeList();
			}

			NamedAttrMapImpl *attrs = static_cast<ElementImpl*>(context)->attributes( true /*read-only*/ );
			if ( !attrs ) {
				qDebug() << "Node::attributes() returned NULL!";
				return nodes;
			}

			for ( unsigned long i = 0; i < attrs->length(); ++i ) {
				nodes.append( attrs->item( i ) );
			}
			return nodes;
		}
		case NamespaceAxis: {
			if ( context->nodeType() != Node::ELEMENT_NODE ) {
				return DomNodeList();
			}

			bool foundXmlNsNode = false;
			NodeImpl *node = context;
			while ( node ) {
				NamedAttrMapImpl *attrs = static_cast<ElementImpl*>(node)->attributes();
				if ( !attrs ) {
					qDebug() << "Node::attributes() returned NULL!";
					node = node->parentNode();
					continue;
				}

				for ( unsigned long i = 0; i < attrs->length(); ++i ) {
					NodeImpl *n = attrs->item( i );
					if ( n->nodeName().string().startsWith( "xmlns:" ) ) {
						nodes.append( n );
					} else if ( n->nodeName() == "xmlns" &&
					            !foundXmlNsNode
					            ) {
						foundXmlNsNode = true;
						if ( !n->nodeValue().string().isEmpty() ) {
							nodes.append( n );
						}
					}
				}
				node = node->parentNode();
			}
			return nodes;
		}
		case SelfAxis:
			nodes.append( context );
			return nodes;
		case DescendantOrSelfAxis:
			nodes.append( context );
			nodes += getChildrenRecursively( context );
			return nodes;
		case AncestorOrSelfAxis: {
			nodes.append( context );
			NodeImpl *n = context->parentNode();
			while ( n ) {
				nodes.append( n );
				n = n->parentNode();
			}
			return nodes;
		}
		default:
			qDebug() << "Unknown axis " << axisAsString( m_axis ) << " passed to Step::nodesInAxis";
	}

	return DomNodeList();
}

DomNodeList Step::nodeTestMatches( const DomNodeList &nodes ) const
{
	QString ns = namespaceFromNodetest( m_nodeTest );
	DomNodeList matches;
	if ( m_nodeTest == "*" ) {
		foreach( NodeImpl *node, nodes ) {
			if ( node->nodeType() == primaryNodeType( m_axis ) ) {
				if ( ns.isEmpty() ||
				     node->namespaceURI().string() == ns ) {
					matches.append( node );
				}
			}
		}
		return nodes;
	} else if ( m_nodeTest == "text()" ) {
		foreach( NodeImpl *node, nodes ) {
			if ( node->nodeType() == Node::TEXT_NODE ||
			     node->nodeType() == Node::CDATA_SECTION_NODE ) {
				matches.append( node );
			}
		}

		return compressTextNodes( matches );
	} else if ( m_nodeTest == "comment()" ) {
		foreach( NodeImpl *node, nodes ) {
			if ( node->nodeType() == Node::COMMENT_NODE ) {
				matches.append( node );
			}
		}
		return matches;
	} else if ( m_nodeTest.startsWith( "processing-instruction" ) ) {
		QString param;
		const int space = m_nodeTest.indexOf( ' ' );
		if ( space > -1 ) {
			param = m_nodeTest.mid( space + 1 );
		}

		foreach( NodeImpl *node, nodes ) {
			if ( node->nodeType() == Node::PROCESSING_INSTRUCTION_NODE ) {
				if ( param.isEmpty() ||
				     node->nodeName() == param ) {
					matches.append( node );
				}
			}
		}
		return matches;
	} else if ( m_nodeTest == "node()" ) {
		return nodes;
	} else {
		QString prefix;
        QString localName;
		const int colon = m_nodeTest.indexOf( ':' );
		if (  colon > -1 ) {
			prefix = m_nodeTest.left( colon );
			localName = m_nodeTest.mid( colon + 1 );
		} else {
			localName = m_nodeTest;
		}

		if ( !prefix.isEmpty() ) {
			DOMString s( prefix );
            //FIXME: isNull or isEmpty?
			if ( Expression::evaluationContext().resolver->lookupNamespaceURI( s ).isNull() ) {
				throw new XPathExceptionImpl( DOMException::NAMESPACE_ERR );
			}
		}

		if ( m_axis == AttributeAxis ) {
			// In XPath land, namespace nodes are not accessible
			// on the attribute axis.
			if ( localName == "xmlns" ) {
				return matches;
			}

			foreach( NodeImpl *node, nodes ) {
				if ( node->nodeName() == localName ) {
					matches.append( node );
					break; // There can only be one.
				}
			}
			return matches;
		} else if ( m_axis == NamespaceAxis ) {
			qWarning() << "Node test " << m_nodeTest << " on axis " << axisAsString( m_axis ) << " is not implemented yet.";
		} else {
			foreach( NodeImpl *node, nodes ) {
				if ( node->nodeType() == Node::ELEMENT_NODE &&
				     node->nodeName() == localName ) {
					matches.append( node );
				}
			}
			return matches;
		}
	}

	qWarning() << "Node test " << m_nodeTest << " on axis " << axisAsString( m_axis ) << " is not implemented yet.";
	return matches;
}

void Step::optimize()
{
	foreach( Predicate *predicate, m_predicates ) {
		predicate->optimize();
	}
}

QString Step::dump() const
{
	QString s = QString( "<step axis=\"%1\" nodetest=\"%2\">" ).arg( axisAsString( m_axis ) ).arg( m_nodeTest );
	foreach( Predicate *predicate, m_predicates ) {
		s += predicate->dump();
	}
	s += "</step>";
	return s;
}

QString Step::namespaceFromNodetest( const QString &nodeTest ) const
{
	int i = nodeTest.indexOf( ':' );
	if ( i == -1 ) {
		return QString();
	}

	DOMString prefix( nodeTest.left( i ) );
	NodeImpl *ctxNode = Expression::evaluationContext().node;
	DOMString uri = ctxNode->lookupNamespaceURI( prefix );
	return uri.string();
}

unsigned int Step::primaryNodeType( AxisType axis ) const
{
	switch ( axis ) {
		case AttributeAxis:
			return Node::ATTRIBUTE_NODE;
		case NamespaceAxis:
			return XPATH_NAMESPACE_NODE;
		default:
			return Node::ELEMENT_NODE;
	}
}
