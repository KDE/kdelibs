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
#include <QTextLayout>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <khbox.h>
#include <kdebug.h>


/** @internal */
struct KPasswordDialog::KPasswordDialogPrivate
{
    QGridLayout *layout;
    QLineEdit* userEdit;
    KLineEdit* passEdit;
    QLabel* userNameLabel;
    QLabel* prompt;
    QLabel* pixmapLabel;
    QCheckBox* keepCheckBox;
    QMap<QString,QString> knownLogins;
    KComboBox* userEditCombo;
    KHBox* userNameHBox;

    short unsigned int nRow;
};

KPasswordDialog::KPasswordDialog( QWidget* parent ,  const KPasswordDialogFlags& flags )
   :KDialog( parent ),d(new KPasswordDialogPrivate)
{
    setCaption( i18n("Password") );
    setButtons( Ok | Cancel );
    showButtonSeparator( true );
    setDefaultButton( Ok );
    init ( flags );
}



KPasswordDialog::~KPasswordDialog()
{
    delete d;
}

void KPasswordDialog::init( const KPasswordDialogFlags& flags )
{
    QWidget *main = mainWidget();

    d->nRow = 0;
    d->keepCheckBox = 0;



    d->layout = new QGridLayout( main );
    d->layout->setSpacing( spacingHint() );
    d->layout->setMargin( marginHint() );
    d->layout->addItem(new QSpacerItem(5,0),0,1); //addColSpacing(1, 5);

    // Row 0: pixmap  prompt
    QPixmap pix( kapp->iconLoader()->loadIcon( "password", K3Icon::NoGroup, K3Icon::SizeHuge, 0, 0, true));
    d->pixmapLabel = new QLabel( main );
    d->pixmapLabel->setPixmap( pix );
    d->pixmapLabel->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
    d->pixmapLabel->setFixedSize( d->pixmapLabel->sizeHint() );
    d->layout->addWidget( d->pixmapLabel, 0, 0, Qt::AlignLeft );

    d->prompt = new QLabel( main );
    d->prompt->setAlignment( Qt::AlignLeft|Qt::AlignVCenter);
    d->prompt->setWordWrap( true );
    d->layout->addWidget( d->prompt, 0, 2, Qt::AlignLeft );
    setPrompt( i18n( "You need to supply a username and a password" ) );

    // Row 1: Row Spacer
    d->layout->addItem(new QSpacerItem(0,7),1,0); //addRowSpacing( 1, 7 );

    // Row 2-3: Reserved for an additional comment

    // Row 4: Username field
    if(flags & ShowUsernameLine)
    {
        d->userNameLabel = new QLabel( i18n("&Username:"), main );
        d->userNameLabel->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
        d->userNameLabel->setFixedSize( d->userNameLabel->sizeHint() );
        d->userNameHBox = new KHBox( main );
    
        d->userEdit = new KLineEdit( d->userNameHBox );
        QSize s = d->userEdit->sizeHint();
        d->userEdit->setFixedHeight( s.height() );
        d->userEdit->setMinimumWidth( s.width() );
        d->userNameLabel->setBuddy( d->userEdit );
        d->layout->addWidget( d->userNameLabel, 4, 0 );
        d->layout->addWidget( d->userNameHBox, 4, 2 );

        d->userEdit->setReadOnly( flags & UsernameReadOnly );
        
        // Row 5: Row spacer
        d->layout->addItem(new QSpacerItem(0,4),5,0); //addRowSpacing( 5, 4 );
    }
    else
    {
        d->userNameLabel = 0L;
        d->userEdit =0;
    }


    // Row 6: Password field
    QLabel *lbl = new QLabel( i18n("&Password:"), main );
    lbl->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    KHBox* hbox = new KHBox( main );
    d->passEdit = new KLineEdit( hbox );
    d->passEdit->setPasswordMode( true );
    
    QSize s = d->passEdit->sizeHint();
    d->passEdit->setFixedHeight( s.height() );
    d->passEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->passEdit );
    d->layout->addWidget( lbl, 6, 0 );
    d->layout->addWidget( hbox, 6, 2 );

    if ( flags & ShowKeepPassword )
    {
        // Row 7: Add spacer
        d->layout->addItem(new QSpacerItem(0,4),7,0); //addRowSpacing( 7, 4 );
        // Row 8: Keep Password
        hbox = new KHBox( main );
        d->keepCheckBox = new QCheckBox( i18n("&Keep password"), hbox );
        d->keepCheckBox->setFixedSize( d->keepCheckBox->sizeHint() );
        KConfigGroup cg( KGlobal::config(), "Passwords" );
        bool keep = cg.readEntry("Keep", false);
        d->keepCheckBox->setChecked( keep );
        d->layout->addWidget( hbox, 8, 2 );
    }

    // Configure necessary key-bindings and connect necessar slots and signals
    connect( d->passEdit, SIGNAL(returnPressed()), SLOT(accept()) );
    
    if( d->userEdit )
    {
        d->userEdit->setFocus();
        connect( d->userEdit, SIGNAL(returnPressed()), d->passEdit, SLOT(setFocus()) );
    }
    else
        d->passEdit->setFocus();

    d->userEditCombo = 0;
//    setFixedSize( sizeHint() );
}

void KPasswordDialog::setPixmap(const QPixmap &pixmap)
{
    d->pixmapLabel->setPixmap(pixmap);
    d->pixmapLabel->setFixedSize(d->pixmapLabel->sizeHint());
}

QPixmap KPasswordDialog::pixmap() const
{
    return *d->pixmapLabel->pixmap();
}


void KPasswordDialog::setUsername(const QString& user)
{
    if(d->userEdit)
    {
        d->userEdit->setText(user);
        d->passEdit->setFocus();
        slotActivated(user);
    }
    else
        kWarning(240) << "KPasswordDialog::setUsername() called while the dialog has not username line" << endl;
}


QString KPasswordDialog::username() const
{
    if(d->userEdit)
    {
        return d->userEdit->text();
    }
    else
    {
        kWarning(240) << "KPasswordDialog::username() called while the dialog has not username line" << endl;
        return QString();
    }
}

QString KPasswordDialog::password() const
{
    return d->passEdit->text();
}

void KPasswordDialog::setKeepPassword( bool b )
{
    if ( d->keepCheckBox )
        d->keepCheckBox->setChecked( b );
    else
        kWarning(240) << "KPasswordDialog::setKeepPassword() called while the dialog has not keep checkbox" << endl;
}

bool KPasswordDialog::keepPassword() const
{
    if ( d->keepCheckBox )
        return d->keepCheckBox->isChecked();
    else
    {
        kWarning(240) << "KPasswordDialog::keepPassword() called while the dialog has not keep checkbox" << endl;
        return false;
    }
}

static void calculateLabelSize(QLabel *label)
{
    QString qt_text = label->text();

    QRect rect;
    QRect d = KGlobalSettings::desktopGeometry(label->topLevelWidget());
    QTextLayout newLayout(qt_text, label->font());

    qreal curWidth = d.width() / 4;
    int leading = QFontMetrics(label->font()).leading();
    int lineCount = -1;

    do {
        qreal new_width = (curWidth * 0.9);
        qreal maxWidth = 0.;
        int cur_height = 0;

        // save current size
        rect = newLayout.boundingRect().toRect();
        // do layout calculation
        newLayout.beginLayout();
        while (1) {
            QTextLine line = newLayout.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(new_width);
            cur_height += leading;
            line.setPosition(QPoint(0, cur_height));
            cur_height += (int)(line.height()+0.5);
            maxWidth = qMax(maxWidth, line.naturalTextWidth());
        }
        newLayout.endLayout();
        if( lineCount == -1 ) {
            lineCount = newLayout.lineCount();;
            rect = newLayout.boundingRect().toRect();
            if( lineCount == 1 )
                rect.setWidth( (int)maxWidth );
        }
        curWidth = rect.width();
    } while( newLayout.lineCount() > 1 &&
              newLayout.lineCount() == lineCount );

    label->setFixedSize(rect.width(), rect.height()+leading);
}

void KPasswordDialog::addCommentLine( const QString& label,
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
    lbl->setAlignment( Qt::AlignVCenter|Qt::AlignLeft);
    lbl->setWordWrap(true);
    calculateLabelSize(lbl);
    d->layout->addWidget( lbl, d->nRow+2, 2, Qt::AlignLeft );
    d->layout->addItem(new QSpacerItem(0,10),3,0); //addRowSpacing( 3, 10 ); // Add a spacer
    d->nRow++;
}

static QString qrichtextify( const QString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  QStringList lines = text.split('\n', QString::SkipEmptyParts);
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = Qt::convertFromPlainText( *it, Qt::WhiteSpaceNormal );
  }

  return lines.join(QString());
}

void KPasswordDialog::setPrompt(const QString& prompt)
{
    QString text = qrichtextify(prompt);
    d->prompt->setText(text);
    calculateLabelSize(d->prompt);
}

QString KPasswordDialog::prompt() const
{
    return d->prompt->text();
}

void KPasswordDialog::setPassword(const QString &p)
{
    d->passEdit->setText(p);
}

void KPasswordDialog::setUsernameReadOnly( bool readOnly )
{
    if(!d->userEdit)
    {
        kWarning(240) << "KPasswordDialog::setUsernameReadOnly called while the dialog has not username line" << endl;
        return;
    }   
    d->userEdit->setReadOnly( readOnly );
    if ( readOnly && d->userEdit->hasFocus() )
        d->passEdit->setFocus();
}

void KPasswordDialog::setKnownLogins( const QMap<QString, QString>& knownLogins )
{
    if(!d->userEdit)
    {
        kWarning(240) << "KPasswordDialog::setKnownLogins called while the dialog has not username line" << endl;
        return;
    }
    const int nr = knownLogins.count();
    if ( nr == 0 )
        return;
    if ( nr == 1 ) {
        d->userEdit->setText( knownLogins.begin().key() );
        setPassword( knownLogins.begin().value() );
        return;
    }

    Q_ASSERT( !d->userEdit->isReadOnly() );
    if ( !d->userEditCombo ) {
        delete d->userEdit;
        d->userEditCombo = new KComboBox( true, d->userNameHBox );
        d->userEdit = d->userEditCombo->lineEdit();
        QSize s = d->userEditCombo->sizeHint();
        d->userEditCombo->setFixedHeight( s.height() );
        d->userEditCombo->setMinimumWidth( s.width() );
        d->userNameLabel->setBuddy( d->userEditCombo );
        d->layout->addWidget( d->userNameHBox, 4, 2 );
        connect( d->userEdit, SIGNAL(returnPressed()), d->passEdit, SLOT(setFocus()) );
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->addItems( knownLogins.keys() );
    d->userEditCombo->setFocus();

    connect( d->userEditCombo, SIGNAL( activated( const QString& ) ),
             this, SLOT( slotActivated( const QString& ) ) );
}

void KPasswordDialog::slotActivated( const QString& userName )
{
    QMap<QString, QString>::ConstIterator it = d->knownLogins.find( userName );
    if ( it != d->knownLogins.end() )
        setPassword( it.value() );
}

void  KPasswordDialog::accept()
{
    bool keep=d->keepCheckBox ? d->keepCheckBox->isChecked() : false;
    emit gotPassword( password() , keep);
    if( d->userEdit )
        emit gotUsernameAndPassword( username(), password() , keep);
            
    KDialog::accept();
}


#include "kpassworddialog.moc"
