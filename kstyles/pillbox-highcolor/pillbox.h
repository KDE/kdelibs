#ifndef __PILLBOXSTYLE_H
#define __PILLBOXSTYLE_H

#include <kstyle.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <kpixmap.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <limits.h>

class PillBoxStyle : public KStyle
{
public:
    PillBoxStyle();
    ~PillBoxStyle();
    void polish(QWidget *w);
    void unPolish(QWidget *w);
    void polish(QPalette &);
    void drawPushButton(QPushButton *btn, QPainter *p);
    void drawPushButtonLabel(QPushButton *btn, QPainter *p);
    virtual void drawButtonMask(QPainter *p, int x, int y, int w, int h);
    void drawButton(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &cg, bool sunken,
                    const QBrush *fill=NULL);
    QRect buttonRect(int x, int y, int w, int h);
    void drawComboButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &cg, bool sunken, bool, bool,
                         const QBrush *fill);
    void drawComboButtonMask(QPainter *p, int x, int y, int w, int h);
    QRect comboButtonRect(int x, int y, int w, int h);
    void drawBevelButton(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, bool sunken,
                         const QBrush *fill=NULL);
    void drawKToolBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, KToolBarPos type,
                      QBrush *fill=NULL);
    void drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g,
                        KToolBarPos type, QBrush *fill=NULL);
    void drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                      const QColorGroup &g, bool macMode,
                      QBrush *fill=NULL);

    void drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, bool active,
                       QMenuItem *item, QBrush *fill=NULL);
    void drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            bool raised, bool enabled, bool popup,
                            KToolButtonType icontext,
                            const QString& btext, const QPixmap *pixmap,
                            QFont *font, QWidget *);
    void drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                               int sliderStart, unsigned int controls,
                               unsigned int activeControl);
    QSize indicatorSize() const;
    void drawIndicator(QPainter *p, int x, int y, int w, int h,
                       const QColorGroup &g, int state, bool down, bool);
    QSize exclusiveIndicatorSize() const;
    void drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                int h, const QColorGroup &g, bool on,
                                bool down, bool);
    void drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                          const QColorGroup &g, QCOORD, Orientation);
    int sliderLength() const;
    void drawSlider(QPainter *p, int x, int y, int w, int h,
                    const QColorGroup &g, Orientation orient, bool, bool);
    void drawArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                   int y, int w, int h, const QColorGroup &g,
                   bool enabled=true, const QBrush *fill=NULL);
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                            int tab, QMenuItem* mi, const QPalette &pal,
                            bool act, bool enabled, int x, int y, int w,
                            int h);
    void drawLightShadeRect(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, const QBrush *fill=NULL);
    void drawFocusRect(QPainter *p, const QRect &r, const QColorGroup &g,
                       const QColor *bg, bool f);
    // for repainting toolbuttons when the toolbar is resized
    bool eventFilter(QObject *obj, QEvent *ev);
protected:
    void makeWallpaper(QPixmap &dest, const QColor &base);
    void drawVGradient(QPainter *p, const QBrush &fill, int x, int y, int w,
                       int h);
    void drawHGradient(QPainter *p, const QBrush &fill, int x, int y, int w,
                       int h);
    bool macMode;
    QBitmap checkOutline;
    QBitmap checkFill;
    QBitmap comboDeco;
    QBitmap up;
    QBitmap down;
    QBitmap left;
    QBitmap right;
    QBitmap paper1;
    QBitmap paper2;
    QBitmap paper3;
    KPixmap *vSmall, *vMed, *vLarge, *hSmall, *hMed, *hLarge;
};

#endif
