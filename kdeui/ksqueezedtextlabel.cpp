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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "ksqueezedtextlabel.h"
#include <qtooltip.h>

KSqueezedTextLabel::KSqueezedTextLabel( const QString &text , QWidget *parent, const char *name )
 : QLabel ( parent, name ) {
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  fullText = text;
  squeezeTextToLabel();
}

KSqueezedTextLabel::KSqueezedTextLabel( QWidget *parent, const char *name )
 : QLabel ( parent, name ) {
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

void KSqueezedTextLabel::setText( const QString &text ) {
  fullText = text;
  squeezeTextToLabel();
}

void KSqueezedTextLabel::squeezeTextToLabel() {
  QFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(fullText);
  if (textWidth > labelWidth) {
    // start with the dots only
    QString squeezedText = "...";
    int squeezedWidth = fm.width(squeezedText);

    // estimate how many letters we can add to the dots on both sides
    int letters = fullText.length() * (labelWidth - squeezedWidth) / textWidth / 2;
    if (labelWidth < squeezedWidth) letters=1;
    squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
    squeezedWidth = fm.width(squeezedText);

    if (squeezedWidth < labelWidth) {
        // we estimated too short
        // add letters while text < label
        do {
                letters++;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fm.width(squeezedText);
        } while (squeezedWidth < labelWidth);
        letters--;
        squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
    } else if (squeezedWidth > labelWidth) {
        // we estimated too long
        // remove letters while text > label
        do {
            letters--;
            squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
            squeezedWidth = fm.width(squeezedText);
        } while (letters && squeezedWidth > labelWidth);
    }

    if (letters < 5) {
    	// too few letters added -> we give up squeezing
    	QLabel::setText(fullText);
    } else {
	QLabel::setText(squeezedText);
    }

    QToolTip::remove( this );
    QToolTip::add( this, fullText );

  } else {
    QLabel::setText(fullText);

    QToolTip::remove( this );
    QToolTip::hide();

  };
}

void KSqueezedTextLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksqueezedtextlabel.moc"
