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

#ifndef KSQUEEZEDTEXTLABEL_H
#define KSQUEEZEDTEXTLABEL_H

#include <qlabel.h>
#include <qtooltip.h>

/**
 * A label class that squeezes its text into the label
 *
 * If the text is too long to fit into the label it is divided into
 * remaining left and right parts which are separated by three dots.
 *
 * Example:
 * http://www.kde.org/documentation/index.html could be squeezed to
 * http://www.kde...ion/index.html

 * @short A replacement for QLabel that squeezes its text
 * @author Ronny Standtke <Ronny.Standtke@gmx.de>
 * @version $Id$
 *
 */

/*
 * @ref QLabel
 */
class KSqueezedTextLabel : public QLabel {
  Q_OBJECT

public:
  /**
   * Default constructor.
   */
  KSqueezedTextLabel( QWidget *parent, const char *name = 0 );
  KSqueezedTextLabel( const QString &text, QWidget *parent, const char *name = 0 );

public slots:
  void setText( const QString & );

protected:
  /**
   * used when widget is resized
   */
  void resizeEvent( QResizeEvent * );
  /**
   * does the dirty work
   */
  void squeezeTextToLabel();
  QString fullText;

private:
  QFontMetrics *unused; //### unused, remove in 3.0
  class KSqueezedTextLabelPrivate;
  KSqueezedTextLabelPrivate *d;
};

#endif // KSQUEEZEDTEXTLABEL_H
