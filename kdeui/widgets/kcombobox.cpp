/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcombobox.h"

#include <QtGui/QClipboard>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QApplication>
#include <QtGui/QActionEvent>

#include <kselectaction.h>
#include <kcompletionbox.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurl.h>
#include <kicon.h>

#include <kdebug.h>

class KComboBox::KComboBoxPrivate
{
public:
    KComboBoxPrivate() : klineEdit(0L), trapReturnKey(false)
    {
    }
    ~KComboBoxPrivate()
    {
    }

    KLineEdit *klineEdit;
    bool trapReturnKey;
};

KComboBox::KComboBox( QWidget *parent )
    : QComboBox( parent ), d(new KComboBoxPrivate)
{
    init();
}

KComboBox::KComboBox( bool rw, QWidget *parent )
    : QComboBox( parent ), d(new KComboBoxPrivate)
{
    init();
    setEditable( rw );
}

KComboBox::~KComboBox()
{
    delete d;
}

void KComboBox::init()
{
    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );
}


bool KComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i )
    {
        if ( itemText(i) == _text )
            return true;
    }
    return false;
}

int KComboBox::cursorPosition() const
{
    return ( lineEdit() ) ? lineEdit()->cursorPosition() : -1;
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    if ( d->klineEdit )
    {
        if ( autocomplete )
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
            setCompletionMode( KGlobalSettings::CompletionAuto );
        }
        else
        {
            d->klineEdit->setCompletionMode( KGlobalSettings::completionMode() );
            setCompletionMode( KGlobalSettings::completionMode() );
        }
    }
}

bool KComboBox::autoCompletion() const
{
    return completionMode() == KGlobalSettings::CompletionAuto;
}

void KComboBox::setContextMenuEnabled( bool showMenu )
{
    if( d->klineEdit )
        d->klineEdit->setContextMenuEnabled( showMenu );
}


void KComboBox::setUrlDropsEnabled( bool enable )
{
    if ( d->klineEdit )
        d->klineEdit->setUrlDropsEnabled( enable );
}

bool KComboBox::urlDropsEnabled() const
{
    return d->klineEdit && d->klineEdit->urlDropsEnabled();
}


void KComboBox::setCompletedText( const QString& text, bool marked )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text, marked );
}

void KComboBox::setCompletedText( const QString& text )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text );
}

void KComboBox::makeCompletion( const QString& text )
{
    if( d->klineEdit )
        d->klineEdit->makeCompletion( text );

    else // read-only combo completion
    {
        if( text.isNull() || !view() )
            return;

	view()->keyboardSearch(text);
    }
}

void KComboBox::rotateText( KCompletionBase::KeyBindingType type )
{
    if ( d->klineEdit )
        d->klineEdit->rotateText( type );
}

bool KComboBox::eventFilter( QObject* o, QEvent* ev )
{
    // hack for the fact that QWidgetAction does not sync all its created widgets
    // to its enabled state, just QToolButtons (Qt 4.4.3)
    if( ev->type() == QEvent::ActionChanged )
    {
        KSelectAction* selectAction = qobject_cast<KSelectAction*>( o );
        if ( selectAction )
            setEnabled( selectAction->isEnabled() );
    }

    return QComboBox::eventFilter( o, ev );
}

void KComboBox::setTrapReturnKey( bool grab )
{
    d->trapReturnKey = grab;

    if ( d->klineEdit )
        d->klineEdit->setTrapReturnKey( grab );
    else
        qWarning("KComboBox::setTrapReturnKey not supported with a non-KLineEdit.");
}

bool KComboBox::trapReturnKey() const
{
    return d->trapReturnKey;
}


void KComboBox::setEditUrl( const KUrl& url )
{
    QComboBox::setEditText( url.prettyUrl() );
}

void KComboBox::addUrl( const KUrl& url )
{
    QComboBox::addItem( url.prettyUrl() );
}

void KComboBox::addUrl( const QIcon& icon, const KUrl& url )
{
    QComboBox::addItem( icon, url.prettyUrl() );
}

void KComboBox::insertUrl( int index, const KUrl& url )
{
    QComboBox::insertItem( index, url.prettyUrl() );
}

void KComboBox::insertUrl( int index, const QIcon& icon, const KUrl& url )
{
    QComboBox::insertItem( index, icon, url.prettyUrl() );
}

void KComboBox::changeUrl( int index, const KUrl& url )
{
    QComboBox::setItemText( index, url.prettyUrl() );
}

void KComboBox::changeUrl( int index, const QIcon& icon, const KUrl& url )
{
	QComboBox::setItemIcon( index, icon );
	QComboBox::setItemText( index, url.prettyUrl() );
}

void KComboBox::setCompletedItems( const QStringList& items, bool autosubject )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedItems( items, autosubject );
}

KCompletionBox * KComboBox::completionBox( bool create )
{
    if ( d->klineEdit )
        return d->klineEdit->completionBox( create );
    return 0;
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KComboBox::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QComboBox::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( lineEdit(), true, true );
}

void KComboBox::wheelEvent( QWheelEvent *ev )
{
    // Not necessary anymore
    QComboBox::wheelEvent( ev );
}

QSize KComboBox::minimumSizeHint() const
{
    QSize size = QComboBox::minimumSizeHint();
    if (isEditable() && d->klineEdit) {
        // if it's a KLineEdit and it's editable add the clear button size
        // to the minimum size hint, otherwise looks ugly because the
        // clear button will cover the last 2/3 letters of the biggest entry
        QSize bs = d->klineEdit->clearButtonUsedSize();
        if (bs.isValid()) {
            size.rwidth() += bs.width();
            size.rheight() = qMax(size.height(), bs.height());
        }
    }
    return size;
}

void KComboBox::setLineEdit( QLineEdit *edit )
{
    if ( !isEditable() && edit &&
         !qstrcmp( edit->metaObject()->className(), "QLineEdit" ) )
    {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable( true ), which causes QComboBox to set up
        // a dumb QLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        KLineEdit* kedit = new KLineEdit( this );

        if ( isEditable() ) {
            kedit->setClearButtonShown( true );
        }

        edit = kedit;
    }

    QComboBox::setLineEdit( edit );
    d->klineEdit = qobject_cast<KLineEdit*>( edit );
    setDelegate( d->klineEdit );

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit)
        connect( edit, SIGNAL( returnPressed() ), SIGNAL( returnPressed() ));

    if ( d->klineEdit )
    {
        // someone calling KComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect( edit, SIGNAL( destroyed() ), SLOT( lineEditDeleted() ));

        connect( d->klineEdit, SIGNAL( returnPressed( const QString& )),
                 SIGNAL( returnPressed( const QString& ) ));

        connect( d->klineEdit, SIGNAL( completion( const QString& )),
                 SIGNAL( completion( const QString& )) );

        connect( d->klineEdit, SIGNAL( substringCompletion( const QString& )),
                 SIGNAL( substringCompletion( const QString& )) );

        connect( d->klineEdit,
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )),
                 SIGNAL( textRotation( KCompletionBase::KeyBindingType )) );

        connect( d->klineEdit,
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion )),
                 SIGNAL( completionModeChanged( KGlobalSettings::Completion)));

        connect( d->klineEdit,
                 SIGNAL( aboutToShowContextMenu( QMenu * )),
                 SIGNAL( aboutToShowContextMenu( QMenu * )) );

        connect( d->klineEdit,
                 SIGNAL( completionBoxActivated( const QString& )),
                 SIGNAL( activated( const QString& )) );

        d->klineEdit->setTrapReturnKey( d->trapReturnKey );
    }
}

void KComboBox::setCurrentItem( const QString& item, bool insert, int index )
{
    int sel = -1;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i)
    {
        if (itemText(i) == item)
        {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert)
    {
        if (index >= 0) {
            insertItem(index, item);
            sel = index;
        } else {
            addItem(item);
            sel = count() - 1;
        }
    }
    setCurrentIndex(sel);
}

void KComboBox::lineEditDeleted()
{
    // yes, we need those ugly casts due to the multiple inheritance
    // sender() is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const KCompletionBase *base = static_cast<const KCompletionBase*>( static_cast<const KLineEdit*>( sender() ));

    // is it our delegate, that is destroyed?
    if ( base == delegate() )
        setDelegate( 0L );
}

void KComboBox::setEditable(bool editable)
{
    if (editable) {
        // Create a KLineEdit instead of a QLineEdit
        // Compared to QComboBox::setEditable, we might be missing the SH_ComboBox_Popup code though...
        // If a style needs this, then we'll need to call QComboBox::setEditable and then setLineEdit again
        KLineEdit *edit = new KLineEdit( this );
        edit->setClearButtonShown( true );
        setLineEdit( edit );
    } else {
        QComboBox::setEditable(editable);
    }
}

#include "kcombobox.moc"
