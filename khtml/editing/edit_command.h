/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Leo Savernik <l.savernik@aon.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef EDIT_COMMAND_H
#define EDIT_COMMAND_H

#include "misc/shared.h"

namespace DOM {
    class DocumentImpl;
    class Selection;
}

namespace khtml {

//------------------------------------------------------------------------------------------
// Constants

/**
 * Edit-command IDs.
 * @internal
 */
enum ECommandID { 
    EditCommandID, // leave the base class first, others in alpha order
    AppendNodeCommandID,
    ApplyStyleCommandID,
    CompositeEditCommandID,
    DeleteCollapsibleWhitespaceCommandID,
    DeleteSelectionCommandID,
    DeleteTextCommandID,
    InputNewlineCommandID,
    InputTextCommandID,
    InsertNodeBeforeCommandID,
    InsertTextCommandID,
    JoinTextNodesCommandID,
    MoveSelectionCommandID,
    ReplaceSelectionCommandID,
    RemoveCSSPropertyCommandID,
    RemoveNodeAttributeCommandID,
    RemoveNodeCommandID,
    RemoveNodeAndPruneCommandID,
    RemoveNodePreservingChildrenCommandID,
    SetNodeAttributeCommandID,
    SplitTextNodeCommandID,
    TypingCommandID,
};

//------------------------------------------------------------------------------------------
// SharedCommandImpl

class EditCommand;
class EditCommandImpl;

class SharedCommandImpl : public Shared<SharedCommandImpl>
{
public:
    SharedCommandImpl() {}
    virtual ~SharedCommandImpl() {}

    virtual int commandID() const = 0;
    virtual bool isCompositeStep() const = 0;

    virtual void apply() = 0;
    virtual void unapply() = 0;
    virtual void reapply() = 0;

    virtual DOM::DocumentImpl * const document() const = 0;

    virtual DOM::Selection startingSelection() const = 0;
    virtual DOM::Selection endingSelection() const = 0;

    virtual void setStartingSelection(const DOM::Selection &s) = 0;
    virtual void setEndingSelection(const DOM::Selection &s) = 0;

    virtual EditCommand parent() const = 0;
    virtual void setParent(const EditCommand &) = 0;
};

//------------------------------------------------------------------------------------------
// EditCommand

class EditCommand : public SharedPtr<SharedCommandImpl>
{
public:
    EditCommand();
    EditCommand(EditCommandImpl *);
    EditCommand(const EditCommand &);
    ~EditCommand();

    int commandID() const;
    bool isCompositeStep() const;
    bool isNull() const;
    bool notNull() const;

    void apply();
    void unapply();
    void reapply();

    DOM::DocumentImpl * const document() const;

    DOM::Selection startingSelection() const;
    DOM::Selection endingSelection() const;

    void setStartingSelection(const DOM::Selection &s);
    void setEndingSelection(const DOM::Selection &s);

    EditCommand parent() const;
    void setParent(const EditCommand &);

    EditCommandImpl *handle() const;
    
    static EditCommand &emptyCommand();
};

}/*namespace khtml*/


#endif
