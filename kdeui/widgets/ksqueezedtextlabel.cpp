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

#include <kglobalsettings.h>

class KSqueezedTextLabelPrivate
{
  public:
    QString fullText;
    Qt::TextElideMode elideMode;
};

KSqueezedTextLabel::KSqueezedTextLabel( const QString &text , QWidget *parent )
 : QLabel ( parent ),
  d( new KSqueezedTextLabelPrivate )
{
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  d->fullText = text;
  d->elideMode = Qt::ElideMiddle;
  squeezeTextToLabel();
}

KSqueezedTextLabel::KSqueezedTextLabel( QWidget *parent )
 : QLabel ( parent ),
  d( new KSqueezedTextLabelPrivate )
{
  setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ));
  d->elideMode = Qt::ElideMiddle;
}

KSqueezedTextLabel::~KSqueezedTextLabel()
{
  delete d;
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
  int maxWidth = KGlobalSettings::desktopGeometry( this ).width() * 3 / 4;
  QFontMetrics fm(fontMetrics());
  int textWidth = fm.width(d->fullText);
  if (textWidth > maxWidth) {
    textWidth = maxWidth;
  }
  return QSize(textWidth, QLabel::sizeHint().height());
}

void KSqueezedTextLabel::setText( const QString &text ) {
  d->fullText = text;
  squeezeTextToLabel();
}

void KSqueezedTextLabel::squeezeTextToLabel() {
  QFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(d->fullText);
  if (textWidth > labelWidth) {
    QString squeezedText = fm.elidedText(d->fullText, d->elideMode, labelWidth);
    QLabel::setText(squeezedText);
    setToolTip(d->fullText);
  } else {
    QLabel::setText(d->fullText);
    setToolTip( QString() );
  }
}

void KSqueezedTextLabel::setAlignment( Qt::Alignment alignment )
{
  // save fullText and restore it
  QString tmpFull(d->fullText);
  QLabel::setAlignment(alignment);
  d->fullText = tmpFull;
}

Qt::TextElideMode KSqueezedTextLabel::textElideMode() const
{
  return d->elideMode;
}

void KSqueezedTextLabel::setTextElideMode(Qt::TextElideMode mode)
{
  d->elideMode = mode;
  squeezeTextToLabel();
}

#include "ksqueezedtextlabel.moc"
