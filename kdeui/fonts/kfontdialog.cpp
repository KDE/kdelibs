/*

Requires the Qt widget libraries, available at no cost at
http://www.troll.no

Copyright (C) 1996 Bernd Johannes Wuebben  <wuebben@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
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

#include "kfontdialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <klocalizedstring.h>

class KFontDialog::Private
{
public:
    Private()
        : chooser( 0 )
    {
    }

    KFontChooser *chooser;
};

KFontDialog::KFontDialog( QWidget *parent,
                          const KFontChooser::DisplayFlags& flags,
                          const QStringList &fontList,
                          Qt::CheckState *sizeIsRelativeState )
    : QDialog( parent ),
      d( new Private )
{
    setWindowTitle( i18n("Select Font") );
    d->chooser = new KFontChooser( this, flags, fontList, 8,
                                   sizeIsRelativeState );
    d->chooser->setObjectName( "fontChooser" );

    connect( d->chooser , SIGNAL(fontSelected(QFont)) , this , SIGNAL(fontSelected(QFont)) );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(d->chooser);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

KFontDialog::~KFontDialog()
{
   delete d;
}

void KFontDialog::setFont( const QFont &font, bool onlyFixed)
{
    d->chooser->setFont(font, onlyFixed);
}

QFont KFontDialog::font() const
{
     return d->chooser->font();
}

void KFontDialog::setSizeIsRelative( Qt::CheckState relative )
{
    d->chooser->setSizeIsRelative( relative );
}

Qt::CheckState KFontDialog::sizeIsRelative() const
{
    return d->chooser->sizeIsRelative();
}


int KFontDialog::getFontDiff( QFont &theFont,
                              KFontChooser::FontDiffFlags& diffFlags,
                              const KFontChooser::DisplayFlags& flags,
                              QWidget *parent,
                              Qt::CheckState *sizeIsRelativeState )
{
    KFontDialog dlg( parent, flags | KFontChooser::ShowDifferences,
                     QStringList(), sizeIsRelativeState );
    dlg.setModal( true );
    dlg.setObjectName( "Font Selector" );
    dlg.setFont( theFont, flags & KFontChooser::FixedFontsOnly );

    int result = dlg.exec();
    if( result == Accepted )
    {
        theFont = dlg.d->chooser->font();
        diffFlags = dlg.d->chooser->fontDiffFlags();
        if( sizeIsRelativeState )
            *sizeIsRelativeState = dlg.d->chooser->sizeIsRelative();
    }
    return result;
}

int KFontDialog::getFont( QFont &theFont,
                          const KFontChooser::DisplayFlags& flags,
                          QWidget *parent,
                          Qt::CheckState *sizeIsRelativeState )
{
    KFontDialog dlg( parent, flags, QStringList(), sizeIsRelativeState );
    dlg.setModal( true );
    dlg.setObjectName( "Font Selector" );
    dlg.setFont( theFont, flags & KFontChooser::FixedFontsOnly );

    int result = dlg.exec();
    if( result == Accepted )
    {
        theFont = dlg.d->chooser->font();
        if( sizeIsRelativeState )
            *sizeIsRelativeState = dlg.d->chooser->sizeIsRelative();
    }
    return result;
}


int KFontDialog::getFontAndText( QFont &theFont, QString &theString,
                                 const KFontChooser::DisplayFlags& flags,
                                 QWidget *parent,
                                 Qt::CheckState *sizeIsRelativeState )
{
    KFontDialog dlg( parent, flags,
                     QStringList(), sizeIsRelativeState );
    dlg.setModal( true );
    dlg.setObjectName( "Font and Text Selector" );
    dlg.setFont( theFont, flags & KFontChooser::FixedFontsOnly );

    int result = dlg.exec();
    if( result == Accepted )
    {
        theFont   = dlg.d->chooser->font();
        theString = dlg.d->chooser->sampleText();
        if( sizeIsRelativeState )
            *sizeIsRelativeState = dlg.d->chooser->sizeIsRelative();
    }
    return result;
}


