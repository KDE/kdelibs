/* This file is part of the KDE libraries
    Copyright (C) 2004 Antonio Larrosa <larrosa@kde.org

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

#ifndef KPIXMAPREGIONSELECTORDIALOG_H
#define KPIXMAPREGIONSELECTORDIALOG_H

#include <qimage.h>

#include <kdialogbase.h>
#include <kpixmapregionselectorwidget.h>

/**
 * A dialog that uses a KPixmapRegionSelectorWidget to allow the user
 * to select a region of an image. If you want to use special features
 * like forcing the selected area to have a fixed aspect ratio, you can use
 * @see pixmapRegionSelectorWidget() to get the pointer to the 
 * KPixmapRegionSelectorWidget object and set the desired options there.
 * 
 * There are some convenience methods that allow to easily show a dialog
 * for the user to select a region of an image, and just care about the selected
 * image.
 *
 * @author Antonio Larrosa <larrosa@kde.org>
 * @since 3.4
 */ 
class KDEUI_EXPORT KPixmapRegionSelectorDialog : public KDialogBase
{
public:
   /**
    * The constructor of an empty KPixmapRegionSelectorDialog, you have to call
    * later the setPixmap method of the KPixmapRegionSelectorWidget widget of
    * the new object.
    */
   KPixmapRegionSelectorDialog(QWidget *parent=0L, const char *name=0L,
                      bool modal = false );
   /**
    * The destructor of the dialog
    */
   ~KPixmapRegionSelectorDialog();

   /**
    * @returns the KPixmapRegionSelectorWidget widget so that additional
    * parameters can be set by using it.
    */
   KPixmapRegionSelectorWidget *pixmapRegionSelectorWidget() const 
      { return m_pixmapSelectorWidget; };
   
   /**
    * Creates a modal dialog, lets the user to select a region of the @p pixmap
    * and returns when the dialog is closed.
    *
    * @returns the selected rectangle, or an invalid rectangle if the user 
    * pressed the Cancel button.
    */
   static QRect getSelectedRegion(const QPixmap &pixmap, QWidget *parent = 0L );

   /**
    * Creates a modal dialog, lets the user to select a region of the @p pixmap
    * with the same aspect ratio than @p aspectRatioWidth x @p aspectRatioHeight
    * and returns when the dialog is closed.
    *
    * @returns the selected rectangle, or an invalid rectangle if the user 
    * pressed the Cancel button.
    */
   static QRect getSelectedRegion(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent = 0L );

   /**
    * Creates a modal dialog, lets the user to select a region of the @p pixmap
    * and returns when the dialog is closed.
    *
    * @returns the selected image, or an invalid image if the user 
    * pressed the Cancel button.
    */
   static QImage getSelectedImage(const QPixmap &pixmap, QWidget *parent = 0L );

   /**
    * Creates a modal dialog, lets the user to select a region of the @p pixmap
    * with the same aspect ratio than @p aspectRatioWidth x @p aspectRatioHeight
    * and returns when the dialog is closed.
    *
    * @returns the selected image, or an invalid image if the user 
    * pressed the Cancel button.
    */
   static QImage getSelectedImage(const QPixmap &pixmap, int aspectRatioWidth, int aspectRatioHeight, QWidget *parent = 0L );

protected:
   KPixmapRegionSelectorWidget *m_pixmapSelectorWidget;
};


#endif
