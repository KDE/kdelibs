/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qhbox.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kbuttonbox.h>
#include <kstandarddirs.h>
#include <kseparator.h>

#include "passdlg.h"

using namespace KIO;

struct PasswordDialog::PasswordDialogPrivate
{
    QGridLayout *layout;
    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* prompt;

    bool keep;
    short unsigned int nRow;
};

PasswordDialog::PasswordDialog( const QString& prompt, const QString& user,
                                bool enableKeep, bool modal, QWidget* parent,
                                const char* name )
               :KDialogBase( parent, name, modal, i18n("Password"), Ok|Cancel, Ok, true)
{
    init ( prompt, user, enableKeep );
}

PasswordDialog::~PasswordDialog()
{
    delete d;
}

void PasswordDialog::init( const QString& prompt, const QString& user,
                           bool enableKeep  )
{
    QWidget *main = makeMainWidget();

    d = new PasswordDialogPrivate;
    d->keep = false;
    d->nRow = 0;

    KConfig* cfg = KGlobal::config();
    KConfigGroupSaver saver( cfg, "Passwords" );

    d->layout = new QGridLayout( main, 9, 3, spacingHint(), marginHint());
    d->layout->addColSpacing(1, 5);

    // Row 0: pixmap  prompt
    QLabel* lbl;
    QPixmap pix(locate("data", QString::fromLatin1("kdeui/pics/keys.png")));
    if ( !pix.isNull() )
    {
        lbl = new QLabel( main );
        lbl->setPixmap( pix );
        lbl->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
        lbl->setFixedSize( lbl->sizeHint() );
        d->layout->addWidget( lbl, 0, 0, Qt::AlignLeft );
    }
    d->prompt = new QLabel( main );
    d->prompt->setAlignment( Qt::AlignLeft|Qt::AlignVCenter|Qt::WordBreak );
    d->layout->addWidget( d->prompt, 0, 2, Qt::AlignLeft );
    if ( prompt.isEmpty() )
        setPrompt( i18n( "You need to supply a username and a password" ) );
    else
        setPrompt( prompt );

    // Row 1: Row Spacer
    d->layout->addRowSpacing( 1, 7 );

    // Row 2-3: Reserved for an additional comment

    // Row 4: Username field
    lbl = new QLabel( i18n("&Username:"), main );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    QHBox* hbox = new QHBox( main );
    d->userEdit = new QLineEdit( hbox );
    lbl->setBuddy( d->userEdit );
    QSize s = d->userEdit->sizeHint();
    d->userEdit->setFixedHeight( s.height() );
    d->userEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->userEdit );
    d->layout->addWidget( lbl, 4, 0 );
    d->layout->addWidget( hbox, 4, 2 );

    // Row 5: Row spacer
    d->layout->addRowSpacing( 5, 4 );

    // Row 6: Password field
    lbl = new QLabel( i18n("&Password:"), main );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    hbox = new QHBox( main );
    d->passEdit = new QLineEdit( hbox );
    if ( cfg->readEntry("EchoMode", "OneStar") == "NoEcho" )
        d->passEdit->setEchoMode( QLineEdit::NoEcho );
    else
        d->passEdit->setEchoMode( QLineEdit::Password );
    lbl->setBuddy( d->passEdit );
    s = d->passEdit->sizeHint();
    d->passEdit->setFixedHeight( s.height() );
    d->passEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->passEdit );
    d->layout->addWidget( lbl, 6, 0 );
    d->layout->addWidget( hbox, 6, 2 );

    if ( enableKeep )
    {
        // Row 7: Add spacer
        d->layout->addRowSpacing( 7, 4 );
        // Row 8: Keep Password
        hbox = new QHBox( main );
        QCheckBox *cb = new QCheckBox( i18n("&Keep password"), hbox );
        cb->setFixedSize( cb->sizeHint() );
        d->keep = cfg->readBoolEntry("Keep", false );
        cb->setChecked( d->keep );
        connect(cb, SIGNAL(toggled( bool )), SLOT(slotKeep( bool )));
        d->layout->addWidget( hbox, 8, 2 );
    }

    // Configure necessary key-bindings and connect necessar slots and signals
    connect( d->userEdit, SIGNAL(returnPressed()), d->passEdit, SLOT(setFocus()) );
    connect( d->passEdit, SIGNAL(returnPressed()), SLOT(slotOk()) );

    if ( !user.isEmpty() )
    {
        d->userEdit->setText( user );
        d->passEdit->setFocus();
    }
    else
        d->userEdit->setFocus();

//    setFixedSize( sizeHint() );
}

QString PasswordDialog::username() const
{
    return d->userEdit->text();
}

QString PasswordDialog::password() const
{
    return d->passEdit->text();
}

bool PasswordDialog::keepPassword() const
{
    return d->keep;
}

void PasswordDialog::addCommentLine( const QString& label,
                                     const QString comment )
{
    if (d->nRow > 0)
        return;

    QWidget *main = mainWidget();

    QLabel* lbl = new QLabel( label, main);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    lbl->setFixedSize( lbl->sizeHint() );
    d->layout->addWidget( lbl, d->nRow+2, 0, Qt::AlignLeft );
    lbl = new QLabel( comment, main);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignLeft|Qt::WordBreak );
    int w = QMIN( d->prompt->sizeHint().width(), 250 );
    lbl->setFixedSize( w, lbl->heightForWidth(w) );
    d->layout->addWidget( lbl, d->nRow+2, 2, Qt::AlignLeft );
    d->layout->addRowSpacing( 3, 10 ); // Add a spacer
    d->nRow++;
}

void PasswordDialog::slotKeep( bool keep )
{
    d->keep = keep;
}

void PasswordDialog::setPrompt(const QString& prompt)
{
    d->prompt->setText(prompt);
    int w = QMIN( d->prompt->sizeHint().width(), 250 );
    d->prompt->setFixedSize( w, d->prompt->heightForWidth( w ) );
}

void PasswordDialog::setUserReadOnly( bool readOnly )
{
    d->userEdit->setReadOnly( readOnly );
    if ( readOnly && d->userEdit->hasFocus() )
        d->passEdit->setFocus();
}

int PasswordDialog::getNameAndPassword( QString& user, QString& pass, bool* keep,
                                        const QString& prompt, bool readOnly,
                                        const QString& caption,
                                        const QString& comment,
                                        const QString& label )
{
    PasswordDialog* dlg;
    if( keep )
        dlg = new PasswordDialog( prompt, user, (*keep) );
    else
        dlg = new PasswordDialog( prompt, user );

    if ( !caption.isEmpty() )
        dlg->setPlainCaption( caption );
    else
        dlg->setPlainCaption( i18n("Authorization Dialog") );

    if ( !comment.isEmpty() )
        dlg->addCommentLine( label, comment );

    if ( readOnly )
        dlg->setUserReadOnly( readOnly );

    int ret = dlg->exec();
    if ( ret == Accepted )
    {
        user = dlg->username();
        pass = dlg->password();
        if ( keep ) { (*keep) = dlg->keepPassword(); }
    }
    delete dlg;
    return ret;
 }

void PasswordDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }


#include "passdlg.moc"
