#ifndef KSTEPSTYLE_H
#define KSTEPSTYLE_H

#include <kstyle.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <limits.h>

class KStepStyle : public KStyle
{
public:
    KStepStyle();
    ~KStepStyle();
    void drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, bool sunken = FALSE,
                    const QBrush *fill = 0 );
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
    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator(QPainter* p,  int x, int y, int w, int h,
                                const QColorGroup &g, bool on,
                                bool down = FALSE, bool enabled =  TRUE );
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
    void polish( QApplication*);
    void unPolish( QApplication*);
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
                            QFont *font=NULL);
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
protected:
    void drawStepBarCircle(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g);
    void drawStepBarArrow(QPainter *p, Qt::ArrowType type, int x, int y,
                          const QColorGroup &g);
    void drawStepBarGroove(QPainter *p, QRect r, const QWidget *w,
                           const QColorGroup &g);
private:
    QColorGroup nextGrp;
};

#endif
