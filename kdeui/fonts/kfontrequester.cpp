/*
    Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#include "kfontrequester.h"

#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>

#include <kfontdialog.h>
#include <klocale.h>

class KFontRequester::KFontRequesterPrivate
{
public:
  KFontRequesterPrivate(KFontRequester *q): q(q) {}

  KFontRequester *q;
  bool m_onlyFixed;
  QString m_sampleText, m_title;
  QLabel *m_sampleLabel;
  QPushButton *m_button;
  QFont m_selFont;
};

KFontRequester::KFontRequester( QWidget *parent, bool onlyFixed )
    : QWidget( parent ), d(new KFontRequesterPrivate(this))
{
  d->m_onlyFixed = onlyFixed;

  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );
  layout->setSpacing( KDialog::spacingHint() );

  d->m_sampleLabel = new QLabel( this );
  d->m_button = new QPushButton( i18n( "Choose..." ), this );

  d->m_sampleLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  setFocusProxy( d->m_button );

  layout->addWidget( d->m_sampleLabel, 1 );
  layout->addWidget( d->m_button );

  connect( d->m_button, SIGNAL( clicked() ), SLOT( buttonClicked() ) );

  displaySampleText();
  setToolTip();
}

KFontRequester::~KFontRequester()
{
  delete d;
}

QFont KFontRequester::font() const
{
  return d->m_selFont;
}

bool KFontRequester::isFixedOnly() const
{
  return d->m_onlyFixed;
}

QString KFontRequester::sampleText() const
{
  return d->m_sampleText;
}

QString KFontRequester::title() const
{
  return d->m_title;
}

QLabel *KFontRequester::label() const
{
  return d->m_sampleLabel;
}

QPushButton *KFontRequester::button() const
{
  return d->m_button;
}

void KFontRequester::setFont( const QFont &font, bool onlyFixed )
{
  d->m_selFont = font;
  d->m_onlyFixed = onlyFixed;

  displaySampleText();
  emit fontSelected( d->m_selFont );
}

void KFontRequester::setSampleText( const QString &text )
{
  d->m_sampleText = text;
  displaySampleText();
}

void KFontRequester::setTitle( const QString &title )
{
  d->m_title = title;
  setToolTip();
}

void KFontRequester::buttonClicked()
{
    KFontChooser::DisplayFlags flags = KFontChooser::DisplayFrame;
    if ( d->m_onlyFixed ) {
        flags |= KFontChooser::FixedFontsOnly;
    }

    int result = KFontDialog::getFont( d->m_selFont, flags, parentWidget() );

    if ( result == KDialog::Accepted )
    {
        displaySampleText();
        emit fontSelected( d->m_selFont );
    }
}

void KFontRequester::displaySampleText()
{
  d->m_sampleLabel->setFont( d->m_selFont );

  int size = d->m_selFont.pointSize();
  if(size == -1)
    size = d->m_selFont.pixelSize();

  if ( d->m_sampleText.isEmpty() )
    d->m_sampleLabel->setText( QString( "%1 %2" ).arg( d->m_selFont.family() )
      .arg( size ) );
  else
    d->m_sampleLabel->setText( d->m_sampleText );
}

void KFontRequester::setToolTip()
{
  d->m_button->setToolTip( i18n( "Click to select a font" ) );

  d->m_sampleLabel->setToolTip( QString() );
  d->m_sampleLabel->setWhatsThis(QString());

  if ( d->m_title.isNull() )
  {
    d->m_sampleLabel->setToolTip( i18n( "Preview of the selected font" ) );
    d->m_sampleLabel->setWhatsThis(        i18n( "This is a preview of the selected font. You can change it"
        " by clicking the \"Choose...\" button." ) );
  }
  else
  {
    d->m_sampleLabel->setToolTip( i18n( "Preview of the \"%1\" font" ,  d->m_title ) );
    d->m_sampleLabel->setWhatsThis(        i18n( "This is a preview of the \"%1\" font. You can change it"
        " by clicking the \"Choose...\" button." ,  d->m_title ) );
  }
}

#include "kfontrequester.moc"

/* vim: et sw=2 ts=2
*/
