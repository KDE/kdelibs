/* This file is part of the KDE libraries
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pastedialog.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QClipboard>

KIO::PasteDialog::PasteDialog( const QString &caption, const QString &label,
                               const QString &value, const QStringList& items,
                               QWidget *parent,
                               bool clipboard )
    : KDialog( parent )
{
    setCaption( caption );
    setButtons( Ok | Cancel );
    setModal( true );
    setDefaultButton( Ok );

    QFrame *frame = new QFrame;
    setMainWidget( frame );

    QVBoxLayout *layout = new QVBoxLayout( frame );

    m_label = new QLabel( label, frame );
    layout->addWidget( m_label );

    m_lineEdit = new KLineEdit( value, frame );
    layout->addWidget( m_lineEdit );

    m_lineEdit->setFocus();
    m_label->setBuddy( m_lineEdit );

    layout->addWidget( new QLabel( i18n( "Data format:" ), frame ) );
    m_comboBox = new KComboBox( frame );
    m_comboBox->addItems( items );
    layout->addWidget( m_comboBox );

    layout->addStretch();

    //connect( m_lineEdit, SIGNAL( textChanged( const QString & ) ),
    //    SLOT( slotEditTextChanged( const QString & ) ) );
    //connect( this, SIGNAL( user1Clicked() ), m_lineEdit, SLOT( clear() ) );

    //slotEditTextChanged( value );
    setMinimumWidth( 350 );

    m_clipboardChanged = false;
    if ( clipboard )
        connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
                 this, SLOT( slotClipboardDataChanged() ) );
}

void KIO::PasteDialog::slotClipboardDataChanged()
{
    m_clipboardChanged = true;
}

QString KIO::PasteDialog::lineEditText() const
{
    return m_lineEdit->text();
}

int KIO::PasteDialog::comboItem() const
{
    return m_comboBox->currentIndex();
}

