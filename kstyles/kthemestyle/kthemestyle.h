/*
$Id$

This file is part of the KDE libraries
Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

KDE3 port (C) 2001-2002 Maksim Orlovich <mo002j@mail.rochester.edu>
Port version 0.9.6

Includes code portions from the dotNET style, and the KDE HighColor style.

dotNET Style
 Copyright (C) 2001, Chris Lee        <lee@azsites.com>
                   Carsten Pfeiffer <pfeiffer@kde.org>

KDE3 HighColor Style
Copyright (C) 2001 Karol Szwed       <gallium@kde.org>
  (C) 2001 Fredrik Höglund   <fredrik@kde.org>

Drawing routines adapted from the KDE2 HCStyle,
Copyright (C) 2000 Daniel M. Duley   <mosfet@kde.org>
  (C) 2000 Dirk Mueller      <mueller@kde.org>
  (C) 2001 Martijn Klingens  <klingens@kde.org>


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
#ifndef KTHEMESTYLE_H
#define KTHEMESTYLE_H

#include <qglobal.h>

#include <kthemebase.h>
#include <qwindowdefs.h>
#include <qobject.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qtabbar.h>
#include <qstring.h>
#include <qintdict.h>
#include <qmap.h>


/**
 * KDE themed styles.
 *
 * It provides methods for
 * drawing most widgets with user-specified borders, highlights, pixmaps,
 * etc. It also handles various other settings such as scrollbar types,
 * rounded buttons, and shading types. For a full list of parameters this
 * class handles refer to the KDE theme configuration documentation.
 *
 */

class KThemeStyle: public KThemeBase
{
    Q_OBJECT
public:
    /**
     * Constructs a new KThemeStyle object.
     *
     * @param configFile A KConfig file to use as the theme configuration.
     * Defaults to ~/.kderc.
     */
    KThemeStyle( const QString& configDir, const QString &configFile = QString::null );
    ~KThemeStyle();

    virtual int pixelMetric ( PixelMetric metric, const QWidget * widget = 0 ) const;

    virtual void drawPrimitive ( PrimitiveElement pe, QPainter * p, const QRect & r, const QColorGroup & cg,
                                 SFlags flags = Style_Default,
                                 const QStyleOption& = QStyleOption::Default ) const;

    virtual void drawControl( ControlElement element,
                              QPainter *p,
                              const QWidget *widget,
                              const QRect &r,
                              const QColorGroup &cg,
                              SFlags how = Style_Default,
                              const QStyleOption& = QStyleOption::Default ) const;

    virtual void drawControlMask( ControlElement element,
                                  QPainter *p,
                                  const QWidget *widget,
                                  const QRect &r,
                                  const QStyleOption& = QStyleOption::Default ) const;


    virtual void drawComplexControl( ComplexControl control,
                                     QPainter *p,
                                     const QWidget* widget,
                                     const QRect &r,
                                     const QColorGroup &cg,
                                     SFlags flags = Style_Default,
                                     SCFlags controls = SC_All,
                                     SCFlags active = SC_None,
                                     const QStyleOption& = QStyleOption::Default ) const;

    virtual void drawKStylePrimitive( KStylePrimitive kpe,
                                      QPainter* p,
                                      const QWidget* widget,
                                      const QRect &r,
                                      const QColorGroup &cg,
                                      SFlags flags = Style_Default,
                                      const QStyleOption& = QStyleOption::Default ) const;


    virtual int styleHint( StyleHint sh,
                           const QWidget *widget = 0,
                           const QStyleOption& = QStyleOption::Default,
                           QStyleHintReturn* returnData = 0 ) const;

    virtual QSize sizeFromContents( ContentsType contents,
                                    const QWidget *widget,
                                    const QSize &contentsSize,
                                    const QStyleOption& = QStyleOption::Default ) const;


    virtual void polish( QWidget* );
    virtual void unPolish( QWidget* );
    virtual bool eventFilter( QObject* object, QEvent* event );
    /**
     * By default this just sets the background brushes to the pixmapped
     * background.
     */
    virtual void polish( QApplication *app );
    virtual void unPolish( QApplication* );

    /// @internal
    // to make it possible for derived classes to overload this function
    virtual void polish( QPalette& pal );

    /**
     * This is a convenience method for drawing widgets with
     * borders, highlights, pixmaps, colors, etc...
     * You specify the widget type and it will draw it according to the
     * config file settings.
     *
     * @param p The QPainter to draw on.
     * @param g The color group to use.
     * @param rounded @p true if the widget is rounded, @p false if rectangular.
     * @param type The widget type to paint.
     * @param fill An optional fill brush. Currently ignored (the config file
     * is used instead).
     */
    virtual void drawBaseButton( QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken = FALSE,
                                 bool rounded = FALSE, WidgetType type = Bevel ) const;
    /**
     * Draw a mask with for widgets that may be rounded.
     *
     *Currently used
     * by pushbuttons and comboboxes.
     *
     * @param p The QPainter to draw on.
     * @param rounded @p true if the widget is rounded, @p false if rectangular.
     */
    virtual void drawBaseMask( QPainter *p, int x, int y, int w, int h,
                               bool rounded ) const;



    /**
        * Draw a shaded rectangle using the given style.
        *
        * @param p The painter to draw on.
        * @param g The color group to use.
        * @param rounded Draws a rounded shape if true. Requires bWidth to be
        * at least 1.
        * @param hWidth The highlight width.
        * @param bWidth The border width.
        * @param style The shading style to use.
        */
    virtual void drawShade( QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken, bool rounded,
                            int hWidth, int bWidth, ShadeStyle style ) const;
    int popupMenuItemHeight( bool checkable, QMenuItem *mi,
                             const QFontMetrics &fm );

protected:
    QPalette oldPalette, popupPalette, indiPalette, exIndiPalette;
    bool paletteSaved;
    bool polishLock;
    QStyle *mtfstyle;

    QPixmap* makeMenuBarCache(int w, int h) const;

    mutable QPixmap* menuCache;
    mutable QPixmap* vsliderCache;

    Qt::HANDLE brushHandle;
    bool brushHandleSet;

protected slots:
    void paletteChanged();



};


#endif
