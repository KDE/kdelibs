#ifndef B2STYLE_H
#define B2STYLE_H

#include <qglobal.h>
#if QT_VERSION < 300

#include <kstyle.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qpixmap.h>
#include <limits.h>

/*-
 B2Style (C)2000 Daniel M. Duley <mosfet@kde.org>
 Animated menu code based on code by Mario Weilguni <mweilguni@kde.org>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
*/

class KDEAniMenuPrivate;

class KDEAniMenu : public QObject
{
    Q_OBJECT
public:
    KDEAniMenu(QPopupMenu *menu);
    ~KDEAniMenu();
protected:
    void scrollIn();
protected slots:
    void slotDestroyFake();
    void slotFinished();
private:
    bool eventFilter(QObject *obj, QEvent *ev);

    QPopupMenu *mnu;
    QWidget *widget;
    KDEAniMenuPrivate *d;
};

class KDEStylePrivate;

class KDEStyle : public KStyle
{
public:
    KDEStyle();
    ~KDEStyle();
    virtual void polish(QWidget *w);
    virtual void unPolish(QWidget *w);
    virtual void polish(QPalette &p);
    void drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken = FALSE,
                    const QBrush *fill = 0 );
    void drawButtonMask(QPainter *p, int x, int y, int w, int h);
    void drawComboButtonMask(QPainter *p, int x, int y, int w, int h);
    QRect buttonRect(int x, int y, int w, int h);
    void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         const QBrush *fill = 0 );
    void drawPushButton(QPushButton *btn, QPainter *p);
    virtual void drawPushButtonLabel (QPushButton *btn, QPainter *p);
    void drawScrollBarControls(QPainter*,  const QScrollBar*, int sliderStart,
                               uint controls, uint activeControl );
    QStyle::ScrollControl scrollBarPointOver(const QScrollBar *sb,
                                             int sliderStart, const QPoint &p);
    void scrollBarMetrics(const QScrollBar *sb, int &sliderMin, int &sliderMax,
                          int &sliderLength, int &buttonDim);
    QSize indicatorSize() const;
    void drawIndicator(QPainter* p, int x, int y, int w, int h,
                       const QColorGroup &g, int state, bool down = FALSE,
                       bool enabled = TRUE );
    void drawIndicatorMask(QPainter *p, int x, int y, int w, int h, int);
    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator(QPainter* p,  int x, int y, int w, int h,
                                const QColorGroup &g, bool on,
                                bool down = FALSE, bool enabled =  TRUE );
    void drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                    int h, bool);
    void drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken = FALSE,
                         bool editable = FALSE, bool enabled = TRUE,
                         const QBrush *fill = 0 );
    QRect comboButtonRect(int x, int y, int w, int h);
    QRect comboButtonFocusRect(int x, int y, int w, int h);
    int sliderLength() const;
    void drawArrow(QPainter *p, Qt::ArrowType type, bool down,
                   int x, int y, int w, int h, const QColorGroup &g,
                   bool enabled=true, const QBrush *fill = 0);
    void drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orient,
                    bool tickAbove, bool tickBelow);
    void drawSliderMask(QPainter *p, int x, int y, int w, int h,
                        Orientation orient, bool, bool);
    void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, KToolBarPos type,
                      QBrush *fill=NULL);
    void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g,
                        KToolBarPos type, QBrush *fill=NULL);
    void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool macMode,
                      QBrush *fill=NULL);
    void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken=false,
                            bool raised = true, bool enabled = true,
                            bool popup = false,
                            KToolButtonType icontext = Icon,
                            const QString& btext=QString::null,
                            const QPixmap *icon=NULL,
                            QFont *font=NULL, QWidget *btn=NULL);
    void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, bool active,
                       QMenuItem *item, QBrush *fill=NULL);
    void drawPopupMenuItem(QPainter *p, bool checkable, int maxpmw,
                           int tab, QMenuItem *mi, const QPalette &pal,
                           bool act, bool enabled, int x, int y, int w,
                           int h);
    int popupMenuItemHeight(bool c, QMenuItem *mi, const QFontMetrics &fm);
    void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, QBrush *fill);
    void drawFocusRect(QPainter *p, const QRect &r, const QColorGroup &g,
                       const QColor *pen, bool atBorder);
    int defaultFrameWidth() const {return(2);}
    void polishPopupMenu(QPopupMenu *mnu);
    void drawTab(QPainter *p, const QTabBar *tabBar, QTab *tab,
                 bool selected);
    void drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected );
    void tabbarMetrics(const QTabBar *t, int &hFrame, int &vFrame,
                       int &overlap);

    void drawSplitter(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, Orientation orient);
    int splitterWidth() const {return(6);}
    void drawPanel(QPainter *p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken, int lineWidth,
                   const QBrush *fill);
    void drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, QBrush *);
    bool eventFilter(QObject *obj, QEvent *ev);
protected:
    void drawSBDeco(QPainter *p, const QRect &r, const QColorGroup &g,
                   bool horiz);
    void drawSBButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool down=false);
    void kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y,
                       QBitmap *lightColor=0, QBitmap *midColor=0,
                       QBitmap *midlightColor=0, QBitmap *darkColor=0,
                       QBitmap *blackColor=0, QBitmap *whiteColor=0);
private:
    QColorGroup radioOnGrp;
    QWidget *highlightWidget;
    void polish(QApplication *a) { QStyle::polish(a); }
    void unPolish(QApplication *a) {QStyle::unPolish(a); }

    KDEStylePrivate *d;
};

#endif // QT_VERSION < 300

#endif
