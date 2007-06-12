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

#ifndef SEMANTIC_CHECKER_H
#define SEMANTIC_CHECKER_H

#include "nodes.h"

#include <wtf/HashSet.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>

using WTF::HashSet;
using WTF::HashMap;
using WTF::Vector;

namespace KJS {
  class SemanticContext {
  public:
    SemanticContext() {}

    HashSet<Identifier> seenLabels;
    HashSet<Identifier> pendingLabels;
    LabelNode*          lastLabel;
    HashMap<Identifier, Node*> labelTargets;

    Vector<Node*> defaultContinueTargets;
    Vector<Node*> defaultBreakTargets;
  };


  /**
   This class is used to statically detect some classes of errors in the
   program, and replace them with an appropriate error node.
  */
  class SemanticChecker : public NodeVisitor {
  public:
    void checkSemantics(Node* node);

    SemanticContext* semanticContext() { return &contextStack.last(); }

    void enterSemanticContext();
    void exitSemanticContext();

    //### should prolly make private --- check other uses..
    virtual Node* visit(Node* node);

  private:
    Vector<SemanticContext> contextStack;
  };
}

#endif

// kate: indent-width 2; replace-tabs on; tab-width 2; space-indent on;
