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

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kbuttonbox.h>
#include <kstddirs.h>

#include "passdlg.h"

using namespace KIO;

struct PasswordDialog::PasswordDialogPrivate
{
    QLineEdit* userEdit;
    QLineEdit* passEdit;
    QLabel* prompt;

    bool keep;
    short unsigned int nRow;
};

PasswordDialog::PasswordDialog( QWidget* parent, const char* name,
                                WFlags wflags, bool modal, bool enableKeep )
               :KDialog( parent, name, modal, wflags )
{
    init( QString::null, QString::null, enableKeep );
}

PasswordDialog::PasswordDialog( const QString& prompt, const QString& user,
                                bool enableKeep, bool modal, QWidget* parent,
                                const char* name, WFlags wflags )
               :KDialog( parent, name, modal, wflags )
{
    init ( prompt, user, enableKeep );
}

PasswordDialog::PasswordDialog( QWidget* parent, const char* name, bool modal,
                                 WFlags wflags, const QString& head,
                                 const QString& user, const QString& )
               :KDialog( parent, name, modal, wflags )
{
    init( head, user, false );
}

PasswordDialog::~PasswordDialog()
{
    delete d;
}

void PasswordDialog::init( const QString& prompt, const QString& user,
                           bool enableKeep  )
{
    d = new PasswordDialogPrivate;
    d->nRow = 0;
    d->keep = false;

    KConfig* cfg = KGlobal::config();
    KConfigGroupSaver saver( cfg, "Passwords" );

    m_pLay = new QGridLayout( this, 12, 3, 8, 0);
    m_pLay->addColSpacing(1, 10);

    // Row 0: pixmap  prompt
    QLabel* lbl;
    QPixmap pix(locate("data", QString::fromLatin1("kdeui/pics/keys.png")));
    if ( !pix.isNull() )
    {
        lbl = new QLabel( this );
        lbl->setPixmap( pix );
        lbl->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
        lbl->setFixedSize( lbl->sizeHint() );
        m_pLay->addWidget( lbl, 0, 0, Qt::AlignLeft );
    }
    d->prompt = new QLabel( this );
    d->prompt->setAlignment( Qt::AlignLeft|Qt::AlignVCenter|Qt::WordBreak );
    m_pLay->addWidget( d->prompt, 0, 2, Qt::AlignLeft );
    if ( prompt.isEmpty() )
        setPrompt( i18n( "You need to supply a username and a password" ) );
    else
        setPrompt( prompt );

    // Row 1: Row Spacer
    m_pLay->addRowSpacing( 1, 10 );

    // Row 2-3: Reserved for an additional comment

    // Row 4: Username field
    lbl = new QLabel( i18n("&Username:"), this );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    m_pLay->addWidget( lbl, 4, 0 );
    QHBoxLayout* hbl = new QHBoxLayout();
    m_pLay->addLayout( hbl, 4, 2 );
    d->userEdit = new QLineEdit( this );
    lbl->setBuddy( d->userEdit );
    QSize s = d->userEdit->sizeHint();
    d->userEdit->setFixedHeight( s.height() );
    d->userEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->userEdit );
    hbl->addWidget( d->userEdit, 12 );

    // Row 5: Row spacer
    m_pLay->addRowSpacing( 5, 5 );

    // Row 5: Password field
    lbl = new QLabel( i18n("&Password:"), this );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    m_pLay->addWidget( lbl, 6, 0 );
    hbl = new QHBoxLayout();
    m_pLay->addLayout( hbl, 6, 2 );
    d->passEdit = new QLineEdit( this );
    if ( cfg->readEntry("EchoMode", "OneStar") == "NoEcho" )
        d->passEdit->setEchoMode( QLineEdit::NoEcho );
    else
        d->passEdit->setEchoMode( QLineEdit::Password );
    lbl->setBuddy( d->passEdit );
    s = d->passEdit->sizeHint();
    d->passEdit->setFixedHeight( s.height() );
    d->passEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->passEdit );
    hbl->addWidget( d->passEdit, 12 );

    if ( enableKeep )
    {
        // Row 7: Add spacer
        m_pLay->addRowSpacing( 7, 10 );
        // Row 8: Keep Password
        QCheckBox *cb = new QCheckBox( i18n("&Keep Password"), this );
        cb->setFixedSize( cb->sizeHint() );
        KConfigGroupSaver saver( cfg, "Passwords" );
        d->keep = cfg->readBoolEntry("Keep", false );
        cb->setChecked( d->keep );
        connect(cb, SIGNAL(toggled( bool )), SLOT(slotKeep( bool )));
        m_pLay->addWidget( cb, 8, 2 );
    }

    // Row 9: Add spacer
    m_pLay->addRowSpacing( 9, 8 );

    // Row 10: Add a separator
    hbl = new QHBoxLayout();
    m_pLay->addMultiCellLayout( hbl, 10, 10, 0, 2 );
    lbl = new QLabel( this );
    lbl->setFrameStyle( QFrame::HLine|QFrame::Sunken );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignCenter );
    lbl->setFixedHeight( lbl->sizeHint().height() );
    hbl->addWidget( lbl );

    // Row 11: Add spacer
    m_pLay->addRowSpacing( 11, 2 );

    // Row 12: Add buttons
    KButtonBox *bbox = new KButtonBox( this );
    (void)bbox->addButton( i18n("&OK"), this, SLOT(accept()) );
    (void)bbox->addButton( i18n("&Cancel"), this, SLOT(reject()) );
    bbox->addStretch(4);
    bbox->layout();
    m_pLay->addWidget( bbox, 12, 2 );

    // Configure necessary key-bindings and connect necessar slots and signals
    QAccel *ac = new QAccel(this);
    ac->connectItem( ac->insertItem(Qt::Key_Escape), this, SLOT(reject()) );
    connect( d->userEdit, SIGNAL(returnPressed()), d->passEdit, SLOT(setFocus()) );
    connect( d->passEdit, SIGNAL(returnPressed()), SLOT(accept()) );

    if ( !user.isEmpty() )
    {
        d->userEdit->setText( user );
        d->passEdit->setFocus();
    }
    else
        d->userEdit->setFocus();

    setFixedSize( sizeHint() );
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

    QLabel* lbl = new QLabel( label, this);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    lbl->setFixedSize( lbl->sizeHint() );
    m_pLay->addWidget( lbl, d->nRow+2, 0, Qt::AlignLeft );
    lbl = new QLabel( comment, this);
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignLeft|Qt::WordBreak );
    int w = QMIN( d->prompt->sizeHint().width(), 250 );
    lbl->setFixedSize( w, lbl->heightForWidth(w) );
    m_pLay->addWidget( lbl, d->nRow+2, 2, Qt::AlignLeft );
    m_pLay->addRowSpacing( 3, 10 ); // Add a spacer
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
    PasswordDialog* dlg = new PasswordDialog( prompt, user );
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


#include "passdlg.moc"
