/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2003,2004,2005 Hamish Rodda <rodda@kde.org>
 *  Copyright (C) 2010 Christoph Cullmann <cullmann@kde.org>
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

#include "katesmartcursor.h"

#include "katedocument.h"
#include "kateedit.h"
#include "katesmartmanager.h"
#include "katesmartrange.h"
#include "katesmartgroup.h"

#include <ktexteditor/smartcursornotifier.h>
#include <ktexteditor/smartcursorwatcher.h>

#include <kdebug.h>

/**
 * Internal implementation of the smart cursor notifier.  Basically allows emission of signals.
 * @author Hamish Rodda <rodda@kde.org>
 */
class KateSmartCursorNotifier : public KTextEditor::SmartCursorNotifier
{
  friend class KateSmartCursor;

  public:
    KateSmartCursorNotifier() : KTextEditor::SmartCursorNotifier () {}
    virtual ~KateSmartCursorNotifier() {}
};

//#define DEBUG_KATESMARTCURSOR

KateSmartCursor::KateSmartCursor(const KTextEditor::Cursor& position, KTextEditor::Document* doc, KTextEditor::SmartCursor::InsertBehavior insertBehavior)
  : KTextEditor::SmartCursor(position, doc, insertBehavior)
  , m_oldGroupLineStart(-1)
  , m_lastPosition(position)
  , m_feedbackEnabled(false)
  , m_isInternal(false)
  , m_lastPositionNeeded(false)
  , m_bypassTranslation(0)
  , m_notifier(0L)
  , m_watcher(0L)
{
  if (position.line() > kateDocument()->lastLine()) {
    kWarning() << "Attempted to set cursor position " << position << " past end of document " << doc->documentRange();
    m_line = -1;
    m_column = -1;
  }

  // Replace straight line number with smartgroup + line offset
  m_smartGroup = kateDocument()->smartManager()->groupForLine(m_line);
  m_line = m_line - m_smartGroup->startLine();
  m_smartGroup->joined(this);

#ifdef DEBUG_KATESMARTCURSOR
  kDebug() << "Cursor created at " << *this;
#endif
}

KateSmartCursor::KateSmartCursor( KTextEditor::Document * doc, KTextEditor::SmartCursor::InsertBehavior insertBehavior )
  : KTextEditor::SmartCursor(KTextEditor::Cursor(), doc, insertBehavior)
  , m_oldGroupLineStart(-1)
  , m_feedbackEnabled(false)
  , m_isInternal(false)
  , m_lastPositionNeeded(false)
  , m_bypassTranslation(0)
  , m_notifier(0L)
  , m_watcher(0L)
{
  // Replace straight line number with smartgroup + line offset
  m_smartGroup = kateDocument()->smartManager()->groupForLine(m_line);
  m_line = m_line - m_smartGroup->startLine();
  m_smartGroup->joined(this);

#ifdef DEBUG_KATESMARTCURSOR
  kDebug() << this << "Cursor created at " << *this;
#endif
}

KateSmartCursor::~KateSmartCursor()
{
  if (m_notifier) {
    emit m_notifier->deleted(this);
    delete m_notifier;
  }

  if (m_watcher)
    m_watcher->deleted(this);

  if (!kateDocument()->smartManager()->isClearing())
    m_smartGroup->leaving(this);
}

KateSmartCursor::operator QString()
{
  return QString("[%1,%1]").arg(line()).arg(column());
}

KateDocument* KateSmartCursor::kateDocument() const
{
  return static_cast<KateDocument*>(document());
}

bool KateSmartCursor::isValid( ) const
{
  return line() >= 0 && column() >= 0 && line() <= kateDocument()->lastLine() && column() <= kateDocument()->lineLength(line());
}

bool KateSmartCursor::isValid(const Cursor& position) const
{
  return position.line() >= 0 && position.line() <= kateDocument()->lastLine() && position.column() >= 0 && position.column() <= kateDocument()->lineLength(position.line());
}

bool KateSmartCursor::atEndOfLine( ) const
{
  return line() >= 0 && line() <= kateDocument()->lastLine() && column() >= kateDocument()->lineLength(line());
}

void KateSmartCursor::checkFeedback()
{
  bool feedbackNeeded = m_watcher || m_notifier;

  m_lastPositionNeeded = feedbackNeeded || (range() && static_cast<KateSmartRange*>(range())->feedbackEnabled());

  if (m_feedbackEnabled != feedbackNeeded) {
    m_smartGroup->changeCursorFeedback(this);
    m_feedbackEnabled = feedbackNeeded;
  }
}

int KateSmartCursor::line( ) const
{
  return m_smartGroup->startLine() + m_line;
}

void KateSmartCursor::setLine( int _line )
{
  setPositionInternal(KTextEditor::Cursor(_line, m_column), false);
}

void KateSmartCursor::setPositionInternal( const KTextEditor::Cursor & pos, bool internal )
{
  // Shortcut if there's no change :)
  if (*this == pos)
    return;
  
  //Previous position of the "other" side in the range that this cursor is part of
  KTextEditor::Cursor oldOther;
  
  if(m_range) {
    KTextEditor::SmartRange* smartRange = m_range->toSmartRange();
    
    if(!internal && smartRange) {
      KateSmartCursor& start = dynamic_cast<KateSmartCursor&>(smartRange->smartStart());
      KateSmartCursor& end = dynamic_cast<KateSmartCursor&>(smartRange->smartEnd());
      //Eventually move the other cursor first, so the smart-range cannot temporarily become an invalid range with start > end.
      //If we let it become invalid, that will create serious consistency problems in places that depend on it, like for example
      //the SmartRange::rangeChanged function.
      if(this == &start) {
        oldOther = end;
        if(pos > end)
          //We do the change with "internal" set, so the parent-range is not confined to the not yet transformed changed range
          end.setPositionInternal(pos, true);
      }else{
        Q_ASSERT(this == &end);
        oldOther = start;
        if(pos < start)
          //We do the change with "internal" set, so the parent-range is not confined to the not yet transformed changed range
          start.setPositionInternal(pos, true);
      }
    }
  }

  KTextEditor::Cursor old = *this;

  // Remember this position if the feedback system needs it
  if (m_lastPositionNeeded)
    m_lastPosition = *this;

  // Deal with crossing a smart group border
  bool haveToChangeGroups = !m_smartGroup->containsLine(pos.line());
  if (haveToChangeGroups) {
    m_smartGroup->leaving(this);
    m_smartGroup = kateDocument()->smartManager()->groupForLine(pos.line());
  }

  // Set the new position
  m_line = pos.line() - m_smartGroup->newStartLine();
  m_column = pos.column();

  // Finish dealing with crossing a smart group border
  if (haveToChangeGroups) {
    m_smartGroup->joined(this);
  }

  // Forget this position change if the feedback system doesn't need it
  if (!m_lastPositionNeeded)
    m_lastPosition = *this;

  // Adjustments only needed for non-internal position changes...
  if (!internal) {
    //We notify about the range as a whole, instead of using cursorChangedDirectory. This allows us
    //notifying the change to the other side too, if we did one.
    // Tell the range about this
    if (m_range) {
      if (this == &m_range->start())
        static_cast<KateSmartRange*>(m_range)->rangeChanged(0, KTextEditor::Range(old, oldOther));
      else
        static_cast<KateSmartRange*>(m_range)->rangeChanged(0, KTextEditor::Range(oldOther, old));
    }
  }

#ifdef DEBUG_KATESMARTCURSOR
  kDebug() << this << "Cursor moved from" << old << "to" << *this;
#endif
}

KTextEditor::SmartCursorNotifier* KateSmartCursor::notifier( )
{
  if (!m_notifier) {
    m_notifier = new KateSmartCursorNotifier();
    checkFeedback();
  }
  return m_notifier;
}

void KateSmartCursor::deleteNotifier( )
{
  delete m_notifier;
  m_notifier = 0L;
  checkFeedback();
}

void KateSmartCursor::setWatcher( KTextEditor::SmartCursorWatcher * watcher )
{
  m_watcher = watcher;
  checkFeedback();
}

bool KateSmartCursor::translate( const KateEditInfo & edit )
{
#ifdef DEBUG_KATESMARTCURSOR
  kDebug() << this << "Translating cursor" << *this << "from " << edit.oldRange() << "to" << edit.newRange() << edit.editSource() << &edit;
#endif

  if (m_bypassTranslation) {
    if (m_bypassTranslation == &edit) {
      // This cursor has already been moved for this edit
      m_bypassTranslation = 0;
      return true;
    }

    m_bypassTranslation = 0;
  }

  // If this cursor is before the edit, no action is required
  if (*this < edit.start())
    return false;

  // Calculate the new position
  KTextEditor::Cursor newPos;

  // If this cursor is on a line affected by the edit
  if (edit.oldRange().overlapsLine(line())) {
    // If this cursor is at the start of the edit
    if (*this == edit.start()) {
      // And it doesn't need to move, no action is required
      if (insertBehavior() == KTextEditor::SmartCursor::StayOnInsert)
        return false;
    }

    if (edit.oldRange().contains(*this)) {
      if (insertBehavior() == KTextEditor::SmartCursor::MoveOnInsert)
        newPos = edit.newRange().end();
      else
        newPos = edit.start();

    } else {
      newPos = *this + edit.translate();
    }

  } else {
    // just need to adjust line number
    newPos.setPosition(line() + edit.translate().line(), column());
  }

  if (newPos != *this) {
    // Catch corner case where the range is non-expanding, is zero length, and then the
    // start cursor would otherwise be placed before the end cursor.
    if (KTextEditor::SmartRange* range = smartRange()) {
      
      if (&(range->smartStart()) == this) {
        if (*this == edit.start()) {
          if (range->insertBehavior() == KTextEditor::SmartRange::DoNotExpand) {
            if (range->end() == *this) {
              KateSmartCursor* end = static_cast<KateSmartCursor*>(&(range->smartEnd()));
              end->setPositionInternal(newPos);
              // Don't let the end cursor get translated again
              end->m_bypassTranslation = &edit;
            }
          }
        }
      }else{
      }
    }

    setPositionInternal(newPos);
    return true;
  }

  return false;
}

bool KateSmartCursor::cursorMoved( ) const
{
  bool ret = m_oldGroupLineStart != m_smartGroup->startLine();
  m_oldGroupLineStart = m_smartGroup->startLine();
  return ret;
}

void KateSmartCursor::setLineInternal( int newLine, bool internal )
{
  setPositionInternal(KTextEditor::Cursor(newLine, column()), internal);
}

void KateSmartCursor::translated(const KateEditInfo & edit)
{
  if (*this < edit.start()) {
    if (!range() || !static_cast<KateSmartRange*>(range())->feedbackEnabled())
      m_lastPosition = *this;
    return;
  }

  // We can rely on m_lastPosition because it is updated in translate(), otherwise just shifted() is called
  if (m_lastPosition != *this) {
    // position changed
    if (m_notifier)
      emit m_notifier->positionChanged(this);
    if (m_watcher)
      m_watcher->positionChanged(this);
  }

  if (!edit.oldRange().isEmpty() && edit.start() <= m_lastPosition && edit.oldRange().end() >= m_lastPosition) {
    if (edit.start() == m_lastPosition) {
      // character deleted after
      if (m_notifier)
        emit m_notifier->characterDeleted(this, false);
      if (m_watcher)
        m_watcher->characterDeleted(this, false);

    } else if (edit.oldRange().end() == m_lastPosition) {
      // character deleted before
      if (m_notifier)
        emit m_notifier->characterDeleted(this, true);
      if (m_watcher)
        m_watcher->characterDeleted(this, true);

    } else {
      // position deleted
      if (m_notifier)
        emit m_notifier->positionDeleted(this);
      if (m_watcher)
        m_watcher->positionDeleted(this);
    }
  }

  if (!edit.newRange().isEmpty()) {
    if (*this == edit.newRange().start()) {
      // character inserted after
      if (m_notifier)
        emit m_notifier->characterInserted(this, false);
      if (m_watcher)
        m_watcher->characterInserted(this, false);

    } else if (*this == edit.newRange().end()) {
      // character inserted before
      if (m_notifier)
        emit m_notifier->characterInserted(this, true);
      if (m_watcher)
        m_watcher->characterInserted(this, true);
    }
  }

  if (!range() || !static_cast<KateSmartRange*>(range())->feedbackEnabled())
    m_lastPosition = *this;
}

void KateSmartCursor::shifted( )
{
  Q_ASSERT(m_lastPosition != *this);

  // position changed
  if (m_notifier)
    emit m_notifier->positionChanged(this);
  if (m_watcher)
    m_watcher->positionChanged(this);

  if (!range() || !static_cast<KateSmartRange*>(range())->feedbackEnabled())
    m_lastPosition = *this;
}

void KateSmartCursor::migrate( KateSmartGroup * newGroup )
{
  int lineNum = line();
  m_smartGroup = newGroup;
  m_line = lineNum - m_smartGroup->startLine();
}

void KateSmartCursor::setPosition( const KTextEditor::Cursor & pos )
{
  if (pos.line() > kateDocument()->lastLine()) {
    kWarning() << "Attempted to set cursor position " << pos << " past end of document " << document()->documentRange();
    setPositionInternal(invalid(), false);
    return;
  }

  setPositionInternal(pos, false);
}

void KateSmartCursor::resetLastPosition( )
{
  m_lastPosition = *this;
}

bool KateSmartCursor::hasNotifier( ) const
{
  return m_notifier;
}

KTextEditor::SmartCursorWatcher * KateSmartCursor::watcher( ) const
{
  return m_watcher;
}

void KateSmartCursor::unbindFromRange( )
{
  setRange(0L);
}

void KateSmartCursor::setInternal( )
{
  m_isInternal = true;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
