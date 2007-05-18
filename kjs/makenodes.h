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

namespace KJS {

template<typename ResolverType>
Node* makeDynamicResolver(Node* n) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType());
}

template<typename ResolverType, typename T1>
Node* makeDynamicResolver(Node* n, T1 arg1) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType(arg1));
}

template<typename ResolverType, typename T1, typename T2>
Node* makeDynamicResolver(Node* n, T1 arg1, T2 arg2) {
  DynamicResolver<ResolveIdentifier> *resolve = static_cast<DynamicResolver<ResolveIdentifier> *>(n);
  return new DynamicResolver<ResolverType>(resolve->identifier(), ResolverType(arg1, arg2));
}

static Node* makeAssignNode(Node* loc, Operator op, Node* expr)
{ 
    Node *n = loc->nodeInsideAllParens();

    if (!n->isLocation())
        return new AssignErrorNode(loc, op, expr); ;

    if (n->isResolveNode()) {
        return makeDynamicResolver<ResolveAssign>(n, op, expr);
    }
    if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new AssignBracketNode(bracket->base(), bracket->subscript(), op, expr);
    }
    assert(n->isDotAccessorNode());
    DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
    return new AssignDotNode(dot->base(), dot->identifier(), op, expr);
}

static Node* makePrefixNode(Node* expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return new PrefixErrorNode(n, op);
    
    if (n->isResolveNode()) {
        return makeDynamicResolver<ResolvePrefix>(n, op);
    }
    if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new PrefixBracketNode(bracket->base(), bracket->subscript(), op);
    }
    assert(n->isDotAccessorNode());
    DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
    return new PrefixDotNode(dot->base(), dot->identifier(), op);
}

static Node* makePostfixNode(Node* expr, Operator op)
{ 
    Node *n = expr->nodeInsideAllParens();

    if (!n->isLocation())
        return new PostfixErrorNode(n, op);
    
    if (n->isResolveNode()) {
        return makeDynamicResolver<ResolvePostfix>(n, op);
    }
    if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new PostfixBracketNode(bracket->base(), bracket->subscript(), op);
    }
    assert(n->isDotAccessorNode());
    DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
    return new PostfixDotNode(dot->base(), dot->identifier(), op);
}

static Node *makeFunctionCallNode(Node *func, ArgumentsNode *args)
{
    Node *n = func->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new FunctionCallValueNode(func, args);
    else if (n->isResolveNode()) {
        return makeDynamicResolver<ResolveFunctionCall>(n, args);
    } else if (n->isBracketAccessorNode()) {
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

    if (n->isResolveNode())
        return makeDynamicResolver<ResolveTypeOf>(n);
    else
        return new TypeOfValueNode(n);
}

static Node *makeDeleteNode(Node *expr)
{
    Node *n = expr->nodeInsideAllParens();
    
    if (!n->isLocation())
        return new DeleteValueNode(expr);
    else if (n->isResolveNode()) {
        return makeDynamicResolver<ResolveDelete>(n);
    } else if (n->isBracketAccessorNode()) {
        BracketAccessorNode *bracket = static_cast<BracketAccessorNode *>(n);
        return new DeleteBracketNode(bracket->base(), bracket->subscript());
    } else {
        assert(n->isDotAccessorNode());
        DotAccessorNode *dot = static_cast<DotAccessorNode *>(n);
        return new DeleteDotNode(dot->base(), dot->identifier());
    }
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
    return new BinaryOperatorNode(n1, n2, op);
}

static Node* makeMultNode(Node* n1, Node* n2, Operator op)
{
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

static Node* makeUnaryPlusNode(Node *n)
{
    return new UnaryPlusNode(n);
}

static Node* makeNegateNode(Node *n)
{
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

} // namespace KJS

#endif
