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

#ifndef KHUESATURATIONSELECT_H_
#define KHUESATURATIONSELECT_H_

#include <kdelibs_export.h>
#include "kxyselector.h"

#include "kcolorchoosermode.h"

class KDEUI_EXPORT KHueSaturationSelector : public KXYSelector
{
    Q_OBJECT
public:
  /**
   * Constructs a hue/saturation selection widget.
   */
  KHueSaturationSelector( QWidget *parent=0);

  /**
   * Destructor.
   */
  ~KHueSaturationSelector();
  
  /**
   * Sets the chooser mode. The allowed modes are defined
   * in HueSaturationMode.
   */
  void setChooserMode (KColorChooserMode c);

  /**
   * Returns the chooser mode.
   */
  KColorChooserMode chooserMode () const
	{ return _mode; }


  /**
   * Returns the hue amount
   */
  int hue() const
        { return _hue; }
        
  /**
   * Sets the hue amount
   */
  void setHue( int h )
        { _hue = h; }
        
  /**
   * Returns the saturation
   */
  int saturation() const
        { return _sat; }
        
  /**
   * Sets the saturation
   */
  void setSaturation( int s )
        { _sat = s; }
        
  /**
   * Returns the color value (also known as lumniousity)
   */
  int colorValue() const
        { return _colorValue; }
        
  /**
   * Sets the color value
   */
  void setColorValue( int v )
        { _colorValue = v; }

  void updateContents();
  
protected:
  /**
   * Draws the contents of the widget on a pixmap,
   * which is used for buffering.
   */
  virtual void drawPalette( QPixmap *pixmap );
  virtual void resizeEvent( QResizeEvent * );

  /**
   * Reimplemented from KXYSelector. This drawing is
   * buffered in a pixmap here. As real drawing
   * routine, drawPalette() is used.
   */
  virtual void drawContents( QPainter *painter );

  /**
   * Stores the chooser mode
   */
  KColorChooserMode _mode;

  /**
   * Stores the values for hue, saturation and lumniousity
   */
  int _hue, _sat, _colorValue;
  
private:

  class Private;
  friend class Private;
  Private * const d;
  
  Q_DISABLE_COPY(KHueSaturationSelector)
};

#endif /*KHUESATURATIONSELECT_H_*/
