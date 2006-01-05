/* This file is part of the KDE libraries
   Copyright (C) 2000 Ronny Standtke <Ronny.Standtke@gmx.de>

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

#include "ksqueezedtextlabel.h"
#include "kstringhandler.h"

KSqueezedTextLabel::KSqueezedTextLabel( const QString &text , QWidget *parent )
 : QLabel ( parent ) {
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  fullText = text;
  squeezeTextToLabel();
}

KSqueezedTextLabel::KSqueezedTextLabel( QWidget *parent )
 : QLabel ( parent ) {
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
}

void KSqueezedTextLabel::resizeEvent( QResizeEvent * ) {
  squeezeTextToLabel();
}

QSize KSqueezedTextLabel::minimumSizeHint() const
{
  QSize sh = QLabel::minimumSizeHint();
  sh.setWidth(-1);
  return sh;
}

QSize KSqueezedTextLabel::sizeHint() const
{
  return QSize(contentsRect().width(), QLabel::sizeHint().height());
}

void KSqueezedTextLabel::setText( const QString &text ) {
  fullText = text;
  squeezeTextToLabel();
}

void KSqueezedTextLabel::squeezeTextToLabel() {
  QFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(fullText);
  if (textWidth > labelWidth) {
    QString squeezedText = KStringHandler::cPixelSqueeze(fullText, fm, labelWidth);
	QLabel::setText(squeezedText);

    setToolTip( fullText );

  } else {
    QLabel::setText(fullText);

    setToolTip( QString() );
//dead in qt4?    QToolTip::hide();

  }
}

void KSqueezedTextLabel::setAlignment( int alignment )
{
  // save fullText and restore it
  QString tmpFull(fullText);
  QLabel::setAlignment(alignment);
  fullText = tmpFull;
}

void KSqueezedTextLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksqueezedtextlabel.moc"
