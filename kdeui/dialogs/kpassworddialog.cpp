/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2007 Olivier Goffart <ogoffart at kde.org>

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
#include "kpassworddialog.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QTextDocument>
#include <QTimer>

#include <kcombobox.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <khbox.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <ktitlewidget.h>

#include "ui_kpassworddialog.h"

/** @internal */
class KPasswordDialog::KPasswordDialogPrivate
{
public:
    KPasswordDialogPrivate(KPasswordDialog *q)
        : q(q),
          userEditCombo(0),
          pixmapLabel(0),
          commentRow(0)
    {}

    void actuallyAccept();
    void activated( const QString& userName );

    void updateFields();
    void init();

    KPasswordDialog *q;
    KPasswordDialogFlags m_flags;
    Ui_KPasswordDialog ui;
    QMap<QString,QString> knownLogins;
    KComboBox* userEditCombo;
    QLabel* pixmapLabel;
    unsigned int commentRow;
};

KPasswordDialog::KPasswordDialog( QWidget* parent ,
                                  const KPasswordDialogFlags& flags,
                                  const KDialog::ButtonCodes otherButtons )
   : KDialog( parent ), d(new KPasswordDialogPrivate(this))
{
    setCaption( i18n("Password") );
    setWindowIcon(KIcon("dialog-password"));
    setButtons( Ok | Cancel | otherButtons );
    setDefaultButton( Ok );
    d->m_flags = flags;
    d->init ();
}

KPasswordDialog::~KPasswordDialog()
{
    delete d;
}

void KPasswordDialog::KPasswordDialogPrivate::updateFields()
{
    if (q->anonymousMode())
    {
        ui.userEdit->setEnabled( false );
        ui.domainEdit->setEnabled( false );
        ui.passEdit->setEnabled( false );
    	ui.keepCheckBox->setEnabled( false );
    }
    else
    {
        ui.userEdit->setEnabled(!( m_flags & KPasswordDialog::UsernameReadOnly ));
        ui.domainEdit->setEnabled(!( m_flags & KPasswordDialog::DomainReadOnly ));
        ui.passEdit->setEnabled( true );
    	ui.keepCheckBox->setEnabled( true );
    }
}

void KPasswordDialog::KPasswordDialogPrivate::init()
{
    ui.setupUi( q->mainWidget() );
    ui.errorMessage->setHidden(true);

    // Row 4: Username field
    if ( m_flags & KPasswordDialog::ShowUsernameLine ) {
        ui.userEdit->setFocus();
        QObject::connect( ui.userEdit, SIGNAL(returnPressed()), ui.passEdit, SLOT(setFocus()) );
    } else {
        ui.userNameLabel->hide();
        ui.userEdit->hide();
        ui.domainLabel->hide();
        ui.domainEdit->hide();
        ui.passEdit->setFocus();
    }

    if ( !( m_flags & KPasswordDialog::ShowAnonymousLoginCheckBox ) )
    {
        ui.anonymousCheckBox->hide();
    }
    else
    {
        QObject::connect( ui.anonymousCheckBox, SIGNAL(stateChanged (int)), q, SLOT(updateFields()) );
    }
    
    if ( !( m_flags & KPasswordDialog::ShowDomainLine ) )
    {
        ui.domainLabel->hide();
        ui.domainEdit->hide();
    }    
    
    if ( !( m_flags & KPasswordDialog::ShowKeepPassword ) )
    {
        ui.keepCheckBox->hide();
    }

    updateFields();
    
    QRect desktop = KGlobalSettings::desktopGeometry(q->topLevelWidget());
    q->setMinimumWidth(qMin(1000, qMax(q->sizeHint().width(), desktop.width() / 4)));
    q->setPixmap(KIcon("dialog-password").pixmap(KIconLoader::SizeHuge));
}

void KPasswordDialog::setPixmap(const QPixmap &pixmap)
{
    if ( !d->pixmapLabel )
    {
        d->pixmapLabel = new QLabel( mainWidget() );
        d->pixmapLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop );
        d->ui.hboxLayout->insertWidget( 0, d->pixmapLabel );
    }

    d->pixmapLabel->setPixmap( pixmap );
}

QPixmap KPasswordDialog::pixmap() const
{
    if ( !d->pixmapLabel ) {
        return QPixmap();
    }

    return *d->pixmapLabel->pixmap();
}


void KPasswordDialog::setUsername(const QString& user)
{
    d->ui.userEdit->setText(user);
    if ( user.isEmpty() )
        return;

    d->activated(user);
    if ( d->ui.userEdit->isVisibleTo( this ) )
    {
        d->ui.passEdit->setFocus();
    }
}


QString KPasswordDialog::username() const
{
    return d->ui.userEdit->text();
}

QString KPasswordDialog::password() const
{
    return d->ui.passEdit->text();
}

void KPasswordDialog::setDomain(const QString& domain)
{
    d->ui.domainEdit->setText(domain);
}

QString KPasswordDialog::domain() const
{
    return d->ui.domainEdit->text();
}

void KPasswordDialog::setAnonymousMode(bool anonymous)
{
    d->ui.anonymousCheckBox->setChecked( anonymous );
}

bool KPasswordDialog::anonymousMode() const
{
    return d->ui.anonymousCheckBox->isChecked();
}


void KPasswordDialog::setKeepPassword( bool b )
{
    d->ui.keepCheckBox->setChecked( b );
}

bool KPasswordDialog::keepPassword() const
{
    return d->ui.keepCheckBox->isChecked();
}

void KPasswordDialog::addCommentLine( const QString& label,
                                      const QString& comment )
{
    int gridMarginLeft, gridMarginTop, gridMarginRight, gridMarginBottom;
    d->ui.formLayout->getContentsMargins(&gridMarginLeft, &gridMarginTop, &gridMarginRight, &gridMarginBottom);

    int spacing = d->ui.formLayout->horizontalSpacing();
    if (spacing < 0) {
        // same inter-column spacing for all rows, see comment in qformlayout.cpp
        spacing = style()->combinedLayoutSpacing(QSizePolicy::Label, QSizePolicy::LineEdit, Qt::Horizontal, 0, this);
    }
    
    QLabel* c = new QLabel(comment, mainWidget());
    c->setWordWrap(true);

    d->ui.formLayout->insertRow(d->commentRow, label, c);
    ++d->commentRow;

    // cycle through column 0 widgets and see the max width so we can set the minimum height of
    // column 2 wordwrapable labels
    int firstColumnWidth = 0;
    for (int i = 0; i < d->ui.formLayout->rowCount(); ++i) {
        QLayoutItem *li = d->ui.formLayout->itemAt(i, QFormLayout::LabelRole);
        if (li) {
            QWidget *w = li->widget();
            if (w && !w->isHidden()) {
                firstColumnWidth = qMax(firstColumnWidth, w->sizeHint().width());
            }
        }
    }
    for (int i = 0; i < d->ui.formLayout->rowCount(); ++i) {
        QLayoutItem *li = d->ui.formLayout->itemAt(i, QFormLayout::FieldRole);
        if (li) {
            QLabel *l = qobject_cast<QLabel*>(li->widget());
            if (l && l->wordWrap()) {
                int w = sizeHint().width() - firstColumnWidth - ( 2 * marginHint() ) - gridMarginLeft - gridMarginRight - spacing;
                l->setMinimumSize( w, l->heightForWidth(w) );
            }
        }
    }
}

void KPasswordDialog::showErrorMessage( const QString& message, const ErrorType type )
{
    d->ui.errorMessage->setText( message, KTitleWidget::ErrorMessage );

    QFont bold = font();
    bold.setBold( true );
    switch ( type ) {
        case PasswordError:
            d->ui.passwordLabel->setFont( bold );
            d->ui.passEdit->clear();
            d->ui.passEdit->setFocus();
            break;
        case UsernameError:
            if ( d->ui.userEdit->isVisibleTo( this ) )
            {
                d->ui.userNameLabel->setFont( bold );
                d->ui.userEdit->setFocus();
            }
            break;
        case DomainError:
            if ( d->ui.domainEdit->isVisibleTo( this ) )
            {
                d->ui.domainLabel->setFont( bold );
                d->ui.domainEdit->setFocus();
            }            
            break;
        case FatalError:
            d->ui.userNameLabel->setEnabled( false );
            d->ui.userEdit->setEnabled( false );
            d->ui.passwordLabel->setEnabled( false );
            d->ui.passEdit->setEnabled( false );
            d->ui.keepCheckBox->setEnabled( false );
            enableButton( Ok, false );
            break;
        default:
            break;
    }
    adjustSize();
}

void KPasswordDialog::setPrompt(const QString& prompt)
{
    d->ui.prompt->setText( prompt );
    d->ui.prompt->setWordWrap( true );
    d->ui.prompt->setMinimumHeight( d->ui.prompt->heightForWidth( width() -  ( 2 * marginHint() ) ) );
}

QString KPasswordDialog::prompt() const
{
    return d->ui.prompt->text();
}

void KPasswordDialog::setPassword(const QString &p)
{
    d->ui.passEdit->setText(p);
}

void KPasswordDialog::setUsernameReadOnly( bool readOnly )
{
    d->ui.userEdit->setReadOnly( readOnly );

    if ( readOnly && d->ui.userEdit->hasFocus() ) {
        d->ui.passEdit->setFocus();
    }
}

void KPasswordDialog::setKnownLogins( const QMap<QString, QString>& knownLogins )
{
    const int nr = knownLogins.count();
    if ( nr == 0 ) {
        return;
    }

    if ( nr == 1 ) {
        d->ui.userEdit->setText( knownLogins.begin().key() );
        setPassword( knownLogins.begin().value() );
        return;
    }

    Q_ASSERT( !d->ui.userEdit->isReadOnly() );
    if ( !d->userEditCombo ) {
        d->ui.formLayout->removeWidget(d->ui.userEdit);
        delete d->ui.userEdit;
        d->userEditCombo = new KComboBox( true, mainWidget() );
        d->ui.userEdit = d->userEditCombo->lineEdit();
//        QSize s = d->userEditCombo->sizeHint();
//        d->ui.userEditCombo->setFixedHeight( s.height() );
//        d->ui.userEditCombo->setMinimumWidth( s.width() );
        d->ui.userNameLabel->setBuddy( d->userEditCombo );
        d->ui.formLayout->setWidget( d->commentRow, QFormLayout::FieldRole, d->userEditCombo );
        setTabOrder( d->ui.userEdit, d->ui.anonymousCheckBox );
        setTabOrder( d->ui.anonymousCheckBox, d->ui.domainEdit );
        setTabOrder( d->ui.domainEdit, d->ui.passEdit );
        setTabOrder( d->ui.passEdit, d->ui.keepCheckBox );
        connect( d->ui.userEdit, SIGNAL(returnPressed()), d->ui.passEdit, SLOT(setFocus()) );
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->addItems( knownLogins.keys() );
    d->userEditCombo->setFocus();

    connect( d->userEditCombo, SIGNAL( activated( const QString& ) ),
             this, SLOT( activated( const QString& ) ) );
}

void KPasswordDialog::KPasswordDialogPrivate::activated( const QString& userName )
{
    QMap<QString, QString>::ConstIterator it = knownLogins.constFind( userName );
    if ( it != knownLogins.constEnd() ) {
        q->setPassword( it.value() );
    }
}

void KPasswordDialog::accept()
{
    if (!d->ui.errorMessage->isHidden()) d->ui.errorMessage->setText( QString() );

    // reset the font in case we had an error previously
    if (!d->ui.passwordLabel->isHidden()) d->ui.passwordLabel->setFont( font() );
    if (!d->ui.passwordLabel->isHidden()) d->ui.userNameLabel->setFont( font() );

    // we do this to allow the error message, if any, to go away
    // checkPassword() may block for a period of time
    QTimer::singleShot( 0, this, SLOT(actuallyAccept()) );
}

void KPasswordDialog::KPasswordDialogPrivate::actuallyAccept()
{
    if ( !q->checkPassword() )
    {
        return;
    }

    bool keep = ui.keepCheckBox->isVisibleTo( q ) && ui.keepCheckBox->isChecked();
    emit q->gotPassword( q->password(), keep);

    if ( ui.userEdit->isVisibleTo( q ) ) {
        emit q->gotUsernameAndPassword( q->username(), q->password() , keep);
    }

    q->KDialog::accept();
}

bool KPasswordDialog::checkPassword()
{
    return true;
}

#include "kpassworddialog.moc"
