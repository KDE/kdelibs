/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

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
#ifndef __KTHEMESTYLE_H
#define __KTHEMESTYLE_H

#include <qglobal.h>

#if QT_VERSION < 300

#include <kthemebase.h>
#include <qwindowdefs.h>
#include <qobject.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qstring.h>


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
    KThemeStyle(const QString &configFile = QString::null);
    ~KThemeStyle();
    virtual void polish(QWidget* );
    virtual void unPolish(QWidget* );
    /**
     * By default this just sets the background brushes to the pixmapped
     * background.
     */
    virtual void polish(QApplication *app);
    virtual void unPolish(QApplication*);

    /// @internal
    // to make it possible for derived classes to overload this function
    virtual void polish(QPalette& pal);

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
    virtual void drawBaseButton(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken = FALSE,
                                bool rounded = FALSE, WidgetType type=Bevel,
                                const QBrush *fill = 0);
    /**
     * Draw a mask with for widgets that may be rounded. 
     *
     *Currently used
     * by pushbuttons and comboboxes.
     *
     * @param p The QPainter to draw on.
     * @param rounded @p true if the widget is rounded, @p false if rectangular.
     */
    virtual void drawBaseMask(QPainter *p, int x, int y, int w, int h,
                              bool rounded);
    /**
     * Draw a pushbutton.
     *
     * This calls @ref drawBaseButton() with @p PushButton as the
     * widget type.
     */
    virtual void drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken = FALSE,
                            const QBrush *fill = 0);
    /**
     * Draw a bevel button.
     *
     * This calls @ref drawBaseButton() with Bevel as the
     * widget type.
     */
    virtual void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken = FALSE,
                                 const QBrush *fill = 0);
    /**
     * Draw a toolbar button.
     */
    virtual void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken=false,
                                    bool raised = true, bool enabled = true,
                                    bool popup = false, KToolButtonType type = Icon,
                                    const QString &btext=QString::null,
                                    const QPixmap *icon=NULL,
                                    QFont *font=NULL, QWidget *btn=NULL);
    /**
     * Draw the handle used in toolbars.
     */
    void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g,
                        KToolBarPos type, QBrush *fill=NULL);

    /**
     * Draw a toolbar.
     */
    void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, KToolBarPos type,
                      QBrush *fill=NULL);

    /**
     * Return the space available in a pushbutton, taking configurable
     * borders and highlights into account.
     */
    virtual QRect buttonRect(int x, int y, int w, int h);
    /**
     * Draw an arrow in the style specified by the config file.
     */
    virtual void drawArrow(QPainter *p, Qt::ArrowType type, bool down,
                           int x, int y, int w, int h, const QColorGroup &g,
                           bool enabled=true, const QBrush *fill = 0);
    /**
     * Return the size of the exclusive indicator pixmap if one is specified
     * in the config file, otherwise it uses the base style's size.
     */
    virtual QSize exclusiveIndicatorSize() const;
    /**
     * Draw an exclusive indicator widget.
     *
     * If a pixmap is specified in the
     * config file that is used, otherwise the base style's widget is drawn.
     */
    virtual void drawExclusiveIndicator(QPainter* p, int x, int y, int w,
                                        int h, const QColorGroup &g, bool on,
                                        bool down = FALSE,
                                        bool enabled = TRUE);
    /**
     * Set the mask of an exclusive indicator widget.
     *
     * If a pixmap is specified
     * it is masked according to it's transparent pixels, otherwise the
     * base style's mask is used.
     */
    virtual void drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                            int h, bool on);
    /**
     * Set the mask of an indicator widget.
     *
     * If a pixmap is specified
     * it is masked according to it's transparent pixels, otherwise the
     * base style's mask is used.
     */
    virtual void drawIndicatorMask(QPainter *p, int x, int y, int w, int h,
                                   int state);
    /**
     * Set the mask for pushbuttons.
     */
    virtual void drawButtonMask(QPainter *p, int x, int y, int w, int h);
    /**
     * Set the mask for combo boxes.
     */
    virtual void drawComboButtonMask(QPainter *p, int x, int y, int w, int h);
    /**
     * Return the size of the indicator pixmap if one is specified
     * in the config file, otherwise it uses the base style's size.
     */
    virtual QSize indicatorSize() const;
    /**
     * Draw an indicator widget.
     *
     * If a pixmap is specified in the
     * config file that is used, otherwise the base style's widget is drawn.
     */
    virtual void drawIndicator(QPainter* p, int x, int y, int w, int h,
                               const QColorGroup &g, int state,
                               bool down = FALSE, bool enabled = TRUE);
    /**
     * Draw a combobox.
     */
    virtual void drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken = FALSE,
                                 bool editable = FALSE, bool enabled = TRUE,
                                 const QBrush *fill = 0);
    /**
     * Draw a pushbutton.
     */
    virtual void drawPushButton(QPushButton* btn, QPainter *p);
    /**
     * Return the amount of button content displacement specified by the
     * config file.
     */
    virtual void getButtonShift(int &x, int &y);
    /**
     * Return the frame width.
     */
    virtual int defaultFrameWidth() const;
    /**
     * Calculate the metrics of the scrollbar controls according to the
     * layout specified by the config file.
     */
    virtual void scrollBarMetrics(const QScrollBar*, int&, int&, int&, int&);
    /**
     * Draw a themed scrollbar.
     */
    virtual void drawScrollBarControls(QPainter*, const QScrollBar*,
                                       int sliderStart, uint controls,
                                       uint activeControl);
    /**
     * Return the control that the given point is over according to the
     * layout in the config file.
     */
    virtual ScrollControl scrollBarPointOver(const QScrollBar*,
                                             int sliderStart, const QPoint& );
    /**
     * Return the configurable default slider length.
     */
    virtual int sliderLength() const;
    /**
     * Draw a slider control.
     */
    virtual void drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation, bool tickAbove,
                            bool tickBelow);
    /**
     * Draw a slider groove.
     */
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup& g, QCOORD c,
                          Orientation );
    /**
     * Draw the mask for a slider (both the control and groove.
     */
    virtual void drawSliderMask(QPainter *p, int x, int y, int w, int h,
                                Orientation, bool tickAbove, bool tickBelow);
//    void drawSliderGrooveMask(QPainter *p,int x, int y, int w, int h,
//                              QCOORD c, Orientation );
    /**
     * Convience method for drawing themed scrollbar grooves.
     *
     * Since the
     * grooves may be a scaled pixmap you cannot just bitblt the pixmap at
     * any offset. This generates a cached pixmap at full size if needed and
     * then copies the requested area.
     *
     * @param p The painter to draw on.
     * @param sb The scrollbar (usually given by drawScrollBarControls).
     * @param horizontal Is the scrollBar horizontal?
     * @param r The rectangle to fill.
     * @param g The color group to use.
     */
    virtual void drawScrollBarGroove(QPainter *p, const QScrollBar *sb,
                                     bool horizontal, QRect r, QColorGroup g);
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
    virtual void drawShade(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool sunken, bool rounded,
                           int hWidth, int bWidth, ShadeStyle style);
    /**
     * Draw the text for a pushbutton.
     */
    virtual void drawPushButtonLabel(QPushButton *btn, QPainter *p);
    /**
     * Draw a menubar.
     */
    void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool macMode,
                      QBrush *fill=NULL);
    /**
     * Draw a menubar item.
     */
    virtual void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool active,
                               QMenuItem *item, QBrush *fill=NULL);
    /**
     * Return the width of the splitter as specified in the config file.
     */
    virtual int splitterWidth() const;
    /**
     * Draw a splitter widget.
     */
    virtual void drawSplitter(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, Orientation);
    /**
     * Draw a checkmark.
     */
    virtual void drawCheckMark(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool act, bool dis);
    /**
     * Draw a menu item.
     *
     * Note: This method manually handles applying
     * inactive menu backgrounds to the entire widget.
     */
    virtual void drawPopupMenuItem(QPainter *p, bool checkable, int maxpmw,
                                  int tab, QMenuItem *mi, const QPalette &pal,
                                  bool act, bool enabled, int x, int y, int w,
                                   int h);
    int popupMenuItemHeight(bool checkable, QMenuItem *mi,
                            const QFontMetrics &fm);
    /**
     * Draw the focus rectangle.
     */
    void drawFocusRect(QPainter *p, const QRect &r, const QColorGroup &g,
                       const QColor *c=0, bool atBorder=false);
    /**
     * Draw a @ref KProgress bar.
     */
    virtual void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill);
    /**
     * Return the background for @ref KProgress.
     */
    virtual void getKProgressBackground(const QColorGroup &g, QBrush &bg);
    virtual void tabbarMetrics(const QTabBar*, int&, int&, int&);
    virtual void drawTab(QPainter*, const QTabBar*, QTab*, bool selected);
    virtual void drawTabMask(QPainter*, const QTabBar*, QTab*, bool selected);
protected:
    QPalette oldPalette, popupPalette, indiPalette, exIndiPalette;

    class KThemeStylePrivate;
    KThemeStylePrivate *d;
};

#endif // QT_VERSION < 300

#endif
