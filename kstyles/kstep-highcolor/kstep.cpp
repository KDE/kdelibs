#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include <qmenudata.h>
#include "kstep.h"
#include <kapp.h>
#include <kdrawutil.h>
#include <kpixmapeffect.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <qimage.h>
#include "../highcolor/paperbits.h"

static const unsigned char downarrow_bits[] = {
 0xff,0x01,0xff,0x01,0xfe,0x00,0xfe,0x00,0x7c,0x00,0x7c,0x00,0x38,0x00,0x38,
 0x00,0x10,0x00};

static const unsigned char leftarrow_bits[] = {
 0x80,0x01,0xe0,0x01,0xf8,0x01,0xfe,0x01,0xff,0x01,0xfe,0x01,0xf8,0x01,0xe0,
 0x01,0x80,0x01};

static const unsigned char rightarrow_bits[] = {
 0x03,0x00,0x0f,0x00,0x3f,0x00,0xff,0x00,0xff,0x01,0xff,0x00,0x3f,0x00,0x0f,
 0x00,0x03,0x00};

static const unsigned char uparrow_bits[] = {
  0x10, 0x00, 0x38, 0x00, 0x38, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0xfe, 0x00,
  0xfe, 0x00, 0xff, 0x01, 0xff, 0x01};

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
    if(QPixmap::defaultDepth() > 8){
        vSmall = new KPixmap;
        vSmall->resize(20, 24);
        vMed = new KPixmap;
        vMed->resize(20, 34);
        vLarge = new KPixmap;
        vLarge->resize(20, 64);
        hMed = new KPixmap;
        hMed->resize(34, 20);
        hLarge = new KPixmap;
        hLarge->resize(52, 20);
    }
    else{
        vSmall = vMed = vLarge = NULL;
        hMed = hLarge = NULL;
    }
}

KStepStyle::~KStepStyle()
{
    if(vSmall){
        delete vSmall;
        delete vMed;
        delete vLarge;
        delete hMed;
        delete hLarge;
    }
}

void KStepStyle::polish(QApplication *)
{
    setScrollBarExtent(17, 17);
}

void KStepStyle::polish(QPalette &)
{
    QPalette pal = QApplication::palette();
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    nextGrp = pal.normal();
    nextGrp.setColor(QColorGroup::Dark, Qt::black);

    if(vSmall){
        KPixmapEffect::gradient(*vSmall, nextGrp.midlight(),
                                nextGrp.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*vMed, nextGrp.midlight(),
                                nextGrp.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*vLarge, nextGrp.midlight(),
                                nextGrp.mid(),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*hMed, nextGrp.midlight(),
                                nextGrp.mid(),
                                KPixmapEffect::HorizontalGradient);
        KPixmapEffect::gradient(*hLarge, nextGrp.midlight(),
                                nextGrp.mid(),
                                KPixmapEffect::HorizontalGradient);
    }
    config->setGroup("Highcolor");
    QString tmpStr = config->readEntry("CustomWallpaper", "");
    if(!tmpStr.isEmpty()){
        QPixmap wallPaper(tmpStr);
        if(!wallPaper.isNull())
            pal.setBrush(QColorGroup::Background,
                 QBrush(pal.color(QPalette::Active, QColorGroup::Background),
                        wallPaper));
        else
            qWarning("Highcolor KStep: Unable to load wallpaper %s",
                    tmpStr.latin1());
    }
    else if(config->readBoolEntry("UseWallpaper", true)){
        QPixmap wallPaper;
        makeWallpaper(wallPaper,pal.color(QPalette::Active,
                                          QColorGroup::Background));
        pal.setBrush(QColorGroup::Background,
                     QBrush(pal.color(QPalette::Active, QColorGroup::Background),
                            wallPaper));
    }
    config->setGroup(oldGrp);
}

void KStepStyle::unPolish(QApplication *)
{
    setScrollBarExtent(16, 16);
}

void KStepStyle::polish(QWidget *w)
{
    if(w->isTopLevel())
        return;

    if(w->inherits("QMenuBar") || w->inherits("KToolBarButton")){
        w->setBackgroundMode(QWidget::NoBackground);
        return;
    }
    if(w->inherits("QLabel") || w->inherits("QButton") ||
       w->inherits("QComboBox")){
        if(w->parent() && !w->parent()->inherits("KToolBar") &&
           !w->parent()->inherits("KHTMLView"))
            w->setBackgroundOrigin(QWidget::ParentOrigin);
        else
            w->setAutoMask(true);
        return;
    }
    if(w->inherits("KToolBar")){
        w->installEventFilter(this);
        w->setBackgroundMode(QWidget::NoBackground);
    }
}
    
void KStepStyle::unPolish(QWidget *w)
{
    if (w->isTopLevel())
        return;

    if(w->inherits("QMenuBar") || w->inherits("KToolBarButton")){
        w->setBackgroundMode(QWidget::PaletteBackground);
        return;
    }
    if(w->inherits("QLabel") || w->inherits("QButton") ||
       w->inherits("QComboBox")){
        if(w->parent() && !w->parent()->inherits("KToolBar") &&
           !w->parent()->inherits("KHTMLView"))
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
        else
            w->setAutoMask(false);
        return;
    }
    if(w->inherits("KToolBar")){
        w->removeEventFilter(this);
        w->setBackgroundMode(QWidget::PaletteBackground);
        return;
    }

}

bool KStepStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if(ev->type() == QEvent::Resize){
        // must be a toolbar resize
        QObjectList *tbChildList = obj->queryList("KToolBarButton", NULL,
                                                  false, false);
        QObjectListIt it(*tbChildList);
        QObject *child;
        while((child = it.current()) != NULL){
            ++it;
            if(child->isWidgetType())
                ((QWidget *)child)->repaint(false);
        }
	delete tbChildList;
    }
    return(false);

}


void KStepStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            const QBrush *)
{
    QPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);

    p->setPen(sunken ? Qt::black : g.light());
    p->drawLine(x, y, x2-1, y);
    p->drawLine(x, y, x, y2-1);
    p->setPen(sunken ? g.midlight() : g.mid());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->setPen(sunken ? g.light() : Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);
    p->setPen(oldPen);             
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

        
        drawItem(p,
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
    int deco_y = (h-6)/2;
    drawButton(p, x + w-16, y + deco_y, 10, 6, nextGrp, false, fill);
    p->setPen(g.mid());
    p->drawLine( x+w-6, y+deco_y+6, x+w-15, y+deco_y+6);
    p->drawLine( x+w-6, y+deco_y+1, x+w-6, y+deco_y+6);

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
        subX = 1;
        addX = buttonDim;
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        subPage.setRect(buttonDim*2, 0, sliderStart-1, extent);
        addPage.setRect(sliderEnd, 0, len-sliderEnd, extent);
        slider.setRect(sliderStart, 0, sliderLength, extent);
    }
    else{
        /*
        subX = addX = (extent - buttonDim)/2;
        subY = len - (buttonDim*2);
        addY = len - buttonDim-1;
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        subPage.setRect(0, 1, extent, sliderStart-1);
        addPage.setRect(0, sliderEnd, extent, subY-sliderEnd);
        slider.setRect(0, sliderStart, extent, sliderLength);*/

        subX = addX = 0;
        subY = len - (buttonDim*2);
        addY = len - buttonDim;
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        subPage.setRect(0, 1, extent, sliderStart-1);
        addPage.setRect(0, sliderEnd, extent, subY-sliderEnd);
        slider.setRect(0, sliderStart, extent, sliderLength);
        
    }

    if(controls & AddLine){
        if(add.isValid()){
            p->setPen(g.dark());
            p->drawRect(add);
            drawLightRect(p, add.x()+1, add.y()+1, add.width()-2,
                          add.height()-2, g, activeControl == AddLine);
            drawVGradient(p, g.brush(QColorGroup::Mid), add.x()+2, add.y()+2,
                          add.width()-4, add.height()-4);
            drawStepBarArrow(p, (horizontal) ? RightArrow : DownArrow,
                             add.x()+4, add.y()+4, nextGrp);
        }
    }
    if(controls & SubLine){
        if(sub.isValid()){
            p->setPen(g.dark());
            p->drawRect(sub);
            drawLightRect(p, sub.x()+1, sub.y()+1, sub.width()-2,
                          sub.height()-2, g, activeControl == AddLine);
            drawVGradient(p, g.brush(QColorGroup::Mid), sub.x()+2, sub.y()+2,
                          sub.width()-4, sub.height()-4);
            drawStepBarArrow(p, (horizontal) ? LeftArrow : UpArrow, sub.x()+4,
                             sub.y()+4, nextGrp);
        }
    }
    if((controls & SubPage)){
        drawStepBarGroove(p, subPage, sb, g, horizontal);
    }
    if((controls & AddPage)){
        drawStepBarGroove(p, addPage, sb, g, horizontal);
    }
    if(controls & Slider){
        if(slider.isValid() && slider.width() > 1 && slider.height() > 1){
            p->setPen(g.dark());
            p->drawRect(slider);
            if(horizontal){
                drawSBSlider(p, slider.x()+1, slider.y()+1, slider.width()-2,
                              slider.height()-2, g, activeControl==Slider,
                              Qt::Horizontal);
            }
            else{
                drawSBSlider(p, slider.x()+1, slider.y()+1, slider.width()-2,
                             slider.height()-2, g, activeControl==Slider,
                             Qt::Vertical);
            }
            if(slider.width() > 8 && slider.height() > 8)
                drawStepBarCircle(p, slider.x(), slider.y(), slider.width(),
                                  slider.height(), nextGrp);
        }
    }
}

void KStepStyle::drawLightRect(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, bool down)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(down ? g.mid() : g.light());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(down ? g.light() : g.mid());
    p->drawLine(x2, y, x2, y2);
    p->drawLine(x, y2, x2, y2);
}


void KStepStyle::drawSBSlider(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool sunken, Orientation o)
{
    drawLightRect(p, x, y, w, h, g, sunken);
    if(o == Horizontal)
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    else
        drawHGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
}

void KStepStyle::drawStepBarGroove(QPainter *p, QRect r, const QWidget *w,
                                   const QColorGroup &g, bool horiz)
{
    // dont draw over black lines or it flickers
    if(r.left() == 0)
        r.setLeft(1);
    if(r.top() == 0)
        r.setTop(1);
    if(r.right() == w->width()-1)
        r.setRight(w->width()-2);
    if(r.bottom() == w->height()-1)
        r.setBottom(w->height()-2);
    if(horiz)
        p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(),
                           *vMed, 0, vMed->height()-(r.height()));
    else
        p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(),
                           *hMed, hMed->width()-(r.width()), 0);
    p->setPen(g.dark());
    p->drawRect(0, 0, w->width(), w->height());
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
        sliderMin = buttonDim*2;
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
        if(state == QButton::On){
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
        else{
            drawStepBarCircle(p, x, y, w, h, nextGrp);
        }
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
    p->fillRect(x, y, w, h, Qt::color1);
}

void KStepStyle::drawStepBarArrow(QPainter *p, Qt::ArrowType type, int x,
                                  int y, const QColorGroup &g)
{
    if (upArrow.isNull())
    {
        upArrow = QBitmap(9, 9, uparrow_bits, true);
        downArrow = QBitmap(9, 9, downarrow_bits, true);
        leftArrow = QBitmap(9, 9, leftarrow_bits, true);
        rightArrow = QBitmap(9, 9, rightarrow_bits, true);
        upArrow.setMask(upArrow);
        downArrow.setMask(downArrow);
        leftArrow.setMask(leftArrow);
        rightArrow.setMask(rightArrow);
    }
    p->setPen(g.dark());
    switch(type){
    case Qt::UpArrow:
        p->drawPixmap(x, y, upArrow);
        break;
    case Qt::DownArrow:
        p->drawPixmap(x, y, downArrow);
        break;
    case Qt::LeftArrow:
        p->drawPixmap(x, y, leftArrow);
        break;
    default:
        p->drawPixmap(x, y, rightArrow);
        break;
    }


}

void KStepStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QCOORD,
                                    Orientation orient)
{
    qDrawShadePanel(p, x, y, w, h, nextGrp, true, 1);
    if(orient == Horizontal)
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    else
        drawHGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
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
    drawButton(p, x, y, w, h, g, false);
}

void KStepStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &, bool, QBrush *)
{
    drawButton(p, x, y, w, h, nextGrp, false);
}

void KStepStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, KToolBarPos, QBrush *)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    if(w > h)
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    else
        drawHGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
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
                                     QFont *font, QWidget *btn)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    int dx, dy;

    if(raised || sunken){
        p->setPen(sunken ? g.dark() : g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->setPen(sunken ? g.light() : g.dark());
        p->drawLine(x, y2, x2-1, y2);
        p->drawLine(x2, y, x2, y2-1);
        drawVGradient(p, g.brush(QColorGroup::Mid), x+1, y+1, w-2, h-2);
    }
    else{
        if(btn->parent() && btn->parent()->isWidgetType()){
            QWidget *toolbar = (QWidget*)btn->parent();
            // horizontal toolbar
            if(toolbar->width() > toolbar->height()){
                // See if we are top row. Buttons are offset a few pixels
                // but not visibly.
                if(btn->y() <= 3){
                    if(toolbar->height() <= 24)
                        p->drawTiledPixmap(x, y, w, h, *vSmall);
                    else if(toolbar->height() <= 34)
                        p->drawTiledPixmap(x, y, w, h, *vMed);
                    else
                        p->drawTiledPixmap(x, y, w, h, *vLarge);

                }
                // See if we are in the gradient at all. Two rows always are
                // large.
                else if(btn->y() <= 64){
                    p->fillRect(x, y, w, h, g.mid());
                    p->drawTiledPixmap(x, y, w, 64-btn->y(),
                                       *vLarge, 0, btn->y());
                }
                // nope, we are not in the gradient
                else
                    p->fillRect(x, y, w, h, g.mid());
            }
            // vertical toolbar
            else{
                if(btn->x() <= 3){
                    if(toolbar->width() <= 34)
                        p->drawTiledPixmap(x, y, w, h, *hMed);
                    else
                        p->drawTiledPixmap(x, y, w, h, *hLarge);

                }
                else if(btn->x() <= 52){
                    p->fillRect(x, y, w, h, g.mid());
                    p->drawTiledPixmap(x, y, 52-btn->x(), h,
                                       *hLarge, btn->x(), 0);
                }
                else
                    p->fillRect(x, y, w, h, g.mid());
            }
        }
    }
    p->setPen(g.text());
    
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

// no check here, make sure your highcolor beforehand ;-)
void KStepStyle::drawVGradient(QPainter *p, const QBrush &fill, int x, int y,
                               int w, int h)
{
    if(h <= 24){
        p->drawTiledPixmap(x, y, w, h, *vSmall);
    }
    else if(h <= 34){
        p->drawTiledPixmap(x, y, w, h, *vMed);
    }
    else if(h <= 64){
        p->drawTiledPixmap(x, y, w, h, *vLarge);
    }
    else{
        p->fillRect(x, y+vLarge->height(), w, h-vLarge->height(), fill);
        p->drawTiledPixmap(x, y, w, vLarge->height(), *vLarge);
    }
}

void KStepStyle::drawHGradient(QPainter *p, const QBrush &fill, int x, int y,
                               int w, int h)
{
    if(w <= 34){
        p->drawTiledPixmap(x, y, w, h, *hMed);
    }
    else if(w <= 52){
        p->drawTiledPixmap(x, y, w, h, *hLarge);
    }
    else{
        p->fillRect(x+hLarge->width(), y, w-hLarge->width(), h, fill);
        p->drawTiledPixmap(x, y, hLarge->width(), h, *hLarge);
    }
}

void KStepStyle::makeWallpaper(QPixmap &dest, const QColor &base)
{
    if (paper1.isNull())
    {
        paper1 = QBitmap(45, 45, paper_1_bits, true);
        paper2 = QBitmap(45, 45, paper_2_bits, true);
        paper3 = QBitmap(45, 45, paper_3_bits, true);
        paper1.setMask(paper1);
        paper2.setMask(paper2);
        paper3.setMask(paper3);
    }
    dest.resize(45, 45);
    dest.fill(base);
    QPainter p;
    p.begin(&dest);
    p.setPen(base.dark(104));
    p.drawPixmap(0, 0, paper1);
    p.setPen(base.light(104));
    p.drawPixmap(0, 0, paper2);
    p.setPen(base.light(106));
    p.drawPixmap(0, 0, paper3);
    p.end();

}




