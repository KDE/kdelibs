#ifndef B2STYLE_H
#define B2STYLE_H

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
 * B2Style (C)2000 Daniel M. Duley <mosfet@kde.org>
 * Animated menu code based on code by Mario Weilguni <mweilguni@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
 
class B2AniMenu : public QObject
{
    Q_OBJECT
public:
    B2AniMenu(QPopupMenu *menu);
    ~B2AniMenu();
protected:
    void scrollIn();
protected slots:
    void slotDestroyFake();
    void slotFinished();
private:
    bool eventFilter(QObject *obj, QEvent *ev);

    QPopupMenu *mnu;
    QWidget *widget;
};

class B2Style : public KStyle
{
public:
    B2Style();
    ~B2Style();
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
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup& g, QCOORD c, Orientation );
    void drawArrow(QPainter *p, Qt::ArrowType type, bool down,
                   int x, int y, int w, int h, const QColorGroup &g,
                   bool enabled=true, const QBrush *fill = 0);
    void drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orient,
                    bool tickAbove, bool tickBelow);
    void drawSliderMask(QPainter *p, int x, int y, int w, int h,
                        Orientation orient, bool, bool);
    virtual void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool floating = false);
    void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g, bool horizontal=false,
                        QBrush *fill=NULL);
    void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, QBrush *fill=NULL);
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
    void drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, QBrush *fill);
    void drawFocusRect(QPainter *p, const QRect &r, const QColorGroup &g,
                       const QColor *pen, bool atBorder);
    void drawSliderGrooveMask(QPainter *p, int x, int y, int w, int h,
                              QCOORD, Orientation);
    int defaultFrameWidth() const {return(2);}
    void polishPopupMenu(QPopupMenu *mnu);
    /*
    void drawTab(QPainter *p, const QTabBar *tabBar, QTab *tab,
                 bool selected);
    void drawTabMask( QPainter*, const QTabBar*, QTab*, bool selected );
    void tabbarMetrics(const QTabBar *t, int &hFrame, int &vFrame,
    int &overlap);*/

    void drawSplitter(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, Orientation orient);
    int splitterWidth() const {return(6);}
    void drawPanel(QPainter *p, int x, int y, int w, int h,
                   const QColorGroup &g, bool sunken, int lineWidth,
                   const QBrush *fill);
protected:
    void drawSBDeco(QPainter *p, const QRect &r, const QColorGroup &g,
                   bool horiz);
    void drawSBButton(QPainter *p, const QRect &r, const QColorGroup &g,
                      bool down=false);
private:
    QColorGroup sliderGrp, sliderGrooveGrp, radioOnGrp;
};

#endif
