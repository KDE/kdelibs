/* qktstreelist.cpp
-------------------------------------------------------------------
$Id$
  
KTreeList class implementation

Copyright (C) 1996 Keith Brown and KtSoft

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABLILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details. You should have received a
copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc, 675 Mass Ave, 
Cambridge, MA 02139, USA.
  
-------------------------------------------------------------------
*/

#include <ktreelist.h>
#include "ktreelist.moc"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* 
-------------------------------------------------------------------

KTreeListItem
  
-------------------------------------------------------------------
*/

// constructor
KTreeListItem::KTreeListItem(const char *theText,
				   const QPixmap* thePixmap):
  branch(0),
  indent(22), 	 // seems to be a good default value
  numChildren(0),
  doExpandButton(TRUE),
  expanded(FALSE),
  doTree(TRUE),
  doText(TRUE),
  child(0),
  parent(0),
  sibling(0)
{
  if(theText)
    text = theText;
  if(thePixmap)
    pixmap = *thePixmap;
}

// destructor
KTreeListItem::~KTreeListItem()
{
	// anything need to be done here?
}

// adds child to internal list and fixes pointers, branch and child count
void KTreeListItem::appendChild(KTreeListItem *newChild)
{
  newChild->setParent(this);
  newChild->setBranch(branch + 1);
  if(!getChild())
    setChild(newChild);
  else {
    KTreeListItem *lastChild = getChild();
    while(lastChild->hasSibling())
      lastChild = lastChild->getSibling();
    lastChild->setSibling(newChild);
  }
  numChildren++;
}

// returns the bounding rectangle of the item content (not including indent
// and branches) for hit testing
QRect KTreeListItem::boundingRect(const QFontMetrics& fm) const
{
	int rectX = 3 + indent + branch * indent;
	int rectY = 1;
	int rectW = itemWidth(fm) - rectX;
	int rectH = itemHeight(fm) - 2;
	return QRect(rectX, rectY, rectW, rectH);
}

// returns the child item at the specified index
KTreeListItem *KTreeListItem::childAt(int index)
{
  if(!hasChild()) return 0;
  int counter = 0;
  KTreeListItem *item = getChild();
  while(counter != index && item) {
    item = item->getSibling();
    counter++;
  }
  return item;
}

// returns the number of children this item has
uint KTreeListItem::childCount() const
{
  return numChildren;
}

// returns the index of the given child item in this item's branch
int KTreeListItem::childIndex(KTreeListItem *theChild)
{
  int counter = 0;
  KTreeListItem *item = getChild();
  while(item && item != theChild) {
    item = item->getSibling();
    counter++;
  }
  if(item) return counter;
  return -1;
}

// indicates wheter expand button will be displayed
bool KTreeListItem::drawExpandButton() const
{
  return doExpandButton;
}

// indicates whether item text will be displayed
bool KTreeListItem::drawText() const
{
  return doText;
}

// indicates whether tree branches will be drawn
bool KTreeListItem::drawTree() const
{
  return doTree;
}

// indicates whether mouse press is in expand button
inline bool KTreeListItem::expandButtonClicked(const QPoint& coord) const
{
  return expandButton.contains(coord);
}

// returns the branch level of this item
int KTreeListItem::getBranch() const
{
  return branch;
}

// returns a pointer to first child item
KTreeListItem *KTreeListItem::getChild()
{
  return child;
}

// returns the indent spacing of this item
int KTreeListItem::getIndent() const
{
  return indent;
}

// returns the parent of this item
KTreeListItem *KTreeListItem::getParent()
{
  return parent;
}

// returns reference to the item pixmap
const QPixmap *KTreeListItem::getPixmap() const
{
  return &pixmap;
}

// returns the sibling below this item
KTreeListItem *KTreeListItem::getSibling()
{
  return sibling;
}

// returns a pointer to the item text
const char *KTreeListItem::getText() const
{
  return text; // implicit cast to char *!
}

// indicates whether this item has any children
bool KTreeListItem::hasChild() const
{
  return child != 0;
}

// indicates whether this item has a parent
bool KTreeListItem::hasParent() const
{
  return parent != 0;
}

// indicates whether this item has a sibling below it
bool KTreeListItem::hasSibling() const
{
  return sibling != 0;
}

// returns the the height of the cell
int KTreeListItem::height(const KTreeList *theOwner) const
{
  return itemHeight(theOwner->fontMetrics());
}

// inserts child item at specified index in branch and fixes
// pointers, branch and child count
void KTreeListItem::insertChild(int index,
				   KTreeListItem *newChild)
{
  if(index == -1) {
    appendChild(newChild);
    return;
  }
  newChild->setParent(this);
  newChild->setBranch(branch + 1);
  if(index == 0) {
    newChild->setSibling(getChild());
    setChild(newChild);
  }
  else {
    int counter = 1;
    KTreeListItem *prevItem = getChild();
    KTreeListItem *nextItem = prevItem->getSibling();
    while(counter != index && nextItem) {
      prevItem = nextItem;
      nextItem = prevItem->getSibling();
      counter++;
    }
    prevItem->setSibling(newChild);
    newChild->setSibling(nextItem);
  }
  numChildren++;
}

// indicates whether this item is displayed expanded 
// NOTE: a TRUE response does not necessarily indicate the item 
// has any children
bool KTreeListItem::isExpanded() const
{
  return expanded;
}

// paint this item, including tree branches and expand button
void KTreeListItem::paint(QPainter *p, const QColorGroup& cg, bool highlighted)
{
  p->save();
  p->setPen(cg.text());
  p->setBackgroundColor(cg.base());
  
  // precalculate positions
  
  int cellHeight = itemHeight(p->fontMetrics());
  int pixmapX = 3 + indent + branch * indent;
  int pixmapY = (cellHeight - pixmap.height()) / 2;
  int parentLeaderX = pixmapX - (indent / 2);
  int itemLeaderX = pixmapX - 1;
  int cellCenterY = cellHeight / 2;
  int cellBottomY = cellHeight - 1;
  if(doTree) {
    
    // if this is not the first item in the tree
    // draw the line up towards parent or sibling
 
    if(!(parent->getBranch() == -1 && parent->getChild() == this)) 
      p->drawLine(parentLeaderX, 0, parentLeaderX, cellCenterY);
      
    // draw the line down towards sibling
    
    if(sibling) 
      p->drawLine(parentLeaderX, cellCenterY, parentLeaderX, cellBottomY);
     
    // if this item has children or siblings in the tree or is a child of
    // an item other than the root item then
    // draw the little line from the item out to the left

    if(sibling || (child && doExpandButton) || 
    	(parent && (branch > 0 || (branch == 0 && parent->getChild() != this))))
      p->drawLine(parentLeaderX, cellCenterY, itemLeaderX, cellCenterY); 

    // if there are siblings of ancestors below, draw our portion of
    // the branches that extend through this cell
      
    KTreeListItem *prevRoot = parent;
    for(int i = 1; i <= branch; i++) {
      if(prevRoot->hasSibling()) {
	int sibLeaderX = parentLeaderX - i * indent;
	p->drawLine(sibLeaderX, 0, sibLeaderX, cellBottomY);
      }
      prevRoot = prevRoot->getParent();
    }
  }

  // if this item has at least one child and expand button drawing is enabled, 
  // set the rect for the expand button for later mouse press bounds checking, 
  // and draw the button

  if(doExpandButton && child) {
    expandButton.setRect(parentLeaderX - 4, cellCenterY - 4,
			 9, 9);
    p->drawRect(expandButton);
    if(expanded) 
      p->drawLine(parentLeaderX - 2, cellCenterY, 
		  parentLeaderX + 2, cellCenterY);
    else {
      p->drawLine(parentLeaderX - 2, cellCenterY,
		  parentLeaderX + 2, cellCenterY);
      p->drawLine(parentLeaderX, cellCenterY - 2, 
		  parentLeaderX, cellCenterY + 2);
    }
    // p->setBrush(NoBrush);
  }
  
  // now draw the item pixmap and text, if applicable
  
  p->drawPixmap(pixmapX, pixmapY, pixmap);
  if(doText) {
    int textX = pixmapX + pixmap.width() + 4;
    int textY = cellHeight - ((cellHeight - p->fontMetrics().ascent() - 
      p->fontMetrics().leading()) / 2);
    if(highlighted) {
      p->setPen(cg.base());
      p->setBackgroundColor(cg.text());
    }
    else {
      p->setPen(cg.text());
      p->setBackgroundColor(cg.base());
    }
    p->drawText(textX, textY, text);
  }
  p->restore();
}

// removes the given child from the branch
void KTreeListItem::removeChild(KTreeListItem *theChild)
{
  KTreeListItem *prevItem = 0;
  KTreeListItem *toRemove = getChild();
  while(toRemove && toRemove != theChild) {
    prevItem = toRemove;
    toRemove = toRemove->getSibling();
  }
  if(toRemove) {
    if(!prevItem)
      setChild(toRemove->getSibling());
    else
      prevItem->setSibling(toRemove->getSibling());
    numChildren--;
  }
}

// sets the branch level of this item
void KTreeListItem::setBranch(int level)
{
  branch = level;
}

// sets child pointer to the given item
void KTreeListItem::setChild(KTreeListItem *newChild)
{
  child = newChild;
}

// sets the draw expand button flag of this item
void KTreeListItem::setDrawExpandButton(bool doit)
{
  doExpandButton = doit;
}

// sets the draw text flag of this item
void KTreeListItem::setDrawText(bool doit)
{
  doText = doit;
}

// sets the draw tree branch flag of this item
void KTreeListItem::setDrawTree(bool doit)
{
  doTree = doit;
}

// sets the expanded flag of this item
void KTreeListItem::setExpanded(bool is)
{
  expanded = is;
}

// sets the indent spacing of this item
void KTreeListItem::setIndent(int value)
{
  indent = value;
}

// sets the parent to the given item
void KTreeListItem::setParent(KTreeListItem *newParent)
{
  parent = newParent;
}

// sets the item pixmap to the given pixmap
void KTreeListItem::setPixmap(const QPixmap *pm)
{
  pixmap = *pm;
}

// sets the item's sibling to the given item
void KTreeListItem::setSibling(KTreeListItem *newSibling)
{
  sibling = newSibling;
}

// sets the item text to the given string
void KTreeListItem::setText(const char *t)
{
  text = t;
}

// returns the bounding rect of the item text in cell coordinates
// couldn't get QFontMetrics::boundingRect() to work right
// so I made my own
QRect KTreeListItem::textBoundingRect(const QFontMetrics& fm) const
{
  int cellHeight = itemHeight(fm);
  int rectX = 3 + indent + branch * indent + pixmap.width() + 3;
  int rectY = (cellHeight - fm.ascent() - fm.leading()) / 2 + 2;
  int rectW = fm.width(text) + 1;
  int rectH = fm.ascent() + fm.leading();
  return QRect(rectX, rectY, rectW, rectH);
}

// returns the cell's width
int KTreeListItem::width(const KTreeList *theOwner) const
{
  return itemWidth(theOwner->fontMetrics());
}

// returns the maximum height of text and pixmap including margins
// or -1 if item is empty -- SHOULD NEVER BE -1!
int KTreeListItem::itemHeight(const QFontMetrics& fm) const
{
  int maxHeight = pixmap.height();
  int textHeight = fm.ascent() + fm.leading();
    maxHeight = textHeight > maxHeight ? textHeight : maxHeight;
  return maxHeight == 0 ? -1 : maxHeight + 8;
}

// returns the total width of text and pixmap, including margins, spacing
// and indent, or -1 if empty -- SHOULD NEVER BE -1!
int KTreeListItem::itemWidth(const QFontMetrics& fm) const
{
  int maxWidth = pixmap.width();
  maxWidth += (4 + fm.width(text));
  return maxWidth == 0 ? -1 : indent + maxWidth + 
    indent * branch + 6;
}

// returns the bounding rect of the item text in cell relative
// coordinates

/*
-------------------------------------------------------------------

KTreeList
  
-------------------------------------------------------------------
*/

// constructor
KTreeList::KTreeList(QWidget *parent,
			   const char *name,
			   WFlags f):
  QTableView(parent, name, f),
  clearing(FALSE),
  current(-1),
  drawExpandButton(TRUE),
  drawTree(TRUE),
  expansion(0),
  goingDown(FALSE),
  indent(-1),
  showText(TRUE),
  rubberband_mode(false) 
{
  initMetaObject();
  setCellHeight(0);
  setCellWidth(0);
  setNumRows(0);
  setNumCols(1);
  setTableFlags(Tbl_autoScrollBars | Tbl_clipCellPainting | Tbl_snapToVGrid);
  clearTableFlags(Tbl_scrollLastVCell | Tbl_scrollLastHCell | Tbl_snapToVGrid);
  switch(style()) {
    case WindowsStyle:
    case MotifStyle:
      setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
      setBackgroundColor(colorGroup().base());
      break;
    default:
      setFrameStyle(QFrame::Panel | QFrame::Plain);
      setLineWidth(1);
  }
  setAcceptFocus(TRUE);
  treeRoot = new KTreeListItem;
  treeRoot->setBranch(-1);
  treeRoot->setExpanded(TRUE);
}

// destructor
KTreeList::~KTreeList()
{
  goingDown = TRUE;
  clear();
  delete treeRoot;
}

// appends a child to the item at the given index with the given text
// and pixmap
void KTreeList::addChildItem(const char *theText,
                                const QPixmap *thePixmap,
                                int index)
{
  addChildItem(new KTreeListItem(theText, thePixmap), index);
}

// appends a child to the item at the end of the given path with
// the given text and pixmap
void KTreeList::addChildItem(const char *theText,
                                const QPixmap *thePixmap,
                                const KPath *thePath)
{
  addChildItem(new KTreeListItem(theText, thePixmap), thePath);
}

// appends the given item to the children of the item at the given index
void KTreeList::addChildItem(KTreeListItem *newItem,
                                int index)
{                                                                  
  if(!checkItemText(newItem->getText()))
    return;
  
  // find parent item and append new item to parent's sub tree
  
  KTreeListItem *parentItem = itemAt(index);
  if(!parentItem)
    return;
  addChildItem(parentItem, newItem);
}

// appends the given item to the children of the item at the end of the
// given path
void KTreeList::addChildItem(KTreeListItem *newItem,
                                const KPath *thePath)
{                                
  if(!checkItemText(newItem->getText()))
    return;
  if(!checkItemPath(thePath))
    return;
   
  // find parent item and append new item to parent's sub tree
  
  KTreeListItem *parentItem = itemAt(thePath);
  if(!parentItem)
    return;
  addChildItem(parentItem, newItem);
}
                                 
// indicates whether horizontal scrollbar appears only when needed
bool KTreeList::autoBottomScrollBar() const
{
  return testTableFlags(Tbl_autoHScrollBar);
}

// indicates whether vertical scrollbar appears only when needed
bool KTreeList::autoScrollBar() const
{
  return testTableFlags(Tbl_autoVScrollBar);
}

// indicates whether display updates automatically on changes
bool KTreeList::autoUpdate() const
{
  return QTableView::autoUpdate();
}

// indicates whether horizontal scrollbar is present
bool KTreeList::bottomScrollBar() const
{
  return testTableFlags(Tbl_hScrollBar);
}

// find item at specified index and change pixmap and/or text
void KTreeList::changeItem(const char *newText,
			      const QPixmap *newPixmap,
			      int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    changeItem(item, index, newText, newPixmap);
}

// find item at end of specified path, and change pixmap and/or text
void KTreeList::changeItem(const char *newText,
			      const QPixmap *newPixmap,
			      const KPath *thePath)
{
  KTreeListItem *item = itemAt(thePath);
  if(item) {
    int index = itemIndex(item);
    changeItem(item, index, newText, newPixmap);
  }
}

// clear all items from list and erase display
void KTreeList::clear()
{
	// somewhat of a hack for takeItem so it doesn't update the current item...
	clearing = TRUE;
	
	bool autoU = autoUpdate();
	setAutoUpdate(FALSE);
	QStack<KTreeListItem> stack;
	stack.push(treeRoot);
	while(!stack.isEmpty()) {
		KTreeListItem *item = stack.pop();
		if(item->hasChild()) {
			stack.push(item);
			stack.push(item->getChild());
		}
		else if(item->hasSibling()) {
			stack.push(item);
			stack.push(item->getSibling());
		}
		else if(item->getBranch() != -1) {
			takeItem(item);
			delete item;
		}
	}
	clearing = FALSE;
	setCurrentItem(-1);	
  if(goingDown || QApplication::closingDown())
    return;
  if(autoU && isVisible())
    repaint();
  setAutoUpdate(autoU);
}

// return a count of all the items in the list, whether visible or not
uint KTreeList::count()
{
  int total = 0;
  forEveryItem(&KTreeList::countItem, (void *)&total);
  return total;
}

// returns the index of the current (highlighted) item
int KTreeList::currentItem() const
{
  return current;
}

// only collapses the item if it is expanded. If not expanded, or
// index invalid, does nothing
void KTreeList::collapseItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item && item->isExpanded())
    expandOrCollapse(item);
}

// only expands the item if it is collapsed. If not collapsed, or
// index invalid, does nothing
void KTreeList::expandItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item && !item->isExpanded())
    expandOrCollapse(item);
}

// returns the depth the tree is automatically expanded to when
// items are added
int KTreeList::expandLevel() const
{
  return expansion;
}

// expands or collapses the subtree rooted at the given item,
// as approptiate
// if item has no children, does nothing
void KTreeList::expandOrCollapseItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    expandOrCollapse(item);
}

// visits every item in the tree, visible or not and applies 
// the user supplied function with the item and user data passed as parameters
// if user supplied function returns TRUE, traversal ends and function returns
void KTreeList::forEveryItem(KForEvery func,
			   void *user)
{
  QStack<KTreeListItem> stack;
  KTreeListItem *item = treeRoot->getChild();
  while(item) {
    stack.push(item);
    while(!stack.isEmpty()) {
      KTreeListItem *poppedItem = stack.pop();
      if((*func)(poppedItem, user))
	return;
      if(poppedItem->hasChild()) {
	KTreeListItem *childItem = poppedItem->getChild();
	while(childItem) {
	  stack.push(childItem);
	  childItem = childItem->getSibling();
	}
      }
    }
    item = item->getSibling();
  }
}

// visits every visible item in the tree in order and applies the 
// user supplied function with the item and user data passed as parameters
// if user supplied function returns TRUE, traversal ends and function
// returns
void KTreeList::forEveryVisibleItem(KForEvery func,
				       void *user)
{
  QStack<KTreeListItem> stack;
  KTreeListItem *item = treeRoot->getChild();
  do {
    while(item) {
      if((*func)(item, user)) return;
      if(item->hasChild() && item->isExpanded()) {
        stack.push(item);
        item = item->getChild();
      }
      else
        item = item->getSibling();
    }
    if(stack.isEmpty())
      break;
    item = stack.pop()->getSibling();
  } while(TRUE);
}

// returns a pointer to the KTreeListItem at the current index
// or 0 if no current item
KTreeListItem *KTreeList::getCurrentItem()
{
  if(current == -1) return 0;
  return itemAt(current);
}

// returns the current indent spacing, or -1 if using item default
int KTreeList::indentSpacing()
{
  return indent;
}

// inserts a new item with the specified text and pixmap before
// or after the item at the given index, depending on the value
// of prefix
// if index is negative, appends item to tree at root level
void KTreeList::insertItem(const char *theText,
			      const QPixmap *thePixmap,
			      int index,
			      bool prefix)
{
  insertItem(new KTreeListItem(theText, thePixmap), index, prefix);
}

// inserts a new item with the specified text and pixmap before
// or after the item at the end of the given path, depending on the value
// of prefix
void KTreeList::insertItem(const char *theText,
			      const QPixmap *thePixmap,
			      const KPath *thePath,
			      bool prefix)
{
  insertItem(new KTreeListItem(theText, thePixmap), thePath, prefix);
}

// inserts the given item or derived object into the tree before
// or after the item at the given index, depending on the value
// of prefix
// if index is negative, appends item to tree at root level
void KTreeList::insertItem(KTreeListItem *newItem,
                              int index,
                              bool prefix)
{
  // check for proper parameters
  
  if(!checkItemText(newItem->getText()))
    return;

  // find the item currently at the index, if there is one
  
  KTreeListItem *foundItem = itemAt(index);

  // insert new item at the appropriate place
  
  insertItem(foundItem, newItem, prefix);
}

// inserts the given item or derived object into the tree before
// or after the item at the end of the given path, depending on the value
// of prefix
void KTreeList::insertItem(KTreeListItem *newItem,
                              const KPath *thePath,
                              bool prefix)
{                              
  // check for valid parameters
	
  if(!checkItemText(newItem->getText()))
    return;
  if(!checkItemPath(thePath))
    return;

  // find the item currently at the end of the path, if there is one

  KTreeListItem *foundItem = itemAt(thePath);

  // insert new item at appropriate place
  
  insertItem(foundItem, newItem, prefix);
}
                                              
// returns pointer to KTreeListItem at index or 0 if
// index is invalid
KTreeListItem *KTreeList::itemAt(int index)
{
  KItemSearchInfo searchInfo;
  searchInfo.index = index;
  searchInfo.count = -1;
  forEveryVisibleItem(&KTreeList::findItemAt, &searchInfo);
  return searchInfo.foundItem;
}

// returns pointer to KTreeListItem at the end of the
// path or 0 if not found
KTreeListItem *KTreeList::itemAt(const KPath *path)
{
  // reverse the path so we can use it
  
  KPath pathCopy = *path, *reversePath = new KPath();
  while(!pathCopy.isEmpty())
    reversePath->push(pathCopy.pop());
  KTreeListItem *found = treeRoot->getChild();
  if(!found) {
      delete reversePath;
      return 0;
  }
  found = recursiveFind(found, reversePath);
  delete reversePath;
  return found;
}

// returns the index in the visible tree of the given item or
// -1 if not found
int KTreeList::itemIndex(KTreeListItem *item)
{
  KItemSearchInfo searchInfo;
  searchInfo.index =  searchInfo.count = -1;
  searchInfo.foundItem = item;
  forEveryVisibleItem(&KTreeList::getItemIndex, &searchInfo);
  return searchInfo.index;
}

// returns pointer to item path or 0
// if index is invalid
// the first item popped from the returned path will be
// the item and the last the root item
// in this form, the path can be passed directly to other
// functions that take a path, or consecutive levels pushed
// onto it first
KPath *KTreeList::itemPath(int index)
{
  KTreeListItem *item = itemAt(index);
  if(!item) return 0;
  KPath tempPath, *tempPathCopy = new KPath();
  while(item) {
    if(item != treeRoot)
      tempPath.push(new QString(item->getText()));
    item = item->getParent();
  }
  while(!tempPath.isEmpty())
    tempPathCopy->push(tempPath.pop());
  tempPathCopy->setAutoDelete(TRUE);
  return tempPathCopy;
}

// move the subtree at the specified index up one branch level
// (make root item a sibling of it's current parent)
void KTreeList::join(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    join(item);
}

// move the subtree at the specified index up one branch level
// (make root item a sibling of it's current parent)
void KTreeList::join(const KPath *path)
{
  KTreeListItem *item = itemAt(path);
  if(item)
    join(item);
}

// move item at specified index one slot down in its parent's sub tree
void KTreeList::lowerItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    lowerItem(item);
}

// move item at specified path one slot down in its parent's sub tree
void KTreeList::lowerItem(const KPath *path)
{
  KTreeListItem *item = itemAt(path);
  if(item)
    lowerItem(item);
}

// move item at specified index one slot up in its parent's sub tree
void KTreeList::raiseItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    raiseItem(item);
}

// move item at specified path one slot up in its parent's sub tree
void KTreeList::raiseItem(const KPath *path)
{
  KTreeListItem *item = itemAt(path);
  if(item)
    raiseItem(item);
}

// remove the item at the specified index and delete it
void KTreeList::removeItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item) { 
    takeItem(item);
    delete item;
  }
}

// remove the item at the end of the specified path and delete it
void KTreeList::removeItem(const KPath *thePath)
{
  KTreeListItem *item = itemAt(thePath);
  if(item) {
    takeItem(item);
    delete item;
  }
}

// indicates whether vertical scrollbar is present
bool KTreeList::scrollBar() const
{
  return testTableFlags(Tbl_vScrollBar);
}

// enables/disables auto update of display
void KTreeList::setAutoUpdate(bool enable)
{
  QTableView::setAutoUpdate(enable);
}

// enables/disables horizontal scrollbar
void KTreeList::setBottomScrollBar(bool enable)
{
  enable ? setTableFlags(Tbl_hScrollBar) :
    clearTableFlags(Tbl_hScrollBar);
}

// sets the current item and hightlights it, emitting signal
void KTreeList::setCurrentItem(int index)
{
  if(index == current)
    return;
  int numVisible = visibleCount();
  if(index > numVisible)
    return;
  int oldCurrent = current;
  current = index;
  if(oldCurrent < numVisible)
    updateCell(oldCurrent, 0);
  if(current > -1) {
    updateCell(current, 0, FALSE);
    emit highlighted(current);
  }
}

// enables/disables drawing of expand button
void KTreeList::setExpandButtonDrawing(bool enable)
{
  if(drawExpandButton == enable)
    return;
  drawExpandButton = enable;
  forEveryItem(&KTreeList::setItemExpandButtonDrawing, 0);
  if(autoUpdate() && isVisible())
    repaint();
}

// sets depth to which subtrees are automatically expanded, and
// redraws tree if auto update enabled
void KTreeList::setExpandLevel(int level)
{
  if(expansion == level)
    return;
  expansion = level;
  KTreeListItem *item = getCurrentItem();
  forEveryItem(&KTreeList::setItemExpanded, 0);
  while(item) {
    if(item->getParent()->isExpanded())
      break;
    item = item->getParent();
  }
  setCurrentItem(itemIndex(item));
  if(autoUpdate() && isVisible())
    repaint();
}

// sets the indent margin for all branches and repaints if auto update enabled
// setting to -1 uses default spacing in items themselves
void KTreeList::setIndentSpacing(int spacing)
{
  if(indent == spacing)
    return;
  indent = spacing;
  forEveryItem(&KTreeList::setItemIndent, 0);
  if(autoUpdate() && isVisible())
    repaint();
}

// enables/disables vertical scrollbar
void KTreeList::setScrollBar(bool enable)
{
  enable ? setTableFlags(Tbl_vScrollBar) :
    clearTableFlags(Tbl_vScrollBar);
}

// enables/disables display of item text (keys)
void KTreeList::setShowItemText(bool enable)
{
  if(showText == enable)
    return;
  showText = enable;
  forEveryItem(&KTreeList::setItemShowText, 0);
  if(autoUpdate() && isVisible())
    repaint();
}

// indicates whether vertical scrolling is by pixel or row
void KTreeList::setSmoothScrolling(bool enable)
{
  enable ? setTableFlags(Tbl_smoothVScrolling) :
    clearTableFlags(Tbl_smoothVScrolling);
}

// enables/disables tree branch drawing
void KTreeList::setTreeDrawing(bool enable)
{
  if(drawTree == enable)
    return;
  drawTree = enable;
  forEveryItem(&KTreeList::setItemTreeDrawing, 0);
  if(autoUpdate() && isVisible())
    repaint();
}
    
// indicates whether item text keys are displayed
bool KTreeList::showItemText() const
{
  return showText;
}

// indicates whether scrolling is by pixel or row
bool KTreeList::smoothScrolling() const
{
  return testTableFlags(Tbl_smoothVScrolling);
}

// indents the item at the given index, splitting the tree into
// a new branch
void KTreeList::split(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    split(item);
}

// indents the item at the given path, splitting the tree into
// a new branch
void KTreeList::split(const KPath *path)
{
  KTreeListItem *item = itemAt(path);
  if(item)
    split(item);
}

// removes item at specified index from tree but does not delete it
// returns pointer to the item or 0 if not succesful
KTreeListItem *KTreeList::takeItem(int index)
{
  KTreeListItem *item = itemAt(index);
  if(item)
    takeItem(item);
  return item;
}

// removes item at specified path from tree but does not delete it
// returns pointer to the item or 0 if not successful
KTreeListItem *KTreeList::takeItem(const KPath *path)
{
  KTreeListItem *item = itemAt(path);
  if(item)
    takeItem(item);
  return item;
}

// indicates whether tree branches are drawn
bool KTreeList::treeDrawing() const
{
  return drawTree;
}

// returns the number of items visible
int KTreeList::visibleCount()
{
  int total = 0;
  forEveryVisibleItem(&KTreeList::countItem, (void *)&total);
  return total;
}

// adds the new item at the end of the parent item's branch
void KTreeList::addChildItem(KTreeListItem *theParent,
				KTreeListItem *newItem)
{ 
  theParent->appendChild(newItem);
  
  // set item state
  
  if(indent > -1)  // custom indent
    newItem->setIndent(indent);
  newItem->setDrawExpandButton(drawExpandButton);
  newItem->setDrawTree(drawTree);
  newItem->setDrawText(showText);
  if(newItem->getBranch() < expansion)
    newItem->setExpanded(TRUE); 
	
	// fix up branch levels of any children new item may already have
	
	if(newItem->hasChild())
		fixChildBranches(newItem);
		
  // if necessary, adjust cell width, number of rows and repaint
  
  if(theParent->isExpanded() || theParent->childCount() == 1) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    setNumRows(visibleCount());
    updateCellWidth();
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// returns the height of the cell(row) at the specified row (index)
int KTreeList::cellHeight(int row)
{
  return itemAt(row)->height(this);
}

// changes the given item with the new text and/or pixmap
void KTreeList::changeItem(KTreeListItem *toChange,
			      int itemIndex,
			      const char *newText,
			      const QPixmap *newPixmap)
{
  int oldWidth = toChange->width(this);
  if(newText)
    toChange->setText(newText);
  if(newPixmap)
    toChange->setPixmap(newPixmap);
  if(oldWidth != toChange->width(this))
    updateCellWidth();
  if(itemIndex == -1)
    return;
  if(autoUpdate() && rowIsVisible(itemIndex))
    updateCell(itemIndex, 0);
}

// returns bool indicating valid item path
bool KTreeList::checkItemPath(const KPath *path) const
{
  bool isValid = TRUE;
  if(!path) 
    isValid = FALSE;
  if(path->isEmpty())
    isValid = FALSE; 
  return isValid;
}

// returns bool indicating valid item text
bool KTreeList::checkItemText(const char *text) const
{
  bool isValid = TRUE;
  if(!text)
    isValid = FALSE;
  else if(*text == 0) 
    isValid = FALSE;
  return isValid;
}

// collapses the subtree at the specified item
void KTreeList::collapseSubTree(KTreeListItem *subRoot)
{
  subRoot->setExpanded(FALSE);
  setNumRows(visibleCount());
  updateCellWidth();
}

// used by count() with forEach() function to count total number
// of items in the list
bool KTreeList::countItem(KTreeListItem *, void *total)
{
  (*((int *)total))++;
  return FALSE;
}

// if item is expanded, collapses it or vice-versa
void KTreeList::expandOrCollapse(KTreeListItem *parent)
{
  bool autoU = autoUpdate();
  setAutoUpdate(FALSE);
  int parentIndex = itemIndex(parent);
  if(parent->isExpanded()) { 
    collapseSubTree(parent);
    emit collapsed(parentIndex);
  }
  else {
    expandSubTree(parent);
    emit expanded(parentIndex);
  }
  if(autoU && isVisible())
    repaint();
  setAutoUpdate(autoU);
}

// expands the subtree at the given item
void KTreeList::expandSubTree(KTreeListItem *subRoot)
{
  subRoot->setExpanded(TRUE);
  setNumRows(visibleCount());
  updateCellWidth();
}

// called by itemAt for each item
bool KTreeList::findItemAt(KTreeListItem *item, void *user)
{
  KItemSearchInfo *searchInfo = (KItemSearchInfo *)user;
  if(++searchInfo->count == searchInfo->index) {
    searchInfo->foundItem = item;
    return TRUE;
  }
  return FALSE;
}

// fix up branch levels out of whack from split/join operations on the tree

void KTreeList::fixChildBranches(KTreeListItem *parentItem)
{
	KTreeListItem *childItem = 0, *siblingItem = 0;
	int childBranch = parentItem->getBranch() + 1;
	if(parentItem->hasChild()) {
		childItem = parentItem->getChild(); 
		childItem->setBranch(childBranch);
		fixChildBranches(childItem);
	}
	while(childItem && childItem->hasSibling()) {
		siblingItem = childItem->getSibling();
		siblingItem->setBranch(childBranch);
		fixChildBranches(siblingItem);
		childItem = siblingItem;
	}	
}

// handles QFocusEvent processing by setting current item to top
// row if there is no current item, and updates cell to add or
// delete the focus rectangle on the highlight bar
void KTreeList::focusInEvent(QFocusEvent *)
{
  if(current < 0 && numRows() > 0)
    setCurrentItem(topCell());
  updateCell(current, 0);
}

// visits every item in the tree, visible or not and applies the user 
// supplied member function with the item and user data passed as parameters
// if the user supplied member function returns TRUE, traversal
// ends and the function returns
void KTreeList::forEveryItem(KForEveryM func,
			   void *user)
{
  KTreeListItem *item = treeRoot->getChild();
  QStack<KTreeListItem> stack;
  while(item) {
    stack.push(item);
    while(!stack.isEmpty()) {
      KTreeListItem *poppedItem = stack.pop();
      if((this->*func)(poppedItem, user))
	return;
      if(poppedItem->hasChild()) {
	KTreeListItem *childItem = poppedItem->getChild();
	while(childItem) {
	  stack.push(childItem);
	  childItem = childItem->getSibling();
	}
      }
    }
    item = item->getSibling();
  }
}

// visits every visible item in the tree in order and applies the user 
// supplied member function with the item and user data passed as parameters
// if user supplied function returns TRUE, traversal ends and function
// returns
void KTreeList::forEveryVisibleItem(KForEveryM func,
				       void *user)
{
  QStack<KTreeListItem> stack;
  KTreeListItem *item = treeRoot->getChild();
  do {
    while(item) {
      if((this->*func)(item, user)) return;
      if(item->hasChild() && item->isExpanded()) {
        stack.push(item);
        item = item->getChild();
      }
      else
        item = item->getSibling();
    }
    if(stack.isEmpty())
      break;
    item = stack.pop()->getSibling();
  } while(TRUE);
}

// called by itemIndex() for each item in visible list  
bool KTreeList::getItemIndex(KTreeListItem *item, void *user)
{
  KItemSearchInfo *searchInfo = (KItemSearchInfo *)user;
  searchInfo->count++;
  if(item == searchInfo->foundItem) {
    searchInfo->index = searchInfo->count;
    return TRUE;
  }
  return FALSE;
}

// called by updateCellWidth() for each item in the visible list
bool KTreeList::getMaxItemWidth(KTreeListItem *item, void *user)
{
  int *maxW = (int *)user;
  int w = item->width(this);
  if(w > *maxW)
    *maxW = w;
  return FALSE;
}

// inserts the new item before or after the reference item, depending
// on the value of prefix
void KTreeList::insertItem(KTreeListItem *referenceItem,
			      KTreeListItem *newItem,
			      bool prefix)
{
	
  // set the new item's state
	
  if(indent > -1)  // custom indent
    newItem->setIndent(indent);
  newItem->setDrawExpandButton(drawExpandButton);
  newItem->setDrawTree(drawTree);
  newItem->setDrawText(showText);
  KTreeListItem *parentItem;
  if(referenceItem) {
    parentItem = referenceItem->getParent(); 
    int insertIndex = parentItem->childIndex(referenceItem);
    if(!prefix)
      insertIndex++;
    parentItem->insertChild(insertIndex, newItem);
  }
  
  // no reference item, append at end of visible tree
  // need to repaint
  
  else {
    parentItem = treeRoot;
    parentItem->appendChild(newItem);
  }
  
  // set item expansion
  
  if(newItem->getBranch() < expansion)
    newItem->setExpanded(TRUE); 
  
  // fix up branch levels of any children
  
  if(newItem->hasChild())
		fixChildBranches(newItem);
	  	
  // if repaint necessary, do it if visible and auto update
  // enabled
  
  //if(parentItem->isExpanded())
  //  setNumRows(visibleCount());
  
  if(parentItem->isExpanded() || parentItem->childCount() == 1) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    setNumRows(visibleCount());
    updateCellWidth();
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// joins the item's branch into the tree, making the item a sibling
// of its parent
void KTreeList::join(KTreeListItem *item)
{
  KTreeListItem *itemParent = item->getParent();
  if(itemParent->hasParent()) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    takeItem(item);
    insertItem(itemParent, item, FALSE);
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// handles keyboard interface to tree list
void KTreeList::keyPressEvent(QKeyEvent *e)
{
  if(numRows() == 0) 
    
    // nothing to be done
    
    return;
  if(currentItem() < 0)
    
    // if no current item, make the top item current
  
    setCurrentItem(topCell()); 
  int pageSize, delta;
  switch(e->key()) {
    case Key_Up:

      // make previous item current, scroll up if necessary

      if(currentItem() > 0) {
	setCurrentItem(currentItem() - 1);
	if(currentItem() < topCell())
	  setTopCell(currentItem());
      }
      break;
    case Key_Down:
    
      // make next item current, scroll down if necessary
      
      if(currentItem() < visibleCount() - 1) {
	setCurrentItem(currentItem() + 1);
	if(currentItem() > lastRowVisible())
	  setTopCell(topCell() + currentItem() - lastRowVisible());
      }
      break;
    case Key_Next:
    
      // move highlight one page down and scroll down
       
      delta = currentItem() - topCell();
      pageSize = lastRowVisible() - topCell();
      setTopCell(QMIN(topCell() +  pageSize, visibleCount() - 1));
      setCurrentItem(QMIN(topCell() + delta, visibleCount() - 1));
      break;
    case Key_Prior:
    
      // move highlight one page up and scroll up
      
      delta = currentItem() - topCell();
      pageSize = lastRowVisible() - topCell();
      setTopCell(QMAX(topCell() - pageSize, 0));
      setCurrentItem(QMAX(topCell() + delta, 0));
      break;
    case Key_Plus:
    
      // if current item has subtree and is collapsed, expand it
      
      if(currentItem() >= 0)
	expandItem(currentItem());
      break;
    case Key_Minus:
    
      // if current item has subtree and is expanded, collapse it
      
      if(currentItem() >= 0)
	collapseItem(currentItem());
      break;
    case Key_Return:
    case Key_Enter:
    
      // select the current item
      
      if(currentItem() >= 0)
	emit selected(currentItem());
      break;
    default:
      break;
  }
}

// move specified item down one slot in parent's subtree
void KTreeList::lowerItem(KTreeListItem *item)
{
  KTreeListItem *itemParent = item->getParent();
  uint itemChildIndex = itemParent->childIndex(item);
  if(itemChildIndex < itemParent->childCount() - 1) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    takeItem(item);
    insertItem(itemParent->childAt(itemChildIndex), item, FALSE);
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// handle mouse double click events by selecting the clicked item
// and emitting the signal
void KTreeList::mouseDoubleClickEvent(QMouseEvent *e)
{
  // find out which row has been clicked
	
  QPoint mouseCoord = e->pos();
  int itemClicked = findRow(mouseCoord.y());
  
  // if a valid row was not clicked, do nothing
  
  if(itemClicked == -1) 
    return;

  KTreeListItem *item = itemAt(itemClicked);
  if(!item) return;
  
  // translate mouse coord to cell coord
  
  int  cellX, cellY;
  colXPos(0, &cellX);
  rowYPos(itemClicked, &cellY);
  QPoint cellCoord(mouseCoord.x() - cellX, mouseCoord.y() - cellY);
  
  // hit test item
 
  if(item->boundingRect(fontMetrics()).contains(cellCoord))
    emit selected(itemClicked);
}

// handle mouse movement events
void KTreeList::mouseMoveEvent(QMouseEvent *e)
{
  // in rubberband_mode we actually scroll the window now
  if (rubberband_mode) 
	{
	  move_rubberband(e->pos());
	}
}


// handle single mouse presses
void KTreeList::mousePressEvent(QMouseEvent *e)
{
  // first: check which button was pressed

  if (e->button() == MidButton) 
	{
	  // RB: the MMB is hardcoded to the "rubberband" scroll mode
	  if (!rubberband_mode) {
		start_rubberband(e->pos());
	  }
	  return;
	} 
  else if (rubberband_mode) 
	{
	  // another button was pressed while rubberbanding, stop the move.
	  // RB: if we allow other buttons while rubberbanding the tree can expand
	  //     while rubberbanding - we then need to reclaculate and resize the
	  //     rubberband rect and show the new size)
	  end_rubberband();
	  return;  // should we process the button press?
	}

  // find out which row has been clicked
	
  QPoint mouseCoord = e->pos();
  int itemClicked = findRow(mouseCoord.y());
  
  // if a valid row was not clicked, do nothing
  
  if(itemClicked == -1) 
    return;

  KTreeListItem *item = itemAt(itemClicked);
  if(!item) return;
  
  // translate mouse coord to cell coord
  
  int  cellX, cellY;
  colXPos(0, &cellX);
  rowYPos(itemClicked, &cellY);
  QPoint cellCoord(mouseCoord.x() - cellX, mouseCoord.y() - cellY);
  
  // hit test item
 
  if(item->boundingRect(fontMetrics()).contains(cellCoord))
    setCurrentItem(itemClicked);

  // hit test expand button
  
  else if(item->hasChild()) {
    if(item->expandButtonClicked(cellCoord)) {
      expandOrCollapse(item);
      setCurrentItem(itemClicked);
    }
  }
}

// handle mouse release events
void KTreeList::mouseReleaseEvent(QMouseEvent *e)
{
  // if it's the MMB end rubberbanding
  if (rubberband_mode && e->button()==MidButton) 
	{
	  end_rubberband();
	}
}

// rubberband move: draw the rubberband
void KTreeList::draw_rubberband()
{
  // RB: I'm using a white pen because of the XorROP mode. I would prefer to
  //     draw the rectangle in red but I don't now how to get a pen which
  //     draws red in XorROP mode (this depends on the background). In fact
  //     the color should be configurable.

  if (!rubberband_mode) return;
  QPainter paint(this);
  paint.setPen(white);
  paint.setRasterOp(XorROP);
  paint.drawRect(xOffset()*viewWidth()/totalWidth(),
                 yOffset()*viewHeight()/totalHeight(),
                 rubber_width+1, rubber_height+1);
  paint.end();
}

// rubberband move: start move
void KTreeList::start_rubberband(const QPoint& where)
{
  if (rubberband_mode) { // Oops!
    end_rubberband();
  }
  // RB: Don't now, if this check is necessary
  if (!viewWidth() || !viewHeight()) return; 
  if (!totalWidth() || !totalHeight()) return;

  // calculate the size of the rubberband rectangle
  rubber_width = viewWidth()*viewWidth()/totalWidth();
  if (rubber_width > viewWidth()) rubber_width = viewWidth();
  rubber_height = viewHeight()*viewHeight()/totalHeight();
  if (rubber_height > viewHeight()) rubber_height = viewHeight();

  // remember the cursor position and the actual offset
  rubber_startMouse = where;
  rubber_startX = xOffset();
  rubber_startY = yOffset();
  rubberband_mode=TRUE;
  draw_rubberband();
}

// rubberband move: end move
void KTreeList::end_rubberband()
{
  if (!rubberband_mode) return;
  draw_rubberband();
  rubberband_mode = FALSE;
}

// rubberband move: hanlde mouse moves
void KTreeList::move_rubberband(const QPoint& where)
{
  if (!rubberband_mode) return;

  // look how much the mouse moved and calc the new scroll position
  QPoint delta = where - rubber_startMouse;
  int nx = rubber_startX + delta.x() * totalWidth() / viewWidth();
  int ny = rubber_startY + delta.y() * totalHeight() / viewHeight();

  // check the new position (and make it valid)
  if (nx < 0) nx = 0;
  else if (nx > maxXOffset()) nx = maxXOffset();
  if (ny < 0) ny = 0;
  else if (ny > maxYOffset()) ny = maxYOffset();

  // redraw the rubberband at the new position
  draw_rubberband();
  setOffset(nx,ny);
  draw_rubberband();
}


// paints the cell at the specified row and col
// col is ignored for now since there is only one
void KTreeList::paintCell(QPainter *p, int row, int)
{
  KTreeListItem *item = itemAt(row);
  if(!item)
    return;

  // if current item, draw highlight and item with reverse text
  
  if(current == row) {
    if(item->drawText())
      paintHighlight(p, item);
    paintItem(p, item, TRUE);
  }
  
  // else just paint the item

  else
    paintItem(p, item, FALSE);
}

// paint the highlight 
void KTreeList::paintHighlight(QPainter *p, 
				  KTreeListItem *item)
{
  p->save(); // save state of painter
  QColorGroup cg = colorGroup();
  QColor fc;
  if(style() == WindowsStyle) 
    fc = darkBlue; // hardcoded in Qt
  else
    fc = cg.text();
  QRect textRect = item->textBoundingRect(p->fontMetrics());
  int t,l,b,r;
  textRect.coords(&l, &t, &r, &b);
  p->fillRect(textRect, fc); // draw base highlight
  if(hasFocus()) { 
    if(style() == WindowsStyle) {  // draw Windows style highlight
      textRect.setCoords(l - 1, t - 1, r + 1, b + 1);
      p->setPen(QPen(yellow, 0, DotLine));
      p->setBackgroundColor(fc);
      p->setBackgroundMode(OpaqueMode);
      p->drawRect(textRect);
      textRect.setCoords(l - 2, t - 2, r + 2, b + 2);
      p->setPen(fc);
      p->drawRect(textRect);
    }
    else { // draw Motif style highlight
      textRect.setCoords(l - 2, t - 2, r + 2, b + 2);
      p->drawRect(textRect);
    }
  }
  p->restore(); // restore painter
}

// paint the items content
void KTreeList::paintItem(QPainter *p,
  KTreeListItem *item,
  bool highlighted)
{
  p->save(); // save state of painter
  QColorGroup cg = colorGroup();
  item->paint(p, cg, highlighted);
  p->restore();
}

// raise the specified item up one slot in parent's subtree
void KTreeList::raiseItem(KTreeListItem *item)
{
  KTreeListItem *itemParent = item->getParent();
  int itemChildIndex = itemParent->childIndex(item);
  if(itemChildIndex > 0) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    takeItem(item);
    insertItem(itemParent->childAt(--itemChildIndex), item, TRUE);
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// descend the path recursively and find the item at the end if
// there is one
KTreeListItem *KTreeList::recursiveFind
  (KTreeListItem *subRoot, 
   KPath *path)
{
  
  KTreeListItem *retVal = 0;
  // get the next key
  
  QString *searchString = path->pop();
  KTreeListItem *currentSubTree = subRoot, *nextSubTree = 0;
  
  // iterate through all branches at same level until
  // we find the one we're looking for or run out
  
  while(currentSubTree) {
    if(strcmp(*searchString, currentSubTree->getText()) == 0) {
      if(path->isEmpty()) {
	retVal = currentSubTree; // found match and end of path
	nextSubTree = 0;
	break;
      }
      if(!currentSubTree->hasChild() && !path->isEmpty()) 
	break; // not end of path but no more subtrees
      
      // not end of path, find next level
      
      nextSubTree = currentSubTree->getChild();
      break;
    }
    
    // not in this branch, search next branch
    
    currentSubTree = currentSubTree->getSibling();
  }

  // search the next level
  
  if(nextSubTree)
    return recursiveFind(nextSubTree, path);
  return retVal;
}

// called by setExpandLevel for each item in tree
bool KTreeList::setItemExpanded(KTreeListItem *item, void *)
{
  if(item->getBranch() < expansion) {
    if(item->hasChild() && !item->isExpanded())
      expandSubTree(item);
    else
      item->setExpanded(TRUE);
  }
  else {
    if(item->hasChild() && item->isExpanded())
      collapseSubTree(item);
    else
      item->setExpanded(FALSE);
  }
  return FALSE;
}

// called by setExpandButtonDrawing for every item in tree
bool KTreeList::setItemExpandButtonDrawing(KTreeListItem *item,
					      void *)
{
  item->setDrawExpandButton(drawExpandButton);
  return FALSE;
}

// called by setIndentSpacing for every item in tree
bool KTreeList::setItemIndent(KTreeListItem *item, 
				 void *)
{
  item->setIndent(indent);
  return FALSE;
}

// called by setShowItemText for every item in tree
bool KTreeList::setItemShowText(KTreeListItem *item, 
				   void *)
{
  item->setDrawText(showText);
  return FALSE;
}

// called by setTreeDrawing for every item in tree
bool KTreeList::setItemTreeDrawing(KTreeListItem *item, void *)
{
  item->setDrawTree(drawTree);
  return FALSE;
}

// makes the item a child of the item above it, splitting
// the tree into a new branch
void KTreeList::split(KTreeListItem *item)
{
  KTreeListItem *itemParent = item->getParent();
  int itemChildIndex = itemParent->childIndex(item);
  if(itemChildIndex == 0)
    return;
  bool autoU = autoUpdate();
  setAutoUpdate(FALSE);
  takeItem(item);
  addChildItem(itemParent->childAt(--itemChildIndex), item);
  if(autoU && isVisible())
    repaint();
  setAutoUpdate(autoU);
}

// removes the item from the tree without deleting it
void KTreeList::takeItem(KTreeListItem *item)
{
  int index = itemIndex(item);
  KTreeListItem *parentItem = item->getParent();
  parentItem->removeChild(item);
  item->setSibling(0);
  if(parentItem->isExpanded() || parentItem->childCount() == 0) {
    bool autoU = autoUpdate();
    setAutoUpdate(FALSE);
    setNumRows(visibleCount());
    
		// If we're not clearing the entire tree, make sure this item is not
		// the last one and current, or current item index will be invalid! If it
		// is, then set current to last item.
		
		if(!clearing) 
			if(index == currentItem() && index > numRows() - 1)
				setCurrentItem(numRows() - 1);
			
    if(autoU && isVisible())
      repaint();
    setAutoUpdate(autoU);
  }
}

// visits each item, calculates the maximum width  
// and updates QTableView
void KTreeList::updateCellWidth()
{
  int maxW = 0;
  forEveryVisibleItem(&KTreeList::getMaxItemWidth, &maxW);
  setCellWidth(maxW);
}
