/* This file is part of the KDE project
 *
 * Copyright (C) 2003-2004 Leo Savernik <l.savernik@aon.at>
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

#include <qvaluevector.h>

#define DEBUG_CARETMODE 0

class QFontMetrics;

namespace DOM {
  class NodeImpl;
  class ElementImpl;
}

namespace khtml {

/** caret advance policy.
 *
 * Used to determine which elements are taken into account when the caret is
 * advanced. Later policies pose refinements of all former
 * policies.
 * @param LeafsOnly advance from leave render object to leaf render object
 *	(It will allow outside flow positions if a flow wouldn't be reachable
 *	otherwise).
 * @param IndicatedFlows place caret also at the beginning/end of flows
 *	that have at least one visible border at any side.
 *	(It will allow not indicated flow positions if a flow wouldn't
 *	be reachable otherwise).
 * @param VisibleFlows place caret also at the beginning/end of any flow
 *	that has a renderer.
 */
enum CaretAdvancePolicy {
    LeafsOnly, IndicatedFlows, VisibleFlows
};

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

#if 0
/**
 * Stores objects of a certain type, and calls delete on each of them
 * when this data structure is destroyed.
 *
 * This is useful for caret mode related intermediate objects. As they are
 * only used within an event handler, they can be deallocated at mass when
 * the final caret coordinates and position have been calculated.
 *
 * The objects are stored in a vector. Therefore, they can be iterated in
 * a deterministic way.
 *
 * To use this mass deleter, its managed objects must fulfill the following
 * conditions:
 * @li No derived objects can be used.
 * @li The managed object must have an overloaded new operator, which makes
 *	use of MassDeleter::allocate
 * @li The managed object must have an overloaded delete operator, which does
 *	*nothing*
 *
 * @author Leo Savernik
 * @since 3.3
 * @internal
 */
template<class T> class MassDeleter {
  void *obj_arr;		// object array
  size_t res;			// capacity in objects
  size_t n_objs;		// current object count
public:
  /** Creates a new mass deleter with an initial capacity of \c res objects
   */
  MassDeleter(size_t res = 1) : obj_arr(0), res(res), n_objs(0)
  {
    obj_arr = malloc(res*sizeof(T));
  }
  /** Deletes the mass deleter by deleting all its objects stored in its
   * object array
   */
  ~MassDeleter()
  {
    for (size_t i = 0; i < n_objs; i++)
      // This delete mustn't do any actual deallocation
      delete &base()[i];
    free(obj_arr);
  }

  /** Allocates new space for an object whose deletion is to be scheduled by
   * this mass deleter.
   * @return address of new object
   */
  T *allocate();

  /** Returns the base address of this mass deleter at which the allocation
   * vector starts.
   */
  T *base() const { return reinterpret_cast<T *>(obj_arr); }

  /** Returns the number of objects managed by this mass deleter.
   */
  int size() const { return (int)n_objs; }
};
#else
/**
 * Stores objects of a certain type, and calls delete on each of them
 * when this data structure is destroyed.
 *
 * As this structure merely consists of a vector of pointers, all objects
 * allocated can be traversed as seen fit.
 *
 * @author Leo Savernik
 * @since 3.3
 * @internal
 */
template<class T> class MassDeleter : public QValueVector<T *> {
public:
  MassDeleter(size_t reserved = 1) { reserve(reserved); }
  ~MassDeleter()
  {
    Iterator nd = end();
    for (Iterator it = begin(); it != nd; ++it)
      delete *it;
  }
};
#endif

class CaretBoxLine;

/**
 * Represents a rectangular box within which the caret is located.
 *
 * The caret box serves as a wrapper for inline boxes of all kind. It either
 * wraps an InlineBox, InlineTextBox, or InlineFlowBox, or if no such boxes
 * exist for a certain context, it contains the relevant information directly.
 *
 * This class will be constructed whenever a caret position has to be described.
 * @since 3.3
 * @author Leo Savernik
 * @internal
 */
class CaretBox {
protected:
  InlineBox *_box;		// associated inline box if available.
  short _w;			// width of box in pixels
  int _h;			// height of box in pixels
  int _x;			// x coordinate relative to containing block
  int _y;			// y coordinate relative to containing block
  RenderBox *cb;		// containing block
  bool _outside:1;		// true when representing the outside of the element
  bool outside_end:1;		// at ending outside of element rather than at beginning
  // 29 bits unused

public:
  /** empty constructor for later assignment */
  CaretBox() {}
  /** initializes the caret box from the given inline box */
  CaretBox(InlineBox *ibox, bool outside, bool outsideEnd) : _box(ibox),
  	_w((short)ibox->width()), _h(ibox->height()), _x(ibox->xPos()),
        _y(ibox->yPos()), cb(0), _outside(outside), outside_end(outsideEnd)
  {
    RenderObject *r = ibox->object();
    if (r) cb = r->containingBlock();
  }
  /** initializes the caret box from scratch */
  CaretBox(int x, int y, int w, int h, RenderBox *cb, bool outside, bool outsideEnd) :
         _box(0), _w((short)w), _h(h), _x(x), _y(y), cb(cb), _outside(outside),
         outside_end(outsideEnd)
  {}

  int width() const { return _w; }
  int height() const { return _h; }
  int xPos() const { return _x; }
  int yPos() const { return _y; }
  RenderBox *enclosingObject() const { return cb; }
  InlineBox *inlineBox() const { return _box; }

  /** returns the containing block of this caret box. If the caret box
   * resembles a block itself, its containing block is returned.
   */
  RenderBlock *containingBlock() const { return _box ? static_cast<RenderBlock *>(cb) : cb->containingBlock(); }

  /** returns the replaced render object if this caret box represents one,
   * 0 otherwise.
   */


  /** returns true if this caret box represents an inline element, or text box,
   * otherwise false.
   */
  bool isInline() const { return _box; }
  /** returns true if this caret box represents an inline text box.
   */
  bool isInlineTextBox() const { return _box && _box->isInlineTextBox(); }
  /** returns true if this caret box represents a line break
   */
  bool isLineBreak() const
  {
    return _box && _box->object() && _box->object()->isBR();
  }
  /** returns true when this caret box represents an ouside position of an
   * element.
   */
  bool isOutside() const { return _outside; }
  /** returns the position at which the outside is targeted at.
   *
   * This method's return value is meaningless if isOutside() is not true.
   * @return true if the outside end is meant, false if the outside beginning
   *	is meant.
   */
  bool isOutsideEnd() const { return outside_end; }
  /** returns the associated render object. */
  RenderObject *object() const { return _box ? _box->object() : cb; }

  /** returns the minimum offset for this caret box.
   */
  long minOffset() const { return _box && !isLineBreak() ? _box->minOffset() : 0; }
  /** returns the maximum offset for this caret box.
   */
  long maxOffset() const { return _box && !isLineBreak() ? _box->maxOffset() : 0; }

#if 0
  // Overloaded new operator.
  void *operator new(size_t sz, MassDeleter<CaretBox> *deleter) KDE_NO_EXPORT throw();

  // Overridden to prevent the normal delete from being called.
  void operator delete(void * /*ptr*/, size_t /*sz*/)
  {
    // don't do anything here. This constructor is only good for the compiler
    // to emit proper destructor calls.
  }
#endif

#if DEBUG_CARETMODE > 0
  void dump(QTextStream &ts, const QString &ind) const;
#endif

  friend class CaretBoxLine;
};

typedef MassDeleter<CaretBox> CaretBoxDeleter;

#if 1
/**
 * Iterates over the elements of a caret box line.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.3
 */
class CaretBoxIterator {
protected:
  CaretBoxLine *cbl;		// associated caret box line
  int index;			// current index

public:
  // Let standard constructor/copy constructor/destructor/assignment operator
  // be defined by the compiler. They do exactly what we want.

  bool operator ==(const CaretBoxIterator &it) const
  {
    return cbl == it.cbl && index == it.index;
  }

  bool operator !=(const CaretBoxIterator &it) const
  {
    return !operator ==(it);
  }

  /** returns the current caret box.
   * @return current caret box
   */
  CaretBox *data() const;
  /** shortcut for \c data
   * @return current caret box
   */
  CaretBox *operator *() const { return data(); }

  /** increments the iterator to point to the next caret box.
   */
  CaretBoxIterator &operator ++() { index++; return *this; }
  /** decrements the iterator to point to the previous caret box.
   */
  CaretBoxIterator &operator --() { index--; return *this; }

  friend class CaretBoxLine;
  friend class EditableCaretBoxIterator;
};

/**
 * Resembles a line consisting of caret boxes.
 *
 * To the contrary of InlineFlowBoxes which are nested as needed to map the
 * DOM to the rendered representation, it is sufficient for caret navigation
 * to provide a linear list of unnested caret boxes.
 *
 * Example: The document fragment <p>a <i><b>c</b> f</i> g</p> will be
 * represented by three caret box lines which each one consists of caret boxes
 * as follows:
 * CaretBoxLine 1:
 *   CaretBox(cb=<p>, _box=0, _outside=true, outside_end=false)
 * CaretBoxLine 2:
 *   CaretBox(cb=<p>, _box=InlineTextBox("a "), _outside=false)
 *   CaretBox(cb=<p>, _box=InlineFlowBox(<i>), _outside=true, outside_end=false)
 *   CaretBox(cb=<p>, _box=InlineFlowBox(<b>), _outside=true, outside_end=false)
 *   CaretBox(cb=<p>, _box=InlineTextBox("c"), _outside=false)
 *   CaretBox(cb=<p>, _box=InlineFlowBox(<b>), _outside=true, outside_end=true)
 *   CaretBox(cb=<p>, _box=InlineTextBox(" f"), _outside=false)
 *   CaretBox(cb=<p>, _box=InlineFlowBox(<i>), _outside=true, outside_end=true)
 *   CaretBox(cb=<p>, _box=InlineTextBox(" g"), _outside=true, outside_end=true)
 * CaretBoxLine 3:
 *   CaretBox(cb=<p>, _box=0, _outside=true, outside_end=true)
 */
class CaretBoxLine {
protected:
  CaretBoxDeleter caret_boxes;
  // base flow box which caret boxes have been constructed for
  InlineFlowBox *basefb;

  CaretBoxLine() : caret_boxes(8), basefb(0) {}
  CaretBoxLine(InlineFlowBox *basefb) : caret_boxes(8), basefb(basefb) {}
public:
#if DEBUG_CARETMODE > 3
  ~CaretBoxLine() { kdDebug(6200) << k_funcinfo << "called" << endl; }
#endif

  CaretBoxIterator begin()
  {
    CaretBoxIterator it;
    it.cbl = this;
    it.index = 0;
    return it;
  }
  CaretBoxIterator end()
  {
    CaretBoxIterator it;
    it.cbl = this;
    it.index = caret_boxes.size();
    return it;
  }
  CaretBoxIterator preBegin()
  {
    CaretBoxIterator it;
    it.cbl = this;
    it.index = -1;
    return it;
  }
  CaretBoxIterator preEnd()
  {
    CaretBoxIterator it;
    it.cbl = this;
    it.index = caret_boxes.size() - 1;
    return it;
  }

  /** returns the base inline flow box which the caret boxes of this
   * caret box line have been constructed from.
   *
   * This is generally a root line box, but may be an inline flow box when the
   * base is restricted to an inline element.
   */
  InlineFlowBox *baseFlowBox() const { return basefb; }

  /** returns the containing block */
  RenderBlock *containingBlock() const { return caret_boxes[0]->containingBlock(); }
  /** returns the enclosing object */
  RenderBox *enclosingObject() const { return caret_boxes[0]->enclosingObject(); }

  /** returns whether this caret box line is outside.
   * @return true if this caret box represents an outside position of this
   *	line box' containing block, false otherwise.
   */
  bool isOutside() const
  {
    const CaretBox *cbox = caret_boxes[0];
    return !cbox->isInline() && cbox->isOutside();
  }

  /** returns whether this caret box line is at the outside end.
   *
   * The result cannot be relied upon unless isOutside() returns true.
   */
  bool isOutsideEnd() const { return caret_boxes[0]->isOutsideEnd(); }

  /** constructs a new caret box line out of the given inline flow box
   * @param deleter deleter which handles alloc+dealloc of the object
   * @param baseFlowBox basic flow box which to create a caret line box from
   * @param seekBox seek this box within the constructed line
   * @param seekOutside denoting whether position is outside of seekBox
   * @param seekOutsideEnd whether at the outside end of seekBox
   * @param iter returns an iterator that corresponds to seekBox. If no suitable
   *	caret box exists, it will return end()
   * @param seekObject seek this render object within the constructed line.
   *	It will only be regarded if \c seekBox is 0. \c iter will then point
   *	to the first caret box whose render object matches.
   */
  static CaretBoxLine *constructCaretBoxLine(MassDeleter<CaretBoxLine> *deleter,
  	InlineFlowBox *baseFlowBox, InlineBox *seekBox, bool seekOutside,
        bool seekOutsideEnd, CaretBoxIterator &iter,
	RenderObject *seekObject = 0) /*KDE_NO_EXPORT*/;

  /** constructs a new caret box line for the given render block.
   * @param deleter deleter which handles alloc+dealloc of the object
   * @param cb render block or render replaced
   * @param outside true when line is to be constructed outside
   * @param outsideEnd true when the ending outside is meant
   * @param iter returns the iterator to the caret box representing the given
   *	position for \c cb
   */
  static CaretBoxLine *constructCaretBoxLine(MassDeleter<CaretBoxLine> *deleter,
  	RenderBox *cb, bool outside, bool outsideEnd, CaretBoxIterator &iter) /*KDE_NO_EXPORT*/;

#if 0
  // Overloaded new operator.
  void* operator new(size_t sz, MassDeleter<CaretBoxLine> *deleter) KDE_NO_EXPORT throw();

  // Overridden to prevent the normal delete from being called.
  void operator delete(void* /*ptr*/, size_t /*sz*/)
  {
    // don't do anything here. This constructor is only good for the compiler
    // to emit proper destructor calls.
  }
#endif

#if DEBUG_CARETMODE > 0
  void dump(QTextStream &ts, const QString &ind) const;
  QString information() const
  {
    QString result;
    QTextStream ts(&result, IO_WriteOnly);
    dump(ts, QString::null);
    return result;
  }
#endif

protected:
  /** contains the seek parameters */
  struct SeekBoxParams {
    InlineBox *box;
    bool outside;
    bool outsideEnd;
    bool found;
    RenderObject *r;	// if box is 0, seek for equal render objects instead
    CaretBoxIterator &it;

    SeekBoxParams(InlineBox *box, bool outside, bool outsideEnd, RenderObject *obj, CaretBoxIterator &it)
        : box(box), outside(outside), outsideEnd(outsideEnd), found(false), r(obj), it(it)
    {}

    /** compares whether this seek box matches the given specification */
    bool equalsBox(const InlineBox *box, bool outside, bool outsideEnd) const
    {
      return (this->box && this->box == box
              || this->r == box->object())
          && this->outside == outside
          && (!this->outside || this->outsideEnd == outsideEnd);
    }
    /** compares whether this seek box matches the given caret box */
    bool operator ==(const CaretBox *cbox) const
    {
      return equalsBox(cbox->inlineBox(), cbox->isOutside(), cbox->isOutsideEnd());
    }
    /** checks whether this box matches the given iterator.
     *
     * On success, it sets \c found, and assigns the iterator to \c it.
     * @return true on match
     */
    bool check(const CaretBoxIterator &chit)
    {
      if (*this == *chit) {
        Q_ASSERT(!found);
        found = true;
        it = chit;
      }
      return found;
    }
  };

  /** recursively converts the given inline box into caret boxes and adds them
   * to this caret box line.
   *
   * It will additionally look for the caret box specified in SeekBoxParams.
   */
  void addConvertedInlineBox(InlineBox *, SeekBoxParams &) /*KDE_NO_EXPORT*/;

  /** creates and adds the edge of a generic inline box
   * @param box inline box
   * @param fm font metrics of inline box
   * @param left true to add left edge, false to add right edge
   * @param rtl true if direction is rtl
   */
  void addCreatedInlineBoxEdge(InlineBox *box, const QFontMetrics &fm,
  	bool left, bool rtl) /*KDE_NO_EXPORT*/;
  /** creates and adds the edge of an inline flow box
   * @param flowBox inline flow box
   * @param fm font metrics of inline flow box
   * @param left true to add left edge, false to add right edge
   * @param rtl true if direction is rtl
   */
  void addCreatedFlowBoxEdge(InlineFlowBox *flowBox, const QFontMetrics &fm,
  	bool left, bool rtl) /*KDE_NO_EXPORT*/;
  /** creates and adds the inside of an inline flow box
   * @param flowBox inline flow box
   * @param fm font metrics of inline flow box
   */
  void addCreatedFlowBoxInside(InlineFlowBox *flowBox, const QFontMetrics &fm) /*KDE_NO_EXPORT*/;

  friend class CaretBoxIterator;
};

typedef MassDeleter<CaretBoxLine> CaretBoxLineDeleter;

inline CaretBox *CaretBoxIterator::data() const { return cbl->caret_boxes[index]; }
#endif

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
  CaretBoxLine *cbl;		// current caret box line
  // ### those two are redundant, they can be determined from the cbl state
//   bool cb_outside:1;		// is this line iterator denoting the outside
//   				// position of its containing block cb?
//   bool cb_outside_end:1;	// is the containing block's outside position
//   				// at the end?

  static CaretBoxIterator currentBox;	// current inline box
  static long currentOffset;

  // Note: cbl == 0 indicates a position beyond the beginning or the
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
  /** dereferences current caret box line.
   *
   * @returns the caret line box or 0 if end of document
   */
  CaretBoxLine *operator *() const { return cbl; }

  /** returns the associated linear document
   */
  LinearDocument *linearDocument() const { return lines; }

  /** seek next line
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator ++() { advance(false); return *this; }
#if 0		// not used
  /** seek next line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator ++(int);
#endif

  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   */
  LineIterator &operator --() { advance(true); return *this; }
#if 0		// not used
  /** seek previous line.
   *
   * Guaranteed to crash if beyond beginning/end of document.
   *
   * Note: The postfix operator is slow. Use the prefix operator whenever
   * possible.
   */
  LineIterator operator --(int);
#endif

#if 0		// not used
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
#endif

  /** compares two iterators. The comparator actually works only for
   * comparing arbitrary iterators to begin() and end().
   */
  bool operator ==(const LineIterator &it) const
  {
    return lines == it.lines && cbl == it.cbl;
  }

  /** compares two iterators
   */
  bool operator !=(const LineIterator &it) const
  {
    return !operator ==(it);
  }

  /** Returns whether this line represents the outside end of the containing
   * block.
   *
   * This result can only be relied on when isOutside is true.
   */
  bool isOutsideEnd() { return cbl->isOutsideEnd(); }

  /** Tells whether the offset is meant to be outside or inside the
   * containing block.
   */
  bool isOutside() const { return cbl->isOutside(); }

  /** advances to the line to come.
   * @param toBegin true, move to previous line, false, move to next line.
   */
  void advance(bool toBegin);

  /** Whenever a new line iterator is created, it gets a caret box created.
   * For memory reasons, it's saved in a static instance,
   * thus making this function not thread-safe.
   *
   * This value can only be trusted immediately after having instantiated
   * a line iterator or one of its derivatives.
   * @return an iterator onto the corresponing caret box within the
   *	line represented by the last instantiation of a line iterator,
   *	or 0 if there was none.
   */
  static CaretBoxIterator &currentCaretBox() { return currentBox; }

  /** Whenever a new line iterator is created, it calculates a modified offset
   * that is to be used with respect to the current render object.
   * This offset can be queried with this function.
   *
   * This value can only be trusted immediately after having instantiated
   * a line iterator or one of its derivatives.
   * @return the modified offset.
   */
  static long currentModifiedOffset() { return currentOffset; }

protected:
  /** seeks next block.
   */
  void nextBlock();
  /** seeks previous block.
   */
  void prevBlock();

  friend class CaretBoxIterator;
  friend class EditableLineIterator;
  friend class EditableCaretBoxIterator;
  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class EditableCharacterIterator;
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
 * The lines itself are represented as caret line boxes.
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
   * @param advancePolicy caret advance policy
   * @param baseElem base element which the caret must not advance beyond
   *	(0 means whole document). The base element will be ignored if it
   *	cannot serve as a base (to see if this is the case, check whether
   *	LinearDocument::baseFlow()->element() != base)
   */
  LinearDocument(KHTMLPart *part, DOM::NodeImpl *node, long offset,
  		CaretAdvancePolicy advancePolicy, DOM::ElementImpl *baseElem);

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

  /**
   * Returns the current caret advance policy
   */
  CaretAdvancePolicy advancePolicy() const { return advPol; }

  /**
   * Returns the base render object which the caret must not advance beyond.
   *
   * Note that HTML documents are usually restricted to the body element.
   *
   * @return the base render object or 0 if the whole document is valid.
   */
  RenderObject *baseObject() const { return base; }

protected:
  void initPreBeginIterator();
  void initEndIterator();

protected:
  // ### ditch arena, but leave for now to make the code compile
  RenderArena *arena;		// We need an arena for intermediate render
  				// objects that have no own inline box
  CaretBoxLineDeleter cblDeleter;	// mass deleter for caret box lines
  DOM::NodeImpl *node;
  long offset;

  Iterator _preBegin;
  Iterator _end;

  KHTMLPart *m_part;
  CaretAdvancePolicy advPol;
  RenderObject *base;

  friend class LineIterator;
  friend class EditableLineIterator;
  friend class ErgonomicEditableLineIterator;
  friend class CaretBoxIterator;
  friend class EditableCaretBoxIterator;
  friend class InlineBoxIterator;
  friend class EditableInlineBoxIterator;
  friend class EditableCharacterIterator;
};


#if 0
/**
 * Iterates over the inner elements of an inline flow box in terms of
 * caret boxes.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.3
 */
class CaretBoxIterator {
  CaretBoxDeleter *cbdeleter;	// mass deleter for caret boxes
  InlineFlowBox *fb;		// topmost inline flow box
  CaretBox *box;		// currently traversed caret box

public:
  /** creates a new iterator, initialized with the given flow box.
   * @param cbdeleter mass deleter for caret boxes
   * @param flowBox inline flow box which this iterator is restricted to.
   * @param cb containing block.
   * @param cb_outside whether the outside of \c cb is meant or not. This is
   *	only regarded when flowBox is 0.
   * @param cb_outside_end whether the outside position at the end is meant
   * @param fromEnd \c true to iterate the caret boxes from the end of the
   *	line box, otherwise from the beginning.
   */
  CaretBoxIterator(CaretBoxDeleter *cbdeleter, InlineFlowBox *flowBox,
    		   RenderBlock *cb, bool cb_outside, bool cb_outside_end,
                   bool fromEnd = false)
 	: cbdeleter(cbdeleter), fb(flowBox)
  {
    init(cb, cb_outside, cb_outside_end);
  }


  /** creates a new iterator, initialized with the given line iterator.
   * @param lit given line iterator
   * @param fromEnd \c true to iterate the caret boxes from the end of the
   *	line box, otherwise from the beginning.
   * @param initBox start iterating with this very caret box, thus effectively
   *	ignoring \c fromEnd
   * out_ofs and outside are tied to the given line iterator's.
   */
  CaretBoxIterator(LineIterator &lit, bool fromEnd = false, CaretBox *initBox = 0)
	: cbdeleter(cbdeleter), fb(flowBox)
  {
    if (initBox) box = initBox;
    else init(cb, cb_outside, cb_outside_end);
  }

  /** empty constructor.
   */
  CaretBoxIterator() {}

  bool operator ==(const CaretBoxIterator &it) const
  {
    return fb == it.fb && box == it.box;
  }

  bool operator !=(const CaretBoxIterator &it) const
  {
    return !operator ==(it);
  }

  /** returns the current caret box.
   * @return pointer to caret box, or 0 if the end of the line has been reached
   */
  CaretBox *operator *() { return box; }

  /** increments the iterator to point to the next caret box on this line box.
   */
  CaretBoxIterator &operator ++();
  /** increments the iterator to point to the previous inline box on this line box.
   */
  CaretBoxIterator &operator --();

private:
  /** initializes this iterator */
  void init(RenderBlock *cb, bool cb_outside, bool cb_outside_end);
};
#endif

/**
 * Iterates over the editable inner elements of a caret line box.
 *
 * The incrementor will traverse all caret boxes according to the associated
 * linear document's caret advance policy. In contrast to \c CaretBoxIterator
 * this iterator only regards caret boxes which are editable.
 *
 * @author Leo Savernik
 * @internal
 * @since 3.3
 */
class EditableCaretBoxIterator : public CaretBoxIterator {
  KHTMLPart *m_part;
  bool adjacent;
  CaretAdvancePolicy advpol;	// caret advance policy

public:
#if 0
  /** creates a new iterator, initialized with the given caret line box.
   * @param part khtml part within which all operations are taking place.
   * @param cbl caret line box to be iterated
   * @param advpol current caret advance policy
   * @param fromEnd @p true, start with last box in line
   */
  EditableCaretBoxIterator(KHTMLPart *part, CaretBoxLine *cbl,
  	CaretAdvancePolicy advpol, bool fromEnd = false)
  	: m_part(part), adjacent(false), advpol(advpol)
  {
    *this = fromEnd ? cbl->preEnd() : cbl->begin();
    if (!isEditable(*this, cbl->end(), fromEnd)) {
      if (fromEnd) --*this; else ++*this;
    }
  }
#endif

  /** initializes a new iterator from the given line iterator,
   * beginning with the given caret box iterator, if specified
   */
  EditableCaretBoxIterator(LineIterator &lit, bool fromEnd = false,
  		CaretBoxIterator *it = 0)
  		: CaretBoxIterator(it ? *it : (fromEnd ? (*lit)->end() : (*lit)->preBegin())),
                m_part(lit.lines->m_part), adjacent(false),
                advpol(lit.lines->advancePolicy())
  {
#if 0
    if (*this != cbl->end() && *this != cbl->preBegin()
    	&& !isEditable(*this, cbl->preBegin(), fromEnd)) {
#else
    if (!it) {
#endif
      if (fromEnd) --*this; else ++*this;
    }
  }

  /** empty constructor. Use only to copy another iterator into this one.
   */
  EditableCaretBoxIterator() {}

  /** returns @p true when the current caret box is adjacent to the
   * previously iterated caret box, i. e. no intervening caret boxes.
   */
  bool isAdjacent() const { return adjacent; }

  /** increments the iterator to point to the next editable caret box.
   */
  EditableCaretBoxIterator &operator ++() { advance(false); return *this; }

  /** decrements the iterator to point to the previous editable caret box.
   */
  EditableCaretBoxIterator &operator --() { advance(true); return *this; }

  /** advances to the editable caret box to come
   * @param toBegin true, move towards beginning, false, move towards end.
   */
  void advance(bool toBegin);

protected:
  /** finds out if the given box is editable.
   * @param boxit iterator to given caret box
   * @param fromEnd true when advancing towards the beginning
   * @return @p true if box is editable
   */
  bool isEditable(const CaretBoxIterator &boxit, bool fromEnd);
};

/**
 * Iterates over the inner elements of an inline flow box.
 *
 * The given inline flow box must be a line box. The incrementor will
 * traverse all inline boxes according to the associated linear document's
 * caret advance policy. [It will also generate transient inline boxes
 * for those render objects that do not have one. (This is nonsense)]
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class InlineBoxIterator {
protected:
    RenderArena *arena;	// arena for allocating transient inline boxes
    InlineBox *box;	// currently traversed inline box
    CaretAdvancePolicy advpol;	// caret advance policy
    long out_ofs;	// outside offset of the associated line box. Only
    			// relevant when outside is true
    bool outside:1;	// true when the associated line box represents an
    			// outside position
    bool fromEnd:1;	// true when advancing towards the beginning

public:
    /** creates a new iterator, initialized with the given flow box.
   * @param outside whether the associated inline flow box represents an
   *	outside position.
     */
    InlineBoxIterator(RenderArena *arena, InlineFlowBox *flowBox,
    		      CaretAdvancePolicy advpol, long out_ofs, bool outside,
		      bool fromEnd = false);

    /** creates a new iterator, initialized with the given line iterator,
     * using the given inline box, if specified.
     * out_ofs and outside are tied to the given line iterator's.
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
 * traverse all inline boxes according to the associated linear document's
 * caret advance policy. In contrast to @p InlineBoxIterator this
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
   * @param out_ofs outside offset of the associated line (irrelevant if
   *	outside is false).
   * @param outside whether the associated inline flow box represents an
   *	outside position.
   * @param fromEnd @p true, start with last box in line
   */
  EditableInlineBoxIterator(LinearDocument *ld, RenderArena *arena,
  		InlineFlowBox *flowBox, long out_ofs, bool outside,
		bool fromEnd = false)
  	: InlineBoxIterator(arena, flowBox, ld->advancePolicy(), out_ofs, outside, fromEnd),
	m_part(ld->m_part), adjacent(true)
  {
    if (box && !isEditable(box)) {
      if (fromEnd) --*this; else ++*this;
    }
  }

  /** initializes a new iterator from the given line iterator,
   * beginning with the given inline box, if specified.
   */
  EditableInlineBoxIterator(LineIterator &lit, bool fromEnd = false,
  		InlineBox *initBox = 0)
  		: InlineBoxIterator(lit, fromEnd, initBox),
		m_part(lit.lines->m_part), adjacent(true)
  {
    if (box && !isEditable(box)) {
      if (fromEnd) --*this; else ++*this;
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
  bool isEditable(InlineBox *b);
};

/**
 * Iterates through the editable lines of a document.
 *
 * This iterator, opposing to @p LineIterator, only regards editable lines.
 * Additionally, this iterator enforces the caret advance policy.
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
    if (!cbl) return;
    if (!isEditable(*this)) advance(fromEnd);
#if 0
    if (!flowBox || !cb) {
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "EditableLineIterator: findFlowBox failed" << endl;
#endif
      cb = 0;
    }/*end if*/
#endif
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
  EditableLineIterator &operator ++() { advance(false); return *this; }
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
  EditableLineIterator &operator --() { advance(true); return *this; }
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

  /** advances to the line to come.
   * @param toBegin true, move to previous line, false, move to next line.
   */
  void advance(bool toBegin);

protected:
  /** finds out if the current line is editable.
   *
   * @param it check caret box line iterator points to
   * @param lastit previous caret box line
   * @return @p true if line is editable
   */
  bool isEditable(LineIterator &it)
  {
    EditableCaretBoxIterator fbit = it;
    return fbit != (*it)->end();
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
  int xCoor;		// x-coordinate to determine cell position
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
  void calcAndStoreNewLine(RenderBlock *newBlock, bool toBegin);

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
  EditableLineIterator _it;
  EditableCaretBoxIterator ebit;
  long _offset;		// offset within current caret box.
  int _char;
  bool _end:1;		// true when end of document has been reached

public:

  /** empty constructor.
   *
   * Only use if you want to assign another iterator as no fields will
   * be initialized.
   */
  EditableCharacterIterator() {}

  /** constructs a new iterator from the given linear document.
   *
   * @param ld linear representation of document.
   */
  EditableCharacterIterator(LinearDocument *ld)
  		: _it(ld->current()),
                ebit(_it, false, &_it.currentCaretBox()),
      		_offset(_it.currentModifiedOffset()), _char(-1), _end(false)
  {
    // ### temporary fix for illegal nodes
    if (_it == ld->end()) { _end = true; return; }

#if 0
    _r = ld->node->renderer();

    // seeks the node's inline box
    // ### redundant, implement means to get it from ld or _it
    // ### if node is not there?
    EditableInlineBoxIterator copy = ebit;
    for (; *ebit; ++ebit) {
      copy = ebit;
      InlineBox *b = *ebit;

      if (b == _it.currentInlineBox() || b->object() == _r) {
        if (!_it.outside) _it._offset = kMin(kMax(_it._offset, b->minOffset()), b->maxOffset());
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
      _r = b->object();
      long max_ofs = b->maxOffset();
      _it._offset = cache_offset == max_ofs ? b->minOffset() : max_ofs;
      cache_offset = _it._offset;
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "There was no node! Fixup applied!" << endl;
      if (cache_offset == max_ofs) kdDebug(6200) << "offset fixup applied as well" << endl;
#endif
    }/*end if*/
#endif

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

  /** returns true when the end of the document has been reached.
   */
  bool isEnd() const { return _end; }
  /** returns the current offset
   */
  long offset() const { return _offset; }
  /** returns the current render object.
   */
  RenderObject *renderer() const { return (*ebit)->object(); }
  /** returns the current caret box.
   *
   * Will crash if beyond end.
   */
  CaretBox *caretBox() const { return *ebit; }
  /** returns the current inline box.
   *
   * May be 0 if the current element has none, or if the end has been reached.
   * Therefore, do *not* use this to test for the end condition, use node()
   * instead.
   */
  InlineBox *inlineBox() const { return (*ebit)->inlineBox(); }
  /** returns whether the current line box represents the outside of its
   * render object.
   */
//  bool boxIsOutside() const { return _it.isOutside(); }

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
    EditableCaretBoxIterator copy = ebit;
    ++copy;
    if (copy == (*_it)->end()) { _char = -1; return; }

    CaretBox *box = *copy;
    InlineBox *b = box->inlineBox();
    if (b && !box->isOutside() && b->isInlineTextBox())
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


}/*namespace khtml*/


#endif
