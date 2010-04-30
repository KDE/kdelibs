/* This file is part of the KDE libraries

   Copyright (c) 2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>
   Copyright (c) 2000,2001,2002,2003,2004 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "kcompletionbox.h"
#include "klineedit.h"

#include <QtCore/QEvent>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QStyle>
#include <QtGui/QScrollBar>
#include <QtGui/QKeyEvent>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobalsettings.h>

class KCompletionBox::KCompletionBoxPrivate
{
public:
    QWidget *m_parent; // necessary to set the focus back
    QString cancelText;
    bool tabHandling : 1;
    bool upwardBox : 1;
    bool emitSelected : 1;
};

KCompletionBox::KCompletionBox( QWidget *parent )
 :KListWidget( parent), d(new KCompletionBoxPrivate)
{
    d->m_parent        = parent;
    d->tabHandling     = true;
    d->upwardBox       = false;
    d->emitSelected    = true;

    setWindowFlags( Qt::ToolTip ); // calls setVisible, so must be done after initializations

    setLineWidth( 1 );
    setFrameStyle( QFrame::Box | QFrame::Plain );

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect( this, SIGNAL( itemDoubleClicked( QListWidgetItem * )),
             SLOT( slotActivated( QListWidgetItem * )) );
    connect( this, SIGNAL( itemClicked( QListWidgetItem * )),
             SLOT( slotItemClicked( QListWidgetItem * )) );
}

KCompletionBox::~KCompletionBox()
{
    d->m_parent = 0L;
    delete d;
}

QStringList KCompletionBox::items() const
{
    QStringList list;

    for (int i = 0 ; i < count() ; i++)
    {
        const QListWidgetItem* currItem = item(i);

        list.append(currItem->text());
    }

    return list;
}

void KCompletionBox::slotActivated( QListWidgetItem *item )
{
    if ( !item )
        return;

    hide();
    emit activated( item->text() );
}

bool KCompletionBox::eventFilter( QObject *o, QEvent *e )
{
    int type = e->type();
    QWidget *wid = qobject_cast<QWidget*>(o);

    if (o == this) {
        return false;
    }

    if (wid && wid == d->m_parent &&
        (type == QEvent::Move || type == QEvent::Resize)) {
        hide();
        return false;
    }

    if (wid && (wid->windowFlags() & Qt::Window) &&
        type == QEvent::Move && wid == d->m_parent->window()) {
        hide();
        return false;
    }

    if (type == QEvent::MouseButtonPress && (wid && !isAncestorOf(wid))) {
        if (!d->emitSelected && currentItem() && !qobject_cast<QScrollBar*>(o)) {
            Q_ASSERT(currentItem());
            emit currentTextChanged(currentItem()->text() );
        }
        hide();
        e->accept();
        return true;
    }

    if (wid && wid->isAncestorOf(d->m_parent) && isVisible()) {
        if ( type == QEvent::KeyPress ) {
            QKeyEvent *ev = static_cast<QKeyEvent *>( e );
            switch ( ev->key() ) {
            case Qt::Key_Backtab:
                if ( d->tabHandling && (ev->modifiers() == Qt::NoButton ||
                                        (ev->modifiers() & Qt::ShiftModifier)) ) {
                    up();
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Tab:
                if ( d->tabHandling && (ev->modifiers() == Qt::NoButton) ) {
                    down();
                    // #65877: Key_Tab should complete using the first
                    // (or selected) item, and then offer completions again
                    if (count() == 1) {
                        KLineEdit* parent = qobject_cast<KLineEdit*>(d->m_parent);
                        if (parent) {
                            parent->doCompletion(currentItem()->text());
                        } else {
                            hide();
                        }
                    }
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Down:
                down();
                ev->accept();
                return true;
            case Qt::Key_Up:
                // If there is no selected item and we've popped up above
                // our parent, select the first item when they press up.
                if ( !selectedItems().isEmpty() ||
                     mapToGlobal( QPoint( 0, 0 ) ).y() >
                     d->m_parent->mapToGlobal( QPoint( 0, 0 ) ).y() )
                    up();
                else
                    down();
                ev->accept();
                return true;
            case Qt::Key_PageUp:
                pageUp();
                ev->accept();
                return true;
            case Qt::Key_PageDown:
                pageDown();
                ev->accept();
                return true;
            case Qt::Key_Escape:
                canceled();
                ev->accept();
                return true;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if ( ev->modifiers() & Qt::ShiftModifier ) {
                    hide();
                    ev->accept();  // Consume the Enter event
                    return true;
                }
                break;
            case Qt::Key_End:
                if ( ev->modifiers() & Qt::ControlModifier )
                {
                    end();
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Home:
                if ( ev->modifiers() & Qt::ControlModifier )
                {
                    home();
                    ev->accept();
                    return true;
                }
            default:
                break;
            }
        } else if ( type == QEvent::ShortcutOverride ) {
            // Override any accelerators that match
            // the key sequences we use here...
            QKeyEvent *ev = static_cast<QKeyEvent *>( e );
            switch ( ev->key() ) {
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->accept();
                return true;
                break;
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                if ( ev->modifiers() == Qt::NoButton ||
                     (ev->modifiers() & Qt::ShiftModifier))
                {
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Home:
            case Qt::Key_End:
                if ( ev->modifiers() & Qt::ControlModifier )
                {
                    ev->accept();
                    return true;
                }
                break;
            default:
                break;
            }
        } else if ( type == QEvent::FocusOut ) {
            QFocusEvent* event = static_cast<QFocusEvent*>( e );
            if (event->reason() != Qt::PopupFocusReason
#ifdef Q_WS_WIN
                && (event->reason() != Qt::ActiveWindowFocusReason || QApplication::activeWindow() != this)
#endif
                )
                hide();
        }
    }

    return KListWidget::eventFilter( o, e );
}

void KCompletionBox::popup()
{
    if ( count() == 0 )
        hide();
    else {
        bool block = signalsBlocked();
        blockSignals( true );
        setCurrentRow( -1 );
        blockSignals( block );
        clearSelection();
        if ( !isVisible() )
            show();
        else if ( size().height() != sizeHint().height() )
            sizeAndPosition();
    }
}

void KCompletionBox::sizeAndPosition()
{
    int currentGeom = height();
    QPoint currentPos = pos();
    QRect geom = calculateGeometry();
    resize( geom.size() );

    int x = currentPos.x(), y = currentPos.y();
    if ( d->m_parent ) {
      if ( !isVisible() ) {
        QPoint orig = globalPositionHint();
        QRect screenSize = KGlobalSettings::desktopGeometry(orig);

        x = orig.x() + geom.x();
        y = orig.y() + geom.y();

        if ( x + width() > screenSize.right() )
            x = screenSize.right() - width();
        if (y + height() > screenSize.bottom() ) {
            y = y - height() - d->m_parent->height();
            d->upwardBox = true;
        }
      }
      else {
        // Are we above our parent? If so we must keep bottom edge anchored.
        if (d->upwardBox)
          y += (currentGeom-height());
      }
      move( x, y);
    }
}

QPoint KCompletionBox::globalPositionHint() const
{
    if (!d->m_parent)
        return QPoint();
    return d->m_parent->mapToGlobal( QPoint(0, d->m_parent->height()) );
}

void KCompletionBox::setVisible( bool visible )
{
    if (visible) {
        d->upwardBox = false;
        if ( d->m_parent ) {
            sizeAndPosition();
            qApp->installEventFilter( this );
        }

        // ### we shouldn't need to call this, but without this, the scrollbars
        // are pretty b0rked.
        //triggerUpdate( true );

        // Workaround for I'm not sure whose bug - if this KCompletionBox' parent
        // is in a layout, that layout will detect inserting new child (posted
        // ChildInserted event), and will trigger relayout (post LayoutHint event).
        // QWidget::show() sends also posted ChildInserted events for the parent,
        // and later all LayoutHint events, which causes layout updating.
        // The problem is, KCompletionBox::eventFilter() detects resizing
        // of the parent, and calls hide() - and this hide() happen in the middle
        // of show(), causing inconsistent state. I'll try to submit a Qt patch too.
        qApp->sendPostedEvents();
    } else {
        if ( d->m_parent )
            qApp->removeEventFilter( this );
        d->cancelText.clear();
    }

    KListWidget::setVisible(visible);
}

QRect KCompletionBox::calculateGeometry() const
{
    QRect visualRect;
    if (count() == 0 || !(visualRect = visualItemRect(item(0))).isValid())
        return QRect();

    int x = 0, y = 0;
    int ih = visualRect.height();
    int h = qMin( 15 * ih, (int) count() * ih ) + 2*frameWidth();

    int w = (d->m_parent) ? d->m_parent->width() : KListWidget::minimumSizeHint().width();
    w = qMax( KListWidget::minimumSizeHint().width(), w );

  //### M.O.: Qt4 doesn't actually honor SC_ComboBoxListBoxPopup ???
#if 0
    //If we're inside a combox, Qt by default makes the dropdown
    // as wide as the combo, and gives the style a chance
    // to adjust it. Do that here as well, for consistency
    const QObject* combo;
    if ( d->m_parent && (combo = d->m_parent->parent() ) &&
        qobject_cast<QComboBox*>(combo) )
    {
        const QComboBox* cb = static_cast<const QComboBox*>(combo);

        //Expand to the combo width
        w = qMax( w, cb->width() );

        QPoint parentCorner = d->m_parent->mapToGlobal(QPoint(0, 0));
        QPoint comboCorner  = cb->mapToGlobal(QPoint(0, 0));

        //We need to adjust our horizontal position to also be WRT to the combo
        x += comboCorner.x() -  parentCorner.x();

        //The same with vertical one
        y += cb->height() - d->m_parent->height() +
             comboCorner.y() - parentCorner.y();

        //Ask the style to refine this a bit
        QRect styleAdj = style().querySubControlMetrics(QStyle::CC_ComboBox,
                                    cb, QStyle::SC_ComboBoxListBoxPopup,
                                    QStyleOption(x, y, w, h));
        //QCommonStyle returns QRect() by default, so this is what we get if the
        //style doesn't implement this
        if (!styleAdj.isNull())
            return styleAdj;

    }
#endif
    return QRect(x, y, w, h);
}

QSize KCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

void KCompletionBox::down()
{
    const int i = currentRow();
    if (i < count() - 1) {
        setCurrentRow(i + 1);
    }
}

void KCompletionBox::up()
{
    const int i = currentRow();
    if (i > 0) {
        setCurrentRow(i - 1);
    }
}

void KCompletionBox::pageDown()
{
    //int i = currentItem() + numItemsVisible();
    //i = i > (int)count() - 1 ? (int)count() - 1 : i;
    //setCurrentRow( i );
    moveCursor(QAbstractItemView::MovePageDown , Qt::NoModifier);
}

void KCompletionBox::pageUp()
{
    //int i = currentItem() - numItemsVisible();
    //i = i < 0 ? 0 : i;
    //setCurrentRow( i );

    moveCursor(QAbstractItemView::MovePageUp , Qt::NoModifier);
}

void KCompletionBox::home()
{
    setCurrentRow( 0 );
}

void KCompletionBox::end()
{
    setCurrentRow( count() -1 );
}

void KCompletionBox::setTabHandling( bool enable )
{
    d->tabHandling = enable;
}

bool KCompletionBox::isTabHandling() const
{
    return d->tabHandling;
}

void KCompletionBox::setCancelledText( const QString& text )
{
    d->cancelText = text;
}

QString KCompletionBox::cancelledText() const
{
    return d->cancelText;
}

void KCompletionBox::canceled()
{
    if ( !d->cancelText.isNull() )
        emit userCancelled( d->cancelText );
    if ( isVisible() )
        hide();
}

class KCompletionBoxItem : public QListWidgetItem
{
public:
    //Returns true if dirty.
    bool reuse( const QString& newText )
    {
        if ( text() == newText )
            return false;
        setText( newText );
        return true;
    }
};


void KCompletionBox::insertItems( const QStringList& items, int index )
{
    bool block = signalsBlocked();
    blockSignals( true );
    KListWidget::insertItems( index, items );
    blockSignals( block );
    setCurrentRow(-1);
}

void KCompletionBox::setItems( const QStringList& items )
{
    bool block = signalsBlocked();
    blockSignals( true );

    int rowIndex = 0;

    if (!count()) {
        addItems(items);
    } else {
        // Keep track of whether we need to change anything,
        // so we can avoid a repaint for identical updates,
        // to reduce flicker
        bool dirty = false;

        QStringList::ConstIterator it = items.constBegin();
        const QStringList::ConstIterator itEnd = items.constEnd();

        for ( ; it != itEnd; ++it) {
            if ( rowIndex < count() ) {
                const bool changed = ((KCompletionBoxItem*)item(rowIndex))->reuse( *it );
                dirty = dirty || changed;
            } else {
                dirty = true;
                // Inserting an item is a way of making this dirty
                addItem(*it);
            }
            rowIndex++;
        }

        // If there is an unused item, mark as dirty -> less items now
        if (rowIndex < count()) {
            dirty = true;
        }

        // remove unused items with an index >= rowIndex
        for ( ; rowIndex < count() ; ) {
            QListWidgetItem* item = takeItem(rowIndex);
            Q_ASSERT(item);
            delete item;
        }

        //TODO KDE4 : Port me
        //if (dirty)
        //    triggerUpdate( false );
    }

    if (isVisible() && size().height() != sizeHint().height())
        sizeAndPosition();

    blockSignals(block);
}

void KCompletionBox::slotItemClicked( QListWidgetItem *item )
{
    if ( item )
    {
        hide();
        emit currentTextChanged( item->text() );
        emit activated( item->text() );
    }
}

void KCompletionBox::setActivateOnSelect(bool state)
{
    d->emitSelected = state;
}

bool KCompletionBox::activateOnSelect() const
{
    return d->emitSelected;
}

#include "kcompletionbox.moc"
