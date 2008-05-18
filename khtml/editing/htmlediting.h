/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef __htmlediting_h__
#define __htmlediting_h__

#include "edit_command.h"

#include "xml/dom_nodeimpl.h"

namespace DOM {
    class CSSStyleDeclarationImpl;
    class DocumentFragmentImpl;
    class DocumentImpl;
    class DOMString;
    class ElementImpl;
    class NodeImpl;
    class Position;
    class Selection;
    class TextImpl;
};

namespace khtml {

class AppendNodeCommandImpl;
class ApplyStyleCommandImpl;
class CompositeEditCommandImpl;
class DeleteCollapsibleWhitespaceCommandImpl;
class DeleteSelectionCommandImpl;
class DeleteTextCommandImpl;
class EditCommand;
class EditCommandImpl;
class InputNewlineCommandImpl;
class InputTextCommandImpl;
class InsertNodeBeforeCommandImpl;
class InsertTextCommandImpl;
class JoinTextNodesCommandImpl;
class MoveSelectionCommandImpl;
class ReplaceSelectionCommandImpl;
class RemoveCSSPropertyCommandImpl;
class RemoveNodeAttributeCommandImpl;
class RemoveNodeCommandImpl;
class RemoveNodeAndPruneCommandImpl;
class RemoveNodePreservingChildrenCommandImpl;
class SetNodeAttributeCommandImpl;
class SplitTextNodeCommandImpl;
class TypingCommandImpl;

//------------------------------------------------------------------------------------------
// CompositeEditCommand

class CompositeEditCommand : public EditCommand
{
public:
	CompositeEditCommand();
	CompositeEditCommand(CompositeEditCommandImpl *);
	CompositeEditCommand(const CompositeEditCommand &);
	~CompositeEditCommand();

private:
    inline CompositeEditCommandImpl *impl() const;
};

//==========================================================================================
// Concrete commands
//------------------------------------------------------------------------------------------
// AppendNodeCommand

class AppendNodeCommand : public EditCommand
{
public:
    AppendNodeCommand(DOM::DocumentImpl *, DOM::NodeImpl *parentNode, DOM::NodeImpl *appendChild);
	~AppendNodeCommand();

    DOM::NodeImpl *parentNode() const;
    DOM::NodeImpl *appendChild() const;
    
private:
    inline AppendNodeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// ApplyStyleCommand

class ApplyStyleCommand : public CompositeEditCommand
{
public:
	ApplyStyleCommand(DOM::DocumentImpl *, DOM::CSSStyleDeclarationImpl *);
	~ApplyStyleCommand();

    DOM::CSSStyleDeclarationImpl *style() const;

private:
    inline ApplyStyleCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// DeleteCollapsibleWhitespaceCommand

class DeleteCollapsibleWhitespaceCommand : public CompositeEditCommand
{
public:
	DeleteCollapsibleWhitespaceCommand(DOM::DocumentImpl *document);
	DeleteCollapsibleWhitespaceCommand(DOM::DocumentImpl *document, const DOM::Selection &selection);
    
	~DeleteCollapsibleWhitespaceCommand();

private:
    inline DeleteCollapsibleWhitespaceCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// DeleteSelectionCommand

class DeleteSelectionCommand : public CompositeEditCommand
{
public:
	DeleteSelectionCommand(DOM::DocumentImpl *document);
	DeleteSelectionCommand(DOM::DocumentImpl *document, const DOM::Selection &selection);
	~DeleteSelectionCommand();

private:
    inline DeleteSelectionCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// DeleteTextCommand

class DeleteTextCommand : public EditCommand
{
public:
	DeleteTextCommand(DOM::DocumentImpl *document, DOM::TextImpl *, long offset, long count);
	DeleteTextCommand(const DeleteTextCommand &);
	~DeleteTextCommand();

    DOM::TextImpl *node() const;
    long offset() const;
    long count() const;

private:
    inline DeleteTextCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// InputNewlineCommand

class InputNewlineCommand : public CompositeEditCommand
{
public:
    InputNewlineCommand(DOM::DocumentImpl *document);
    ~InputNewlineCommand();

private:
    inline InputNewlineCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// InputTextCommand

class InputTextCommand : public CompositeEditCommand
{
public:
    InputTextCommand(DOM::DocumentImpl *document);
    ~InputTextCommand();

    void deleteCharacter();
    void input(const DOM::DOMString &text);

    unsigned long charactersAdded() const;

private:
    inline InputTextCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// InsertNodeBeforeCommand

class InsertNodeBeforeCommand : public EditCommand
{
public:
    InsertNodeBeforeCommand();
    InsertNodeBeforeCommand(DOM::DocumentImpl *, DOM::NodeImpl *insertChild, DOM::NodeImpl *refChild);
    InsertNodeBeforeCommand(const InsertNodeBeforeCommand &);
    ~InsertNodeBeforeCommand();

    DOM::NodeImpl *insertChild() const;
    DOM::NodeImpl *refChild() const;
    
private:
    inline InsertNodeBeforeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// InsertTextCommand

class InsertTextCommand : public EditCommand
{
public:
	InsertTextCommand(DOM::DocumentImpl *document, DOM::TextImpl *, long, const DOM::DOMString &);
	~InsertTextCommand();

    DOM::TextImpl *node() const;
    long offset() const;
    DOM::DOMString text() const;

private:
    inline InsertTextCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// JoinTextNodesCommand

class JoinTextNodesCommand : public EditCommand
{
public:
	JoinTextNodesCommand(DOM::DocumentImpl *, DOM::TextImpl *, DOM::TextImpl *);
	~JoinTextNodesCommand();

    DOM::TextImpl *firstNode() const;
    DOM::TextImpl *secondNode() const;
    
private:
    inline JoinTextNodesCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// ReplaceSelectionCommand

class ReplaceSelectionCommand : public CompositeEditCommand
{
public:
    ReplaceSelectionCommand(DOM::DocumentImpl *document, DOM::DocumentFragmentImpl *fragment, bool selectReplacement=true);
    ~ReplaceSelectionCommand();

private:
    inline ReplaceSelectionCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// MoveSelectionCommand

class MoveSelectionCommand : public CompositeEditCommand
{
public:
    MoveSelectionCommand(DOM::DocumentImpl *document, DOM::DocumentFragmentImpl *fragment, DOM::Position &position);
    ~MoveSelectionCommand();
    
private:
        inline MoveSelectionCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// RemoveCSSPropertyCommand

class RemoveCSSPropertyCommand : public EditCommand
{
public:
	RemoveCSSPropertyCommand(DOM::DocumentImpl *, DOM::CSSStyleDeclarationImpl *, int property);
	~RemoveCSSPropertyCommand();

    DOM::CSSStyleDeclarationImpl *styleDeclaration() const;
    int property() const;
    
private:
    inline RemoveCSSPropertyCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// RemoveNodeAttributeCommand

class RemoveNodeAttributeCommand : public EditCommand
{
public:
	RemoveNodeAttributeCommand(DOM::DocumentImpl *, DOM::ElementImpl *, DOM::NodeImpl::Id attribute);
	~RemoveNodeAttributeCommand();

    DOM::ElementImpl *element() const;
    DOM::NodeImpl::Id attribute() const;
    
private:
    inline RemoveNodeAttributeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// RemoveNodeCommand

class RemoveNodeCommand : public EditCommand
{
public:
	RemoveNodeCommand(DOM::DocumentImpl *, DOM::NodeImpl *node);
	~RemoveNodeCommand();

    DOM::NodeImpl *node() const;
    
private:
    inline RemoveNodeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// RemoveNodeAndPruneCommand

class RemoveNodeAndPruneCommand : public CompositeEditCommand
{
public:
	RemoveNodeAndPruneCommand(DOM::DocumentImpl *, DOM::NodeImpl *pruneNode, DOM::NodeImpl *stopNode=0);
	~RemoveNodeAndPruneCommand();

    DOM::NodeImpl *pruneNode() const;
    DOM::NodeImpl *stopNode() const;
    
private:
    inline RemoveNodeAndPruneCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// RemoveNodePreservingChildrenCommand

class RemoveNodePreservingChildrenCommand : public CompositeEditCommand
{
public:
    RemoveNodePreservingChildrenCommand(DOM::DocumentImpl *document, DOM::NodeImpl *node);
    ~RemoveNodePreservingChildrenCommand();

    DOM::NodeImpl *node() const;

private:
    inline RemoveNodePreservingChildrenCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// SetNodeAttributeCommand

class SetNodeAttributeCommand : public EditCommand
{
public:
	SetNodeAttributeCommand(DOM::DocumentImpl *, DOM::ElementImpl *, DOM::NodeImpl::Id attribute, const DOM::DOMString &value);
	~SetNodeAttributeCommand();

    DOM::ElementImpl *element() const;
    DOM::NodeImpl::Id attribute() const;
    DOM::DOMString value() const;
    
private:
    inline SetNodeAttributeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// SplitTextNodeCommand

class SplitTextNodeCommand : public EditCommand
{
public:
	SplitTextNodeCommand(DOM::DocumentImpl *, DOM::TextImpl *, long);
	~SplitTextNodeCommand();

    DOM::TextImpl *node() const;
    long offset() const;
    
private:
    inline SplitTextNodeCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------
// TypingCommand

class TypingCommand : public CompositeEditCommand
{
public:
    static void deleteKeyPressed(DOM::DocumentImpl *document);
    static void insertText(DOM::DocumentImpl *document, const DOM::DOMString &text);
    static void insertNewline(DOM::DocumentImpl *document);
    static bool isOpenForMoreTypingCommand(const EditCommand &);
    static void closeTyping(EditCommand);

    bool openForMoreTyping() const;
    void closeTyping();

private:
	TypingCommand(DOM::DocumentImpl *document);
	TypingCommand(TypingCommand *);
	TypingCommand(const TypingCommand &);
	~TypingCommand();

    void deleteKeyPressed();
    void insertText(const DOM::DOMString &text);
    void insertNewline();

    inline TypingCommandImpl *impl() const;
};

//------------------------------------------------------------------------------------------

} // end namespace khtml

#endif
