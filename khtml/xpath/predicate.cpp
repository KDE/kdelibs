/*
 * predicate.cc - Copyright 2005 Frerich Raabe <raabe@kde.org>
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
#include "predicate.h"
#include "functions.h"

#include <QString>

#include "xml/dom_nodeimpl.h"
#include "xml/dom_nodelistimpl.h"

#include <math.h>

using namespace DOM;
using namespace khtml;
using namespace khtml::XPath;

Number::Number( double value )
	: m_value( value )
{
}

bool Number::isConstant() const
{
	return true;
}

QString Number::dump() const
{
	return "<number>" + QString::number( m_value ) + "</number>";
}

Value Number::doEvaluate() const
{
	return Value( m_value );
}

String::String( const DOMString &value )
	: m_value( value )
{
}

bool String::isConstant() const
{
	return true;
}

QString String::dump() const
{
	return "<string>" + m_value.string() + "</string>";
}

Value String::doEvaluate() const
{
	return Value( m_value );
}

Value Negative::doEvaluate() const
{
	Value p( subExpr( 0 )->evaluate() );
	if ( !p.isNumber() ) {
		qWarning( "Unary minus is undefined for non-numeric types." );
		return Value();
	}
	return Value( -p.toNumber() );
}

QString Negative::dump() const
{
	return "<negative>" + subExpr( 0 )->dump() + "</number>";
}

QString BinaryExprBase::dump() const
{
	QString s = "<" + opName() + ">";
	s += "<operand>" + subExpr( 0 )->dump() + "</operand>";
	s += "<operand>" + subExpr( 1 )->dump() + "</operand>";
	s += "</" + opName() + ">";
	return s;
}

NumericOp::NumericOp( int _opCode, Expression* lhs, Expression* rhs ) :
	opCode( _opCode )
{
	addSubExpression( lhs );
	addSubExpression( rhs );
}

Value NumericOp::doEvaluate() const
{
	Value lhs( subExpr( 0 )->evaluate() );
	Value rhs( subExpr( 1 )->evaluate() );
	if ( !lhs.isNumber() || !rhs.isNumber() ) {
		qWarning( "Cannot perform operation on non-numeric types." );
		return Value();
	}

	double leftVal = lhs.toNumber(), rightVal = rhs.toNumber();

	switch (opCode) {
		case OP_Add:
			return Value( leftVal + rightVal );
		case OP_Sub:
			return Value( leftVal - rightVal );
		case OP_Mul:
			return Value( leftVal * rightVal );
		case OP_Div:
			if ( rightVal == 0.0 || rightVal == -0.0 )
				return Value(); //Divide by 0;
			else
				return Value( leftVal / rightVal );
		case OP_Mod:
			if ( rightVal == 0.0 || rightVal == -0.0 )
				return Value(); //Divide by 0;
			else
				return Value( remainder( leftVal, rightVal ) );
		case OP_GT:
			return Value ( leftVal > rightVal );
		case OP_GE:
			return Value ( leftVal >= rightVal );
		case OP_LT:
			return Value ( leftVal < rightVal );
		case OP_LE:
			return Value ( leftVal <= rightVal );
		default:
			assert(0);
		return Value();
	}
}

QString NumericOp::opName() const
{
	switch (opCode) {
		case OP_Add:
			return QLatin1String( "addition" );
		case OP_Sub:
			return QLatin1String( "subtraction" );
		case OP_Mul:
			return QLatin1String( "multiplication" );
		case OP_Div:
			return QLatin1String( "division" );
		case OP_Mod:
			return QLatin1String( "modulo" );
		case OP_GT:
			return QLatin1String( "relationGT" );
		case OP_GE:
			return QLatin1String( "relationGE" );
		case OP_LT:
			return QLatin1String( "relationLT" );
		case OP_LE:
			return QLatin1String( "relationLE" );
		default:
			assert(0);
			return QString();
	}
}

EqTestOp::EqTestOp( int _opCode, Expression* lhs, Expression* rhs ) :
	opCode(_opCode)
{
	addSubExpression( lhs );
	addSubExpression( rhs );
}

Value EqTestOp::doEvaluate() const
{
	Value lhs( subExpr( 0 )->evaluate() );
	Value rhs( subExpr( 1 )->evaluate() );

	bool equal;
	if ( lhs.isBoolean() || rhs.isBoolean() ) {
		equal = ( lhs.toBoolean() == rhs.toBoolean() );
	} else if ( lhs.isNumber() || rhs.isNumber() ) {
		equal = ( lhs.toNumber() == rhs.toNumber() );
	} else {
		equal = ( lhs.toString() == rhs.toString() );
	}

	if ( opCode == OP_EQ )
		return Value( equal );
	else
		return Value( !equal );
}

QString EqTestOp::opName() const
{
	if ( opCode == OP_EQ )
		return QLatin1String( "relationEQ" );
	else
		return QLatin1String( "relationNE" );
}

LogicalOp::LogicalOp( int _opCode, Expression* lhs, Expression* rhs ) :
	opCode( _opCode )
{
	addSubExpression( lhs );
	addSubExpression( rhs );
}

bool LogicalOp::shortCircuitOn() const
{
	if (opCode == OP_And)
		return false; //false and foo
	else
		return true;  //true or bar
}

bool LogicalOp::isConstant() const
{
	return subExpr( 0 )->isConstant() &&
	       subExpr( 0 )->evaluate().toBoolean() == shortCircuitOn();
}

QString LogicalOp::opName() const
{
	if ( opCode == OP_And )
		return QLatin1String( "conjunction" );
	else
		return QLatin1String( "disjunction" );
}

Value LogicalOp::doEvaluate() const
{
	Value lhs( subExpr( 0 )->evaluate() );

	// This is not only an optimization, http://www.w3.org/TR/xpath
	// dictates that we must do short-circuit evaluation
	bool lhsBool = lhs.toBoolean();
	if ( lhsBool == shortCircuitOn() ) {
		return Value( lhsBool );
	}

	return Value( subExpr( 1 )->evaluate().toBoolean() );
}

QString Union::opName() const
{
	return QLatin1String("union");
}

Value Union::doEvaluate() const
{
	Value lhs = subExpr( 0 )->evaluate();
	Value rhs = subExpr( 1 )->evaluate();
	if ( !lhs.isNodeset() || !rhs.isNodeset() ) {
		qWarning( "Union operator '|' works only with nodesets." );
		return Value( DomNodeList() );
	}

	DomNodeList lhsNodes = lhs.toNodeset();
	DomNodeList rhsNodes = rhs.toNodeset();
	DomNodeList result = new StaticNodeListImpl;

	for ( unsigned long n = 0; n < lhsNodes->length(); ++n )
		result->append( lhsNodes->item( n ) );

	for ( unsigned long n = 0; n < rhsNodes->length(); ++n )
		result->append( rhsNodes->item( n ) );
	
	return Value( result );
}

Predicate::Predicate( Expression *expr )
	: m_expr( expr )
{
}

Predicate::~Predicate()
{
	delete m_expr;
}

bool Predicate::evaluate() const
{
	Q_ASSERT( m_expr != 0 );

	Value result( m_expr->evaluate() );

	// foo[3] really means foo[position()=3]
	if ( result.isNumber() ) {
		Expression *realExpr = new EqTestOp(EqTestOp::OP_EQ,
		                FunctionLibrary::self().getFunction( "position" ),
		                new Number( result.toNumber() ) );
		result = realExpr->evaluate();
		delete realExpr;
	}

	return result.toBoolean();
}

void Predicate::optimize()
{
	m_expr->optimize();
}

QString Predicate::dump() const
{
	return QString() + "<predicate>" + m_expr->dump() + "</predicate>";
}
