#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>
#include "kstep.h"
#include <kapplication.h>
#include <kdrawutil.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <qpointarray.h>

static const unsigned char arrow_dark_bits[] = {
 0x30,0x3c,0x08,0x84,0xc4,0x87,0x02,0x80,0x01,0x00,0x02,0x80,0x04,0x80,0x08,
 0x80,0x10,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x29,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x40,0xf0,0x29,0x40 };

static const unsigned char arrow_mid_bits[] = {
 0x00,0x00,0x20,0x80,0x20,0x80,0x00,0x80,0x00,0x00,0x00,0x80,0x00,0x80,0x00,
 0x80,0x00,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x31,0x00,0x00,0x00,0x68,0x80,
 0xb2,0x40,0x00,0x00,0x00,0x00 };

static const unsigned char arrow_light_bits[] = {
 0x00,0x40,0x00,0xc0,0x00,0xc0,0x00,0xc0,0x00,0x40,0x00,0x40,0xe0,0xff,0x20,
 0x80,0x20,0x00,0x00,0x00,0x50,0x00,0x00,0x00,0x29,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x40,0xf0,0x29,0x40 };

KStepStyle::KStepStyle()
    :KStyle()
{
    setButtonDefaultIndicatorWidth(4);
    setScrollBarExtent(20,20);
}

KStepStyle::~KStepStyle()
{
    ;
}

void KStepStyle::polish(QApplication *app)
{
    app->setPalette( QPalette(nextGrp, nextGrp, nextGrp), TRUE, "QPopupMenu" );
}

void KStepStyle::polish(QPalette &)
{
    nextGrp = QApplication::palette().active();
    nextGrp.setColor(QColorGroup::Dark, Qt::black);
}

void KStepStyle::unPolish(QApplication *)
{
}

void KStepStyle::polish(QWidget *)
{
}

void KStepStyle::unPolish(QWidget *)
{
}

void KStepStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            const QBrush *fill)
{
    kDrawNextButton(p, x, y, w, h, g, sunken, fill);
}

void KStepStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QRect r = btn->rect();
    if(btn->isDefault() || btn->autoDefault()){
        if(btn->isDefault())
            kDrawNextButton(p, r, nextGrp, true);
        drawButton(p, r.x()+4, r.y()+4, r.width()-8, r.height()-8, nextGrp,
                   btn->isOn() || btn->isDown());

    }
    else
        drawButton(p, r.x(), r.y(), r.width(), r.height(), nextGrp,
                   btn->isOn() || btn->isDown());

}

void KStepStyle::drawPushButtonLabel(QPushButton *btn, QPainter *p)
{
    int x1, y1, x2, y2, w, h;
    btn->rect().coords(&x1, &y1, &x2, &y2);
    w = btn->width();
    h = btn->height();

    bool act = btn->isOn() || btn->isDown();
    if (arrowLightBmp.isNull())
    {
        arrowLightBmp = QBitmap(15, 9, arrow_light_bits, true);
        arrowDarkBmp = QBitmap(15, 9, arrow_dark_bits, true);
        arrowMidBmp = QBitmap(15, 9, arrow_mid_bits, true);
        arrowLightBmp.setMask(arrowLightBmp);
        arrowDarkBmp.setMask(arrowDarkBmp);
        arrowMidBmp.setMask(arrowMidBmp);
    }

    // Draw iconset first, if any
    if ( btn->iconSet() && !btn->iconSet()->isNull() )
    {
	QIconSet::Mode mode = btn->isEnabled()
			      ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && btn->hasFocus() )
	    mode = QIconSet::Active;
	QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();

	p->drawPixmap( x1+6, y1+h/2-pixh/2, pixmap );
	x1 += pixw + 8;
	w -= pixw + 8;
    }
    
    if (btn->isDefault()) {

      // If this is a default button, we have a 4 pixel border which is
      // used to give a frame.
      // We need to add the 'return' pixmap in the correct position too.
      // We adjust the width available for the label according to how
      // much space is left after adding the pixmap at the right edge.
      // - rikkus

        // Changed to kColorBitmaps (mosfet
        kColorBitmaps(p, btn->colorGroup(), btn->width() - 24,
                      btn->height() / 2 - 4, &arrowLightBmp, &arrowMidBmp,
                      NULL, &arrowDarkBmp, NULL, NULL);


        drawItem( p,
          ( x1 + ( act ? 1 : 0 ) ) + 6, y1 + ( act ? 1 : 0 ), w - 26, h,
          AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
          btn->pixmap(), btn->text(), -1,
          act ? &btn->colorGroup().light() : &btn->colorGroup().buttonText());

    } else {

        drawItem( p, x1 + ( act ? 1 : 0 ), y1 + ( act ? 1 : 0 ), w, h,
          AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
          btn->pixmap(), btn->text(), -1,
          act ? &btn->colorGroup().light() : &btn->colorGroup().buttonText());
   }
}

void KStepStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &, bool sunken,
                                 const QBrush *fill)
{
    drawButton(p, x, y, w, h, nextGrp, sunken, fill);
}

QRect KStepStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x, y, w, h));
}

void KStepStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken,
                                 bool, bool, const QBrush
                                 *fill)
{
    drawButton(p, x, y, w, h, nextGrp, sunken, fill);
    int deco_y = (h-8)/2;
    QPen oldPen = p->pen();
    p->setPen(g.mid());
    p->fillRect(x + w-13, y + deco_y + 2, 9, 6, g.brush(QColorGroup::Mid));
    p->setPen(oldPen);
    drawButton(p, x + w-15, y + deco_y, 9, 6, nextGrp, false, fill);
}

QRect KStepStyle::comboButtonRect(int x, int y, int w, int h){
    return(QRect(x+4, y+4, w-8-16, h-8));
}

QRect KStepStyle::comboButtonFocusRect(int x, int y, int w, int h)
{
    return(QRect(x+5, y+5, w-10-16, h-10));
}

void KStepStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                       int sliderStart, uint controls,
                                       uint activeControl)
{
    int sliderMin, sliderMax, sliderLength, buttonDim;
    QRect add, sub, addPage, subPage, slider;
    int addX, addY, subX, subY, subPageX, offsetX, offsetY;
    bool horizontal = sb->orientation() == QScrollBar::Horizontal;
    int len = (horizontal) ? sb->width() : sb->height();
    int extent = (horizontal) ? sb->height() : sb->width();
    QColorGroup g = sb->colorGroup();

    scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);
    if(sliderStart > sliderMax)
        sliderStart = sliderMax;

    int sliderEnd = sliderStart + sliderLength;

    if(horizontal){
        offsetY = (extent - buttonDim)/2;
        subX = 2;
        addX = subX + buttonDim + 1;
        subPageX = addX + buttonDim + 1;
        sub.setRect(2, offsetY, buttonDim, buttonDim);
        add.setRect(addX, offsetY, buttonDim, buttonDim);
        subPage.setRect(subPageX, offsetY, sliderStart-subPageX, extent-4);
        addPage.setRect(sliderEnd, offsetY, len-sliderEnd-2, extent-4);
        slider.setRect(sliderStart, offsetY, sliderLength, extent-4);
    }
    else{
        offsetX = (extent - buttonDim)/2;
        addY = len - buttonDim - 2;
        subY = addY - buttonDim - 1;
        sub.setRect(offsetX, subY, buttonDim, buttonDim);
        add.setRect(offsetX, addY, buttonDim, buttonDim);
        subPage.setRect(offsetX, 2, extent-4, sliderStart-2);
        addPage.setRect(offsetX, sliderEnd, extent-4, subY-sliderEnd-1);
        slider.setRect(offsetX, sliderStart, extent-4, sliderLength);
    }

    if(controls & AddLine){
        if(add.isValid()){
            drawButton(p, add.x(), add.y(), add.width(), add.height(),
                            nextGrp, activeControl == AddLine);
            drawStepBarArrow(p, (horizontal) ? RightArrow : DownArrow,
                             add.x()+3, add.y()+3, add.width()-7, nextGrp);
        }
    }

    if(controls & SubLine){
        if(sub.isValid()){
            drawButton(p, sub.x(), sub.y(), sub.width(), sub.height(),
                            nextGrp, activeControl == SubLine);
            drawStepBarArrow(p, (horizontal) ? LeftArrow : UpArrow,
                            sub.x()+3, sub.y()+3, sub.width()-7, nextGrp);
        }
    }

    if((controls & SubPage))
        p->fillRect(subPage, g.brush(QColorGroup::Mid));

    if((controls & AddPage))
        p->fillRect(addPage, g.brush(QColorGroup::Mid));

    if(controls & Slider){
        if(slider.isValid() && slider.width() > 1 && slider.height() > 1){
            if(horizontal)
                drawButton(p, slider.x(), slider.y(), slider.width(),
                           slider.height(), nextGrp);
            else
                drawButton(p, slider.x(), slider.y(), slider.width(),
                           slider.height(), nextGrp);
            if(slider.width() > 8 && slider.height() > 8)
                drawStepBarCircle(p, slider.x(), slider.y(), slider.width(),
                                  slider.height(), nextGrp);
        }
    }
    p->setPen(Qt::black);
    p->drawRect(0, 0, sb->width(), sb->height());
}

void KStepStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim)
{
    int maxlen;
    bool horizontal = sb->orientation() == QScrollBar::Horizontal;
    int len = (horizontal) ? sb->width() : sb->height();
    int extent = (horizontal) ? sb->height() : sb->width();

    if(len > (extent - 1)*2)
        buttonDim = extent - 4;
    else
        buttonDim = len/2 - 1;

    if(horizontal)
        sliderMin = buttonDim*2 + 4;
    else
        sliderMin = 2;

    maxlen = len - buttonDim*2 - 6;
    sliderLength = (sb->pageStep()*maxlen) / (sb->maxValue() -
        sb->minValue() + sb->pageStep());

    if(sliderLength < 12)
        sliderLength = 12;
    if(sliderLength > maxlen)
        sliderLength = maxlen;
    sliderMax = sliderMin + maxlen - sliderLength;
}

QStyle::ScrollControl KStepStyle::scrollBarPointOver(const QScrollBar *sb,
                                                     int sliderStart,
                                                     const QPoint &p)
{
    if(!sb->rect().contains(p))
        return(NoScroll);
    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

    if(sb->orientation() == QScrollBar::Horizontal){
        int x = p.x();
        if(x <= buttonDim)
            return(SubLine);
        else if(x <= buttonDim*2)
            return(AddLine);
        else if(x < sliderStart)
            return(SubPage);
        else if(x < sliderStart+sliderLength)
            return(Slider);
        return(AddPage);
    }
    else{
        int y = p.y();
        if(y < sliderStart)
            return(SubPage);
        else if(y < sliderStart+sliderLength)
            return(Slider);
        else if(y < sliderMax+sliderLength)
            return(AddPage);
        else if(y < sliderMax+sliderLength+buttonDim)
            return(SubLine);
        return(AddLine);
    }
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

void KStepStyle::drawStepBarCircle(QPainter *p, int x, int y, int w,
                                   int h, const QColorGroup &g)
{
    static const QCOORD circle_dark[] = {2,0, 3,0, 4,0,
    1,1,
    0,2,
    0,3,
    0,4
    };

    static const QCOORD circle_mid[] = {1,0,
    0,1, 2,1, 3,1, 4,1, 5,1,
    1,2, 2,2,
    1,3,
    1,4,
    1,5
    };

    static const QCOORD circle_light[] = { 4,3, 5,3,
    3, 4, 4,4, 5,4,
    3,5, 4,5
    };

    QPen oldPen = p->pen();
    x = x + w/2 - 3;
    y = y + h/2 - 3;

    QPointArray a( QCOORDARRLEN(circle_dark), circle_dark );
    a.translate( x, y );
    p->setPen( g.dark() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_light), circle_light );
    a.translate( x, y );
    p->setPen( g.light() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_mid), circle_mid );
    a.translate( x, y );
    p->setPen( g.mid() );
    p->drawPoints( a );
    p->setPen(oldPen);
}

QSize KStepStyle::exclusiveIndicatorSize() const
{
    return(QSize(15,15));
}

void KStepStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                        int h, const QColorGroup &g, bool on,
                                        bool down, bool)
{
    static const QCOORD circle_dark[] = {5,1, 6,1, 7,1, 8,1, 9,1, 10,1,
    3,2, 4,2,
    2,3,
    2,4,
    1,5,
    1,6,
    1,7,
    1,8,
    1,9,
    2,10
    };

    static const QCOORD circle_mid[] = {5,0, 6,0, 7,0, 8,0, 9,0,
    3,1, 4,1, 11,1,
    2,2, 10,2, 12,2,
    1,3, 3,3,
    1,4,
    0,5,
    0,6,
    0,7,
    0,8,
    0,9,
    1,10,
    1,11,
    2,12
    };

    static const QCOORD circle_light[] = {14,5,
    14,6,
    14,7,
    14,8,
    14,9,
    13,10,
    13,11,
    12,12,
    4,13, 5,13, 10,13, 11,13,
    5,14, 6,14, 7,14, 8,14, 9,14
    };

    static const QCOORD fill_lines[] = {6,3, 9,3, 3,6, 3,9, 12,6, 12,9, 6,12, 9,12};

    QPen oldPen = p->pen();
    p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));
    QPointArray a( QCOORDARRLEN(circle_dark), circle_dark );
    a.translate( x, y );
    p->setPen(nextGrp.dark() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_mid), circle_mid );
    a.translate( x, y );
    p->setPen(nextGrp.mid() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_light), circle_light );
    a.translate( x, y );
    p->setPen(nextGrp.light() );
    p->drawPoints( a );
    if(on || down){
        if(down)
            p->setPen(nextGrp.mid());
        a.setPoints(QCOORDARRLEN(fill_lines), fill_lines);
        a.translate(x,y);
        p->drawLineSegments(a);
        p->fillRect(4, 4, 8, 8, (down)? g.brush(QColorGroup::Mid) :
                    nextGrp.brush(QColorGroup::Light));
    }
    p->setPen(oldPen);
}

QSize KStepStyle::indicatorSize() const
{
    return(QSize(16, 16));
}

void KStepStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &, int state, bool down, bool)
{
    drawButton(p, x, y, w, h, nextGrp, down);
    if(state != QButton::Off){
        QPen oldPen = p->pen();
        p->setPen(nextGrp.light());
        p->drawLine(x+5, y+6, x+5, y+10);
        p->drawLine(x+5, y+10, x+w-4, y+3);
        p->setPen(nextGrp.dark());
        p->drawLine(x+5, y+11, x+w-4, y+4);
        p->drawLine(x+6, y+6, x+6, y+7);
        p->setPen(nextGrp.mid());
        p->drawLine(x+6, y+11, x+w-4, y+5);
        p->drawLine(x+6, y+8, x+7, y+7);
        p->setPen(oldPen);
    }
}

void KStepStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h,
                                   int)
{
    p->fillRect(x, y, w, h, Qt::color1);
}

void KStepStyle::drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                            int h, bool)
{
  p->setPen(Qt::color1);
  p->setBrush(Qt::color1);
  p->drawEllipse(x, y, w, h);
}

void KStepStyle::drawStepBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, int base, const QColorGroup &g)
{
    QBrush oldBrush = p->brush();
    QPen oldPen = p->pen();
    p->setBrush(g.brush(QColorGroup::Dark));
    p->setPen(g.dark());

    int x2=x+base-1, y2=y+base-1;
    int i=0, j=0;

    switch (type){
    case Qt::UpArrow:
        do {
          p->drawLine(x+i/2, y2-j, x2-i/2, y2-j);
          i++; j++;
        } while (j<base);
      break;
    case Qt::DownArrow:
        do {
          p->drawLine(x+i/2, y+j, x2-i/2, y+j);
          i++; j++;
        } while (j<base);
        break;
    case Qt::LeftArrow:
        do {
          p->drawLine(x2-j, y+i/2, x2-j, y2-i/2);
          i++; j++;
        } while (j<base);
        break;
    default:
        do {
          p->drawLine(x+j, y+i/2, x+j, y2-i/2);
          i++; j++;
        } while (j<base);
        break;
    }

    p->setBrush(oldBrush);
    p->setPen(oldPen);
}

void KStepStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation)
{
    qDrawShadePanel(p, x, y, w, h, nextGrp, true, 1,
                    &g.brush(QColorGroup::Dark));
    //drawButton(p, x, y, w, h, nextGrp, true);
}

int KStepStyle::sliderLength() const
{
    return(30);
}

void KStepStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation orient,
                            bool, bool)
{
    drawButton(p, x+1, y+1, w-2, h-2, nextGrp, false);
    if(orient == Horizontal){
        int mid = x+w/2;
        qDrawShadeLine(p, mid, y+1, mid, y+h-2, g, true, 1);
    }
    else{
        int mid = y+w/2;
        qDrawShadeLine(p, x+1, mid, x+w-2, mid, g, true, 1);
    }
}

void KStepStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)
{
    qDrawArrow(p, type, Qt::MotifStyle, down, x, y, w, h, g, enabled);
}

void KStepStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, KToolBarPos, QBrush *)
{
    drawButton(p, x, y, w, h, g, false, &g.brush(QColorGroup::Mid));
}

void KStepStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool, QBrush *)
{
    drawButton(p, x, y, w, h, nextGrp, false, &g.brush(QColorGroup::Background));
}

void KStepStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, KToolBarPos, QBrush *)
{
   // drawButton(p, x, y, w, h, nextGrp, false, &g.brush(QColorGroup::Background));
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Background));
    p->setPen(g.light());
    p->drawLine(x, y, x2-1, y);
    p->drawLine(x, y, x, y2-1);
    p->setPen(Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
}

void KStepStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken,
                                    bool raised, bool enabled, bool popup,
                                    KToolButtonType icontext,
                                    const QString& btext, const QPixmap *pixmap,
                                    QFont *font, QWidget *)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    if(sunken || raised){
        p->setPen(sunken ? g.dark() : g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(sunken ? g.light() : g.dark());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        p->fillRect(x+1, y+1, w-2, h-2, sunken? g.mid() : g.background());
    }
    else
        p->fillRect(x, y, w, h, g.background());
    int dx, dy;

    if (icontext == Icon){ // icon only
        if (pixmap){
            dx = ( w - pixmap->width() ) / 2;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken )
            {
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
    }
    else if (icontext == IconTextRight){ // icon and text (if any)
        if (pixmap){
            dx = 1;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            if (pixmap)
                dx= pixmap->width();
            else
                dx= 1;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= 1;
            dy= 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = 1;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            if (!enabled)
                p->setPen(g.dark());
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(Qt::blue);
            p->drawText(x, y, w, h-4, tf, btext);
        }
    }
    if (popup){
        if (enabled)
            qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, Qt::DownArrow, Qt::WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
}

void KStepStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool active, QMenuItem *mi,
                               QBrush *)
{
    QColor btext = active ? g.light() : g.buttonText();
    if(active){
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(g.dark());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        p->fillRect(x+1, y+1, w-2, h-2, g.mid());
    }
    QApplication::style().drawItem(p, x, y, w, h,
                                   AlignCenter|ShowPrefix|DontClip|SingleLine,
                                   g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                   -1, &btext );
}

void KStepStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
static const int motifItemFrame         = 2;
static const int motifItemHMargin       = 3;
static const int motifItemVMargin       = 2;
static const int motifArrowHMargin      = 6;
static const int windowsRightBorder     = 12;
    if(act){
        bool dis = !enabled;
        QColorGroup itemg = dis ? pal.disabled() : pal.active();

        if (checkable)
            maxpmw = QMAX( maxpmw, 12 );
        int checkcol = maxpmw;

        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(itemg.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(itemg.dark());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        p->fillRect(x+1, y+1, w-2, h-2, itemg.mid());
        if ( mi->iconSet() ) {
            QIconSet::Mode mode = dis? QIconSet::Disabled : QIconSet::Normal;
            if (!dis)
                mode = QIconSet::Active;
            QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            QRect cr(x, y, checkcol, h);
            QRect pmr(0, 0, pixw, pixh);
            pmr.moveCenter( cr.center() );
            p->setPen(itemg.text());
            p->drawPixmap(pmr.topLeft(), pixmap );

        }
        else if(checkable) {
            int mw = checkcol + motifItemFrame;
            int mh = h - 2*motifItemFrame;
            if (mi->isChecked()){
                drawCheckMark( p, x + motifItemFrame,
                               y+motifItemFrame, mw, mh, itemg, act, dis );
            }
        }
        p->setPen(itemg.light());
        QColor discol;
        if (dis) {
            discol = itemg.text();
            p->setPen(discol);
        }
        int xm = motifItemFrame + checkcol + motifItemHMargin;
        QString s = mi->text();
        if (!s.isNull()) {
            int t = s.find( '\t' );
            int m = motifItemVMargin;
            const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
            if (t >= 0) {
                p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
                            y+m, tab, h-2*m, text_flags, s.mid( t+1 ));
            }
            p->drawText(x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
        } else if (mi->pixmap()) {
            QPixmap *pixmap = mi->pixmap();
            if (pixmap->depth() == 1)
                p->setBackgroundMode(OpaqueMode);
            p->drawPixmap( x+xm, y+motifItemFrame, *pixmap);
            if (pixmap->depth() == 1)
                p->setBackgroundMode(TransparentMode);
        }
        if (mi->popup()) {
            int dim = (h-2*motifItemFrame) / 2;
            if (!dis)
                discol = itemg.text();
            QColorGroup g2(discol, itemg.highlight(),
                           white, white,
                           dis ? discol : white,
                           discol, white);
            drawArrow(p, RightArrow, true,
                      x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                      dim, dim, itemg, TRUE);
        }
    }
    else
        KStyle::drawPopupMenuItem(p, checkable, maxpmw, tab, mi, pal, act,
                                  enabled, x, y, w, h);
}

void KStepStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill)
{
    qDrawShadePanel(p, x, y, w, h, nextGrp, false, 1, fill ? fill : &g.brush(QColorGroup::Dark));
}

void KStepStyle::drawFocusRect(QPainter *p, const QRect &r,
                               const QColorGroup &, const QColor *,
                               bool atBorder)
{
    if (!atBorder)
        qDrawShadePanel(p, r, nextGrp, true, 1);
    else
        qDrawShadePanel(p, r.x()+1, r.y()+1, r.width()-2, r.height()-2, nextGrp,
                        true, 1);
}



