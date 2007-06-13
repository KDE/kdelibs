/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef MAKENODES_H
#define MAKENODES_H

#include "nodes.h"
#include "identifier.h"

//#define TRACE_OPTIMIZER

namespace KJS {

static Node* makeAssignNode(Node* loc, Operator op, Node* expr)
{ 
    Node *n = loc->nodeInsideAllParens();

    if (!n->isLocation())
        return new AssignErrorNode(loc, op, expr);

    if (n->isVarAccessNode()) {
        return new AssignNode(static_cast<LocationNode*>(n), op, expr);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new AssignBracketNode(bracket->base(), bracket->subscript(), op, expr);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        return new AssignDotNode(dot->base(), dot->identifier(), op, expr);
    }
}

static Node* makeConditionalNode(Node* l, Node* e1, Node* e2)
{
    return new ConditionalNode(l, e1, e2);
}

static Node* makePrefixNode(Node *expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return new PrefixErrorNode(n, op);

    LocationNode *l = static_cast<LocationNode*>(n);
    return new PrefixNode(l, op);
}

static Node* makePostfixNode(Node* expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return new PostfixErrorNode(n, op);

    LocationNode *l = static_cast<LocationNode*>(n);
    return new PostfixNode(l, op);
}

static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args)
{
    Node *n = func->nodeInsideAllParens();

    if (!n->isLocation())
        return new FunctionCallValueNode(func, args);
    else if (n->isVarAccessNode())
        return new FunctionCallReferenceNode(static_cast<VarAccessNode*>(func), args);
    else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenBracketNode(bracket->base(), bracket->subscript(), args);
        else
            return new FunctionCallBracketNode(bracket->base(), bracket->subscript(), args);
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        if (n != func)
            return new FunctionCallParenDotNode(dot->base(), dot->identifier(), args);
        else
            return new FunctionCallDotNode(dot->base(), dot->identifier(), args);
    }
}

static Node *makeTypeOfNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();

    // We only need to use the special path for variable references,
    // since they may throw a ResolveError on evaluate where we don't 
    // want that...
    if (n->isVarAccessNode())
        return new TypeOfReferenceNode(static_cast<LocationNode*>(n));
    else
        return new TypeOfValueNode(n);
}

static Node *makeDeleteNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new DeleteValueNode(expr);
    else 
        return new DeleteReferenceNode(static_cast<LocationNode*>(n));//### not 100% faithful listing?
}

static bool makeGetterOrSetterPropertyNode(PropertyNode*& result, Identifier& getOrSet, Identifier& name, ParameterNode *params, FunctionBodyNode *body)
{
    PropertyNode::Type type;
    
    if (getOrSet == "get")
        type = PropertyNode::Getter;
    else if (getOrSet == "set")
        type = PropertyNode::Setter;
    else
        return false;
    
    result = new PropertyNode(new PropertyNameNode(name), 
                              new FuncExprNode(CommonIdentifiers::shared()->nullIdentifier, body, params), type);

    return true;
}

static Node* makeAddNode(Node* n1, Node* n2, Operator op)
{
#ifdef TRACE_OPTIMIZER
    printf("making Add Node\n");
#endif
#ifdef OPTIMIZE_NODES
    if (n1->isNumber()) {
	if (n2->isNumber()) {
#ifdef TRACE_OPTIMIZER
	    printf("Optimizing as NUMBER\n");
#endif
	    NumberNode* number1 = static_cast< NumberNode * >(n1);
	    NumberNode* number2 = static_cast< NumberNode * >(n2);
	    number1->setValue(add(number1->value(), number2->value(), op));
	    return number1;
	}
#ifdef TRACE_OPTIMIZER
	printf("could optimize as ADD NODE NUMBER\n");
#endif
    }
    if (n2->isNumber()) {
#ifdef TRACE_OPTIMIZER
	printf("could optimize as ADD NODE NUMBER\n");
#endif
    }
    if (op == OpPlus && n1->isString() && n2->isString()) {
#ifdef TRACE_OPTIMIZER
	printf("Optimizing as STRING\n");
#endif
	StringNode* str1 = static_cast<StringNode*>(n1);
	StringNode* str2 = static_cast<StringNode*>(n2);
	str1->setValue(str1->value() + str2->value());
	return str1;
    }
#endif
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeMultNode(Node* n1, Node* n2, Operator op)
{
#ifdef TRACE_OPTIMIZER
    printf("making Mult Node\n");
#endif
#ifdef OPTIMIZE_NODES
    if (n1->isNumber()) {
	if (n2->isNumber()) {
#ifdef TRACE_OPTIMIZER
	    printf("Optimizing as NUMBER\n");
#endif
	    NumberNode* number1 = static_cast< NumberNode * >(n1);
	    NumberNode* number2 = static_cast< NumberNode * >(n2);
	    number1->setValue(mult(number1->value(), number2->value(), op));
	    return number1;
	}
#ifdef TRACE_OPTIMIZER
	printf("could optimize as MULT NODE NUMBER\n");
#endif
    }
    if (n2->isNumber()) {
#ifdef TRACE_OPTIMIZER
	printf("could optimize as MULT NODE NUMBER\n");
#endif
    }
#endif
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeShiftNode(Node* n1, Node* n2, Operator op)
{
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeRelationalNode(Node* n1, Operator op, Node* n2)
{
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeEqualNode(Node* n1, Operator op, Node* n2)
{
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeBitOperNode(Node* n1, Operator op, Node* n2)
{
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeBinaryLogicalNode(Node* n1, Operator op, Node* n2)
{
    return new BinaryLogicalNode(n1, op, n2);
}

static Node* makeUnaryPlusNode(Node *n)
{
#ifdef TRACE_OPTIMIZER
    printf("making UnaryPlus Node\n");
#endif
#ifdef OPTIMIZE_NODES
    if (n->isNumber()) {
#ifdef TRACE_OPTIMIZER
    	printf("Optimizing UNARYPLUS NUMBER\n");
#endif
	return n;
    }
#endif
    return new UnaryPlusNode(n);
}

static Node* makeNegateNode(Node *n)
{
#ifdef TRACE_OPTIMIZER
    printf("making Negate Node\n");
#endif
#ifdef OPTIMIZE_NODES
    if (n->isNumber()) {
#ifdef TRACE_OPTIMIZER
    	printf("Optimizing NEGATE NUMBER\n");
#endif
	NumberNode *number = static_cast <NumberNode *>(n);
	number->setValue(-number->value());
	return number;
    }
#endif
    return new NegateNode(n);
}

static Node* makeBitwiseNotNode(Node *n)
{
    return new BitwiseNotNode(n);
}

static Node* makeLogicalNotNode(Node *n)
{
    return new LogicalNotNode(n);
}

static Node* makeGroupNode(Node *n)
{
    if (n->isVarAccessNode() || n->isGroupNode())
	return n;
    return new GroupNode(n);
}

static StatementNode* makeIfNode(Node *e, StatementNode *s1, StatementNode *s2)
{
    return new IfNode(e, s1, s2);
}

static StatementNode *makeImportNode(PackageNameNode *n,
				     bool wildcard, const Identifier &a)
{
    ImportStatement *stat = new ImportStatement(n);
    if (wildcard)
	stat->enableWildcard();
    stat->setAlias(a);

    return stat;
}

static StatementNode *makeLabelNode(const Identifier& l, StatementNode* s)
{
    return new LabelNode(l, s);
}

} // namespace KJS

#endif
// vi: set sw=4 :
