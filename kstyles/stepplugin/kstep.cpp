#include "kstep.h"
#include <kapp.h>
#include <qpalette.h>


KStepStyle::KStepStyle()
    :KStyle()
{
    ;
}

KStepStyle::~KStepStyle()
{
    ;
}

void KStepStyle::polish(QApplication *app)
{
    oldPalette = app->palette();

    // Play with the palette a little
    QColorGroup normal = oldPalette.normal();
    QColorGroup disabled = oldPalette.disabled();
    QColorGroup active = oldPalette.active();

    normal.setColor(QColorGroup::Dark, Qt::black);
    disabled.setColor(QColorGroup::Dark, Qt::black);
    active.setColor(QColorGroup::Dark, Qt::black);
    
    app->setPalette(QPalette(normal, disabled, active));
}

void KStepStyle::unPolish(QApplication *app)
{
    app->setPalette(oldPalette, TRUE);
}

void KStepStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            const QBrush *fill)
{
    QPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    
    if(!sunken){
        p->fillRect(x+1, y+1, w-2, h-2,
                    fill ? *fill : g.brush(QColorGroup::Button));
        p->setPen(g.light());
        p->drawLine(x, y, x2-1, y);
        p->drawLine(x, y, x, y2-1);
        p->setPen(g.mid());
        p->drawLine(x+1, y2-1, x2-1, y2-1);
        p->drawLine(x2-1, y+1, x2-1, y2-1);
        p->setPen(g.dark());
        p->drawLine(x, y2, x2, y2);
        p->drawLine(x2, y, x2, y2);
    }
    else{
        p->fillRect(x+1, y+1, w-2, h-2,
                    fill ? *fill : g.brush(QColorGroup::Mid));
        p->setPen(g.dark());
        p->drawLine(x, y, x2-1, y);
        p->drawLine(x, y, x, y2-1);
        p->setPen(g.light());
        p->drawLine(x, y2, x2, y2);
        p->drawLine(x2, y, x2, y2);
    }
    p->setPen(oldPen);
}

void KStepStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken,
                                 const QBrush *fill)
{
    drawButton(p, x, y, w, h, g, sunken, fill);
}

QRect KStepStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+2, y+2, w-4, h-4));
}

void KStepStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool,
                                 bool, bool, const QBrush
                                 *fill)
{
    drawButton(p, x, y, w, h, g, fill);

    int deco_y = (h-6)/2;
    drawButton(p, w-16, deco_y, 10, 6, g, false, fill);
    p->setPen(g.mid());
    p->drawLine(w-6, deco_y+6, w-15, deco_y+6);
    p->drawLine(w-6, deco_y+1, w-6, deco_y+6);
    
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
    int addX, addY, subX, subY;
    bool horizontal = sb->orientation() == QScrollBar::Horizontal;
    int len = (horizontal) ? sb->width() : sb->height();
    int extent = (horizontal) ? sb->height() : sb->width();
    QColorGroup g = sb->colorGroup();
    
    scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);
    if(sliderStart > sliderMax)
        sliderStart = sliderMax;

    int sliderEnd = sliderStart + sliderLength;

    if(horizontal){
        subY = addY = (extent - buttonDim)/2;
        subX = 0;
        addX = buttonDim;
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        subPage.setRect(buttonDim*2, 0, sliderStart-1, extent);
        addPage.setRect(sliderEnd, 0, len-sliderEnd, extent);
        slider.setRect(sliderStart, 0, sliderLength, extent);
    }
    else{
        subX = addX = (extent - buttonDim)/2;
        subY = len - (buttonDim*2);
        addY = len - buttonDim;
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        subPage.setRect(0, 1, extent, sliderStart-1);
        addPage.setRect(0, sliderEnd, extent, subY-sliderEnd);
        slider.setRect(0, sliderStart, extent, sliderLength);
    }

    if(controls & AddLine){
        p->setPen(g.dark());
        p->drawRect(add);
        qDrawShadePanel(p, add.x()+1, add.y()+1, add.width()-2,
                        add.height()-2, g, activeControl == AddLine, 1,
                        &g.brush(QColorGroup::Button));
        drawStepBarArrow(p, (horizontal) ? RightArrow : DownArrow,
                         add.x()+3, add.y()+3, add.width()-6,
                         add.height()-6, g);
    }
    if(controls & SubLine){
        p->setPen(g.dark());
        p->drawRect(sub);
        qDrawShadePanel(p, sub.x()+1, sub.y()+1, sub.width()-2,
                        sub.height()-2, g, activeControl == SubLine, 1,
                        &g.brush(QColorGroup::Button));
        drawStepBarArrow(p, (horizontal) ? LeftArrow : UpArrow, sub.x()+3,
                         sub.y()+3, sub.width()-6, sub.height()-6, g);
    }
    if((controls & SubPage)){
        drawStepBarGroove(p, subPage, horizontal, false, g);
    }
    if((controls & AddPage)){
        drawStepBarGroove(p, addPage, horizontal, true, g);
    }
    if(controls & Slider){
        p->setPen(g.dark());
        p->drawRect(slider);
        if(slider.width() > 1 && slider.height() > 1){
            drawButton(p, slider.x()+1, slider.y()+1, slider.width()-2,
                       slider.height()-2, g);
            if(slider.width() > 8 && slider.height() > 8)
                drawStepBarCircle(p, slider.x(), slider.y(), slider.width(),
                                  slider.height(), g);
        }
    }
}

void KStepStyle::drawStepBarGroove(QPainter *p, QRect &r, bool horizontal,
                                   bool addPage, const QColorGroup &g)
{
    p->setBackgroundMode(OpaqueMode);
    p->setBackgroundColor(g.button());
    QBrush fillBrush(g.dark(), Dense4Pattern);
    if(r.width() > 4 && r.height() >4){
        p->setPen(g.button());
        if(horizontal){
            p->fillRect(r.x(), r.y()+2, r.width(), r.height()-4, fillBrush);
            p->drawLine(r.x(), r.y()+1, r.right(), r.y()+1);
            p->drawLine(r.x(), r.bottom()-1, r.right(), r.bottom()-1);
            if(addPage)
                p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom()-1);
        }
        else{
            p->fillRect(r.x()+2, r.y(), r.width()-4, r.height(), fillBrush);
            p->drawLine(r.x()+1, r.y(), r.x()+1, r.bottom());
            p->drawLine(r.right()-1, r.y(), r.right()-1, r.bottom());
            if(!addPage)
                p->drawLine(r.x()+1, r.top()+1, r.right()-1, r.top()+1);
        }
    }
    else
        p->fillRect(r, fillBrush);
    p->setPen(g.dark());
    if(horizontal){
        p->drawLine(r.x(), r.y(), r.right(),
                    r.y());
        p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
        p->drawLine(r.right(), r.y(), r.right(), r.bottom());
    }
    else{
        p->drawLine(r.x(), r.y(), r.x(), r.bottom());
        p->drawLine(r.right(), r.y(), r.right(), r.bottom());
        p->drawLine(r.x(), r.top(), r.right(), r.top());
    }
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
        buttonDim = extent;
    else
        buttonDim = len/2 - 1;

    if(horizontal)
        sliderMin = buttonDim*2+1;
    else
        sliderMin = 1;
    
    maxlen = len - buttonDim*2 - 1;
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
    static QCOORD circle_dark[] = {2,0, 3,0, 4,0,
    1,1,
    0,2,
    0,3,
    0,4
    };

    static QCOORD circle_mid[] = {1,0,
    0,1, 2,1, 3,1, 4,1, 5,1,
    1,2, 2,2,
    1,3,
    1,4,
    1,5
    };

    static QCOORD circle_light[] = { 4,3, 5,3,
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
    static QCOORD circle_dark[] = {5,1, 6,1, 7,1, 8,1, 9,1, 10,1,
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

    static QCOORD circle_mid[] = {5,0, 6,0, 7,0, 8,0, 9,0,
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
    
    static QCOORD circle_light[] = {14,5,
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

    static QCOORD fill_lines[] = {6,3, 9,3, 3,6, 3,9, 12,6, 12,9, 6,12, 9,12};

    QPen oldPen = p->pen();
    p->fillRect( x, y, w, h, g.brush(QColorGroup::Background));
    QPointArray a( QCOORDARRLEN(circle_dark), circle_dark );
    a.translate( x, y );
    p->setPen(g.dark() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_mid), circle_mid );
    a.translate( x, y );
    p->setPen( g.mid() );
    p->drawPoints( a );
    a.setPoints( QCOORDARRLEN(circle_light), circle_light );
    a.translate( x, y );
    p->setPen( g.light() );
    p->drawPoints( a );
    if(on || down){
        if(down)
            p->setPen(g.mid());
        a.setPoints(QCOORDARRLEN(fill_lines), fill_lines);
        a.translate(x,y);
        p->drawLineSegments(a);
        p->fillRect(4, 4, 8, 8, (down)? g.brush(QColorGroup::Mid) :
                    g.brush(QColorGroup::Light));
    }
    p->setPen(oldPen);
}

QSize KStepStyle::indicatorSize() const
{
    return(QSize(16, 16));
}

void KStepStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, int state, bool down, bool)
{
    drawButton(p, x, y, w, h, g, down);
    if(state != QButton::Off){
        QPen oldPen = p->pen();
        p->setPen(g.light());
        p->drawLine(x+5, y+6, x+5, y+10);
        p->drawLine(x+5, y+10, x+w-4, y+3);
        p->setPen(g.dark());
        p->drawLine(x+5, y+11, x+w-4, y+4);
        p->drawLine(x+6, y+6, x+6, y+7);
        p->setPen(g.mid());
        p->drawLine(x+6, y+11, x+w-4, y+5);
        p->drawLine(x+6, y+8, x+7, y+7);
        p->setPen(oldPen);
    }
}

void KStepStyle::drawStepBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, int w, int h, const QColorGroup &g)
{
    QPointArray a;
    int x2 = x+w-1;
    int y2 = y+h-1;
    switch(type){
    case Qt::UpArrow:
        a.setPoints(4, x,y2, x2,y2, x+w/2,y, x,y2);
        break;
    case Qt::DownArrow:
        a.setPoints(4, x,y, x2,y, x+w/2,y2, x,y);
        break;
    case Qt::LeftArrow:
        a.setPoints(4, x2,y, x2,y2, x,y+h/2, x2,y);
        break;
    default:
        a.setPoints(4, x,y, x,y2, x2,y+h/2, x,y);
        break;
    }

    QBrush oldBrush = p->brush();
    p->setBrush(g.brush(QColorGroup::Dark));
    p->drawPolygon(a);
    p->setBrush(oldBrush);
}

void KStepStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation)
{
    drawButton(p, x, y, w, h, g, true);
}

int KStepStyle::sliderLength() const
{
    return(30);
}

void KStepStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, Orientation orient,
                            bool, bool)
{
    drawButton(p, x, y, w, h, g, false);
    if(orient == Horizontal){
        int mid = x+w/2;
        qDrawShadeLine(p, mid, y, mid, y+h, g, true, 1);
    }
    else{
        int mid = y+w/2;
        qDrawShadeLine(p, x, mid, x+w, mid, g, true, 1);
    }
}

void KStepStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)        
{
    qDrawArrow(p, type, Qt::MotifStyle, down, x, y, w, h, g, enabled);
}

