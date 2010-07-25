/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>
 *  Copyright (C) 2007 Mirko Stocker <me@misto.ch>
 *  Copyright (C) 2008 David Nolden <david.nolden.kdevelop@art-master.de>
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
 */

#include "katerenderrange.h"

#include <limits.h>

#include "katesmartrange.h"
#include <kcolorutils.h>

void mergeAttributes(KTextEditor::Attribute::Ptr base, KTextEditor::Attribute::Ptr add)
{
  if(!add)
    return;

  bool hadBg = base->hasProperty(KTextEditor::Attribute::BackgroundBrush);
  bool hasBg = add->hasProperty(KTextEditor::Attribute::BackgroundBrush);

  bool hadFg = base->hasProperty(KTextEditor::Attribute::ForegroundBrush);
  bool hasFg = add->hasProperty(KTextEditor::Attribute::ForegroundBrush);

  if(((!hadBg || !hasBg) && (!hadFg || !hasFg))) {
    //Nothing to blend
    *base += *add;
    return;
  }

  //We eventually have to blend

  QBrush baseBgBrush, baseFgBrush;

  if(hadBg)
    baseBgBrush = base->background();

  if(hadFg)
    baseFgBrush = base->foreground();

  *base += *add;

  if(hadBg && hasBg)
  {
    QBrush bg = add->background();
    if(!bg.isOpaque()) {
      QColor mixWithColor = bg.color();
      mixWithColor.setAlpha(255);
      bg.setColor(KColorUtils::mix(baseBgBrush.color(), mixWithColor, bg.color().alphaF()));
      base->setBackground(bg);
    }
  }
  if(hadFg && hasFg)
  {
    QBrush fg = add->foreground();
    if(!fg.isOpaque()) {
      QColor mixWithColor = fg.color();
      mixWithColor.setAlpha(255);
      fg.setColor(KColorUtils::mix(baseFgBrush.color(), mixWithColor, fg.color().alphaF()));
      base->setForeground(fg);
    }
  }
}

///Returns the first index of a range that contains @param pos, or the index of the first range that is behind pos(or ranges.count() if pos is behind all ranges)
///The list must be sorted by the ranges end-positions.
static int lowerBound(const QList<KTextEditor::SmartRange*>& ranges, const KTextEditor::Cursor& pos)
{
    int begin = 0;
    int n = ranges.count();

    int half;
    int middle;

    while (n > 0) {
        half = n >> 1;
        middle = begin + half;
        if(ranges[middle]->end() > pos) {
          n = half;
        }else{
          begin = middle + 1;
          n -= half + 1;
        }
    }
    return begin;
}

bool KateRenderRange::isReady() const {
    return false;
}

bool SmartRenderRange::isReady() const {
    return !m_currentRange;
}

SmartRenderRange::SmartRenderRange(KTextEditor::SmartRange* range, const SmartRenderRange& cloneFrom) : m_currentRange(cloneFrom.m_currentRange), m_endAtRange(range), m_view(cloneFrom.m_view), m_list(cloneFrom.m_list) {
  Q_ASSERT(range);
  addTo(range);
  Q_ASSERT(m_currentRange == range);
  m_currentPos = range->start();
}

SmartRenderRange::SmartRenderRange(KateSmartRange* range, KateView* view, RenderRangeList* list)
  : m_currentRange(0L)
  , m_endAtRange(0)
  , m_view(view)
  , m_list(list)
{
  Q_ASSERT(range);
  addTo(range);
  m_currentPos = range->start();
}

KTextEditor::Cursor SmartRenderRange::nextBoundary() const
{
  if (!m_currentRange || m_currentRange->end() <= m_currentPos)
    return KTextEditor::Cursor(INT_MAX,INT_MAX);
  if(m_currentPos < m_currentRange->start())
    return m_currentRange->start();

  KTextEditor::Cursor ret = m_currentRange->end();

  for(int child = lowerBound(m_currentRange->childRanges(), m_currentPos); child != m_currentRange->childRanges().size(); ++child) {
    KTextEditor::SmartRange* c = m_currentRange->childRanges()[child];

    if(!c->isEmpty() && !m_ignoreChildRanges.contains(c)) {
      if(c->start() > m_currentPos) {
        if(c->start() < ret)
            ret = c->start();

        if(c->overlapCount() == 0)
	       //We don't need to search on, since there is no range overlapping the given one, thus we have already found
	       //the nearest range-start.
          break;
      }
    }
  }
  return ret;
}

bool SmartRenderRange::advanceTo(const KTextEditor::Cursor& pos)
{
  m_currentPos = pos;

  if (!m_currentRange)
    return false;

  //Go up until the position is contained in m_currentRange
  while (m_currentRange && !m_currentRange->contains(pos) && m_currentRange->parentRange() && m_currentRange != m_endAtRange) {
    m_currentRange = m_currentRange->parentRange();
    m_attribs.pop();
  }

  if(m_currentPos >= m_currentRange->end()) {
    m_currentRange = 0; //We're ready with this range
    return false;
  }

  int currentChildCount = m_currentRange->childRanges().size();
  const QList<KTextEditor::SmartRange*>& currentChildRanges(m_currentRange->childRanges());

  int nextChild = lowerBound(currentChildRanges, pos);
  //If we skip a child, we must not recurse this range into another child,
  //else we will never see the skipped child again.
//   int initialNextChild = nextChild;
  Q_ASSERT(nextChild <= currentChildCount);

  for(; nextChild < currentChildCount; ++nextChild)
  {
      if(m_ignoreChildRanges.contains(currentChildRanges[nextChild])) {
        if(!currentChildRanges[nextChild]->overlapCount())
            nextChild = currentChildCount; //No chance to find a child that contains this cursor
        continue;
      }
      break;
  }

  //Now nextChild may contain the position, or not. But it is definitely the range
  //we should next recurse into. Create copies for all ranges behind nextChild that overlap it.

  if(nextChild < currentChildCount) {
    int findOverlaps = currentChildRanges[nextChild]->overlapCount();

    int overlapCandidate = nextChild+1;
    while(findOverlaps && overlapCandidate < currentChildCount) {
        ///@todo remove the hash, just measure where the overlaps start!
        if(currentChildRanges[overlapCandidate]->start() < currentChildRanges[nextChild]->end()) {
            //found an overlap
            if(!m_ignoreChildRanges.contains(currentChildRanges[overlapCandidate]) && currentChildRanges[overlapCandidate]->contains(pos)) {
                //Create additional SmartRenderRange's for all overlaps that are not ignored, and recurse into the first one
                SmartRenderRange* additional = new SmartRenderRange(currentChildRanges[overlapCandidate], *this);
                Q_ASSERT(additional->m_endAtRange == currentChildRanges[overlapCandidate]);
                Q_ASSERT(additional->m_currentRange == currentChildRanges[overlapCandidate]);
                additional->advanceTo(pos);
                m_list->append(additional);
                m_ignoreChildRanges.insert(currentChildRanges[overlapCandidate]);
            }
            --findOverlaps;
        }
        ++overlapCandidate;
    }


    if(nextChild < currentChildCount && currentChildRanges[nextChild]->contains(pos)) {
        addTo(currentChildRanges[nextChild]);
        //Recurse, to enter all needed ranges
        advanceTo(pos);
    }
  }

  return true;
}

KTextEditor::Attribute::Ptr SmartRenderRange::currentAttribute() const
{
  if (m_attribs.count() && m_currentRange->contains(m_currentPos))
    return m_attribs.top();
  return KTextEditor::Attribute::Ptr();
}

void SmartRenderRange::addTo(KTextEditor::SmartRange* _range, bool intermediate) const
{
  KateSmartRange* range = static_cast<KateSmartRange*>(_range);

  if(range->parentRange() != m_currentRange)
    addTo(range->parentRange(), true);

  KTextEditor::SmartRange* r = range;
  QStack<KTextEditor::SmartRange*> reverseStack;
  while (r != m_currentRange) {
    reverseStack.push(r);
    r = r->parentRange();
  }

  if(m_attribs.isEmpty() || (range->attribute() && (range->attribute()->isValid() || range->attribute()->hasAnyProperty()))) {
    //Only merge attributes if it's required
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute());
    if (!m_attribs.isEmpty())
      *a = *m_attribs.top();

    if (KTextEditor::Attribute::Ptr a2 = range->attribute())
      *a += *a2;

    m_attribs.push(a);
  }else{
    m_attribs.push(m_attribs.top());
  }

  if(!intermediate)
    m_currentRange = range;
}

NormalRenderRange::NormalRenderRange()
  : m_currentRange(0)
{
}

NormalRenderRange::~NormalRenderRange()
{
  QListIterator<pairRA> it = m_ranges;
  while (it.hasNext())
    delete it.next().first;
}

void NormalRenderRange::addRange(KTextEditor::Range* range, KTextEditor::Attribute::Ptr attribute)
{
  m_ranges.append(pairRA(range, attribute));
}

KTextEditor::Cursor NormalRenderRange::nextBoundary() const
{
  return m_nextBoundary;
}

bool NormalRenderRange::advanceTo(const KTextEditor::Cursor& pos)
{
  int index = m_currentRange;
  while (index < m_ranges.count()) {
    const pairRA& p = m_ranges.at(index);
    KTextEditor::Range* r = p.first;
    if (r->end() <= pos) {
      ++index;
    } else {
      bool ret = index != m_currentRange;
      m_currentRange = index;

      if (r->start() > pos) {
        m_nextBoundary = r->start();
      } else {
        m_nextBoundary = r->end();
      }
      if (r->contains(pos)) {
        m_currentAttribute = p.second;
      } else {
        m_currentAttribute.clear();
      }

      return ret;
    }
  }

  m_nextBoundary = KTextEditor::Cursor(INT_MAX, INT_MAX);
  m_currentAttribute.clear();
  return false;
}

KTextEditor::Attribute::Ptr NormalRenderRange::currentAttribute() const
{
  return m_currentAttribute;
}

void RenderRangeList::appendRanges(const QList<KTextEditor::SmartRange*>& startingRanges, KateView* view)
{
  foreach (KTextEditor::SmartRange* range, startingRanges)
    append(new SmartRenderRange(static_cast<KateSmartRange*>(range), view, this));
}

KTextEditor::Cursor RenderRangeList::nextBoundary() const
{
  KTextEditor::Cursor ret = m_currentPos;
  bool first = true;
  foreach (KateRenderRange* r, *this) {
    if (first) {
      ret = r->nextBoundary();
      first = false;

    } else {
      KTextEditor::Cursor nb = r->nextBoundary();
      if (ret > nb)
        ret = nb;
    }
  }
  return ret;
}

RenderRangeList::~RenderRangeList()
{
}

void RenderRangeList::advanceTo(const KTextEditor::Cursor& pos)
{
  foreach (KateRenderRange* r, *this)
    r->advanceTo(pos);

  //Delete lists that are ready, else the list may get too large due to temporaries
  for(int a = size()-1; a >= 0; --a) {
      KateRenderRange* r = at(a);
      if(r->isReady()) {
          delete r;
          removeAt(a);
      }
  }
}

bool RenderRangeList::hasAttribute() const
{
  foreach (KateRenderRange* r, *this)
    if (r->currentAttribute())
      return true;

  return false;
}

KTextEditor::Attribute::Ptr RenderRangeList::generateAttribute() const
{
  KTextEditor::Attribute::Ptr a;
  bool ownsAttribute = false;

  foreach (KateRenderRange* r, *this) {
    if (KTextEditor::Attribute::Ptr a2 = r->currentAttribute()) {
      if(!a) {
	a = a2;
      }else {
	if(!ownsAttribute) {
	  //Make an own copy of the attribute..
	  ownsAttribute = true;
	  a = new KTextEditor::Attribute(*a);
	}
        mergeAttributes(a, a2);
      }
    }
  }

  return a;
}

