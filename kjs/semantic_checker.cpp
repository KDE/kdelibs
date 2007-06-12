// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003, 2004, 2005, 2006 Apple Computer, Inc.
 *  Copyright (C) 2007 Maksim Orlovich (maksim@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "semantic_checker.h"

namespace KJS {


void SemanticChecker::enterSemanticContext()
{
  contextStack.append(SemanticContext());
}

void SemanticChecker::exitSemanticContext()
{
  contextStack.removeLast();
}

Node* SemanticChecker::visit(Node* node)
{
  return node->checkSemantics(this);
}

void SemanticChecker::checkSemantics(Node* node) {
  enterSemanticContext();
  visit(node);
  exitSemanticContext();
}


Node* Node::checkSemantics(SemanticChecker* checkerState)
{
  recurseVisit(checkerState);
  return 0;
}

Node* StatementNode::checkSemantics(SemanticChecker* checkerState)
{
  SemanticContext* ctx = checkerState->semanticContext();

  bool isLoop   = isIterationStatement();
  bool isSwitch = isSwitchStatement();
  
  if (!ctx->pendingLabels.isEmpty()) {
    // We would like to properly bind the labels at this point,
    // now that we got to some real code. How we bind depends on
    // what the current statement is: if it's a switch or a loop,
    // we bind to that, otherwise bind to the label itself, for the
    // label: break label; type of usage.
    Node* labelTarget = (isLoop || isSwitch) ? this : ctx->lastLabel;
    
    for (HashSet<Identifier>::iterator i =   ctx->pendingLabels.begin();
                                       i != ctx->pendingLabels.end(); ++i) {
      ctx->labelTargets.add(*i, labelTarget);
    }

    ctx->pendingLabels.clear();
  }

  if (isLoop) {
    ctx->defaultContinueTargets.append(this);
    ctx->defaultBreakTargets.append(this);
  }

  if (isSwitch)
    ctx->defaultBreakTargets.append(this);


  (void)Node::checkSemantics(checkerState);

  if (isLoop) {
    ctx->defaultContinueTargets.removeLast();
    ctx->defaultBreakTargets.removeLast();
  }

  if (isSwitch)
    ctx->defaultBreakTargets.removeLast();

  return 0;
}

Node* LabelNode::checkSemantics(SemanticChecker* checkerState)
{
  SemanticContext* ctx = checkerState->semanticContext();

  // Add the label to the seen set, and see if it's already there..
  if (!ctx->seenLabels.add(label).second)
    return createErrorNode(SyntaxError, "Duplicated label %s found.", label);

  // Add it to the set of pending labels, memorize this node as
  // candidate for binding the name
  ctx->pendingLabels.add(label);
  ctx->lastLabel = this;

  // We do not call up to StatementNode::checkSemantics, since that would
  // apply accumulated labels, and we want to defer them until we get the
  // first non-label statement.
  (void)Node::checkSemantics(checkerState);

  ctx->labelTargets.remove(label);
  ctx->seenLabels.remove(label);
  return 0;
}

Node* FuncDeclNode::checkSemantics(SemanticChecker* checkerState)
{
  checkerState->enterSemanticContext();
  (void)StatementNode::checkSemantics(checkerState);
  checkerState->exitSemanticContext();
  return 0;
}

Node* FuncExprNode::checkSemantics(SemanticChecker* checkerState)
{
  checkerState->enterSemanticContext();
  (void)Node::checkSemantics(checkerState);
  checkerState->exitSemanticContext();
  return 0;
}

Node* ContinueNode::checkSemantics(SemanticChecker* checkerState)
{
  StatementNode::checkSemantics(checkerState); //Resolve queued labels if need be..

  SemanticContext* ctx = checkerState->semanticContext();

  // Bind the label..
  if (ident.isEmpty()) {
    if (ctx->defaultContinueTargets.isEmpty())
      return createErrorNode(SyntaxError, "Invalid continue statement.");
    else
      target = ctx->defaultContinueTargets.last();
  } else  {
    // Make sure that we know this label, and that it is a 
    // loop-style labels.
    if (!ctx->seenLabels.contains(ident))
      return createErrorNode(SyntaxError, "Label %s not found.", ident);
    target = ctx->labelTargets.get(ident);

    if (!target->isIterationStatement())
      return createErrorNode(SyntaxError, "Invalid continue statement.");
  }

  return 0;
}

Node* BreakNode::checkSemantics(SemanticChecker* checkerState)
{
  StatementNode::checkSemantics(checkerState); //Resolve queued labels if need be..

  SemanticContext* ctx = checkerState->semanticContext();

  // Bind the label..
  if (ident.isEmpty()) {
    if (ctx->defaultBreakTargets.isEmpty())
      return createErrorNode(SyntaxError, "Invalid break statement.");
    else
      target = ctx->defaultBreakTargets.last();
  } else  {
    // Make sure that we know this label
    if (!ctx->seenLabels.contains(ident))
      return createErrorNode(SyntaxError, "Label %s not found.", ident);
    target = ctx->labelTargets.get(ident);
  }

  return 0;
}


} //namespace KJS

