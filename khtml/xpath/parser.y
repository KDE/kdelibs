%{
#include "functions.h"
#include "path.h"
#include "predicate.h"
#include "util.h"
#include "tokenizer.h"

#include "expression.h"
#include "util.h"
#include "variablereference.h"

#include "dom/dom_string.h"
#include "dom/dom3_xpath.h"
#include "xml/dom3_xpathimpl.h"

using namespace DOM;
using namespace DOM::XPath;
using namespace khtml;
using namespace khtml::XPath;



#include <QList>
#include <QPair>
#include <QtDebug>

#define YYDEBUG 1

Expression * khtmlParseXPathStatement( const DOM::DOMString &statement );

static Expression *_topExpr;

%}

%union
{
	khtml::XPath::Step::AxisType axisType;
	int        num;
	DOM::DOMString *str;
	khtml::XPath::Expression *expr;
	QList<khtml::XPath::Predicate *> *predList;
	QList<khtml::XPath::Expression *> *argList;
	khtml::XPath::Step *step;
	khtml::XPath::LocationPath *locationPath;
}

%{
%}

%left <num> MULOP RELOP EQOP
%left <str> PLUS MINUS
%left <str> OR AND
%token <axisType> AXISNAME
%token <str> NODETYPE PI FUNCTIONNAME LITERAL
%token <str> VARIABLEREFERENCE NUMBER
%token <str> DOTDOT SLASHSLASH NAMETEST
%token ERROR

%type <locationPath> LocationPath
%type <locationPath> AbsoluteLocationPath
%type <locationPath> RelativeLocationPath
%type <step> Step
%type <axisType> AxisSpecifier
%type <step> DescendantOrSelf
%type <str> NodeTest
%type <expr> Predicate
%type <predList> PredicateList
%type <step> AbbreviatedStep
%type <expr> Expr
%type <expr> PrimaryExpr
%type <expr> FunctionCall
%type <argList> ArgumentList
%type <expr> Argument
%type <expr> UnionExpr
%type <expr> PathExpr
%type <expr> FilterExpr
%type <expr> OrExpr
%type <expr> AndExpr
%type <expr> EqualityExpr
%type <expr> RelationalExpr
%type <expr> AdditiveExpr
%type <expr> MultiplicativeExpr
%type <expr> UnaryExpr

%%

Expr:
	OrExpr
	{
		_topExpr = $1;
	}
	;

LocationPath:
	RelativeLocationPath
	{
		$$->m_absolute = false;
	}
	|
	AbsoluteLocationPath
	{
		$$->m_absolute = true;
	}
	;

AbsoluteLocationPath:
	'/'
	{
		$$ = new LocationPath;
	}
	|
	'/' RelativeLocationPath
	{
		$$ = $2;
	}
	|
	DescendantOrSelf RelativeLocationPath
	{
		$$ = $2;
		$$->m_steps.prepend( $1 );
	}
	;

RelativeLocationPath:
	Step
	{
		$$ = new LocationPath;
		$$->m_steps.append( $1 );
	}
	|
	RelativeLocationPath '/' Step
	{
		$$->m_steps.append( $3 );
	}
	|
	RelativeLocationPath DescendantOrSelf Step
	{
		$$->m_steps.append( $2 );
		$$->m_steps.append( $3 );
	}
	;

Step:
	NodeTest
	{
		$$ = new Step( Step::ChildAxis, *$1 );
		delete $1;
	}
	|
	NodeTest PredicateList
	{
		$$ = new Step( Step::ChildAxis, *$1, *$2 );
		delete $1;
		delete $2;
	}
	|
	AxisSpecifier NodeTest
	{
		$$ = new Step( $1, *$2 );
		delete $2;
	}
	|
	AxisSpecifier NodeTest PredicateList
	{
		$$ = new Step( $1, *$2,  *$3 );
		delete $2;
		delete $3;
	}
	|
	AbbreviatedStep
	;

AxisSpecifier:
	AXISNAME
	|
	'@'
	{
		$$ = Step::AttributeAxis;
	}
	;

NodeTest:
	NAMETEST
	{
		const int colon = $1->indexOf( ':' );
		if ( colon > -1 ) {
			DOMStringImpl prefix( $1->left( colon ) );
			XPathNSResolverImpl *resolver = Expression::evaluationContext().resolver;
			if ( !resolver || !resolver->lookupNamespaceURI( &prefix ) ) {
				qWarning() << "Found unknown namespace prefix " << prefix.string();
				//throw new XPathExceptionImpl( NAMESPACE_ERR );
				// ### FIXME: abort parsing, return code.
			}
		}
	}
	|
	NODETYPE '(' ')'
	{
		$$ = new DomString( *$1 + "()" );
	}
	|
	PI '(' ')'
	|
	PI '(' LITERAL ')'
	{
		DomString s = *$1 + " " + *$3;
		s = s.trimmed();
		$$ = new DomString( s );
		delete $1;
		delete $3;
	}
	;

PredicateList:
	Predicate
	{
		$$ = new QList<Predicate *>;
		$$->append( new Predicate( $1 ) );
	}
	|
	PredicateList Predicate
	{
		$$->append( new Predicate( $2 ) );
	}
	;

Predicate:
	'[' Expr ']'
	{
		$$ = $2;
	}
	;

DescendantOrSelf:
	SLASHSLASH
	{
		$$ = new Step( Step::DescendantOrSelfAxis, "node()" );
	}
	;

AbbreviatedStep:
	'.'
	{
		$$ = new Step( Step::SelfAxis, "node()" );
	}
	|
	DOTDOT
	{
		$$ = new Step( Step::ParentAxis, "node()" );
	}
	;

PrimaryExpr:
	VARIABLEREFERENCE
	{
		$$ = new VariableReference( *$1 );
		delete $1;
	}
	|
	'(' Expr ')'
	{
		$$ = $2;
	}
	|
	LITERAL
	{
		$$ = new String( *$1 );
		delete $1;
	}
	|
	NUMBER
	{
		$$ = new Number( $1->toDouble() );
		delete $1;
	}
	|
	FunctionCall
	;

FunctionCall:
	FUNCTIONNAME '(' ')'
	{
		$$ = FunctionLibrary::self().getFunction( $1->toLatin1() );
		delete $1;
	}
	|
	FUNCTIONNAME '(' ArgumentList ')'
	{
		$$ = FunctionLibrary::self().getFunction( $1->toLatin1(), *$3 );
		delete $1;
		delete $3;
	}
	;

ArgumentList:
	Argument
	{
		$$ = new QList<Expression *>;
		$$->append( $1 );
	}
	|
	ArgumentList ',' Argument
	{
		$$->append( $3 );
	}
	;

Argument:
	Expr
	;


UnionExpr:
	PathExpr
	|
	UnionExpr '|' PathExpr
	{
		$$ = new Union;
		$$->addSubExpression( $1 );
		$$->addSubExpression( $3 );
	}
	;

PathExpr:
	LocationPath
	{
		$$ = $1;
	}
	|
	FilterExpr
	{
		$$ = $1;
	}
	|
	FilterExpr '/' RelativeLocationPath
	{
		$3->m_absolute = true;
		$$ = new Path( static_cast<Filter *>( $1 ), $3 );
	}
	|
	FilterExpr DescendantOrSelf RelativeLocationPath
	{
		$3->m_steps.prepend( $2 );
		$3->m_absolute = true;
		$$ = new Path( static_cast<Filter *>( $1 ), $3 );
	}
	;

FilterExpr:
	PrimaryExpr
	{
		$$ = $1;
	}
	|
	PrimaryExpr PredicateList
	{
		$$ = new Filter( $1, *$2 );
	}
	;

OrExpr:
	AndExpr
	|
	OrExpr OR AndExpr
	{
		$$ = new LogicalOp( LogicalOp::OP_Or, $1, $3 );
	}
	;

AndExpr:
	EqualityExpr
	|
	AndExpr AND EqualityExpr
	{
		$$ = new LogicalOp( LogicalOp::OP_And, $1, $3 );
	}
	;

EqualityExpr:
	RelationalExpr
	|
	EqualityExpr EQOP RelationalExpr
	{
		$$ = new EqTestOp( $2, $1, $3 );
	}
	;

RelationalExpr:
	AdditiveExpr
	|
	RelationalExpr RELOP AdditiveExpr
	{
		$$ = new NumericOp( $2, $1, $3 );
	}
	;

AdditiveExpr:
	MultiplicativeExpr
	|
	AdditiveExpr PLUS MultiplicativeExpr
	{
		$$ = new NumericOp( NumericOp::OP_Add, $1, $3 );
	}
	|
	AdditiveExpr MINUS MultiplicativeExpr
	{
		$$ = new NumericOp( NumericOp::OP_Sub, $1, $3 );
	}
	;

MultiplicativeExpr:
	UnaryExpr
	|
	MultiplicativeExpr MULOP UnaryExpr
	{
		$$ = new NumericOp( $2, $1, $3 );
	}
	;

UnaryExpr:
	UnionExpr
	|
	MINUS UnaryExpr
	{
		$$ = new Negative;
		$$->addSubExpression( $2 );
	}
	;

%%

Expression *khtmlParseXPathStatement( const DOM::DOMString &statement )
{
//	qDebug() << "Parsing " << statement;
	initTokenizer( statement );
	yyparse();
	return _topExpr;
}

