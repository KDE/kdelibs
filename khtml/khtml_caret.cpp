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


#include "khtml_caret_p.h"

namespace khtml {

static InlineFlowBox *findFlowBox(DOM::NodeImpl *node, long offset,
		RenderArena *arena, RenderFlow *&cb, InlineBox **ibox = 0);
static RenderObject *nextLeafRenderObject(RenderObject *r);

/** Returns the previous suitable render object that is a leaf.
 *
 * Suitable means suitable for caret navigation.
 */
static inline RenderObject *nextSuitableLeafRenderObject(RenderObject *r)
{
  do {
    r = nextLeafRenderObject(r);
  } while (r && r->isTableCol());
  return r;
}

/** Make sure the given node is a leaf node. */
static void ensureLeafNode(NodeImpl *&node)
{
  if (node && node->hasChildNodes()) node = node->nextLeafNode();
}

/** Finds the next node that has a renderer.
 *
 * Note that if the initial @p node has a renderer, this will be returned,
 * regardless of being a leaf node.
 * Otherwise, for the next nodes, only leaf nodes are considered.
 * @param node node to start with, will be updated accordingly
 * @return renderer or 0 if no following node has a renderer.
 */
static RenderObject* findRenderer(NodeImpl *&node)
{
  if (!node) return 0;
  RenderObject *r = node->renderer();
  while (!r) {
    node = node->nextLeafNode();
    if (!node) break;
    r = node->renderer();
  }
  if (r && r->isTableCol()) r = nextSuitableLeafRenderObject(r);
  return r;
}

/** Bring caret information position into a sane state */
static void sanitizeCaretState(NodeImpl *&caretNode, long &offset)
{
  ensureLeafNode(caretNode);

  // FIXME: this leaves caretNode untouched if there are no more renderers.
  // It better should search backwards then.
  // This still won't solve the problem what to do if *no* element has a
  // renderer.
  NodeImpl *tmpNode = caretNode;
  if (findRenderer(tmpNode)) caretNode = tmpNode;
  if (!caretNode) return;

  long max = caretNode->maxOffset();
  long min = caretNode->minOffset();
  if (offset < min) offset = min;
  else if (offset > max) offset = max;
}

/** Returns the previous render object that is a leaf object. */
static RenderObject *prevLeafRenderObject(RenderObject *r)
{
  RenderObject *n = r->objectAbove();
  while (n && n == r->parent()) {
    if (n->previousSibling()) return n->objectAbove();
    r = n;
    n = r->parent();
  }
  return n;
}

/** Returns the following render object that is a leaf object. */
static RenderObject *nextLeafRenderObject(RenderObject *r)
{
  RenderObject *n = r->objectBelow();
  r = n;
  while (n) r = n, n = n->firstChild();
  return r;
}

/** Returns the previous suitable render object that is a leaf.
 *
 * Suitable means suitable for caret navigation.
 */
static RenderObject *prevSuitableLeafRenderObject(RenderObject *r)
{
  do {
    r = prevLeafRenderObject(r);
  } while (r && r->isTableCol());
  return r;
}

/** Seeks the next leaf inline box.
 */
static inline InlineBox *seekLeafInlineBox(InlineBox *box)
{
  while (box && box->isInlineFlowBox()) {
//    if (box->isInlineFlowBox()) {
      box = static_cast<InlineFlowBox *>(box)->firstChild();
//    else if (box->object()->isFlow())
//      box = static_cast<RenderFlow *>(box->object())->firstLineBox();
//    else
//      break;
  }/*wend*/
  return box;
}

/** Seeks the next leaf inline box, beginning from the end.
 */
static inline InlineBox *seekLeafInlineBoxFromEnd(InlineBox *box)
{
  while (box && box->isInlineFlowBox()) {
      box = static_cast<InlineFlowBox *>(box)->lastChild();
  }/*wend*/
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "seekLeafFromEnd: box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;
#endif
  return box;
}



InlineBox *LineIterator::currentBox;

InlineBoxIterator::InlineBoxIterator(RenderArena *arena, InlineFlowBox *flowBox, bool fromEnd)
    : arena(arena)
{
    box = fromEnd ? seekLeafInlineBoxFromEnd(flowBox) : seekLeafInlineBox(flowBox);
}

InlineBoxIterator::InlineBoxIterator(LineIterator &lit, bool fromEnd,
                      InlineBox *initBox)
    : arena(lit.lines->arena)
{
    if (initBox) box = initBox;
    else box = fromEnd ? seekLeafInlineBoxFromEnd(*lit) : seekLeafInlineBox(*lit);
}


InlineBoxIterator& InlineBoxIterator::operator ++()
{
    InlineBox *newBox = box->nextOnLine();

    if (newBox)
      box = seekLeafInlineBox(newBox);
    else {
      InlineFlowBox *flowBox = box->parent();
      box = 0;
      while (flowBox) {
        InlineBox *newBox2 = flowBox->nextOnLine();
	if (newBox2) {
	  box = seekLeafInlineBox(newBox2);
	  break;
	}/*end if*/

	flowBox = flowBox->parent();
      }/*wend*/
    }/*end if*/

    return *this;
}

  /** decrements the iterator to point to the previous inline box on this
   * line box.
   */
InlineBoxIterator& InlineBoxIterator::operator --()
{
    InlineBox *newBox = box->prevOnLine();

    if (newBox)
        box = seekLeafInlineBoxFromEnd(newBox);
    else {
        InlineFlowBox *flowBox = box->parent();
        box = 0;
        while (flowBox) {
        InlineBox *newBox2 = flowBox->prevOnLine();
	if (newBox2) {
	  box = seekLeafInlineBoxFromEnd(newBox2);
	  break;
	}/*end if*/

	flowBox = flowBox->parent();
      }/*wend*/
    }/*end if*/

    return *this;
}

/** generates a transient inline flow box.
 *
 * Empty blocks don't have inline flow boxes constructed. This function
 * constructs a dummy flow box which contains a single empty inline box with
 * coordinate information to be able to display the caret correctly.
 *
 * You can find out whether an inline flow box is transient by checking its
 * render object and the render object of its only child. If both are equal,
 * it's transient.
 *
 * @param arena render arena in which to create the flow box
 * @param cb containing block which to create the inline flow box for.
 * @param childNodeHint if given, use this as the associated render object for
 *	the filled-in inline box.
 * @return the constructed inline flow box.
 */
static InlineFlowBox* generateDummyFlowBox(RenderArena *arena, RenderFlow *cb,
			RenderObject *childNodeHint = 0)
{
  InlineFlowBox *flowBox = new(arena) InlineFlowBox(cb);
  int width = cb->width();
  // FIXME: this does neither take into regard :first-line nor :first-letter
  // However, as soon as some content is entered, the line boxes will be
  // constructed properly and this kludge is not called any more. So only
  // the caret size of an empty :first-line'd block is wrong, but I think we
  // can live with that.
  int height = cb->style()->fontMetrics().height();
  flowBox->setWidth(0);
  flowBox->setHeight(height);

  // Add single child at the right position
  InlineBox *child = new(arena) InlineBox(childNodeHint ? childNodeHint : cb);
  // ### regard direction
  switch (cb->style()->textAlign()) {
    case LEFT:
    case TAAUTO:	// ### find out what this does
    case JUSTIFY:
      child->setXPos(0);
      break;
    case CENTER:
    case KONQ_CENTER:
      child->setXPos(width / 2);
      break;
    case RIGHT:
      child->setXPos(width);
      break;
  }/*end switch*/
  child->setYPos(0);
  child->setWidth(1);
  child->setHeight(height);

  flowBox->setXPos(child->xPos());
  flowBox->setYPos(child->yPos());
  flowBox->addToLine(child);
  //kdDebug(6200) << "generateDummyFlowBox: " << flowBox << " with child " << child << endl;
  return flowBox;
}

/** generates a dummy block for elements whose containing block is not a flow.
 * @param arena render arena in which to create the flow
 * @param cb block which to create the flow for.
 * @return the constructed flow.
 */
static RenderFlow* generateDummyBlock(RenderArena */*arena*/, RenderObject *cb)
{
    // ### will fail if positioned
  RenderFlow *result = RenderFlow::createFlow(cb->element(), cb->style(), cb->renderArena());
  result->setParent(cb->parent());
  result->setPreviousSibling(cb->previousSibling());
  result->setNextSibling(cb->nextSibling());

  result->setOverhangingContents(cb->overhangingContents());
  result->setPositioned(cb->isPositioned());
  result->setRelPositioned(cb->isRelPositioned());
  result->setFloating(cb->isFloating());
  result->setInline(cb->isInline());
  result->setMouseInside(cb->mouseInside());

  result->setPos(cb->xPos(), cb->yPos());
  result->setWidth(cb->width());
  result->setHeight(cb->height());

  return result;
}

/** determines the inline flow box that contains the given node.
 *
 * If the node does not map to an inline flow box, the function will snap to
 * the next inline box following it.
 *
 * @param node node to begin with
 * @param offset zero-based offset within node.
 * @param arena sometimes the function must create a temporary inline flow box
 *	therefore it needs a render arena.
 * @param cb returns the containing block
 * @param ibox returns the inline box that contains the node.
 * @return the found inlineFlowBox or 0 if either the node is 0 or
 *	there is no inline flow box containing this node. The containing block
 *	will still be set. If it is 0 too, @p node was invalid.
 */
static InlineFlowBox* findFlowBox(DOM::NodeImpl *node, long offset,
		RenderArena *arena, RenderFlow *&cb, InlineBox **ibox)
{
  RenderObject *r = findRenderer(node);
  if (!r) { cb = 0; return 0; }
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "=================== findFlowBox" << endl;
  kdDebug(6200) << "node " << node << " r " << r->renderName() << "[" << r << "].node " << r->element()->nodeName().string() << "[" << r->element() << "]" << " offset: " << offset << endl;
#endif

  // If we have a totally empty render block, we simply construct a
  // transient inline flow box, and be done with it.
  // This case happens only when the render block is a leaf object itself.
  if (r->isRenderBlock() && !static_cast<RenderBlock *>(r)->firstLineBox()) {
    cb = static_cast<RenderBlock *>(r);
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "=================== end findFlowBox (dummy)" << endl;
#endif
    InlineFlowBox *fb = generateDummyFlowBox(arena, cb);
    if (ibox) *ibox = fb;
    return fb;
  }/*end if*/

  // There are two strategies to find the correct line box.
  // (A) First, if node's renderer is a RenderText, we only traverse its text
  // runs and return the root line box (saves much time for long blocks).
  // This should be the case 99% of the time.
  // (B) Otherwise, we iterate linearly through all line boxes in order to find
  // the renderer. (A reverse mapping would be favorable, but needs memory)
  if (r->isText()) do {
    RenderText *t = static_cast<RenderText *>(r);
    int dummy;
    InlineBox *b = t->findInlineTextBox(offset, dummy, true);
    // Actually b should never be 0, but some render texts don't have text
    // boxes, so we insert the last run as an error correction.
    // If there is no last run, we resort to (B)
    if (!b) {
      if (t->m_lines.count() > 0)
        b = t->m_lines[t->m_lines.count() - 1];
      else
        break;
    }/*end if*/
    Q_ASSERT(b);
    if (ibox) *ibox = b;
    while (b->parent()) {	// seek root line box
      b = b->parent();
    }/*wend*/
    // FIXME: replace with isRootInlineBox after full WebCore merge.
    Q_ASSERT(b->isRootInlineBox());
    cb = static_cast<RenderFlow *>(b->object());
    Q_ASSERT(cb->isRenderBlock());
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "=================== end findFlowBox (renderText)" << endl;
#endif
    return static_cast<InlineFlowBox *>(b);
  } while(false);/*end if*/

  cb = r->containingBlock();
  if ( !cb ) return 0L;

  if (!cb->isRenderBlock()) {
    cb = generateDummyBlock(arena, cb);
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "dummy block created: " << cb << endl;
#endif
  }/*end if*/

  InlineFlowBox *flowBox = cb->firstLineBox();
  // This case strikes when there are children but none of it is represented
  // by an inline box (for example, all of them are empty:
  // <div><b></b><i></i></div>)
  if (!flowBox) {
    flowBox = generateDummyFlowBox(arena, cb, r);
    if (ibox) *ibox = flowBox->firstChild();
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "=================== end findFlowBox (2)" << endl;
#endif
    return flowBox;
  }/*end if*/

  // We iterate the inline flow boxes of the containing block until
  // we find the given node. This has one major flaw: it is linear, and therefore
  // painfully slow for really large blocks.
  for (; flowBox; flowBox = static_cast<InlineFlowBox *>(flowBox->nextLineBox())) {
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "[scan line]" << endl;
#endif

    // Iterate children, and look for node
    InlineBox *box;
    InlineBoxIterator it(arena, flowBox);
    for (; (box = *it) != 0; ++it) {
      RenderObject *br = box->object();
      if (!br) continue;

#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "box->obj " << br->renderName() << "[" << br << "]" << " minOffset: " << box->minOffset() << " maxOffset: " << box->maxOffset() << endl;
#endif
      if (br == r && offset >= box->minOffset() && offset <= box->maxOffset())
        break;	// If Dijkstra hadn't brainwashed me, I'd have used a goto here
    }/*next it*/
    if (box) {
      if (ibox) *ibox = box;
      break;
    }

  }/*next flowBox*/

  // no inline flow box found, approximate to nearest following node.
  // Danger: this is O(n^2). It's only called to recover from
  // errors, that means, theoretically, never. (Practically, far too often :-( )
  if (!flowBox) flowBox = findFlowBox(node->nextLeafNode(), 0, arena, cb, ibox);

#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "=================== end findFlowBox" << endl;
#endif
  return flowBox;
}

/** finds the innermost table object @p r is contained within, but no
 * farther than @p cb.
 * @param r leaf element to begin with
 * @param cb bottom element where to stop search at least.
 * @return the table object or 0 if none found.
 */
static inline RenderTable *findTableUpTo(RenderObject *r, RenderFlow *cb)
{
  while (r && r != cb && !r->isTable()) r = r->parent();
  return r && r->isTable() ? static_cast<RenderTable *>(r) : 0;
}

/** checks whether @p r is a descendant of @p cb.
 */
static inline bool isDescendant(RenderObject *r, RenderObject *cb)
{
  while (r && r != cb) r = r->parent();
  return r;
}

/** checks whether the given block contains at least one editable element.
 *
 * Warning: This function has linear complexity, and therefore is expensive.
 * Use it sparingly, and cache the result.
 * @param part part
 * @param cb block to be searched
 * @param table returns the nested table if there is one directly at the beginning
 *	or at the end.
 * @param fromEnd begin search from end (default: begin from beginning)
 */
static bool containsEditableElement(KHTMLPart *part, RenderFlow *cb,
	RenderTable *&table, bool fromEnd = false)
{
  RenderObject *r = cb;
  if (fromEnd)
    while (r->lastChild()) r = r->lastChild();
  else
    while (r->firstChild()) r = r->firstChild();

  RenderTable *tempTable = 0;
  table = 0;
  bool withinCb;
  do {
    tempTable = findTableUpTo(r, cb);
    withinCb = isDescendant(r, cb);

#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "r " << (r ? r->renderName() : QString::null) << "@" << r << endl;
#endif
    if (r && withinCb && r->element() && !r->isTableCol()
    	&& (part->isCaretMode() || part->isEditable()
		|| r->style()->userInput() == UI_ENABLED)) {
      table = tempTable;
      return true;
    }/*end if*/

    r = fromEnd ? prevSuitableLeafRenderObject(r) : nextSuitableLeafRenderObject(r);
  } while (r && withinCb);
  return false;
}

/** checks whether the given block contains at least one editable child
 * element, beginning with but excluding @p start.
 *
 * Warning: This function has linear complexity, and therefore is expensive.
 * Use it sparingly, and cache the result.
 * @param part part
 * @param cb block to be searched
 * @param table returns the nested table if there is one directly before/after
 *	the start object.
 * @param fromEnd begin search from end (default: begin from beginning)
 * @param start object after which to begin search.
 */
static bool containsEditableChildElement(KHTMLPart *part, RenderFlow *cb,
	RenderTable *&table, bool fromEnd, RenderObject *start)
{
  RenderObject *r = start;
  if (fromEnd)
    while (r->firstChild()) r = r->firstChild();
  else
    while (r->lastChild()) r = r->lastChild();

  if (!r) return false;

  RenderTable *tempTable = 0;
  table = 0;
  bool withinCb = false;
  do {
    r = fromEnd ? prevSuitableLeafRenderObject(r) : nextSuitableLeafRenderObject(r);
    if (!r) break;

    withinCb = isDescendant(r, cb) && r != cb;
    tempTable = findTableUpTo(r, cb);

#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "r " << (r ? r->renderName() : QString::null) << "@" << r << endl;
#endif
    if (r && withinCb && r->element() && !r->isTableCol()
    	&& (part->isCaretMode() || part->isEditable()
		|| r->style()->userInput() == UI_ENABLED)) {
      table = tempTable;
      return true;
    }/*end if*/

  } while (withinCb);
  return false;
}

// == class LinearDocument implementation

LinearDocument::LinearDocument(KHTMLPart *part, NodeImpl *node, long offset)
	: arena(0), node(node), offset(offset), m_part(part)
{
  if (node == 0) return;
  sanitizeCaretState(this->node, this->offset);

  arena = new RenderArena(512);

  initPreBeginIterator();
  initEndIterator();
  //m_part = node->getDocument()->view()->part();
}

LinearDocument::~LinearDocument()
{
  delete arena;
}

int LinearDocument::count() const
{
  // FIXME: not implemented
  return 1;
}

LinearDocument::Iterator LinearDocument::current()
{
  return LineIterator(this, node, offset);
}

LinearDocument::Iterator LinearDocument::begin()
{
  DocumentImpl *doc = node ? node->getDocument() : 0;
  if (!doc) return end();

  NodeImpl *firstLeaf = doc->nextLeafNode();
  if (!firstLeaf) return end();		// must be empty document (is this possible?)
  return LineIterator(this, firstLeaf, firstLeaf->minOffset());
}

LinearDocument::Iterator LinearDocument::preEnd()
{
  DocumentImpl *doc = node ? node->getDocument() : 0;
  if (!doc) return preBegin();

  NodeImpl *lastLeaf = doc;
  while (lastLeaf->lastChild()) lastLeaf = lastLeaf->lastChild();

  if (!lastLeaf) return preBegin();	// must be empty document (is this possible?)
  return LineIterator(this, lastLeaf, lastLeaf->maxOffset());
}

void LinearDocument::initPreBeginIterator()
{
  _preBegin = LineIterator(this, 0, 0);
}

void LinearDocument::initEndIterator()
{
  _end = LineIterator(this, 0, 1);
}

// == class LineIterator implementation

LineIterator::LineIterator(LinearDocument *l, DOM::NodeImpl *node, long offset)
		: lines(l)
{
//  kdDebug(6200) << "LineIterator: node " << node << " offset " << offset << endl;
  flowBox = findFlowBox(node, offset, lines->arena, cb, &currentBox);
  if (!flowBox) {
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "LineIterator: findFlowBox failed" << endl;
#endif
    cb = 0;
  }/*end if*/
}

void LineIterator::nextBlock()
{
  RenderObject *r = cb;
  RenderObject *n = r->lastChild();
  while (n) r = n, n = r->lastChild();
  r = nextSuitableLeafRenderObject(r);
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "++: r " << r << "[" << (r?r->renderName():QString::null) << "]" << endl;
#endif
  if (!r) {
    cb = 0;
    return;
  }/*end if*/

  // If we hit a leaf block (which can happen on empty blocks), use this
  // as its containing block
  if (r->isRenderBlock()) {
    cb = static_cast<RenderBlock *>(r);
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "r->isFlow is cb. continuation @" << cb->continuation() << endl;
#endif
    // Disregard empty continuations, they get the caret stuck otherwise.
    // This is because both cont_a and cont_o point to the same
    // DOM element. When the caret should move to cont_o, findFlowBox finds
    // cont_a, and the caret will be placed there.
    RenderFlow *flow = static_cast<RenderFlow *>(cb->element()
    			? cb->element()->renderer() : 0);
    if (cb->continuation() || flow && flow->isRenderBlock() && flow != cb
    		&& flow->continuation()) {
      nextBlock();
      return;
    }/*end if*/
  } else {
    cb = static_cast<RenderFlow *>(r->containingBlock());
    if (!cb->isRenderBlock()) {
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "dummy cb created " << cb << endl;
#endif
      cb = generateDummyBlock(lines->arena, r);
    }/*end if*/
  }/*end if*/
  flowBox = cb->firstLineBox();
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "++: flowBox " << flowBox << endl;
#endif

  if (!flowBox) flowBox = generateDummyFlowBox(lines->arena, cb, r);
#if DEBUG_CARETMODE > 0
  if (!cb->firstLineBox()) kdDebug(6200) << "++: dummy flowBox " << flowBox << endl;
#endif
}

inline LineIterator &LineIterator::operator ++()
{
  flowBox = static_cast<InlineFlowBox *>(flowBox->nextLineBox());

  // if there are no more lines in this block, begin with first line of
  // next block
  if (!flowBox) nextBlock();

  return *this;
}

inline LineIterator LineIterator::operator ++(int)
{
  LineIterator it(*this);
  operator ++();
  return it;
}

void LineIterator::prevBlock()
{
  RenderObject *r = cb;
  RenderObject *n = r->firstChild();
  while (n) r = n, n = r->firstChild();
  r = prevSuitableLeafRenderObject(r);
  if (!r) {
    cb = 0;
    return;
  }/*end if*/

  // If we hit a leaf block (which can happen on empty blocks), use this
  // as its containing block
  if (r->isRenderBlock()) {
    cb = static_cast<RenderFlow *>(r);
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "r->isFlow is cb. continuation @" << cb->continuation() << endl;
#endif
    // Disregard empty continuations, they get the caret stuck otherwise.
    // This is because both cont_a and cont_o point to the same
    // DOM element. When the caret should move to cont_o, findFlowBox finds
    // cont_a, and the caret will be placed there.
    RenderFlow *flow = static_cast<RenderFlow *>(cb->element()
    			? cb->element()->renderer() : 0);
    if (cb->continuation() || flow && flow->isRenderBlock() && flow != cb
    		&& flow->continuation()) {
      prevBlock();
      return;
    }/*end if*/
  } else {
    cb = static_cast<RenderFlow *>(r->containingBlock());
    if (!cb->isRenderBlock()) {
#if DEBUG_CARETMODE > 0
      kdDebug(6200) << "dummy cb created " << cb << endl;
#endif
      cb = generateDummyBlock(lines->arena, r);
    }/*end if*/
  }/*end if*/
  flowBox = cb->lastLineBox();

  if (!flowBox) flowBox = generateDummyFlowBox(lines->arena, cb, r);
}

inline LineIterator &LineIterator::operator --()
{
  flowBox = static_cast<InlineFlowBox *>(flowBox->prevLineBox());

  // if there are no more lines in this block, begin with last line of
  // previous block
  if (!flowBox) prevBlock();

  return *this;
}

inline LineIterator LineIterator::operator --(int)
{
  LineIterator it(*this);
  operator --();
  return it;
}

#if 0 // not implemented because it's not needed
LineIterator LineIterator::operator +(int /*summand*/) const
{
  // FIXME: not implemented
  return LineIterator();
}

LineIterator LineIterator::operator -(int /*summand*/) const
{
  // FIXME: not implemented
  return LineIterator();
}
#endif

LineIterator &LineIterator::operator +=(int summand)
{
  if (summand > 0)
    while (summand-- && *this != lines->end()) ++*this;
  else if (summand < 0)
    operator -=(-summand);
  return *this;
}

LineIterator &LineIterator::operator -=(int summand)
{
  if (summand > 0)
    while (summand-- && *this != lines->preBegin()) --*this;
  else if (summand < 0)
    operator +=(-summand);
  return *this;
}

// == class EditableCharacterIterator implementation

void EditableCharacterIterator::initFirstChar()
{
  InlineBox *b = *ebit;
  if (b) {
    if (_offset == b->maxOffset())
      peekNext();
    else if (b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->str->s[_offset].unicode();
    else
      _char = -1;
  }/*end if*/
}

EditableCharacterIterator &EditableCharacterIterator::operator ++()
{
  _offset++;

  InlineBox *b = *ebit;
  RenderObject *r = b->object();
  // BRs have no extent, so their maximum offset must be their minimum.
  // A block element can only be the target if it is empty -- in this case
  // its extent is zero, too.
  long maxofs = r->isBR() || r->isRenderBlock() ? b->minOffset() : b->maxOffset();
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "b->maxOffset() " << b->maxOffset() << " b->minOffset() " << b->minOffset() << endl;
#endif
  if (_offset == maxofs) {
#if DEBUG_CARETMODE > 2
kdDebug(6200) << "_offset == maxofs: " << _offset << " == " << maxofs << endl;
#endif
//    _peekPrev = b;
    peekNext();
  } else if (_offset > maxofs) {
#if DEBUG_CARETMODE > 2
kdDebug(6200) << "_offset > maxofs: " << _offset << " > " << maxofs /*<< " _peekNext: " << _peekNext*/ << endl;
#endif
    if (/*!_peekNext*/true) {
      if (*ebit)
        ++ebit;
      if (!*ebit) {		// end of line reached, go to next line
        ++_it;
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "++_it" << endl;
#endif
        if (_it != ld->end()) {
	  ebit = _it;
          b = *ebit;
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "b " << b << " isText " << b->isInlineTextBox() << endl;
#endif
	  _node = b->object()->element();
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "_node " << _node << ":" << _node->nodeName().string() << endl;
#endif
	  _offset = b->minOffset();
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "_offset " << _offset << endl;
#endif
	} else {
	  _node = 0;
	  b = 0;
	}/*end if*/
        goto readchar;
      }/*end if*/
    }/*end if*/
    bool adjacent = ebit.isAdjacent();
    // Jump over element if this one is not a text node.
    if (adjacent && !(*ebit)->isInlineTextBox()) {
      EditableInlineBoxIterator copy = ebit;
      ++ebit;
      if (*ebit && (*ebit)->isInlineTextBox()) adjacent = false;
      else ebit = copy;
    }/*end if*/
    _node = (*ebit)->object()->element();
    _offset = (*ebit)->minOffset() + adjacent;
    //_peekNext = 0;
    b = *ebit;
    goto readchar;
  } else {
readchar:
    // get character
    if (b && b->isInlineTextBox() && _offset < b->maxOffset())
      _char = static_cast<RenderText *>(b->object())->str->s[_offset].unicode();
    else
      _char = -1;
  }/*end if*/
#if DEBUG_CARETMODE > 2
kdDebug(6200) << "_offset: " << _offset /*<< " _peekNext: " << _peekNext*/ << " char '" << (char)_char << "'" << endl;
#endif

#if DEBUG_CARETMODE > 0
  if (*ebit) {
    InlineBox *box = *ebit;
    kdDebug(6200) << "echit++(1): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << " node " << (_node ? _node->nodeName().string() : QString("<nil>")) << endl;
  }
#endif
  return *this;
}

EditableCharacterIterator &EditableCharacterIterator::operator --()
{
  _offset--;
  //kdDebug(6200) << "--: _offset=" << _offset << endl;

  InlineBox *b = *ebit;
  InlineBox *_peekPrev = 0;
  InlineBox *_peekNext = 0;
  long minofs = b ? b->minOffset() : _offset + 1;
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "b->maxOffset() " << b->maxOffset() << " b->minOffset() " << b->minOffset() << endl;
#endif
  if (_offset == minofs) {
#if DEBUG_CARETMODE > 2
kdDebug(6200) << "_offset == minofs: " << _offset << " == " << minofs << endl;
#endif
    _peekNext = b;
    // get character
    if (b && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->text()[_offset].unicode();
    else
      _char = -1;

    //peekPrev();
    bool do_prev = false;
    {
      EditableInlineBoxIterator copy = ebit;
      --ebit;
      _peekPrev = *ebit;
      // Jump to end of previous element if it's adjacent, and a text box
      if (ebit.isAdjacent() && *ebit && (*ebit)->isInlineTextBox())
        //operator --();
	do_prev = true;
      else
        ebit = copy;
    }
    if (do_prev) goto prev;
  } else if (_offset < minofs) {
prev:
#if DEBUG_CARETMODE > 2
kdDebug(6200) << "_offset < minofs: " << _offset << " < " << minofs /*<< " _peekNext: " << _peekNext*/ << endl;
#endif
    if (!_peekPrev) {
      _peekNext = *ebit;
      if (*ebit)
        --ebit;
      if (!*ebit) {		// end of line reached, go to previous line
        --_it;
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "--_it" << endl;
#endif
        if (_it != ld->preBegin()) {
//	  kdDebug(6200) << "begin from end!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	  ebit = EditableInlineBoxIterator(_it, true);
	  RenderObject *r = (*ebit)->object();
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "b " << *ebit << " isText " << (*ebit)->isInlineTextBox() << endl;
#endif
	  _node = r->element();
	  _offset = r->isBR() ? (*ebit)->minOffset() : (*ebit)->maxOffset();
	  _char = -1;
#if DEBUG_CARETMODE > 0
          {InlineBox *box = *ebit; kdDebug(6200) << "echit--(2): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;}
#endif
	} else
	  _node = 0;
	return *this;
      }/*end if*/
    }/*end if*/

    bool adjacent = ebit.isAdjacent();
    // Ignore this box if it isn't a text box, but the previous box was
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "adjacent " << adjacent << " _peekNext " << _peekNext << " _peekNext->isInlineTextBox: " << (_peekNext ? _peekNext->isInlineTextBox() : false) << " !((*ebit)->isInlineTextBox): " << (*ebit ? !(*ebit)->isInlineTextBox() : true) << endl;
#endif
    if (adjacent && _peekNext && _peekNext->isInlineTextBox()
    	&& !(*ebit)->isInlineTextBox()) {
      EditableInlineBoxIterator copy = ebit;
      --ebit;
      if (!*ebit) /*adjacent = false;
      else */ebit = copy;
    }/*end if*/
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << "(*ebit)->obj " << (*ebit)->object()->renderName() << "[" << (*ebit)->object() << "]" << " minOffset: " << (*ebit)->minOffset() << " maxOffset: " << (*ebit)->maxOffset() << endl;
#endif
    _node = (*ebit)->object()->element();
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "_node " << _node << ":" << _node->nodeName().string() << endl;
#endif
    _offset = (*ebit)->maxOffset()/* - adjacent*/;
#if DEBUG_CARETMODE > 3
kdDebug(6200) << "_offset " << _offset << endl;
#endif
    _peekPrev = 0;
  } else {
#if DEBUG_CARETMODE > 0
kdDebug(6200) << "_offset: " << _offset << " _peekNext: " << _peekNext << endl;
#endif
    // get character
    if (_peekNext && _offset >= b->maxOffset() && _peekNext->isInlineTextBox())
      _char = static_cast<RenderText *>(_peekNext->object())->text()[_peekNext->minOffset()].unicode();
    else if (b && _offset < b->maxOffset() && b->isInlineTextBox())
      _char = static_cast<RenderText *>(b->object())->text()[_offset].unicode();
    else
      _char = -1;
  }/*end if*/

#if DEBUG_CARETMODE > 0
  if (*ebit) {
    InlineBox *box = *ebit;
    kdDebug(6200) << "echit--(1): box " << box << (box && box->isInlineTextBox() ? QString(" contains \"%1\"").arg(QConstString(static_cast<RenderText *>(box->object())->str->s+box->minOffset(), box->maxOffset() - box->minOffset()).string()) : QString::null) << endl;
  }
#endif
  return *this;
}

// == class TableRowIterator implementation

TableRowIterator::TableRowIterator(RenderTable *table, bool fromEnd,
  		RenderTableSection::RowStruct *row)
		: sec(table, fromEnd)
{
  // set index
  if (*sec) {
    if (fromEnd) index = (*sec)->grid.size() - 1;
    else index = 0;
  }/*end if*/

  // initialize with given row
  if (row && *sec) {
    while (operator *() != row)
      if (fromEnd) operator --(); else operator ++();
  }/*end if*/
}

TableRowIterator &TableRowIterator::operator ++()
{
  index++;

  if (index >= (int)(*sec)->grid.size()) {
    ++sec;

    if (*sec) index = 0;
  }/*end if*/
  return *this;
}

TableRowIterator &TableRowIterator::operator --()
{
  index--;

  if (index < 0) {
    --sec;

    if (*sec) index = (*sec)->grid.size() - 1;
  }/*end if*/
  return *this;
}

// == class ErgonomicEditableLineIterator implementation

// some decls
static RenderTableCell *findNearestTableCellInRow(KHTMLPart *part, int x,
		RenderTableSection::RowStruct *row, bool fromEnd);

/** finds the cell corresponding to absolute x-coordinate @p x in the given
 * table.
 *
 * If there is no direct cell, or the cell is not accessible, the function
 * will return the nearest suitable cell.
 * @param part part containing the document
 * @param x absolute x-coordinate
 * @param it table row iterator, will be adapted accordingly as more rows are
 *	investigated.
 * @param fromEnd @p true to begin search from end and work towards the
 *	beginning
 * @return the cell, or 0 if no editable cell was found.
 */
static inline RenderTableCell *findNearestTableCell(KHTMLPart *part, int x,
		TableRowIterator &it, bool fromEnd)
{
  RenderTableCell *result = 0;

  while (*it) {
    result = findNearestTableCellInRow(part, x, *it, fromEnd);
    if (result) break;

    if (fromEnd) --it; else ++it;
  }/*wend*/

  return result;
}

/** finds the nearest editable cell around the given absolute x-coordinate
 *
 * It will dive into nested tables as necessary to provide seamless navigation.
 *
 * If the cell at @p x is not editable, its left neighbor is tried, then its
 * right neighbor, then the left neighbor's left neighbor etc. If no
 * editable cell can be found, 0 is returned.
 * @param part khtml part
 * @param x absolute x-coordinate
 * @param row table row to be searched
 * @param fromEnd @p true, begin from end (applies only to nested tables)
 * @return the found cell or 0 if no editable cell was found
 */
static RenderTableCell *findNearestTableCellInRow(KHTMLPart *part, int x,
		RenderTableSection::RowStruct *row, bool fromEnd)
{
  // First pass. Find spatially nearest cell.
  int n = (int)row->row->size();
  int i;
  for (i = 0; i < n; i++) {
    RenderTableCell *cell = row->row->at(i);
    if (!cell || (int)cell == -1) continue;

    int absx, absy;
    cell->absolutePosition(absx, absy, false); // ### position: fixed?
#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "i/n " << i << "/" << n << " absx " << absx << " absy " << absy << endl;
#endif

    // I rely on the assumption that all cells are in ascending visual order
    // ### maybe this assumption is wrong for bidi?
#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "x " << x << " < " << (absx + cell->width()) << "?" << endl;
#endif
    if (x < absx + cell->width()) break;
  }/*next i*/
  if (i >= n) i = n - 1;

  // Second pass. Find editable cell, beginning with the currently found,
  // extending to the left, and to the right, alternating.
  for (int cnt = 0; cnt < 2*n; cnt++) {
    int index = i - ((cnt >> 1) + 1)*(cnt & 1) + (cnt >> 1)*!(cnt & 1);
    if (index < 0 || index >= n) continue;

    RenderTableCell *cell = row->row->at(index);
    if (!cell || (int)cell == -1) continue;

#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "index " << index << " cell " << cell << endl;
#endif
    RenderTable *nestedTable;
    if (containsEditableElement(part, cell, nestedTable, fromEnd)) {

      if (nestedTable) {
        TableRowIterator it(nestedTable, fromEnd);
	while (*it) {
	  cell = findNearestTableCell(part, x, it, fromEnd);
	  if (cell) break;
	  if (fromEnd) --it; else ++it;
	}/*wend*/
      }/*end if*/

      return cell;
    }/*end if*/
  }/*next i*/
  return 0;
}

/** returns the nearest common ancestor of two objects that is a table cell,
 * a table section, or 0 if not inside a common table.
 *
 * If @p r1 and @p r2 belong to the same table, but different sections, @p r1's
 * section is returned.
 */
static RenderObject *commonAncestorTableSectionOrCell(RenderObject *r1,
		RenderObject *r2)
{
  if (!r1 || !r2) return 0;
  RenderTableSection *sec = 0;
  int start_depth=0, end_depth=0;
  // First we find the depths of the two objects in the tree (start_depth, end_depth)
  RenderObject *n = r1;
  while (n->parent()) {
    n = n->parent();
    start_depth++;
  }/*wend*/
  n = r2;
  while( n->parent()) {
    n = n->parent();
    end_depth++;
  }/*wend*/
  // here we climb up the tree with the deeper object, until both objects have equal depth
  while (end_depth > start_depth) {
    r2 = r2->parent();
    end_depth--;
  }/*wend*/
  while (start_depth > end_depth) {
    r1 = r1->parent();
//    if (r1->isTableSection()) sec = static_cast<RenderTableSection *>(r1);
    start_depth--;
  }/*wend*/
  // Climb the tree with both r1 and r2 until they are the same
  while (r1 != r2){
    r1 = r1->parent();
    if (r1->isTableSection()) sec = static_cast<RenderTableSection *>(r1);
    r2 = r2->parent();
  }/*wend*/

  // At this point, we found the most approximate common ancestor. Now climb
  // up until the condition of the function return value is satisfied.
  while (r1 && !r1->isTableCell() && !r1->isTableSection() && !r1->isTable())
    r1 = r1->parent();

  return r1 && r1->isTable() ? sec : r1;
}

/** Finds the row that contains the given cell, directly, or indirectly
 * @param section section to be searched
 * @param cell table cell
 * @param row returns the row
 * @param directCell returns the direct cell that contains @p cell
 * @return the index of the row.
 */
static int findRowInSection(RenderTableSection *section, RenderTableCell *cell,
		RenderTableSection::RowStruct *&row, RenderTableCell *&directCell)
{
  // Seek direct cell
  RenderObject *r = cell;
  while (r != section) {
    if (r->isTableCell()) directCell = static_cast<RenderTableCell *>(r);
    r = r->parent();
  }/*wend*/

  // So, and this is really nasty: As we have no indices, we have to do a
  // linear comparison. Oh, that sucks so much for long tables, you can't
  // imagine.
  int n = section->numRows();
  for (int i = 0; i < n; i++) {
    row = &section->grid[i];

    // check for cell
    int m = row->row->size();
    for (int j = 0; j < m; j++) {
      RenderTableCell *c = row->row->at(j);
      if (c == directCell) return i;
    }/*next j*/

  }/*next i*/
  Q_ASSERT(false);
  return -1;
}

/** finds the table that is the first direct or indirect descendant of @p block.
 * @param leaf object to begin search from.
 * @param block object to search to, or 0 to search up to top.
 * @return the table or 0 if there were none.
 */
static inline RenderTable *findFirstDescendantTable(RenderObject *leaf, RenderFlow *block)
{
  RenderTable *result = 0;
  while (leaf && leaf != block) {
    if (leaf->isTable()) result = static_cast<RenderTable *>(leaf);
    leaf = leaf->parent();
  }/*wend*/
  return result;
}

/** looks for the table cell the given object @p r is contained within.
 * @return the table cell or 0 if not contained in any table.
 */
static inline RenderTableCell *containingTableCell(RenderObject *r)
{
  while (r && !r->isTableCell()) r = r->parent();
  return static_cast<RenderTableCell *>(r);
}

inline void ErgonomicEditableLineIterator::calcAndStoreNewLine(
			RenderFlow *newBlock, bool toBegin)
{
  // take the first/last editable element in the found cell as the new
  // value for the iterator
  cb = newBlock;
  if (toBegin) prevBlock(); else nextBlock();

  if (!cb) {
    flowBox = 0;
    return;
  }/*end if*/

  if (!isEditable(*this)) {
    if (toBegin) EditableLineIterator::operator --();
    else EditableLineIterator::operator ++();
  }/*end if*/
}

void ErgonomicEditableLineIterator::determineTopologicalElement(
		RenderTableCell *oldCell, RenderObject *newObject, bool toBegin)
{
  // When we arrive here, a transition between cells has happened.
  // Now determine the type of the transition. This can be
  // (1) a transition from this cell into a table inside this cell.
  // (2) a transition from this cell into another cell of this table

  TableRowIterator it;

  RenderObject *commonAncestor = commonAncestorTableSectionOrCell(oldCell, newObject);
#if DEBUG_CARETMODE > 1
  kdDebug(6201) << " ancestor " << commonAncestor << endl;
#endif

  // The whole document is treated as a table cell.
  if (!commonAncestor || commonAncestor->isTableCell()) {	// (1)

    RenderTableCell *cell = static_cast<RenderTableCell *>(commonAncestor);
    RenderTable *table = findFirstDescendantTable(newObject, cell);

#if DEBUG_CARETMODE > 0
    kdDebug(6201) << "table cell: " << cell << endl;
#endif

    // if there is no table, we fell out of the previous table, and are now
    // in some table-less block. Therefore, done.
    if (!table) return;

    it = TableRowIterator(table, toBegin);

  } else if (commonAncestor->isTableSection()) {		// (2)

    RenderTableSection *section = static_cast<RenderTableSection *>(commonAncestor);
    RenderTableSection::RowStruct *row;
    int idx = findRowInSection(section, oldCell, row, oldCell);
#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "table section: row idx " << idx << endl;
#endif

    it = TableRowIterator(section, idx);

    // advance rowspan rows
    int rowspan = oldCell->rowSpan();
    while (*it && rowspan--) {
      if (toBegin) --it; else ++it;
    }/*wend*/

  } else {
    kdError(6201) << "Neither common cell nor section! " << commonAncestor->renderName() << endl;
    // will crash on uninitialized table row iterator
  }/*end if*/

  RenderTableCell *cell = findNearestTableCell(lines->m_part, xCoor, it, toBegin);
#if DEBUG_CARETMODE > 1
  kdDebug(6201) << "findNearestTableCell result: " << cell << endl;
#endif

  RenderFlow *newBlock = cell;
  if (!cell) {
    Q_ASSERT(commonAncestor->isTableSection());
    RenderTableSection *section = static_cast<RenderTableSection *>(commonAncestor);
    cell = containingTableCell(section);
#if DEBUG_CARETMODE > 1
    kdDebug(6201) << "containing cell: " << cell << endl;
#endif

    RenderTable *nestedTable;
    bool editableChild = cell && containsEditableChildElement(lines->m_part,
    		cell, nestedTable, toBegin, section->table());

    if (cell && !editableChild) {
#if DEBUG_CARETMODE > 1
      kdDebug(6201) << "========= recursive invocation outer =========" << endl;
#endif
      determineTopologicalElement(cell, cell->section(), toBegin);
#if DEBUG_CARETMODE > 1
      kdDebug(6201) << "========= end recursive invocation outer =========" << endl;
#endif
      return;

    } else if (cell && nestedTable) {
#if DEBUG_CARETMODE > 1
      kdDebug(6201) << "========= recursive invocation inner =========" << endl;
#endif
      determineTopologicalElement(cell, nestedTable, toBegin);
#if DEBUG_CARETMODE > 1
      kdDebug(6201) << "========= end recursive invocation inner =========" << endl;
#endif
      return;

    } else {
#if DEBUG_CARETMODE > 1
      kdDebug(6201) << "newBlock is table: " << section->table() << endl;
#endif
      newBlock = section->table();
//      if (toBegin) prevBlock(); else nextBlock();
    }/*end if*/
  } else {
    // adapt cell so that prevBlock/nextBlock works as expected
    RenderObject *r = cell;
    if (toBegin) {
      while (r->lastChild()) r = r->lastChild();
      r = nextSuitableLeafRenderObject(r);
    } else
      r = prevSuitableLeafRenderObject(r);
    newBlock = static_cast<RenderFlow *>(!r || r->isRenderBlock() ? r : r->containingBlock());
  }/*end if*/

  calcAndStoreNewLine(newBlock, toBegin);
}

ErgonomicEditableLineIterator &ErgonomicEditableLineIterator::operator ++()
{
  RenderTableCell *oldCell = containingTableCell(cb);

  EditableLineIterator::operator ++();
  if (*this == lines->end() || *this == lines->preBegin()) return *this;

  RenderTableCell *newCell = containingTableCell(cb);

  if (!newCell || newCell == oldCell) return *this;

  determineTopologicalElement(oldCell, newCell, false);

  return *this;
}

ErgonomicEditableLineIterator &ErgonomicEditableLineIterator::operator --()
{
  RenderTableCell *oldCell = containingTableCell(cb);

  EditableLineIterator::operator --();
  if (*this == lines->end() || *this == lines->preBegin()) return *this;

  RenderTableCell *newCell = containingTableCell(cb);

  if (!newCell || newCell == oldCell) return *this;

  determineTopologicalElement(oldCell, newCell, true);

  return *this;
}

// == Navigational helper functions ==

/** seeks the inline box which contains or is the nearest to @p x
 * @param it line iterator pointing to line to be searched
 * @param cv caret view context
 * @param x returns the cv->origX approximation, relatively positioned to the
 *	containing block.
 * @param absx returns absolute x-coordinate of containing block
 * @param absy returns absolute y-coordinate of containing block
 * @return the most suitable inline box
 */
static InlineBox *nearestInlineBox(LineIterator &it, CaretViewContext *cv,
	int &x, int &absx, int &absy)
{
  InlineFlowBox *fbox = *it;

  // Find containing block
  RenderObject *cb = fbox->object();

  if (cb) cb->absolutePosition(absx, absy);
  else absx = absy = 0;

  // Otherwise find out in which inline box the caret is to be placed.

  // this horizontal position is to be approximated
  x = cv->origX - absx;
  InlineBox *caretBox = 0; // Inline box containing the caret
//  NodeImpl *lastnode = 0;  // node of previously checked render object.
  int xPos;		   // x-coordinate of current inline box
  int oldXPos = -1;	   // x-coordinate of last inline box
  EditableInlineBoxIterator fbit = it;
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "*fbit = " << *fbit << endl;
#endif
  // Either iterate through all children or take the flow box itself as a
  // child if it has no children
  for (InlineBox *b; *fbit != 0; ++fbit) {
    b = *fbit;

//    RenderObject *r = b->object();
#if DEBUG_CARETMODE > 0
	if (b->isInlineFlowBox()) kdDebug(6200) << "b is inline flow box" << endl;
//	kdDebug(6200) << "approximate r" << r << ": " << (r ? r->renderName() : QString::null) << (r && r->isText() ? " contains \"" + QString(((RenderText *)r)->str->s, ((RenderText *)r)->str->l) + "\"" : QString::null) << endl;
#endif
//    NodeImpl *node = r->element();
    xPos = b->xPos();

    // the caret is before this box
    if (x < xPos) {
      // snap to nearest box
      if (oldXPos < 0 || x - (oldXPos + caretBox->width()) > xPos - x) {
	caretBox = b;	// current box is nearer
//        lastnode = node;
      }/*end if*/
      break;		// Otherwise, preceding box is implicitly used
    }

    caretBox = b;
//    lastnode = node;

    // the caret is within this box
    if (x >= xPos && x < xPos + caretBox->width())
      break;
    oldXPos = xPos;

    // the caret can only be after the last box which is automatically
    // contained in caretBox when we fall out of the loop.

    if (b == fbox) break;
  }/*next fbit*/

  return caretBox;
}

/** moves the given iterator to the beginning of the next word.
 *
 * If the end is reached, the iterator will be positioned there.
 * @param it character iterator to be moved
 */
static void moveItToNextWord(EditableCharacterIterator &it)
{
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "%%%%%%%%%%%%%%%%%%%%% moveItToNextWord" << endl;
#endif
  EditableCharacterIterator copy;
  while (it.node() && !(*it).isSpace() && !(*it).isPunct()) {
#if DEBUG_CARETMODE > 2
    kdDebug(6200) << "reading1 '" << (*it).latin1() << "'" << endl;
#endif
    copy = it;
    ++it;
  }

  if (!it.node()) {
    it = copy;
    return;
  }/*end if*/

  while (it.node() && ((*it).isSpace() || (*it).isPunct())) {
#if DEBUG_CARETMODE > 2
    kdDebug(6200) << "reading2 '" << (*it).latin1() << "'" << endl;
#endif
    copy = it;
    ++it;
  }

  if (!it.node()) it = copy;
}

/** moves the given iterator to the beginning of the previous word.
 *
 * If the beginning is reached, the iterator will be positioned there.
 * @param it character iterator to be moved
 */
static void moveItToPrevWord(EditableCharacterIterator &it)
{
  if (!it.node()) return;

#if DEBUG_CARETMODE > 0
  kdDebug(6200) << "%%%%%%%%%%%%%%%%%%%%% moveItToPrevWord" << endl;
#endif
  EditableCharacterIterator copy;

  // Jump over all space and punctuation characters first
  do {
    copy = it;
    --it;
#if DEBUG_CARETMODE > 2
    if (it.node()) kdDebug(6200) << "reading1 '" << (*it).latin1() << "'" << endl;
#endif
  } while (it.node() && ((*it).isSpace() || (*it).isPunct()));

  if (!it.node()) {
    it = copy;
    return;
  }/*end if*/

  do {
    copy = it;
    --it;
#if DEBUG_CARETMODE > 0
    if (it.node()) kdDebug(6200) << "reading2 '" << (*it).latin1() << "'" << endl;
#endif
  } while (it.node() && !(*it).isSpace() && !(*it).isPunct());

  it = copy;
}


/** moves the iterator by one page.
 * @param ld linear document
 * @param it line iterator, will be updated accordingly
 * @param mindist minimum distance in pixel the iterator should be moved
 *	(if possible)
 * @param next @p true, move downward, @p false move upward
 */
static void moveIteratorByPage(LinearDocument &ld,
		ErgonomicEditableLineIterator &it, int mindist, bool next)
{
  if (it == ld.end() || it == ld.preBegin()) return;

  ErgonomicEditableLineIterator copy = it;
#if DEBUG_CARETMODE > 0
  kdDebug(6200) << " mindist: " << mindist << endl;
#endif

  InlineFlowBox *flowBox = *copy;
  int absx = 0, absy = 0;

  RenderFlow *lastcb = static_cast<RenderFlow *>(flowBox->object());
  Q_ASSERT(lastcb->isRenderBlock());
  lastcb->absolutePosition(absx, absy, false);	// ### what about fixed?

  // ### actually flowBox->yPos() should suffice, but this is not ported
  // over yet from WebCore
  int lastfby = flowBox->firstChild()->yPos();
  int lastheight = 0;
  do {
    if (next) ++copy; else --copy;
    if (copy == ld.end() || copy == ld.preBegin()) break;

    // ### change to RootInlineBox after full WebCore merge
    flowBox = static_cast<InlineFlowBox *>(*copy);
    Q_ASSERT(flowBox->isInlineFlowBox());

    RenderFlow *cb = static_cast<RenderFlow *>(flowBox->object());
    Q_ASSERT(cb->isRenderBlock());

    int diff = 0;
    // ### actually flowBox->yPos() should suffice, but this is not ported
    // over yet from WebCore
    int fby = flowBox->firstChild()->yPos();
    if (cb != lastcb) {
      if (next) {
        diff = absy + lastfby + lastheight;
        cb->absolutePosition(absx, absy, false);	// ### what about fixed?
        diff = absy - diff + fby;
        lastfby = 0;
      } else {
        diff = absy;
        cb->absolutePosition(absx, absy, false);	// ### what about fixed?
        diff -= absy + fby + lastheight;
	lastfby = fby - lastheight;
      }/*end if*/
#if DEBUG_CARETMODE > 2
      kdDebug(6200) << "absdiff " << diff << endl;
#endif
    } else {
      diff = QABS(fby - lastfby);
    }/*end if*/
#if DEBUG_CARETMODE > 2
    kdDebug(6200) << "flowBox->firstChild->yPos: " << fby << " diff " << diff << endl;
#endif

    mindist -= diff;

    lastheight = QABS(fby - lastfby);
    lastfby = fby;
    lastcb = cb;
    it = copy;
#if DEBUG_CARETMODE > 0
    kdDebug(6200) << " mindist: " << mindist << endl;
#endif
    // trick: actually the distance is always one line short, but we cannot
    // calculate the height of the first line (### WebCore will make it better)
    // Therefore, we simply approximate that excess line by using the last
    // caluculated line height.
  } while (mindist - lastheight > 0);
}


}/*end namespace*/
