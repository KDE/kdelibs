/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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

#ifndef KVALUESELECTOR_H
#define KVALUESELECTOR_H

#include "kselector.h"
#include "kcolorchoosermode.h"
#include <QtGui/QPixmap>

class KDEUI_EXPORT KColorValueSelector : public KSelector
{
    Q_OBJECT
public:
  /**
   * Constructs a widget for color selection.
   */
  KColorValueSelector( QWidget *parent=0 );
  /**
   * Constructs a widget for color selection with a given orientation
   */
  KColorValueSelector( Qt::Orientation o, QWidget *parent = 0 );

  ~KColorValueSelector();
  
  void updateContents();

  int hue() const;
  void setHue( int h );
  int saturation() const;
  void setSaturation( int s );
  int colorValue() const;
  void setColorValue( int v );

  void setChooserMode (KColorChooserMode c);

  KColorChooserMode chooserMode ();
	
protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( QPixmap *pixmap );
  virtual void resizeEvent( QResizeEvent * );

  /**
   * Reimplemented from KSelector. The drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( QPainter *painter );

private:
  class Private;
  friend class Private;
  Private *const d;
  
  int _hue;
  int _sat;
  int _colorValue;
  
  Q_DISABLE_COPY(KColorValueSelector)
};

#endif /* KVALUESELECTOR_H */

