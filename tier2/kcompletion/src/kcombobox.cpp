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

#include <QClipboard>
#include <QLineEdit>
#include <QMenu>
#include <QActionEvent>
#include <QUrl>

#include <kcompletionbox.h>
#include <kcursor.h>
#include <klineedit.h>


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
    initWidget();
}

KComboBox::KComboBox( bool rw, QWidget *parent )
    : QComboBox( parent ), d(new KComboBoxPrivate)
{
    initWidget();
    setEditable( rw );
}

KComboBox::~KComboBox()
{
    delete d;
}

void KComboBox::initWidget()
{
    // Permanently set some parameters in the parent object.
    QComboBox::setAutoCompletion( false );

    // Enable context menu by default if widget
    // is editable.
    if (lineEdit()) {
        lineEdit()->setContextMenuPolicy( Qt::DefaultContextMenu );
    }
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
            d->klineEdit->setCompletionMode( KCompletion::CompletionAuto );
            setCompletionMode( KCompletion::CompletionAuto );
        }
        else
        {
            d->klineEdit->setCompletionMode( KCompletion::CompletionPopup );
            setCompletionMode( KCompletion::CompletionPopup );
        }
    }
}

bool KComboBox::autoCompletion() const
{
    return completionMode() == KCompletion::CompletionAuto;
}

#ifndef KDE_NO_DEPRECATED
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
#endif

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

// Not needed anymore
bool KComboBox::eventFilter( QObject* o, QEvent* ev )
{
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


void KComboBox::setEditUrl(const QUrl& url)
{
    QComboBox::setEditText(url.toDisplayString());
}

void KComboBox::addUrl(const QUrl& url)
{
    QComboBox::addItem(url.toDisplayString());
}

void KComboBox::addUrl(const QIcon& icon, const QUrl& url)
{
    QComboBox::addItem(icon, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QUrl& url)
{
    QComboBox::insertItem(index, url.toDisplayString());
}

void KComboBox::insertUrl(int index, const QIcon& icon, const QUrl& url)
{
    QComboBox::insertItem(index, icon, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QUrl& url)
{
    QComboBox::setItemText(index, url.toDisplayString());
}

void KComboBox::changeUrl(int index, const QIcon& icon, const QUrl& url)
{
    QComboBox::setItemIcon(index, icon);
    QComboBox::setItemText(index, url.toDisplayString());
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
        connect( edit, SIGNAL(returnPressed()), SIGNAL(returnPressed()));

    if ( d->klineEdit )
    {
        // someone calling KComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And KCompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect( edit, SIGNAL(destroyed()), SLOT(lineEditDeleted()));

        connect( d->klineEdit, SIGNAL(returnPressed(QString)),
                 SIGNAL(returnPressed(QString)));

        connect( d->klineEdit, SIGNAL(completion(QString)),
                 SIGNAL(completion(QString)) );

        connect( d->klineEdit, SIGNAL(substringCompletion(QString)),
                 SIGNAL(substringCompletion(QString)) );

        connect( d->klineEdit,
                 SIGNAL(textRotation(KCompletionBase::KeyBindingType)),
                 SIGNAL(textRotation(KCompletionBase::KeyBindingType)) );

        connect( d->klineEdit,
                 SIGNAL(completionModeChanged(KCompletion::CompletionMode)),
                 SIGNAL(completionModeChanged(KCompletion::CompletionMode)));

        connect( d->klineEdit,
                 SIGNAL(aboutToShowContextMenu(QMenu*)),
                 SIGNAL(aboutToShowContextMenu(QMenu*)) );

        connect( d->klineEdit,
                 SIGNAL(completionBoxActivated(QString)),
                 SIGNAL(activated(QString)) );

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

