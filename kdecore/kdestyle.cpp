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


#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <qglobal.h>

#if QT_VERSION < 300

#include <qmenudata.h>
#include "kdestyle.h"
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <qpalette.h>
#include <qbitmap.h>
#include <qtabbar.h>
#include <qpopupmenu.h>
#include <qimage.h>
#include <qtimer.h>
#include <unistd.h>

static unsigned char radiooff_light_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0xf0, 0x09, 0xf8, 0x0b, 0xfc, 0x17, 0xfc, 0x17,
    0xfc, 0x17, 0xfc, 0x17, 0xfc, 0x17, 0xf8, 0x0b, 0xf0, 0x09, 0x0c, 0x06,
    0xf0, 0x01};

static unsigned char radiooff_gray_bits[] = {
    0xf0, 0x01, 0x0c, 0x06, 0x02, 0x00, 0x02, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00};

static unsigned char radiooff_dgray_bits[] = {
    0x00, 0x00, 0xf0, 0x01, 0x0c, 0x02, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00};

static unsigned char radiomask_bits[] = {
  0xf0, 0x01, 0xfc, 0x07, 0xfe, 0x0b, 0xfe, 0x0b, 0xff, 0x17, 0xff, 0x17,
  0xff, 0x17, 0xff, 0x17, 0xff, 0x17, 0xfe, 0x0b, 0xf2, 0x09, 0x0c, 0x06,
  0xf0, 0x01};

KDEAniMenu::KDEAniMenu(QPopupMenu *menu)
    : QObject()
{
    mnu = menu;
    widget = NULL;
    menu->installEventFilter(this);
    connect(mnu, SIGNAL(destroyed()), this, SLOT(slotFinished()));
}

KDEAniMenu::~KDEAniMenu()
{
    if(widget)
        delete widget;
}

bool KDEAniMenu::eventFilter(QObject *, QEvent *ev)
{
    if(ev->type() == QEvent::Show)
        scrollIn();
    else if(ev->type() == QEvent::Hide){
        ;
    }
    return(false);
}

void KDEAniMenu::scrollIn()
{
    int w = mnu->width();
    int h = mnu->height();

    QPixmap bgPix(QPixmap::grabWindow(QApplication::desktop()->winId(),
                                      mnu->x(), mnu->y(), w, h));
    QPixmap mnuPix;

    mnuPix.resize(w, h);
    mnuPix.fill(mnu->colorGroup().color(QColorGroup::Background));
    QPainter::redirect(mnu, &mnuPix);
    mnu->repaint(false);
    QPainter::redirect(mnu, 0);
    if(!widget)
        widget = new QWidget(0, 0, WStyle_Customize | WStyle_NoBorder |
                        WStyle_Tool);
    widget->move(mnu->x(), mnu->y());
    widget->resize(w, h);
    widget->setBackgroundMode(QWidget::NoBackground);
    widget->show();
    bitBlt(widget, 0, 0, &bgPix);
    int x;
    for(x = 0; x <= w-3; x+=2)
        bitBlt(widget, x, 0, &mnuPix, x, 0, x+2, h);
    QTimer::singleShot(1, this, SLOT(slotDestroyFake()));
}

void KDEAniMenu::slotDestroyFake()
{
    if(widget)
        delete(widget);
    widget = NULL;
}

void KDEAniMenu::slotFinished()
{
    if(widget)
        delete(widget);
    delete this;
}


KDEStyle::KDEStyle()
    :KStyle()
{
    setButtonDefaultIndicatorWidth(3);
    highlightWidget = 0L;
}

KDEStyle::~KDEStyle()
{
}

void KDEStyle::polish(QPalette &pal)
{
    KConfig *config = KGlobal::config();
    QString oldGrp = config->group();
    config->setGroup("KDEStyle");

    QColor tmpColor(0, 0, 192);
    if(config->hasKey("RadioOnColor")){
        tmpColor = config->readColorEntry("RadioOnColor", &tmpColor);
        radioOnGrp.setColor(QColorGroup::Mid, tmpColor);
        radioOnGrp.setColor(QColorGroup::Button, tmpColor);
        radioOnGrp.setColor(QColorGroup::Light, tmpColor.light(130));
        radioOnGrp.setColor(QColorGroup::Dark, tmpColor.dark(130));
    }
    else{
        radioOnGrp.setColor(QColorGroup::Mid, tmpColor);
        radioOnGrp.setColor(QColorGroup::Button, tmpColor);
        radioOnGrp.setColor(QColorGroup::Light, QColor(0, 0, 255));
        radioOnGrp.setColor(QColorGroup::Dark, QColor(0, 0, 128));
    }
    config->setGroup("KStyle"); // all kstyle global flags will go here
    if(config->readBoolEntry("inactiveShading", false)){
        QColorGroup aGrp = pal.active();
        QColorGroup dGrp = pal.disabled();
        QColorGroup iGrp = aGrp;

        iGrp.setColor(QColorGroup::Mid, aGrp.button());
        iGrp.setColor(QColorGroup::Dark, aGrp.mid());
        dGrp.setColor(QColorGroup::Mid, aGrp.button());
        dGrp.setColor(QColorGroup::Dark, aGrp.mid());

        pal.setInactive(iGrp);
        pal.setDisabled(dGrp);
    }
    config->setGroup(oldGrp);
}

void KDEStyle::polish(QWidget *w)
{
    if (w->isTopLevel())
        return;
    if(w->inherits("QButton") ) {
        if(!w->parent() || ( !w->parent()->inherits("KToolBar")))
            w->setBackgroundOrigin(QWidget::ParentOrigin);
    }

    // this needs updated draw routine that doesn't depend on being masked
    if(w->inherits("QComboBox"))
        w->setAutoMask(true);

    if(w->inherits("QPushButton")){
        w->installEventFilter(this);
    }
}

void KDEStyle::unPolish(QWidget *w)
{
    if (w->isTopLevel())
        return;

    if(w->inherits("QButton")){
        if(!w->parent() || (!w->parent()->inherits("KToolBar")))
            w->setBackgroundOrigin(QWidget::WidgetOrigin);
    }

    if(w->inherits("QComboBox"))
        w->setAutoMask(false);

    if(w->inherits("QPushButton")){
        w->removeEventFilter(this);
    }
}

bool KDEStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if(ev->type() == QEvent::Enter){
        QWidget *btn = (QWidget *)obj;
        if (btn->isEnabled())
        {
            highlightWidget = btn;
            btn->repaint(false);
        }
    }
    else if(ev->type() == QEvent::Leave){
        QWidget *btn = (QWidget *)obj;
	if (btn == highlightWidget)
	    highlightWidget = 0L;
        btn->repaint(false);
    }
    return(false);
}

void KDEStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken,
                            const QBrush *fill)
{
    QPen oldPen = p->pen();
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawLine(x+1, y, x2-1, y);
    p->drawLine(x, y+1, x, y2-1);
    p->drawLine(x+1, y2, x2-1, y2);
    p->drawLine(x2, y+1, x2, y2-1);

    if(!sunken){
        p->setPen(g.light());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }
    else{
        p->setPen(g.mid());
        p->drawLine(x+2, y+2, x2-1, y+2);
        p->drawLine(x+2, y+3, x2-2, y+3);
        p->drawLine(x+2, y+4, x+2, y2-1);
        p->drawLine(x+3, y+4, x+3, y2-2);
    }


    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-1);
    p->drawLine(x2-2, y+3, x2-2, y2-2);

    if(fill)
        p->fillRect(x+4, y+4, w-6, h-6, *fill);

    p->setPen(oldPen);

}

void KDEStyle::drawPushButton(QPushButton *btn, QPainter *p)
{
    QRect r = btn->rect();
    bool sunken = btn->isOn() || btn->isDown();
    QColorGroup g = btn->colorGroup();

    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();
    
    if(btn->isDefault()){
        int x2 = r.right();
        int y2 = r.bottom();
        p->setPen(g.dark());
        p->drawLine(x+1, y, x2-1, y);
        p->drawLine(x, y+1, x, y2-1);
        p->drawLine(x+1, y2, x2-1, y2);
        p->drawLine(x2, y+1, x2, y2-1);
    }
    if(btn->isDefault() || btn->autoDefault()){
        int indiWidth = buttonDefaultIndicatorWidth();
        x+=indiWidth;
        y+=indiWidth;
        w-=indiWidth*2;
        h-=indiWidth*2;
    }
    drawButton(p, x, y, w, h, g, sunken, (btn == highlightWidget) ?
                                          &g.brush(QColorGroup::Midlight) :
                                          &g.brush(QColorGroup::Button));

}

void KDEStyle::drawPushButtonLabel(QPushButton *btn, QPainter *p)
{
    int x1, y1, x2, y2, w, h;
    btn->rect().coords(&x1, &y1, &x2, &y2);
    w = btn->width();
    h = btn->height();

    bool act = btn->isOn() || btn->isDown();
    if(act){
        ++x1, ++y1;
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
    
    if(act || btn->isDefault()){
        QFont font = btn->font();
        font.setBold(true);
        p->setFont(font);
        drawItem( p, x1 + 1, y1 + 1, w, h,
                 AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1, act ?
                 &btn->colorGroup().dark() : &btn->colorGroup().mid());

        drawItem( p, x1, y1, w, h,
                 AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1, act ?
                 &btn->colorGroup().light() : &btn->colorGroup().buttonText());
    }
    else{
        drawItem( p, x1 + ( act ? 1 : 0 ), y1 + ( act ? 1 : 0 ), w, h,
                 AlignCenter | ShowPrefix, btn->colorGroup(), btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1,
                 act ? &btn->colorGroup().light() : &btn->colorGroup().buttonText());
    }
}

void KDEStyle::drawButtonMask(QPainter *p, int x, int y, int w, int h)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    p->setPen(color0);
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

void KDEStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                              const QColorGroup &g, bool sunken,
                              const QBrush *fill)
{
    // I like black instead of dark() for the darkest shade
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(sunken ? Qt::black : g.light());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(sunken ? g.light() : Qt::black);
    p->drawLine(x, y2, x2, y2);
    p->drawLine(x2, y, x2, y2);

    p->setPen(sunken ? g.dark() : g.midlight());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);
    p->setPen(sunken ? g.midlight() : g.dark());
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->fillRect(x+2, y+2, w-4, h-4, fill ? *fill :
                g.brush(QColorGroup::Button));

}

QRect KDEStyle::buttonRect(int x, int y, int w, int h)
{
    return(QRect(x+4, y+4, w-6, h-6));
}

void KDEStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken,
                                 bool edit, bool, const QBrush *fill)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawRect(x, y, w, h);
    p->fillRect(x+2, y+2, w-4, h-4, fill ? *fill :
                g.brush(QColorGroup::Background));

    p->setPen(sunken? g.light() : g.mid());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);

    p->setPen(sunken? g.mid() : g.light());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+2, x+1, y2-2);

    p->setPen(g.dark());
    p->drawPoint(x+1, y+1);

    if(edit){
        x2 = x2-17;
        y2 = y2-3;
        p->setPen(g.dark());
        //p->drawRect(x+2, y+2, x2, y2);
        p->drawLine(x+3, y+2, x2, y+2);
        p->drawLine(x+2, y+3, x+2, y2);
    }

    int arrow_h = h / 3;
    int arrow_w = arrow_h;
    int arrow_x = w - arrow_w - 6;
    int arrow_y = (h - arrow_h) / 2;
    drawArrow(p, DownArrow, false, arrow_x, arrow_y, arrow_w, arrow_h, g, true);
}

void KDEStyle::drawComboButtonMask(QPainter *p, int x, int y, int w, int h)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    p->setPen(color0);
    p->drawPoint(x, y);
    p->drawPoint(x, y+1);
    p->drawPoint(x+1, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

QRect KDEStyle::comboButtonRect(int x, int y, int w, int h)
{
    return(QRect(x+3, y+3, w - (h / 3) - 13, h-6));
}

QRect KDEStyle::comboButtonFocusRect(int x, int y, int w, int h)
{
    return(QRect(x+3, y+3, w-(h/3)-13, h-6));
}


void KDEStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                       int sliderStart, uint controls,
                                       uint activeControl)
{
    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax)
        sliderStart = sliderMax;

    bool horiz = sb->orientation() == QScrollBar::Horizontal;
    QColorGroup g = sb->colorGroup();
    QRect addB, subB2, subB;
    QRect addPageR, subPageR, sliderR;
    int addX, addY, subX, subY;
    int len = horiz ? sb->width() : sb->height();
    int extent = horiz ? sb->height() : sb->width();

    if (horiz) {
        subY = addY = ( extent - buttonDim ) / 2;
        subX = 0;
        addX = len - buttonDim;
    }
    else {
        subX = addX = ( extent - buttonDim ) / 2;
        subY = 0;
        addY = len - buttonDim;
    }
    subB.setRect( subX,subY,buttonDim,buttonDim );
    addB.setRect( addX,addY,buttonDim,buttonDim );
    if(horiz)
        subB2.setRect(addX-buttonDim,addY,buttonDim,buttonDim );
    else
        subB2.setRect(addX,addY-buttonDim,buttonDim,buttonDim );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent;

    if (horiz) {
        subPageR.setRect( subB.right() + 1, 0,
                          sliderStart - subB.right() - 1 , sliderW );
        addPageR.setRect( sliderEnd, 0, addX - sliderEnd - buttonDim, sliderW );
        sliderR .setRect( sliderStart, 0, sliderLength, sliderW );
    }
    else {
        subPageR.setRect( 0, subB.bottom() + 1, sliderW,
                          sliderStart - subB.bottom() - 1 );
        addPageR.setRect( 0, sliderEnd, sliderW, addY - buttonDim - sliderEnd);
        sliderR .setRect( 0, sliderStart, sliderW, sliderLength );
    }

    bool maxed = sb->maxValue() == sb->minValue();

    if ( controls & AddLine ) {
        drawSBButton(p, addB, g, activeControl == AddLine);
        drawArrow( p, horiz ? RightArrow : DownArrow,
                   false, addB.x()+4, addB.y()+4,
                   addB.width()-8, addB.height()-8, g, !maxed);
    }
    if ( controls & SubLine ) {
        drawSBButton(p, subB, g, activeControl == SubLine);
        drawArrow( p, horiz ? LeftArrow : UpArrow,
                   false, subB.x()+4, subB.y()+4,
                   subB.width()-8, subB.height()-8, g, !maxed);
        drawSBButton(p, subB2, g, activeControl == SubLine);
        drawArrow( p, horiz ? LeftArrow : UpArrow,
                   false, subB2.x()+4, subB2.y()+4,
                   subB2.width()-8, subB2.height()-8, g, !maxed);
    }
    if(controls & AddPage){
        if(addPageR.width()){
            p->fillRect(addPageR, activeControl == AddPage ?
                        g.brush(QColorGroup::Midlight) :
                        g.brush(QColorGroup::Mid));
            p->setPen(g.dark());
            if(horiz){
                p->drawLine(addPageR.x(), addPageR.y(), addPageR.right(),
                            addPageR.y());
                p->drawLine(addPageR.x(), addPageR.bottom(), addPageR.right(),
                            addPageR.bottom());
                p->setPen(activeControl==AddPage ? g.mid() : g.background());
                p->drawLine(addPageR.x(), addPageR.y()+1, addPageR.right(),
                            addPageR.y()+1);
            }
            else{
                p->drawLine(addPageR.x(), addPageR.y(), addPageR.x(),
                            addPageR.bottom());
                p->drawLine(addPageR.right(), addPageR.y(), addPageR.right(),
                            addPageR.bottom());
                p->setPen(activeControl==AddPage ? g.mid() : g.background());
                p->drawLine(addPageR.x()+1, addPageR.y(), addPageR.x()+1,
                            addPageR.bottom());
            }
        }
    }
    if(controls & SubPage){
        if(subPageR.height()){
            p->fillRect(subPageR, activeControl == SubPage ?
                        g.brush(QColorGroup::Midlight) :
                        g.brush(QColorGroup::Mid));
            p->setPen(g.dark());
            if(horiz){
                p->drawLine(subPageR.x(), subPageR.y(), subPageR.right(),
                            subPageR.y());
                p->drawLine(subPageR.x(), subPageR.bottom(), subPageR.right(),
                            subPageR.bottom());
                p->setPen(activeControl==SubPage ? g.mid() : g.background());
                p->drawLine(subPageR.x(), subPageR.y()+1, subPageR.right(),
                            subPageR.y()+1);
                p->drawLine(subPageR.x(), subPageR.y()+1, subPageR.x(),
                            subPageR.bottom()-1);
            }
            else{
                p->drawLine(subPageR.x(), subPageR.y(), subPageR.x(),
                            subPageR.bottom());
                p->drawLine(subPageR.right(), subPageR.y(), subPageR.right(),
                            subPageR.bottom());
                p->setPen(activeControl==SubPage ? g.mid() : g.background());
                p->drawLine(subPageR.x()+1, subPageR.y(), subPageR.x()+1,
                            subPageR.bottom());
                p->drawLine(subPageR.x()+1, subPageR.y(), subPageR.right()-1,
                            subPageR.y());
            }
        }
    }
    if ( controls & Slider ) {
        drawSBButton(p, sliderR, g, activeControl == Slider);
        g.setColor(QColorGroup::Dark, g.mid());
        drawSBDeco(p, sliderR, g, horiz);
    }
}

void KDEStyle::drawSBButton(QPainter *p, const QRect &r, const QColorGroup &g,
                           bool down)
{
    p->setPen(g.dark());
    p->drawRect(r);
    p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2,
                g.brush(down ? QColorGroup::Midlight : QColorGroup::Button));

    p->setPen(down ? g.light() : g.mid());
    p->drawLine(r.x()+1, r.bottom()-1, r.right()-1, r.bottom()-1);
    p->drawLine(r.right()-1, r.top()+1, r.right()-1, r.bottom()-1);
    p->setPen(down ? g.mid() : g.light());
    p->drawLine(r.x()+1, r.y()+1, r.right()-1, r.y()+1);
    p->drawLine(r.x()+1, r.y()+1, r.x()+1, r.bottom()-1);
}

void KDEStyle::drawSBDeco(QPainter *p, const QRect &r, const QColorGroup &g,
                         bool horiz)
{
    if(horiz){
        if(r.width() >= 15){
            int y = r.y()+3;
            int x = r.x() + (r.width()-7)/2;
            int y2 = r.bottom()-3;
            p->setPen(g.light());
            p->drawLine(x, y, x, y2);
            p->drawLine(x+3, y, x+3, y2);
            p->drawLine(x+6, y, x+6, y2);

            p->setPen(g.mid());
            p->drawLine(x+1, y, x+1, y2);
            p->drawLine(x+4, y, x+4, y2);
            p->drawLine(x+7, y, x+7, y2);
        }
    }
    else{
        if(r.height() >= 15){
            int x = r.x()+3;
            int y = r.y() + (r.height()-7)/2;
            int x2 = r.right()-3;
            p->setPen(g.light());
            p->drawLine(x, y, x2, y);
            p->drawLine(x, y+3, x2, y+3);
            p->drawLine(x, y+6, x2, y+6);

            p->setPen(g.mid());
            p->drawLine(x, y+1, x2, y+1);
            p->drawLine(x, y+4, x2, y+4);
            p->drawLine(x, y+7, x2, y+7);
        }
    }
}


void KDEStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                  int &sliderMax, int &sliderLength,
                                  int &buttonDim)
{

    int maxLength;
    int b = 0;
    bool horiz = sb->orientation() == QScrollBar::Horizontal;
    int length = horiz ? sb->width()  : sb->height();
    int extent = horiz ? sb->height() : sb->width();

    if ( length > ( extent - b*2 - 1 )*2 + b*2  )
	buttonDim = extent - b*2;
    else
	buttonDim = ( length - b*2 )/2 - 1;

    sliderMin = b + buttonDim;
    maxLength  = length - b*2 - buttonDim*3;

    if ( sb->maxValue() == sb->minValue() ) {
	sliderLength = maxLength;
    } else {
	sliderLength = (sb->pageStep()*maxLength)/
			(sb->maxValue()-sb->minValue()+sb->pageStep());
	uint range = sb->maxValue()-sb->minValue();
	if ( sliderLength < 9 || range > INT_MAX/2 )
	    sliderLength = 9;
	if ( sliderLength > maxLength )
	    sliderLength = maxLength;
    }
    sliderMax = sliderMin + maxLength - sliderLength;

}

QStyle::ScrollControl KDEStyle::scrollBarPointOver(const QScrollBar *sb,
                                                     int sliderStart,
                                                     const QPoint &p)
{
    if ( !sb->rect().contains( p ) )
        return NoScroll;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    pos = (sb->orientation() == QScrollBar::Horizontal)? p.x() : p.y();
    if ( pos < sliderMin )
	return SubLine;
    if ( pos < sliderStart )
	return SubPage;
    if ( pos < sliderStart + sliderLength )
        return Slider;
    if ( pos < sliderMax + sliderLength)
        return AddPage;
    if(pos > sliderMax + sliderLength + 16)
        return AddLine;

    return SubLine;
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)


QSize KDEStyle::exclusiveIndicatorSize() const
{
    return(QSize(13,13));
}

void KDEStyle::drawExclusiveIndicator(QPainter *p, int x, int y, int w,
                                     int h, const QColorGroup &g, bool on,
                                     bool down, bool)
{
    static QBitmap lightBmp(13, 13, radiooff_light_bits, true);
    static QBitmap grayBmp(13, 13, radiooff_gray_bits, true);
    static QBitmap dgrayBmp(13, 13, radiooff_dgray_bits, true);

    p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
    kColorBitmaps(p, g, x, y, &lightBmp, &grayBmp,
                  NULL, &dgrayBmp);

    if(on || down){
        p->setPen(Qt::black);
        p->drawLine(5, 4, 7, 4);
        p->drawLine(4, 5, 4, 7);
        p->drawLine(5, 8, 7, 8);
        p->drawLine(8, 5, 8, 7);
        p->fillRect(5, 5, 3, 3, Qt::black);
    }
}

void KDEStyle::drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                         int h, bool)
{
    static QBitmap maskBmp(13, 13, radiomask_bits, true);
    p->fillRect(x, y, w, h, Qt::color0);
    p->setPen(Qt::color1);
    p->drawPixmap(x, y, maskBmp);
}


QSize KDEStyle::indicatorSize() const
{
    return(QSize(13, 13));
}

void KDEStyle::drawIndicator(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, int state, bool down, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;

    static unsigned char x_bits[] = {0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63};
    static QBitmap xBmp(7, 7, x_bits, true);
    if(!xBmp.mask())
        xBmp.setMask(xBmp);


    p->setPen(g.mid());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);

    p->setPen(g.light());
    p->drawLine(x2, y+1, x2, y2);
    p->drawLine(x+1, y2, x2, y2);

    p->setPen(g.dark());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);

    p->setPen(g.midlight());
    p->drawLine(x2-1, y+2, x2-1, y2-1);
    p->drawLine(x+2, y2-1, x2-1, y2-1);

    p->fillRect(x+2, y+2, w-4, h-4, down ? g.brush(QColorGroup::Background) :
                g.brush(QColorGroup::Light));

    if(state != QButton::Off){
        if(state == QButton::On){
            p->setPen(Qt::black);
            p->drawPixmap(3, 3, xBmp);
        }
        else{
            p->setPen(g.dark());
            p->drawRect(x+2, y+2, w-4, h-4);
            p->setPen(Qt::black);
            p->drawLine(x+3, (y+h)/2-2, x+w-4, (y+h)/2-2);
            p->drawLine(x+3, (y+h)/2, x+w-4, (y+h)/2);
            p->drawLine(x+3, (y+h)/2+2, x+w-4, (y+h)/2+2);
        }
    }

}

void KDEStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h, int)
{
    // needed for some reason by KHtml, even tho it's all filled ;P
    p->fillRect(x, y, w, h, Qt::color1);
}

void KDEStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                         const QColorGroup &g, Orientation orient,
                         bool, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, g.background());
    p->setPen(g.mid());
    p->drawLine(x, y, x2, y);
    p->drawLine(x, y, x, y2);
    p->setPen(Qt::black);
    p->drawLine(x2, y, x2, y2);
    p->drawLine(x, y2, x2, y2);

    p->setPen(g.dark());
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->setPen(g.light());
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->drawLine(x+1, y+1, x+1, y2-1);

    p->setPen(g.mid());
    p->drawLine(x2-2, y+2, x2-2, y2-2);
    p->drawLine(x+2, y2-2, x2-2, y2-2);
    p->setPen(g.midlight());
    p->drawLine(x+2, y+2, x2-2, y+2);
    p->drawLine(x+2, y+2, x+2, y2-2);

    if (orient == Horizontal){
        p->setPen(g.light());
        p->drawLine(x+5, y+4, x+5, y2-4);
        p->drawLine(x+8, y+4, x+8, y2-4);
        p->drawLine(x+11, y+4, x+11, y2-4);
        p->setPen(g.dark());
        p->drawLine(x+6, y+4, x+6, y2-4);
        p->drawLine(x+9, y+4, x+9, y2-4);
        p->drawLine(x+12, y+4, x+12, y2-4);
    }
    else{
        p->setPen(g.light());
        p->drawLine(x+4, y+5, x2-4, y+5);
        p->drawLine(x+4, y+8, x2-4, y+8);
        p->drawLine(x+4, y+11, x2-4, y+11);
        p->setPen(g.dark());
        p->drawLine(x+4, y+6, x2-4, y+6);
        p->drawLine(x+4, y+9, x2-4, y+9);
        p->drawLine(x+4, y+12, x2-4, y+12);
    }
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

void KDEStyle::drawSliderMask(QPainter *p, int x, int y, int w, int h,
                             Orientation, bool, bool)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->fillRect(x, y, w, h, Qt::color1);
    p->setPen(Qt::color0);
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
}

int KDEStyle::sliderLength() const
{
    return(18);
}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
void KDEStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool on, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)
{
    static QCOORD u_arrow[]={3,1, 4,1, 2,2, 5,2, 1,3, 6,3, 0,4, 7,4, 0,5, 7,5};
    static QCOORD d_arrow[]={0,2, 7,2, 0,3, 7,3, 1,4, 6,4, 2,5, 5,5, 3,6, 4,6};
    static QCOORD l_arrow[]={1,3, 1,4, 2,2, 2,5, 3,1, 3,6, 4,0, 4,7, 5,0, 5,7};
    static QCOORD r_arrow[]={2,0, 2,7, 3,0, 3,7, 4,1, 4,6, 5,2, 5,5, 6,3, 6,4};

    p->setPen(enabled ? on ? g.light() : g.buttonText() : g.mid());
    if(w > 8){
        x = x + (w-8)/2;
        y = y + (h-8)/2;
    }

    QPointArray a;
    switch(type){
    case Qt::UpArrow:
        a.setPoints(QCOORDARRLEN(u_arrow), u_arrow);
        break;
    case Qt::DownArrow:
        a.setPoints(QCOORDARRLEN(d_arrow), d_arrow);
        break;
    case Qt::LeftArrow:
        a.setPoints(QCOORDARRLEN(l_arrow), l_arrow);
        break;
    default:
        a.setPoints(QCOORDARRLEN(r_arrow), r_arrow);
        break;
    }

    a.translate(x, y);
    p->drawLineSegments(a);
}

void KDEStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, KToolBarPos,
                             QBrush *)
{
    if(h > w){
        p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
        int x2 = x+w-1;
        int y2 = y+h-1;

        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->drawLine(x+3, y+4, x+3, y2-4);
        p->drawLine(x+5, y+4, x+5, y2-4);
        p->drawLine(x+7, y+4, x+7, y2-4);

        p->setPen(g.dark());
        p->drawLine(x, y2, x2, y2);

        p->setPen(g.mid());
        p->drawLine(x+4, y+4, x+4, y2-4);
        p->drawLine(x+6, y+4, x+6, y2-4);
        p->drawLine(x+8, y+4, x+8, y2-4);

    }
    else{
        p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
        int x2 = x+w-1;
        int y2 = y+h-1;

        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->drawLine(x2, y, x2, y2);

        p->drawLine(x+4, y+3, x2-4, y+3);
        p->drawLine(x+4, y+5, x2-4, y+5);
        p->drawLine(x+4, y+7, x2-4, y+7);

        p->setPen(g.dark());
        p->drawLine(x2, y, x2, y2);

        p->setPen(g.mid());
        p->drawLine(x+4, y+4, x2-4, y+4);
        p->drawLine(x+4, y+6, x2-4, y+6);
        p->drawLine(x+4, y+8, x2-4, y+8);

    }

}

void KDEStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, bool mac, QBrush *)
{
    if(mac){
        p->setPen(g.dark());
        p->drawRect(x, y, w, h);
        ++x, ++y, w-=2, h-=2;
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.light());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);
        p->setPen(g.mid());
        p->drawLine(x2, y, x2, y2);
        p->drawLine(x, y2, x2, y2);
        p->fillRect(x+1, y+1, w-2, h-2, g.brush(QColorGroup::Midlight));
    }
    else
        qDrawShadePanel(p, x, y, w, h, g, false, 1,
                        &g.brush(QColorGroup::Background));
}

void KDEStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, KToolBarPos, QBrush *)
{
    qDrawShadePanel(p, x, y, w, h, g, false, 1,
                    &g.brush(QColorGroup::Background));
}

void KDEStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, bool sunken,
                                    bool raised, bool enabled, bool popup,
                                    KToolButtonType icontext,
                                    const QString& btext, const QPixmap *pixmap,
                                    QFont *font, QWidget *)
{
    int dx, dy;

    QFontMetrics fm(*font);

    if(raised || sunken){
        int x2 = x+w;
        int y2 = y+h;

        p->fillRect(x, y, w, h, g.brush(QColorGroup::Midlight));

        p->setPen(g.dark());
        p->drawLine(x+1, y+1, x2-2, y+1);
        p->drawLine(x, y+2, x, y2-3);
        p->drawLine(x2-1, y+2, x2-1, y2-3);
        p->drawLine(x+1, y2-2, x2-2, y2-2);

        p->setPen(sunken ? g.mid() : g.light());
        p->drawLine(x+1, y+2, x2-2, y+2);
        p->drawLine(x+1, y+2, x+1, y2-3);
        p->setPen(sunken ? g.light() : g.mid());
        p->drawLine(x2-2, y+3, x2-2, y2-3);
        p->drawLine(x+2, y2-3, x2-2, y2-3);
    }
    else
    {
            p->fillRect(x, y, w, h, g.brush(QColorGroup::Background));
    }
    p->setPen(g.buttonText());

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
            dx = 4;
            dy = ( h - pixmap->height() ) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (pixmap)
                dx= 4 + pixmap->width() + 2;
            else
                dx= 4;
            dy = 0;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x+dx, y+dy, w-dx, h, tf, btext);
        }
    }
    else if (icontext == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignVCenter|AlignLeft;
            if (!enabled)
                p->setPen(g.dark());
            dx= (w - fm.width(btext)) / 2;
            dy= (h - fm.lineSpacing()) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x+dx, y+dy, fm.width(btext), fm.lineSpacing(), tf, btext);
        }
    }
    else if (icontext == IconTextBottom){
        if (pixmap){
            dx = (w - pixmap->width()) / 2;
            dy = (h - fm.lineSpacing() - pixmap->height()) / 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            p->drawPixmap( x+dx, y+dy, *pixmap );
        }
        if (!btext.isNull()){
            int tf = AlignBottom|AlignHCenter;
            dy= pixmap->height();
            dx = 2;
            if ( sunken ){
                ++dx;
                ++dy;
            }
            if (font)
                p->setFont(*font);
            if(raised)
                p->setPen(KGlobalSettings::toolBarHighlightColor());
            p->drawText(x, y, w, h-3, tf, btext);
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


void KDEStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool active, QMenuItem *mi,
                            QBrush *)
{
    if ( p->font() == KGlobalSettings::generalFont() )
      p->setFont( KGlobalSettings::menuFont() );

    if(active){
        qDrawShadePanel(p, x, y, w, h, g, true, 1,
                        &g.brush(QColorGroup::Midlight));
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.buttonText());
    }
    else
        QApplication::style().drawItem(p, x, y, w, h,
                                       AlignCenter|ShowPrefix|DontClip|SingleLine,
                                       g, mi->isEnabled(), mi->pixmap(), mi->text(),
                                       -1, &g.buttonText() );
}

void KDEStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
static const int motifItemFrame         = 2;
static const int motifItemHMargin       = 3;
static const int motifItemVMargin       = 2;
static const int motifArrowHMargin      = 6;
static const int windowsRightBorder     = 12;
    maxpmw = QMAX( maxpmw, 20 );

    if ( p->font() == KGlobalSettings::generalFont() )
      p->setFont( KGlobalSettings::menuFont() );

    if(act){
        bool dis = !enabled;
        QColorGroup itemg = dis ? pal.disabled() : pal.active();

        int checkcol = maxpmw;

        qDrawShadePanel(p, x, y, w, h, itemg, true, 1,
                        dis ? &itemg.brush(QColorGroup::Background):
                        &itemg.brush(QColorGroup::Midlight));

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
            p->setPen(itemg.highlightedText());
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
        p->setPen(itemg.buttonText());
        QColor discol;
        if (dis) {
            discol = itemg.midlight();
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
                discol = itemg.buttonText();
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

int KDEStyle::popupMenuItemHeight(bool /*checkable*/, QMenuItem *mi,
                                 const QFontMetrics &fm)
{
    if (mi->isSeparator())
        return 2;

    int h = 0;
    if (mi->pixmap())
        h = mi->pixmap()->height();

    if (mi->iconSet())
        h = QMAX(mi->iconSet()->
                 pixmap(QIconSet::Small, QIconSet::Normal).height(), h);

    h = QMAX(fm.height() + 4, h);

    // we want a minimum size of 18
    h = QMAX(h, 18);

    return h;
}

void KDEStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                    const QColorGroup &g, QBrush *fill)
{
    p->fillRect(x, y, w, h, fill ? *fill : g.brush(QColorGroup::Text));
}

void KDEStyle::drawFocusRect(QPainter *p, const QRect &r,
                            const QColorGroup &g, const QColor *c,
                            bool atBorder)
{
    /*
    p->setPen(g.foreground());
    p->setBrush(NoBrush);
    if ( atBorder )
        p->drawWinFocusRect( QRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2 ) );
    else
    p->drawWinFocusRect( r ); */
    KStyle::drawFocusRect(p, r, g, c, atBorder);
}

void KDEStyle::polishPopupMenu(QPopupMenu *mnu)
{
    KStyle::polishPopupMenu(mnu);
    // disabled for now because it breaks kicker
    // (void)new KDEAniMenu(mnu);
}

void KDEStyle::drawTab(QPainter *p, const QTabBar *tabBar, QTab *tab,
                      bool selected)
{
    if(tabBar->shape() != QTabBar::RoundedAbove){
        KStyle::drawTab(p, tabBar, tab, selected);
        return;
    }

    QRect r = tab->rect();
    QColorGroup g = tabBar->colorGroup();
    if(!selected){
        r.setY(r.y()+1);
        p->setPen(g.dark());
        p->drawLine(r.x(), r.y()+4, r.x(), r.bottom()-2);
        p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
        p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom()-2);

        p->setPen(g.midlight());
        p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
        p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
        p->drawLine(r.x()+5, r.y()+1, r.right()-2, r.y()+1);
        p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());

        p->setPen(g.mid());
        p->drawPoint(r.right()-1, r.y()+1);
        p->drawLine(r.x()+4, r.y()+2, r.right()-1, r.y()+2);
        p->drawLine(r.x()+3, r.y()+3, r.right()-1, r.y()+3);

        p->fillRect(r.x()+2, r.y()+4, r.width()-3, r.height()-6,
                    g.brush(QColorGroup::Mid));
        p->setPen(g.light());
        p->drawLine(r.x(), r.bottom()-1, r.right(), r.bottom()-1);
        p->setPen(g.midlight());

    }
    else{
        p->setPen(g.light());
        p->drawLine(r.x(), r.y()+4, r.x(), r.bottom());
        p->drawLine(r.x(), r.y()+4, r.x()+4, r.y());
        p->drawLine(r.x()+4, r.y(), r.right()-1, r.y());
        p->setPen(g.dark());
        p->drawLine(r.right(), r.y()+1, r.right(), r.bottom());

        p->setPen(g.midlight());
        p->drawLine(r.x()+1, r.y()+4, r.x()+1, r.bottom());
        p->drawLine(r.x()+1, r.y()+4, r.x()+4, r.y()+1);
        p->drawLine(r.x()+5, r.y()+1, r.right() - 2, r.y()+1);
        p->setPen(g.mid());
        p->drawLine(r.right()-1, r.y()+1, r.right()-1, r.bottom());
    }

}

void KDEStyle::drawTabMask(QPainter *p, const QTabBar*, QTab *tab,
                          bool )

{
    p->fillRect(tab->rect(), Qt::color1);
}

void KDEStyle::tabbarMetrics(const QTabBar *t, int &hFrame, int &vFrame,
                            int &overlap)
{
    if(t->shape() == QTabBar::RoundedAbove){
        overlap = 0;
        hFrame = 24;
        vFrame = 10;
    }
    else
        KStyle::tabbarMetrics(t, hFrame, vFrame, overlap);
}


void KDEStyle::drawSplitter(QPainter *p, int x, int y, int w, int h,
                           const QColorGroup &g, Orientation)
{
    int x2 = x+w-1;
    int y2 = y+h-1;
    p->setPen(g.dark());
    p->drawRect(x, y, w, h);
    p->setPen(g.background());
    p->drawPoint(x, y);
    p->drawPoint(x2, y);
    p->drawPoint(x, y2);
    p->drawPoint(x2, y2);
    p->setPen(g.light());
    p->drawLine(x+1, y+1, x+1, y2-1);
    p->drawLine(x+1, y+1, x2-1, y+1);
    p->setPen(g.midlight());
    p->drawLine(x+2, y+2, x+2, y2-2);
    p->drawLine(x+2, y+2, x2-2, y+2);
    p->setPen(g.mid());
    p->drawLine(x2-1, y+1, x2-1, y2-1);
    p->drawLine(x+1, y2-1, x2-1, y2-1);
    p->fillRect(x+3, y+3, w-5, h-5, g.brush(QColorGroup::Background));
}

void KDEStyle::drawPanel(QPainter *p, int x, int y, int w, int h,
                        const QColorGroup &g, bool sunken,
                        int lineWidth, const QBrush *fill)
{
    if(lineWidth != 2 || !sunken)
        KStyle::drawPanel(p, x, y, w, h, g, sunken, lineWidth, fill);
    else{
        QPen oldPen = p->pen();
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->setPen(g.light());
        p->drawLine(x, y2, x2, y2);
        p->drawLine(x2, y, x2, y2);
        p->setPen(g.mid());
        p->drawLine(x, y, x2, y);
        p->drawLine(x, y, x, y2);

        p->setPen(g.midlight());
        p->drawLine(x+1, y2-1, x2-1, y2-1);
        p->drawLine(x2-1, y+1, x2-1, y2-1);
        p->setPen(g.dark());
        p->drawLine(x+1, y+1, x2-1, y+1);
        p->drawLine(x+1, y+1, x+1, y2-1);
        p->setPen(oldPen);
        if(fill){
            // I believe here we are only supposed to fill if there is a
            // specified fill brush...
            p->fillRect(x+2, y+2, w-4, h-4, *fill);
        }
    }
}

void KDEStyle::kColorBitmaps(QPainter *p, const QColorGroup &g, int x, int y,
                             QBitmap *lightColor, QBitmap *midColor,
                             QBitmap *midlightColor, QBitmap *darkColor,
                             QBitmap *blackColor, QBitmap *whiteColor)
{
    QBitmap *bitmaps[]={lightColor, midColor, midlightColor, darkColor,
        blackColor, whiteColor};

    QColor colors[]={g.light(), g.mid(), g.midlight(), g.dark(),
        Qt::black, Qt::white};

    int i;
    for(i=0; i < 6; ++i){
        if(bitmaps[i]){
            if(!bitmaps[i]->mask())
                bitmaps[i]->setMask(*bitmaps[i]);
            p->setPen(colors[i]);
            p->drawPixmap(x, y, *bitmaps[i]);
        }
    }
}

void KDEStyle::drawKickerAppletHandle(QPainter *p, int x, int y, int w, int h,
                                      const QColorGroup &g, QBrush *)
{
  KStyle::drawKickerAppletHandle(p, x, y, w, h, g);
  /*
    if(h > w){
        int y2 = y+h-1;

        p->setPen(g.light());

        p->drawLine(x+1, y+2, x+1, y2-2);
        p->drawLine(x+4, y+2, x+4, y2-2);

        p->setPen(g.dark());
        p->drawLine(x+2, y+2, x+2, y2-2);
        p->drawLine(x+5, y+2, x+5, y2-2);

    }
    else{
        int x2 = x+w-1;

        p->setPen(g.light());

        p->drawLine(x+2, y+1, x2-2, y+1);
        p->drawLine(x+2, y+4, x2-2, y+4);

        p->setPen(g.dark());
        p->drawLine(x+2, y+2, x2-2, y+2);
        p->drawLine(x+2, y+5, x2-2, y+5);
    }

    */
}

#include "kdestyle.moc"

#endif // QT_VERSION < 300

