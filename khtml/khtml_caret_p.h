/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>
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

#ifndef KHTML_CARET_P_H
#define KHTML_CARET_P_H

#include "rendering/render_table.h"

#define DEBUG_CARETMODE 0

namespace khtml {

/** contextual information about the caret which is related to the view.
 * An object of this class is only instantiated when it is needed.
 */
struct CaretViewContext {
    int freqTimerId;		// caret blink frequency timer id
    int x, y;			// caret position in viewport coordinates
    				// (y specifies the top, not the baseline)
    int width;			// width of caret in pixels
    int height;			// height of caret in pixels
    bool visible;		// true if currently visible.
    bool displayed;		// true if caret is to be displayed at all.
    bool caretMoved;		// set to true once caret has been moved in page
    				// how to display the caret when view is not focused
    KHTMLPart::CaretDisplayPolicy displayNonFocused;

    /** For natural traversal of lines, the original x position is saved, and
     * the actual x is set to the first character whose x position is
     * greater than origX.
     *
     * origX is reset to x whenever the caret is moved horizontally or placed
     * by the mouse.
     */
    int origX;

    bool keyReleasePending;	// true if keypress under caret mode awaits
    				// corresponding release event
    CaretViewContext() : freqTimerId(-1), x(0), y(0), width(1), height(16),
    	visible(true), displayed(false), caretMoved(false),
	displayNonFocused(KHTMLPart::CaretInvisible), origX(0),
	keyReleasePending(false)
    {}
};

/** contextual information about the editing state.
 * An object of this class is only instantiated when it is needed.
 */
struct EditorContext {
    bool override;		// true if typed characters should override
    				// the existing ones.

    EditorContext() : override(false)
    {}
};

class LinearDocument;

/**
 * Iterates through the lines of a document.
 *
 * The line iterator becomes invalid when the associated LinearDocument object
 * is destroyed.
 * @since 3.2
 * @internal
 * @author Leo Savernik
 */
class LineIterator
{
protected:
  LinearDocument *lines;	// associated document
  RenderFlow *cb;		// containing block
  InlineFlowBox *flowBox;	// the line itself

  static InlineBox *currentBox;	// current inline box

  // Note: cb == 0 indicates a position beyond the beginning or the
  // end of a document.

  /** Default constructor, only for internal use
   */
  LineIterator() {}

  /** Initializes a new iterator.
   *
   * Note: This constructor neither cares about the correctness of @p node
   * nor about @p offset. It is the responsibility of the caller to ensure
   * that both point to valid places.
   */
  LineIterator(LinearDocument *l, DOM::NodeImpl *node, long offset);

public:
  /** dereferences current line.
   *
   * Note: The returned inline flow box may be empty. This does <b>not</b>
   *	indicate the end
   *	of the document, but merely that the current paragraph does not contain
   *	any lines.
   */
  InlineFlowBox *operator *() const { return flowBox; }

  /** seek next line
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator ++();
  /** seek next line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator ++(int);

  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator --();
  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator --(int);

  /** does pointer arithmetic.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator operator +(int summand) const;
  /** does pointer arithmetic.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator operator -(int summand) const;

  /** does pointer arithmetic and assignment.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator &operator +=(int summand);
  /** does pointer arithmetic and assignment.
   *
   * This function is O(1) for values of +/-1, O(n) otherwise.
   * @param summand add these many lines
   */
  LineIterator &operator -=(int summand);

  /** compares two iterators. The comparator actually works only for
   * comparing arbitrary iterators to begin() and end().
   */
  bool operator ==(const LineIterator &it) const
  {
    return lines == it.lines
    		&& flowBox == it.flowBox && cb == it.cb;
  }

  /** compares two iterators
   */
  bool operator !=(const LineIterator &it) const
  {
    return !operator ==(it);
  }

  /** Whenever a new line iterator is created, it gets the inline box
   * it points to. For memory reasons, it's saved in a static instance,
   * thus making this function not thread-safe.
   *
   * This value can only be trusted immediately after having instantiated
   * a line iterator or one of its derivatives.
   * @return the corresponing inline box within the line represented by the
   *	last instantiation of a line iterator, or 0 if there was none.
   */
  static InlineBox *currentInlineBox() { return currentBox; }

protected:
  /** seeks next block.
   */
  void nextBlock();
  /** seeks previous block.
   */
  void prevBlock();

  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class LinearDocument;
};


/**
 * Represents the whole document in terms of lines.
 *
 * SGML documents are trees. But for navigation, this representation is
 * not practical. Therefore this class serves as a helper to represent the
 * document as a linear list of lines. Its usage somewhat resembles STL
 * semantics like begin and end as well as iterators.
 *
 * The lines itself are represented as pointers to InlineFlowBox objects.
 *
 * LinearDocument instances are not meant to be kept over the lifetime of their
 * associated document, but constructed from (node, offset) pairs whenever line
 * traversal is needed. This is because the underlying InlineFlowBox objects
 * may be destroyed and recreated (e. g. by resizing the window, adding/removing
 * elements).
 *
 * @author Leo Savernik
 * @since 3.2
 * @internal
 */
class LinearDocument {
public:
  typedef LineIterator Iterator;

  /**
   * Creates a new instance, and initializes it to the line specified by
   * the parameters below.
   *
   * Creation will fail if @p node is invisible or defect.
   * @param part part within which everything is taking place.
   * @param node document node with which to start
   * @param offset zero-based offset within this node.
   */
  LinearDocument(KHTMLPart *part, DOM::NodeImpl *node, long offset);

  virtual ~LinearDocument();

  /**
   * Tells whether this list contains any lines.
   *
   * @returns @p true if this document contains lines, @p false otherwise. Note
   *	that an empty document contains at least one line, so this method
   *	only returns @p false if the document could not be initialised for
   *	some reason.
   */
  bool isValid() const		// FIXME: not yet impl'd
  {
    return true;
  }

  /**
   * Returns the count of lines.
   *
   * Warning: This function is expensive. Call it once and cache the value.
   *
   * FIXME: It's not implemented yet (and maybe never will)
   */
  int count() const;

  /**
   * Returns a line iterator containing the current position as its starting
   * value.
   */
  Iterator current();

  /**
   * Returns a line iterator pointing right after the end of the document.
   */
  const Iterator &end() const { return _end; }

  /**
   * Returns a line iterator pointing to the very last line of the document.
   */
  Iterator preEnd();

  /**
   * Returns a line iterator pointing to the very first line of the document.
   */
  Iterator begin();

  /**
   * Returns a line iterator pointing just before the very first line of the
   * document (this is somewhat an emulation of reverse iterators).
   */
  const Iterator &preBegin() const { return _preBegin; }

protected:
  void initPreBeginIterator();
  void initEndIterator();

protected:
  RenderArena *arena;		// We need an arena for intermediate render
  				// objects that have no own inline box
  DOM::NodeImpl *node;
  long offset;

  Iterator _preBegin;
  Iterator _end;

  KHTMLPart *m_part;

  friend class LineIterator;
  friend class EditableLineIterator;
  friend class ErgonomicEditableLineIterator;
  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class EditableCharacterIterator;
};


/**
 * Iterates over the inner elements of an inline flow box.
 *
 * The given inline flow box must be a line box. The incrementor will
 * traverse all leaf inline boxes. It will also generate transient inline boxes
 * for those render objects that do not have one.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class InlineBoxIterator {
protected:
    RenderArena *arena;	// arena for allocating transient inline boxes
    InlineBox *box;	// currently traversed inline box

public:
    /** creates a new iterator, initialized with the given flow box.
     */
    InlineBoxIterator(RenderArena *arena, InlineFlowBox *flowBox, bool fromEnd = false);

    /** creates a new iterator, initialized with the given line iterator,
     * initialized to the given inline box, if specified.
     */
    InlineBoxIterator(LineIterator &lit, bool fromEnd = false,
                      InlineBox *initBox = 0);

  /** empty constructor.
   */
  InlineBoxIterator() {}

  /** returns the current leaf inline box.
   *
   * @return the box or 0 if the end has been reached.
   */
    InlineBox *operator *() const { return box; }

  /** increments the iterator to point to the next inline box on this line box.
   */
    InlineBoxIterator &operator ++();

  /** decrements the iterator to point to the previous inline box on this
   * line box.
   */
    InlineBoxIterator &operator --();
};

/**
 * Iterates over the editable inner elements of an inline flow box.
 *
 * The given inline flow box must be a line box. The incrementor will
 * traverse all leaf inline boxes. In contrast to @p InlineBoxIterator this
 * iterator only regards inline boxes which are editable.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class EditableInlineBoxIterator : public InlineBoxIterator {
protected:
  KHTMLPart *m_part;
  bool adjacent;

public:
  /** creates a new iterator, initialized with the given flow box.
   * @param part part within which all actions are taking place.
   * @param arena arena for transient allocations
   * @param flowBox line box to be iterated
   * @param fromEnd @p true, start with last box in line
   */
  EditableInlineBoxIterator(KHTMLPart *part, RenderArena *arena,
  		InlineFlowBox *flowBox, bool fromEnd = false)
  	: InlineBoxIterator(arena, flowBox, fromEnd), m_part(part), adjacent(true)
  {
    if (box && !isEditable(box)) fromEnd ? --*this : ++*this;
  }

  /** initializes a new iterator from the given line iterator,
   * beginning with the given inline box, if specified.
   */
  EditableInlineBoxIterator(LineIterator &lit, bool fromEnd = false,
  		InlineBox *initBox = 0)
  		: InlineBoxIterator(lit, fromEnd, initBox), m_part(lit.lines->m_part)
  {
    if (box && !isEditable(box)) 
    {
      if (fromEnd)
        --*this;
      else 
        ++*this;
    }
  }

  /** empty constructor. Use only to copy another iterator into this one.
   */
  EditableInlineBoxIterator() {}

  /** returns @p true when the current inline box is visually adjacent to the
   * previous inline box, i. e. no intervening inline boxes.
   */
  bool isAdjacent() const { return adjacent; }

  /** increments the iterator to point to the next editable inline box
   * on this line box.
   */
  EditableInlineBoxIterator &operator ++()
  {
    adjacent = true;
    do {
      InlineBoxIterator::operator ++();
    } while (box && !isEditable(box));
    return *this;
  }

  /** decrements the iterator to point to the previous editable inline box
   * on this line box.
   */
  EditableInlineBoxIterator &operator --()
  {
    adjacent = true;
    do {
      InlineBoxIterator::operator --();
    } while (box && !isEditable(box));
    return *this;
  }

protected:
  /** finds out if the given box is editable.
   * @param b given inline box
   * @return @p true if box is editable
   */
  bool isEditable(InlineBox *b)
  {
    //if (m_part->isCaretMode() || m_part->isEditable()) return true;

    Q_ASSERT(b);
    RenderObject *r = b->object();
#if DEBUG_CARETMODE > 0
    if (b->isInlineFlowBox()) kdDebug(6200) << "b is inline flow box" << endl;
    kdDebug(6200) << "isEditable r" << r << ": " << (r ? r->renderName() : QString::null) << (r && r->isText() ? " contains \"" + QString(((RenderText *)r)->str->s, QMIN(((RenderText *)r)->str->l,15)) + "\"" : QString::null) << endl;
#endif
    // Must check caret mode or design mode *after* r && r->element(), otherwise
    // lines without a backing DOM node get regarded, leading to a crash.
    // ### check should actually be in InlineBoxIterator
    bool result = r && r->element() && !r->isTableCol()
    	&& (m_part->isCaretMode() || m_part->isEditable()
           	|| r->style()->userInput() == UI_ENABLED);
    if (!result) adjacent = false;
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << result << endl;
#endif
    return result;
  }

};

/**
 * Iterates through the editable lines of a document.
 *
 * This iterator, opposing to @p LineIterator, only regards editable lines.
 * If either @ref KHTMLPart::editable or @ref KHTMLPart::caretMode are true,
 * this class works exactly as if it were a LineIterator.
 *
 * The iterator can be compared to normal LineIterators, especially to
 * @ref LinearDocument::preBegin and @ref LinearDocument::end
 *
 * The line iterator becomes invalid when the associated LinearDocument object
 * is destroyed.
 * @since 3.2
 * @internal
 * @author Leo Savernik
 */
class EditableLineIterator : public LineIterator {
public:
  /** Initializes a new iterator.
   *
   * The iterator is set to the first following editable line or to the
   * end if no editable line follows.
   * @param it a line iterator to initialize this from
   * @param fromEnd @p true, traverse towards the beginning in search of an
   *	editable line
   */
  EditableLineIterator(const LineIterator &it, bool fromEnd = false)
  		: LineIterator(it)
  {
    if (flowBox && !isEditable(*this)) 
    {
      if (fromEnd) 
        operator--();
      else
        operator ++();
    }    
    if (!flowBox || !cb) {
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "EditableLineIterator: findFlowBox failed" << endl;
#endif
      cb = 0;
    }/*end if*/
  }

  /** empty constructor.
   *
   * Only use if you want to copy another iterator onto it later.
   */
  EditableLineIterator() {}

  /** seek next line
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  EditableLineIterator &operator ++()
  {
    // FIXME: MEGA-FLAW! editable empty inlines elements not
    // represented by an inline box aren't considered any more.
    do {
      LineIterator::operator ++();
    } while (cb && !isEditable(*this));
    return *this;
  }
  /** seek next line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  //EditableLineIterator operator ++(int);

  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  EditableLineIterator &operator --()
  {
    // FIXME: MEGA-FLAW! editable empty inlines not
    // represented by an inline box aren't considered any more.
    do {
      LineIterator::operator --();
    } while (cb && !isEditable(*this));
    return *this;
  }
  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  //EditableLineIterator operator --(int);

#if 0	// implement when it's needed
  /** does pointer arithmetic.
   * @param summand add these many lines
   */
  EditableLineIterator operator +(int summand) const;
  /** does pointer arithmetic.
   * @param summand add these many lines
   */
  EditableLineIterator operator -(int summand) const;

  /** does pointer arithmetic and assignment.
   * @param summand add these many lines
   */
  EditableLineIterator &operator +=(int summand);
  /** does pointer arithmetic and assignment.
   * @param summand add these many lines
   */
  EditableLineIterator &operator -=(int summand);
#endif

protected:
  /** finds out if the current line is editable.
   * @param it check flow box iterator points to
   * @return @p true if line is editable
   */
  bool isEditable(LineIterator &it)
  {
#if 0		// these shortcut evaluations are all invalid
    if (lines->m_part->isCaretMode() || lines->m_part->isEditable()) return true;

    // on dummy lines check the containing block itself for editability
    if (!(*it)->firstChild()) {
      kdDebug(6200) << "cb " << cb->renderName() << "[" << cb << "](" << (cb->element() ? cb->element()->nodeName().string() : QString::null) << ") editable? " << (cb->style()->userInput() == UI_ENABLED) << endl;
      return cb->style()->userInput() == UI_ENABLED;
    }/*end if*/
#endif

    EditableInlineBoxIterator fbit = it;
    return *fbit;
  }

};

/** Represents a render table as a linear list of rows.
 *
 * This iterator abstracts from table sections and treats tables as a linear
 * representation of all rows they contain.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class TableRowIterator {
protected:
  TableSectionIterator sec;	// current section
  int index;			// index of row within section
public:
  /** Constructs a new iterator.
   * @param table table to iterate through.
   * @param fromEnd @p true to iterate towards the beginning
   * @param row pointer to row to start with, 0 starts at the first/last
   *	row.
   */
  TableRowIterator(RenderTable *table, bool fromEnd = false,
  		RenderTableSection::RowStruct *row = 0);

  /** Constructs a new iterator.
   * @param section table section to begin with
   * @param index index within table section
   */
  TableRowIterator(RenderTableSection *section, int index)
  	: sec(section), index(index)
  {}

  /** empty constructor. This must be assigned another iterator before it is
   * useable.
   */
  TableRowIterator() {}

  /** returns the current table row.
   * @return the row or 0 if the end of the table has been reached.
   */
  RenderTableSection::RowStruct *operator *()
  {
    if (!*sec) return 0;
    return &(*sec)->grid[index];
  }

  /** advances to the next row
   */
  TableRowIterator &operator ++();

  /** advances to the previous row
   */
  TableRowIterator &operator --();

protected:
};

/** Iterates through the editable lines of a document, in a topological order.
 *
 * The differences between this and the EditableLineIterator lies in the way
 * lines are inquired. While the latter steps through the lines in document
 * order, the former takes into consideration ergonomics.
 *
 * This is especially useful for tables. EditableLineIterator traverses all
 * table cells from left to right, top to bottom, while this one will
 * actually snap to the cell in the right position, and traverse only
 * upwards/downwards, thus providing a more intuitive navigation.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class ErgonomicEditableLineIterator : public EditableLineIterator {
protected:
  int xCoor;		// x-coordinate to determine cell position with
public:
  /** Initializes a new ergonomic editable line iterator from the given one.
   * @param it line iterator
   * @param x absolute x-coordinate for cell determination
   */
  ErgonomicEditableLineIterator(const LineIterator &it, int x)
  	: EditableLineIterator(it), xCoor(x) {}

  /** Constructs an uninitialized iterator which must be assigned a line iterator before
   * it can be used.
   */
  ErgonomicEditableLineIterator() {}

  /** seek next line.
   *
   * The next line will be one that is visually situated below this line.
   */
  ErgonomicEditableLineIterator &operator ++();

  /** seek previous line.
   *
   * The previous line will be one that is visually situated above this line.
   */
  ErgonomicEditableLineIterator &operator --();

protected:
  /** determines the topologically next render object.
   * @param oldCell table cell the original object was under.
   * @param newObject object to determine whether and which transition
   *	between cells is to be handled. It does not have to be an object in the correct
   *	topological cell, a simple delivery from an editable line iterator suffices.
   * @param toBegin if @p true, iterate towards the beginning
   */
  void determineTopologicalElement(RenderTableCell *oldCell,
  		RenderObject *newObject, bool toBegin);

  /** initializes the iterator to point to the first previous/following editable
   * line.
   * @param newBlock take this as base block.
   * @param toBegin @p true, iterate towards beginning.
   */
  void calcAndStoreNewLine(RenderFlow *newBlock, bool toBegin);

#if 0
  /** compares whether two tables cells belong to the same table.
   *
   * If one or both cells are 0, @p false is returned.
   */
  static bool belongToSameTable(const RenderTableCell *t1, const RenderTableCell *t2)
  {
    return t1 && t2 && t1->table() == t2->table();
  }

  /** finds the cell corresponding to absolute x-coordinate @p x in the given
   * table section.
   *
   * If there is not direct cell, or the cell is not accessible, the function
   * will return the nearest suitable cell.
   * @param part part containing the document
   * @param x absolute x-coordinate
   * @param section table section to be searched
   * @param fromEnd @p true to begin search from end and work towards the
   *	beginning
   * @param startIndex start at this index (or < 0 to start with first/last row)
   * @return the cell, or 0 if no editable cell was found.
   */
  static RenderTableCell *findNearestTableCellInSection(KHTMLPart *part, int x,
  	RenderTableSection *section, bool fromEnd = false, int startIndex = -1);

  /** finds a suitable object beyond the given table.
   *
   * This method is to be called whenever there is no more row left
   * in the given table. It scans for a suitable line before/after the table,
   * or searches a table cell if another table happens to follow directly.
   * @param table table to start from
   * @param toBegin @p true, seek towards beginning, @p false, seek towards end.
   * @return a suitable, editable leaf render object, or 0 if the document
   *	boundary was reached.
   */
  RenderObject *findObjectBeyond(RenderTable *table, bool toBegin);
#endif
};

/**
 * Provides iterating through the document in terms of characters. Only the
 * editable characters are regarded.
 *
 * This iterator represents the document, which is structured as a tree itself,
 * as a linear stream of characters.
 */
class EditableCharacterIterator {
protected:
  LinearDocument *ld;
  EditableLineIterator _it;
  EditableInlineBoxIterator ebit;
  DOM::NodeImpl *_node;
  long _offset;
  int _char;

public:

  /** empty constructor.
   *
   * Only use if you want to assign another constructor as no fields will
   * be initialized.
   */
  EditableCharacterIterator() {}

  /** constructs a new iterator from the given linear document.
   *
   * @param ld linear representation of document.
   */
  EditableCharacterIterator(LinearDocument *ld)
  		: ld(ld), _it(ld->current()),
                ebit(_it, false, _it.currentInlineBox()), _char(-1)
  {
    _node = ld->node;
    _offset = ld->offset;

    // ### temporary fix for illegal nodes
    if (_it == ld->end()) { _node = 0; return; }

    // seeks the node's inline box
    // ### redundant, implement means to get it from ld or _it
    // ### if node is not there?
    EditableInlineBoxIterator copy = ebit;
    for (; *ebit; ++ebit) {
      copy = ebit;
      InlineBox *b = *ebit;

      if (b == _it.currentInlineBox() || b->object() == _node->renderer()) {
        _offset = QMIN(kMax(_offset, b->minOffset()), b->maxOffset());
        break;
      }/*end if*/
    }/*next ebit*/
    // If no node is found, we take the last editable node. This is a very
    // feeble approximation as it sometimes makes the caret get stuck, or
    // iterate over the same element indefinitely,
    // but this covers up a case that should never happen in theory.
    if (!*ebit) {
      // this is a really bad hack but solves the caret-gets-stuck issue
      static long cache_offset = -1;
      ebit = copy;
      InlineBox *b = *ebit;
      _node = b->object()->element();
      long max_ofs = b->maxOffset();
      _offset = cache_offset == max_ofs ? b->minOffset() : max_ofs;
      cache_offset = _offset;
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "There was no node! Fixup applied!" << endl;
      if (cache_offset == max_ofs) kdDebug(6200) << "offset fixup applied as well" << endl;
#endif
    }/*end if*/

    initFirstChar();
  }

  /** returns the current character, or -1 if not on a text node, or beyond
   * the end.
   */
  int chr() const { return _char; }

  /** returns the current character as a unicode symbol, substituting
   * a blank for a non-text node.
   */
  QChar operator *() const { return QChar(_char >= 0 ? _char : ' '); }

  /** returns the current offset
   */
  long offset() const { return _offset; }
  /** returns the current node.
   *
   * If it's 0, then there are no more nodes.
   */
  DOM::NodeImpl *node() const { return _node; }
  /** returns the current inline box.
   *
   * May be 0 if the current element has none, or if the end has been reached.
   * Therefore, do *not* use this to test for the end condition, use node()
   * instead.
   */
  InlineBox *box() const { return *ebit; }
  /** moves to the next editable character.
   */
  EditableCharacterIterator &operator ++();

  /** moves to the previous editable character.
   */
  EditableCharacterIterator &operator --();

protected:
  /** initializes the _char member by reading the character at the current
   * offset, peeking ahead as necessary.
   */
  void initFirstChar();
  /** reads ahead the next node and updates the data structures accordingly
   */
  void peekNext()
  {
    EditableInlineBoxIterator copy = ebit;
    ++copy;
    InlineBox *b = *copy;
    if (b && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->str->s[b->minOffset()].unicode();
    else
      _char = -1;
  }
  /** reads ahead the previous node and updates the data structures accordingly
   */
  void peekPrev()
  {
    --ebit;
//    _peekPrev = *ebit;
  }

};


}


#endif
