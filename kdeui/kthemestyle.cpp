/* This file is part of the KDE project
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/                   

#include <kthemestyle.h>
#include <kthemebase.h>
#include <kapp.h>
#include <qbitmap.h>
#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#include <qpopupmenu.h>
#include <qtabbar.h>

#include <limits.h>

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

void KThemeStyle::polish(QApplication *app)
{
  oldPalette = app->palette();
  if(isPixmap(Background)){
      QPalette newPalette = oldPalette;
      QBrush bgBrush(oldPalette.color(QPalette::Normal,
                                      QColorGroup::Background),
                     *uncached(Background));
      newPalette.setBrush(QPalette::Normal, QColorGroup::Background,
                          bgBrush);
      bgBrush.setColor(oldPalette.color(QPalette::Disabled,
                                        QColorGroup::Background));
      newPalette.setBrush(QPalette::Disabled, QColorGroup::Background,
                          bgBrush);
      bgBrush.setColor(oldPalette.color(QPalette::Active,
                                        QColorGroup::Background));
      newPalette.setBrush(QPalette::Active, QColorGroup::Background,
                          bgBrush);

      app->setPalette(newPalette, true);
  }
}

void KThemeStyle::unPolish(QApplication *app)
{
    app->setPalette(oldPalette, true);
}

void KThemeStyle::polish(QWidget *w)
{
    if ( !w->isTopLevel() ) {
        if (w->inherits("QLabel") || w->inherits("QButton")
            || w->inherits("QGroupBox") || w->inherits("QComboBox")
           || w->inherits("QSlider"))
            w->setAutoMask( TRUE );
    }
}

void KThemeStyle::unPolish(QWidget* w)
{
    if ( !w->isTopLevel() ) {
        if (w->inherits("QLabel") || w->inherits("QButton")||
            w->inherits("QGroupBox") || w->inherits("QComboBox")
           || w->inherits("QSlider"))
            w->setAutoMask( FALSE );
    }
}                                     

void KThemeStyle::drawBaseButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken, bool
                                 rounded, WidgetType type, const QBrush *)
{
    int offset = decoWidth(type);
    QPen oldPen = p->pen();

    // This stinks. I wanted to assign a color and optional pixmap to a
    // brush with pixmapBrush and fillRect that, but the pixmap in a brush
    // doesn't necessarily paint at 0,0 in the pixmap. It depends on where
    // you start painting :( Faster to just do this...
    if((w-offset*2) > 0 && (h-offset*2) > 0){
        if(isPixmap(type))
            if(rounded)
                p->drawTiledPixmap(x, y, w, h, *scalePixmap(w, h, type));
            else
                p->drawTiledPixmap(x+offset, y+offset, w-offset*2, h-offset*2,
                                   *scalePixmap(w-offset*2, h-offset*2, type));
        else
            p->fillRect(x+offset, y+offset, w-offset*2, h-offset*2,
                        colorGroup(g, type)->brush(QColorGroup::Button));
    }
    
    drawShade(p, x, y, w, h, *colorGroup(g, type), sunken, rounded,
              highlightWidth(type), borderWidth(type), shade());

    p->setPen(oldPen);
}

void KThemeStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken,
                             const QBrush *fill)
{
    drawBaseButton(p, x, y, w, h, g, sunken, roundButton(), PushButton, fill);
}

void KThemeStyle::drawPushButton(QPushButton* btn, QPainter *p)
{
    int x1, y1, x2, y2;
    btn->rect().coords(&x1, &y1, &x2, &y2);
    drawButton(p, x1, y1, x2-x1+1, y2-y1+1, btn->colorGroup(), btn->isOn() ||
               btn->isDown(), NULL);
}

void KThemeStyle::drawBaseMask(QPainter *p, int x, int y, int w, int h,
                               bool round)
{
    // round edge fills
    static QCOORD btm_left_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,1,
    1,2,2,2,3,2,4,2,2,3,3,3,4,3,3,4,4,4 };

    static QCOORD btm_right_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,
    1,0,2,1,2,2,2,3,2,0,3,1,3,2,3,0,4,1,4 };

    static QCOORD top_left_fill[]={ 3,0,4,0,2,1,3,1,4,1,1,2,2,2,3,2,4,2,0,3,
    1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    static QCOORD top_right_fill[]={ 0,0,1,0,0,1,1,1,2,1,0,2,1,2,2,2,3,2,0,
    3,1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    QBrush fillBrush(color1, SolidPattern);
    p->setPen(color1);
    if(round && w > 19 && h > 19){
        int x2 = x+w-1;
        int y2 = y+h-1;
        QPointArray a(QCOORDARRLEN(top_left_fill), top_left_fill);
        a.translate(1, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_left_fill), btm_left_fill);
        a.translate(1, h-6);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(top_right_fill), top_right_fill);
        a.translate(w-6, 1);
        p->drawPoints(a);
        a.setPoints(QCOORDARRLEN(btm_right_fill), btm_right_fill);
        a.translate(w-6, h-6);
        p->drawPoints(a);

        p->fillRect(x+6, y, w-12, h, fillBrush);
        p->fillRect(x, y+6, x+6, h-12, fillBrush);
        p->fillRect(x2-6, y+6, x2, h-12, fillBrush);
        p->drawLine(x+6, y, x2-6, y);
        p->drawLine(x+6, y2, x2-6, y2);
        p->drawLine(x, y+6, x, y2-6);
        p->drawLine(x2, y+6, x2, y2-6);

    }
    else
        p->fillRect(x, y, w, h, fillBrush);
}

void KThemeStyle::drawButtonMask(QPainter *p, int x, int y, int w, int h)
{

    drawBaseMask(p, x, y, w, h, roundButton());
}                                         

void KThemeStyle::drawComboButtonMask(QPainter *p, int x, int y, int w, int h)
{
    // This isn't getting called by QStyle...
    warning("In drawComboButtonMask");
    drawBaseMask(p, x, y, w, h, roundComboBox());
}
 
void KThemeStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken,
                                  const QBrush *)
{
    drawBaseButton(p, x, y, w, h, g, sunken, false, Bevel);
}

void KThemeStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, bool sunken,
                                     bool raised, bool enabled, bool popup,
                                     KToolButtonType type, const QString &btext,
                                     const QPixmap *pixmap, QFont *font){
    drawBaseButton(p, x, y, w, h, g, sunken, false, ToolButton);
    int dx, dy;
    if (type == Icon){ // icon only
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
    else if (type == IconTextRight){ // icon and text (if any)
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
    else if (type == Text){ // only text, even if there is a icon
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
    else if (type == IconTextBottom){
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
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5, 0, 0,
                        g, true);
        else
            qDrawArrow (p, DownArrow, WindowsStyle, false, w-5, h-5,
                        0, 0, g, false);
    }
}

void KThemeStyle::drawKBarHandle(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool, QBrush *)
{
    drawBaseButton(p, x, y, w, h, g, false, false, BarHandle);
}

void KThemeStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool)
{
    drawBaseButton(p, x, y, w, h, g, false, false, ToolBar);
}

QRect KThemeStyle::buttonRect(int x, int y, int w, int h)
{
    int spacing = decoWidth(PushButton);
    return(QRect(x+spacing+4, y+spacing+4, w-(spacing*2)-4, h-(spacing*2)-4));
}                                                            

void KThemeStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken, bool,
                                  bool, const QBrush *)
{
    drawBaseButton(p, x, y, w, h, g, sunken, roundComboBox(), ComboBox);
    if(isPixmap(ComboDeco))
        p->drawPixmap(w - uncached(ComboDeco)->width()-decoWidth(ComboBox)-2,
                      y +(h - uncached(ComboDeco)->
                          height())/2,
                      *uncached(ComboDeco));
    else{
        qDrawArrow(p, Qt::DownArrow, Qt::MotifStyle, false, w-15, y+6, 10,
                   h-15, *colorGroup(g, ComboBox), true);
        qDrawShadeRect(p, w-14, y+7+(h-15), 10, 3, *colorGroup(g, ComboBox));
    }
}

void KThemeStyle::drawScrollBarControls(QPainter *p, const QScrollBar *sb,
                                        int sliderStart, uint controls,
                                        uint activeControl)
{
    int sliderMin, sliderMax, sliderLength, buttonDim;
    QRect add, sub, addPage, subPage, slider;
    int addX, addY, subX, subY;
    bool horizontal = sb->orientation() == QScrollBar::Horizontal;
    int len = (horizontal) ? sb->width() : sb->height();
    int extent = (horizontal) ? sb->height() : sb->width();
    int offset = decoWidth(horizontal ? HScrollGroove : VScrollGroove);
    QColorGroup g = sb->colorGroup();

    scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);
    if(sliderStart > sliderMax)
        sliderStart = sliderMax;

    int sliderEnd = sliderStart + sliderLength;
    int sliderWidth = extent - offset*2;

    // Scary button placement code >:-P Feel free to improve this if you
    // want. I get a headache just looking at it... (mosfet)
    if(scrollBarLayout() == SBOpposite){
        if (horizontal) {
            subY = addY = (extent - buttonDim)/2;
            subX = offset;
            addX = len - buttonDim - offset;
        } else {
            subX = addX = (extent - buttonDim)/2;
            subY = offset;
            addY = len - buttonDim - offset;
        }
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
        if (horizontal) {
            subPage.setRect(sub.right() + 1, offset,
                             sliderStart - sub.right() - 1 , sliderWidth);
            addPage.setRect(sliderEnd, offset, addX - sliderEnd, sliderWidth);
            slider.setRect(sliderStart, offset, sliderLength, sliderWidth);
        }
        else {
            subPage.setRect(offset, sub.bottom() + 1, sliderWidth,
                            sliderStart - sub.bottom() - 1 );
            addPage.setRect(offset, sliderEnd, sliderWidth, addY - sliderEnd);
            slider.setRect(offset, sliderStart, sliderWidth, sliderLength);
        }
    }
    else if(horizontal){
        subY = addY = (extent - buttonDim)/2;
        if(scrollBarLayout() == SBBottomLeft){
            subX = offset;
            addX = buttonDim+offset;
            subPage.setRect(buttonDim*2, 0, sliderStart-1, extent);
            addPage.setRect(sliderEnd, 0, len-sliderEnd, extent);
            slider.setRect(sliderStart, 0, sliderLength, extent);
        }
        else{
            subX = len - buttonDim - buttonDim - offset;
            addX = len - buttonDim - offset;
            subPage.setRect(offset + 1, offset, sliderStart - 1 , sliderWidth);
            addPage.setRect(sliderEnd, offset, subX - sliderEnd, sliderWidth);
            slider.setRect(sliderStart, offset, sliderLength, sliderWidth);
        }
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
    }
    else{ // BottomLeft and BottomRight vertical bars are the same.
        subX = addX = ( extent - buttonDim )/2;
        subY = len - buttonDim - buttonDim - offset;
        addY = len - buttonDim - offset;
        subPage.setRect(offset, offset + 1, sliderWidth,
                         sliderStart - offset - 1 );
        addPage.setRect(offset, sliderEnd, sliderWidth, subY - sliderEnd );
        slider.setRect(offset, sliderStart, sliderWidth, sliderLength);
        sub.setRect(subX, subY, buttonDim, buttonDim);
        add.setRect(addX, addY, buttonDim, buttonDim);
    }
    // End of the button placement code

    if((controls & QStyle::SubPage)){
        drawScrollBarGroove(p, sb, horizontal, subPage, g);
    }
    if((controls & QStyle::AddPage)){
        drawScrollBarGroove(p, sb, horizontal, addPage, g);
    }
    if(controls & QStyle::AddLine){
        drawBaseButton(p, add.x(), add.y(), add.width(), add.height(), g,
                       activeControl == AddLine, false, ScrollButton);
        drawArrow(p, (horizontal) ? RightArrow : DownArrow, false,
                  add.x()+3, add.y()+3, add.width()-6,
                  add.height()-6, *colorGroup(g, ScrollButton));
    }
    if(controls & QStyle::SubLine){
        p->setPen(g.dark());
        p->drawRect(sub);
        drawBaseButton(p, sub.x(), sub.y(), sub.width(), sub.height(), g,
                       activeControl == SubLine, false, ScrollButton);
        drawArrow(p, (horizontal)  ? LeftArrow : UpArrow, false, sub.x()+3,
                  sub.y()+3, sub.width()-6, sub.height()-6,
                  *colorGroup(g, ScrollButton));
    }
    if(controls & QStyle::Slider){
        drawBaseButton(p, slider.x(), slider.y(), slider.width(),
                       slider.height(), g, false, ScrollBarSlider);
        int spaceW = horizontal ? slider.width()-decoWidth(ScrollBarSlider)-4 :
            slider.width();
        int spaceH = horizontal ? slider.height() :
            slider.height()-decoWidth(ScrollBarSlider)-4;
        if(isPixmap(ScrollDeco)){
            if(spaceW >= uncached(ScrollDeco)->width() &&
               spaceH >= uncached(ScrollDeco)->height()){
                p->drawPixmap(slider.x()+(slider.width() -
                                          uncached(ScrollDeco)->width())/2,
                              slider.y()+(slider.height() -
                                          uncached(ScrollDeco)->height())/2,
                              *uncached(ScrollDeco));
            }
        }
    }
}

void KThemeStyle::drawScrollBarGroove(QPainter *p, const QScrollBar *sb,
                                      bool horizontal, QRect r, QColorGroup g)

{
    WidgetType widget = (horizontal) ? HScrollGroove : VScrollGroove;
    if(!isPixmap(widget)){
        p->fillRect(r, colorGroup(g, widget)->brush(QColorGroup::Mid));
    }
    else{
        // If the groove is pixmapped we make a full-sized image (it gets
        // cached) then bitBlt it to the appropriate rect.
        QPixmap buffer(sb->size());
        QPainter bPainter(&buffer);
        bPainter.drawTiledPixmap(0, 0, buffer.width(), buffer.height(),
                                 *scalePixmap(buffer.width(), buffer.height(),
                                              widget));
        bitBlt(p->device(), r.x(), r.y(), &buffer, r.x(), r.y(), r.width(),
               r.height(), Qt::CopyROP);
    }
    // Do the borders and frame
    drawShade(p, sb->rect().x(), sb->rect().y(), sb->rect().width(),
              sb->rect().height(), *colorGroup(g, widget), true, false,
              highlightWidth(widget), borderWidth(widget), shade());
}

void KThemeStyle::scrollBarMetrics(const QScrollBar *sb, int &sliderMin,
                                   int &sliderMax, int &sliderLength,
                                   int &buttonDim)
{
    bool horizontal = sb->orientation() == QScrollBar::Horizontal;
    int offset = borderWidth(horizontal ? HScrollGroove : VScrollGroove);
    int maxlen;
    int len = horizontal ? sb->width()  : sb->height();
    int extent = horizontal ? sb->height() : sb->width();

    if ( len > ( extent - offset*2 - 1 )*2 + offset*2 )
        buttonDim = extent - offset*2;
    else
        buttonDim = ( len - offset*2 )/2 - 1;
    maxlen = len - offset*2 - buttonDim*2 - 1;

    switch(scrollBarLayout()){
    case SBBottomLeft:
        sliderMin = (horizontal) ? buttonDim*2+offset+1 : offset+1;
        break;
    case SBBottomRight:
        sliderMin = offset + 1;
        break;
    case SBOpposite:
    default:
        sliderMin = offset + buttonDim;
        break;
    }
    if ( sb->maxValue() == sb->minValue() )
        sliderLength = maxlen;
    else
        sliderLength = (sb->pageStep()*maxlen) / (sb->maxValue() -
                                                  sb->minValue() + sb->pageStep());
    if (sliderLength < 12 || (sb->maxValue()-sb->minValue()) > INT_MAX/2)
        sliderLength = 12;
    if (sliderLength > maxlen )
        sliderLength = maxlen;
    sliderMax = sliderMin + maxlen - sliderLength;
}

QStyle::ScrollControl KThemeStyle::scrollBarPointOver(const QScrollBar *sb,
                                                      int sliderStart,
                                                      const QPoint &p)
{
    if(!sb->rect().contains(p))
        return(QStyle::NoScroll);
    int sliderMin, sliderMax, sliderLength, buttonDim;
    int pos = (sb->orientation() == QScrollBar::Horizontal) ? p.x() : p.y();
    scrollBarMetrics(sb, sliderMin, sliderMax, sliderLength, buttonDim);

    if(scrollBarLayout() == SBOpposite){
        if (pos < sliderMin )
            return QStyle::SubLine;
        if (pos < sliderStart )
            return SubPage;
        if (pos < sliderStart + sliderLength )
            return QStyle::Slider;
        if (pos < sliderMax + sliderLength )
            return QStyle::AddPage;
        return QStyle::AddLine;
    }
    if(scrollBarLayout() == SBBottomLeft && sb->orientation() ==
       QScrollBar::Horizontal){
        if(pos <= buttonDim)
            return(QStyle::SubLine);
        else if(pos <= buttonDim*2)
            return(QStyle::AddLine);
        else if(pos < sliderStart)
            return(QStyle::SubPage);
        else if(pos < sliderStart+sliderLength)
            return(QStyle::Slider);
        return(AddPage);
    }
    else{
        if (pos < sliderStart)
            return QStyle::SubPage;
        if (pos < sliderStart + sliderLength)
            return QStyle::Slider;
        if (pos < sliderMax + sliderLength)
            return QStyle::AddPage;
        if (pos < sliderMax + sliderLength + buttonDim)
            return QStyle::SubLine;
        return QStyle::AddLine;
    }
}

QSize KThemeStyle::exclusiveIndicatorSize() const
{
    if(isPixmap(ExIndicatorOn))
        return(uncached(ExIndicatorOn)->size());
    else
        return(QPlatinumStyle::exclusiveIndicatorSize());
}

QSize KThemeStyle::indicatorSize() const
{
    if(isPixmap(IndicatorOn))
        return(uncached(IndicatorOn)->size());
    else
        return(QPlatinumStyle::indicatorSize());
}

void KThemeStyle::drawExclusiveIndicator(QPainter* p, int x, int y, int w,
                                         int h, const QColorGroup &g, bool on,
                                         bool down, bool enabled)
{
    if(isPixmap((on ||down)? ExIndicatorOn : ExIndicatorOff)){
        p->drawPixmap(x, y, *uncached((on || down) ? ExIndicatorOn :
                                     ExIndicatorOff));
    }
    else{
        QPlatinumStyle::drawExclusiveIndicator(p, x, y, w, h,
                                               *colorGroup(g, ExIndicatorOn),
                                               on, down, enabled);
    }

}

void KThemeStyle::drawIndicator(QPainter* p, int x, int y, int w, int h,
                                const QColorGroup &g, int state, bool down,
                                bool enabled)
{
    if(isPixmap((down || state != QButton::Off) ?
                IndicatorOn : IndicatorOff)){
        p->drawPixmap(x, y, *uncached((down || state != QButton::Off) ?
                                     IndicatorOn : IndicatorOff));
    }
    else{
        QPlatinumStyle::drawIndicator(p, x, y, w, h,
                                      *colorGroup(g, IndicatorOn), state,
                                      down, enabled);
    }
}

void KThemeStyle::drawExclusiveIndicatorMask(QPainter *p, int x, int y, int w,
                                             int h, bool on)
{
    if(isPixmap((on) ? ExIndicatorOn : ExIndicatorOff)){
        const QBitmap *mask = uncached((on) ? ExIndicatorOn : ExIndicatorOff)->
            mask();
        if(mask){
            p->drawPixmap(x, y, *mask);
        }
        else
            p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    }
    else
        QPlatinumStyle::drawExclusiveIndicatorMask(p, x, y, w, h, on);
}

void KThemeStyle::drawIndicatorMask(QPainter *p, int x, int y, int w, int h,
                                    int state)
{
    if(isPixmap((state != QButton::Off)? IndicatorOn : IndicatorOff)){
        const QBitmap *mask = uncached((state != QButton::Off ) ? IndicatorOn :
                                      IndicatorOff)->mask();
        if(mask)
            p->drawPixmap(x, y, *mask);
        else
            p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    }
    else
        QPlatinumStyle::drawIndicatorMask(p, x, y, w, h, state);
}

void KThemeStyle::drawSliderGroove(QPainter *p, int x, int y, int w, int h,
                                   const QColorGroup& g, QCOORD c,
                                   Orientation orient)
{
    if(roundSlider())
        QPlatinumStyle::drawSliderGroove(p, x, y, w, h, g, c, orient);
    else
        drawBaseButton(p, x, y, w, h, g, true, false, SliderGroove);
}

void KThemeStyle::drawSlider(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, Orientation orient,
                             bool tickAbove, bool tickBelow)
{
    if(isPixmap(Slider)){
        if(orient == Qt::Horizontal)
            p->drawPixmap(x, y+(h-uncached(Slider)->height())/2,
                          *uncached(Slider));
        else
            p->drawPixmap(x+(w-uncached(Slider)->width())/2,
                          y, *uncached(Slider));
    }
    else{
        QPlatinumStyle::drawSlider(p, x, y, w, h, *colorGroup(g, Slider),
                                   orient, tickAbove, tickBelow);
    }
}

void KThemeStyle::drawSliderMask(QPainter *p, int x, int y, int w, int h,
                                 Orientation orient, bool tickAbove,
                                 bool tickBelow)
{
    // This is odd. If we fill in the entire region it still masks the slider
    // properly. I have no idea, this used to be different in Qt betas...
    if(isPixmap(Slider))
        p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    else
        QPlatinumStyle::drawSliderMask(p, x, y, w, h, orient, tickAbove,
                                       tickBelow);
}

int KThemeStyle::defaultFrameWidth() const
{
    return(frameWidth());
}

void KThemeStyle::getButtonShift(int &x, int &y)
{
    x = buttonXShift();
    y = buttonYShift();
}

int KThemeStyle::sliderLength() const
{
    return(sliderButtonLength());
}

void KThemeStyle::drawArrow(QPainter *p, Qt::ArrowType type, bool down, int x,
                            int y, int w, int h, const QColorGroup &g,
                            bool enabled, const QBrush *)
{
    // Handles pixmapped arrows. A little inefficent because you can specify
    // some as pixmaps and some as default types.
    switch(type){
    case UpArrow:
        if(isPixmap(ArrowUp)){
        p->drawPixmap(x+(w-uncached(ArrowUp)->width())/2,
                      y+(h-uncached(ArrowUp)->height())/2,
                      *uncached(ArrowUp));
        return;
        }
        break;
    case DownArrow:
        if(isPixmap(ArrowDown)){
        p->drawPixmap(x+(w-uncached(ArrowDown)->width())/2,
                      y+(h-uncached(ArrowDown)->height())/2,
                      *uncached(ArrowDown));
        return;
        }
        break;
    case LeftArrow:
        if(isPixmap(ArrowLeft)){
        p->drawPixmap(x+(w-uncached(ArrowLeft)->width())/2,
                      y+(h-uncached(ArrowLeft)->height())/2,
                      *uncached(ArrowLeft));
        return;
        }
        break;
    case RightArrow:
        if(isPixmap(ArrowRight)){
        p->drawPixmap(x+(w-uncached(ArrowRight)->width())/2,
                      y+(h-uncached(ArrowRight)->height())/2,
                      *uncached(ArrowRight));
        return;
        }
        break;
    default:
        break;
    }

    // Standard arrow types
    if(arrowType() == MotifArrow)
        qDrawArrow(p, type, Qt::MotifStyle, down, x, y, w, h, g, enabled);
    else if(arrowType() == SmallArrow){
        QBrush oldBrush = g.brush(QColorGroup::Button);
        QColorGroup tmp(g);
        tmp.setBrush(QColorGroup::Button, QBrush(NoBrush));
        QPlatinumStyle::drawArrow(p, type, false, x, y, w, h,
                                  tmp, true);
    }
    else{
        QPointArray a;
        int x2=x+w-1, y2=y+h-1;
        switch (type){
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
        QPen oldPen = p->pen();
        p->setBrush(g.brush(QColorGroup::Shadow));
        p->setPen(g.shadow());
        p->drawPolygon(a);
        p->setBrush(oldBrush);
        p->setPen(oldPen);
    }
}

/* This is where we draw the borders and highlights. The new round button
 * code is a pain in the arse. We don't want to be calculating arcs so
 * use a whole lotta QPointArray's ;-) The code is made a lot more complex
 * because you can have variable width border and highlights...
 * I may want to cache this if round buttons are used, but am concerned
 * about excessive cache misses. This is a memory/speed tradeoff that I
 * have to test.
 */
void KThemeStyle::drawShade(QPainter *p, int x, int y, int w, int h,
                            const QColorGroup &g, bool sunken, bool rounded,
                            int hWidth, int bWidth, ShadeStyle style)
{
    int i, sc, bc, x2, y2;
    QPen highPen, lowPen;

    if(style == Motif){
        highPen.setColor(sunken ? g.dark() : g.light());
        lowPen.setColor(sunken ? g.light() : g.dark());
    }
    else{
        highPen.setColor(sunken ? g.shadow() : g.light());
        lowPen.setColor(sunken ? g.light() : g.shadow());
    }

    // Advanced round buttons
    if(rounded && w > 19 && h > 19){
        x2 = x+w-1, y2 = y+h-1;
        QPointArray bPntArray, hPntArray, lPntArray;
        QPointArray bLineArray, hLineArray, lLineArray;
        // borders
        for(i=0,bc=0; i < bWidth; ++i){
            bPntArray.putPoints(bc, 24, x+4,y+1, x+5,y+1, x+3,y+2, x+2,y+3,
                                x+1,y+4, x+1,y+5, x+1,y2-5, x+1,y2-4, x+2,y2-3,
                                x2-5,y+1, x2-4,y+1, x2-3,y+2, x2-5,y2-1,
                                x2-4,y2-1, x2-3,y2-2, x2-2,y2-3, x2-1,y2-5,
                                x2-1,y2-4, x+3,y2-2, x+4,y2-1, x+5,y2-1,
                                x2-2,y+3, x2-1,y+4, x2-1,y+5);
            bc+=24;
            // ellispe edges don't match exactly, so fill in blanks
            if(i < bWidth-1 || hWidth != 0){
                bPntArray.putPoints(bc, 20, x+6,y+1, x+4,y+2,  x+3,y+3,
                                    x+2,y+4, x+1,y+6, x2-6,y+1, x2-4,y+2,
                                    x2-3,y+3, x+2,y2-4, x+1,y2-6, x2-6,y2-1,
                                    x2-4,y2-2, x2-3,y2-3,x2-2,y2-4, x2-1,y2-6,
                                    x+6,y2-1, x+4,y2-2, x+3,y2-3, x2-1,y+6,
                                    x2-2,y+4);
                bc+=20;
            }
            bLineArray.putPoints(i*8, 8, x+6,y, x2-6,y, x,y+6, x,y2-6,
                                 x+6,y2, x2-6,y2, x2,y+6, x2,y2-6);
            ++x, ++y;
            --x2, --y2;
        }
        // highlights
        for(i=0,sc=0; i < hWidth; ++i){
            hPntArray.putPoints(sc, 12, x+4,y+1, x+5,y+1, // top left
                                x+3,y+2, x+2,y+3, x+1,y+4, x+1,y+5,
                                x+1,y2-5, x+1,y2-4, x+2,y2-3, // half corners
                                x2-5,y+1, x2-4,y+1, x2-3,y+2);
            lPntArray.putPoints(sc, 12, x2-5,y2-1, x2-4,y2-1, // btm right
                                x2-3,y2-2, x2-2,y2-3, x2-1,y2-5, x2-1,y2-4,
                                x+3,y2-2, x+4,y2-1, x+5,y2-1, //half corners
                                x2-2,y+3, x2-1,y+4, x2-1,y+5);
            sc+=12;
            if(i < hWidth-1){
                hPntArray.putPoints(sc, 10, x+6,y+1, x+4,y+2, // top left
                                    x+3,y+3, x+2,y+4, x+1,y+6,
                                    x2-6,y+1, x2-4,y+2, // half corners
                                    x2-3,y+3, x+2,y2-4, x+1,y2-6);
                lPntArray.putPoints(sc, 10, x2-6,y2-1, x2-4,y2-2, // btm right
                                    x2-3,y2-3,x2-2,y2-4, x2-1,y2-6,
                                    x+6,y2-1, x+4,y2-2, // half corners
                                    x+3,y2-3, x2-1,y+6, x2-2,y+4);
                sc+=10;
            }
            hLineArray.putPoints(i*4, 4, x+6,y, x2-6,y, x,y+6, x,y2-6);
            lLineArray.putPoints(i*4, 4, x+6,y2, x2-6,y2, x2,y+6, x2,y2-6);
            ++x, ++y;
            --x2, --y2;
        }
        p->setPen(Qt::black);
        p->drawPoints(bPntArray);
        p->drawLineSegments(bLineArray);
        p->setPen(highPen);
        p->drawPoints(hPntArray);
        p->drawLineSegments(hLineArray);
        p->setPen(lowPen);
        p->drawPoints(lPntArray);
        p->drawLineSegments(lLineArray);
    }
    // Rectangular buttons
    else{
        QPointArray highShade(hWidth*4);
        QPointArray lowShade(hWidth*4);

        p->setPen(g.shadow());
        for(i=0; i < bWidth && w > 2 && h > 2; ++i, ++x, ++y, w-=2, h-=2)
            p->drawRect(x, y , w, h);

        if(!hWidth)
            return;
        
        x2 = x+w-1, y2 = y+h-1;
        for(i=0; i < hWidth; ++i, ++x, ++y, --x2, --y2){
            highShade.putPoints(i*4, 4, x,y, x2,y, x,y, x,y2);
            lowShade.putPoints(i*4, 4, x,y2, x2,y2, x2,y, x2,y2);
        }
        if(style == Windows && hWidth > 1){
            p->setPen(highPen);
            p->drawLineSegments(highShade, 0, 2);
            p->setPen(lowPen);
            p->drawLineSegments(lowShade, 0, 2);

            p->setPen((sunken) ? g.dark() : g.mid());
            p->drawLineSegments(highShade, 4);
            p->setPen((sunken) ? g.mid() : g.dark());
            p->drawLineSegments(lowShade, 4);
        }
        else{
            p->setPen((sunken) ? g.dark() : g.light());
            p->drawLineSegments(highShade);
            p->setPen((sunken) ? g.light() : g.dark());
            p->drawLineSegments(lowShade);
        }
    }
}

void KThemeStyle::drawPushButtonLabel(QPushButton *btn, QPainter *p)
{
    const QColorGroup *cg = colorGroup(btn->colorGroup(), PushButton);
    int x, y, w, h;

    QRect r = btn->rect();
    r.rect(&x, &y, &w, &h);
    x+=decoWidth(PushButton);
    y+=decoWidth(PushButton);
    w-=decoWidth(PushButton)*2;
    h-=decoWidth(PushButton)*2;
    
    if(btn->isDown() || btn->isOn()){
        drawItem(p, x+buttonXShift(), y+buttonYShift(),
                 w, h, AlignCenter | ShowPrefix, *cg, btn->isEnabled(),
                 btn->pixmap(), btn->text(), -1, &cg->buttonText());
    }
    else{
        drawItem(p, x, y, w, h, AlignCenter | ShowPrefix, *cg,
                 btn->isEnabled(), btn->pixmap(), btn->text(), -1,
                 &cg->buttonText());
    }
}

int KThemeStyle::splitterWidth() const
{
    return(splitWidth());
}

void KThemeStyle::drawSplitter(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, Orientation)
{
    drawBaseButton(p, x, y, w, h, g, false, false, Splitter);
}

void KThemeStyle::drawCheckMark(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool act, bool dis)
{
    if(isPixmap(CheckMark)){
        if(!dis)
            p->drawPixmap(x+(w-uncached(CheckMark)->width())/2,
                          y+(h-uncached(CheckMark)->height())/2,
                          *uncached(CheckMark));
    }
    else
        QPlatinumStyle::drawCheckMark(p, x, y, w, h, *colorGroup(g, CheckMark),
                                      act, dis);
}

int KThemeStyle::popupMenuItemHeight(bool /*checkable*/, QMenuItem *mi,
                                     const QFontMetrics &fm)
{
    int h2, h = 0;
    int offset = QMAX(decoWidth(MenuItemOn), decoWidth(MenuItemOff)) + 4;
    
    if(mi->isSeparator())
        return(2);
    if(mi->isChecked())
        h = isPixmap(CheckMark) ? uncached(CheckMark)->height()+offset :
            offset+12;
    if(mi->pixmap()){
        h2 = mi->pixmap()->height()+offset;
        h = h2 > h ? h2 : h;
    }
    if(mi->iconSet()){
        h2 = mi->iconSet()->
            pixmap(QIconSet::Small,  QIconSet::Normal).height()+offset;
        h = h2 > h ? h2 : h;
    }
    h2 = fm.height()+ offset;
    h = h2 > h ? h2 : h;
    return(h);
}

#ifndef KTHEMESTYLE_CONSTANTS
// I changed the following method to be based from Qt's instead of my own
// wacky code. Works much better now :P (mosfet)
static const int motifItemFrame         = 2;    // menu item frame width
static const int motifItemHMargin       = 3;    // menu item hor text margin
static const int motifItemVMargin       = 2;    // menu item ver text margin

static const int motifSepHeight         = 2;    // separator item height
static const int motifArrowHMargin      = 6;    // arrow horizontal margin
static const int motifTabSpacing        = 12;   // space between text and tab
static const int motifCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int windowsRightBorder     = 12;       // right border on windowsstatic const int windowsCheckMarkWidth = 12;       // checkmarks width on windows                                                                        
#define KTHEMESTYLE_CONSTANTS
#endif

void KThemeStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
    bool dis = !enabled;
    const QColorGroup &g = dis ? *colorGroup(pal.normal(), MenuItemOff) :
        *colorGroup(pal.normal(), MenuItemOn);
    
    QColorGroup itemg = dis ? *colorGroup(pal.disabled(), MenuItemOff)
        : act ? *colorGroup(pal.active(), MenuItemOn)
        : *colorGroup(pal.normal(), MenuItemOff);

    if (checkable)
        maxpmw = QMAX( maxpmw, 12 ); 
    int checkcol = maxpmw;

    if (mi->isSeparator()){
        p->setPen(g.dark());
	p->drawLine(x, y, x+w, y);
	p->setPen(g.light());
	p->drawLine(x, y+1, x+w, y+1);
	return;
    }

    drawBaseButton(p, x, y, w, h, g, act, false, act ? MenuItemOn :
                   MenuItemOff);

    if ( mi->iconSet() ) {
	QIconSet::Mode mode = dis?QIconSet::Disabled:QIconSet::Normal;
	if (act && !dis)
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

    p->setPen(act ? g.highlightedText() : g.buttonText());

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
            if (dis && !act) {
                p->setPen(g.light());
		p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame+1,
                            y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
		p->setPen(discol);
	    }
	    p->drawText(x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
                        y+m, tab, h-2*m, text_flags, s.mid( t+1 ));
	}
	if (dis && !act) {
	    p->setPen(g.light());
	    p->drawText(x+xm+1, y+m+1, w-xm+1, h-2*m, text_flags, s, t);
	    p->setPen(discol);
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
	if (act) {
	    if (!dis)
		discol = white;
            QColorGroup g2(discol, g.highlight(),
                           white, white,
                           dis ? discol : white,
                           discol, white);
	    drawArrow(p, RightArrow, FALSE,
                      x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                      dim, dim, g2, TRUE);
	} else {
	    drawArrow(p, RightArrow,
                      FALSE,
                      x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                      dim, dim, g, mi->isEnabled() );
        }
    }
}

void KThemeStyle::drawFocusRect(QPainter *p, const QRect &r,
                                const QColorGroup &g, const QColor *c,
                                bool atBorder)
{
    p->setPen(g.dark());
    if(!is3DFocus())
        QPlatinumStyle::drawFocusRect(p, r, g, c, atBorder);
    else{
        int i = focusOffset();
        p->drawLine(r.x()+i, r.y()+1+i, r.x()+i, r.bottom()-1-i);
        p->drawLine(r.x()+1+i, r.y()+i, r.right()-1-i, r.y()+i);
        p->setPen(g.light());
        p->drawLine(r.right()-i, r.y()+1+i, r.right()-i, r.bottom()-1-i);
        p->drawLine(r.x()+1+i, r.bottom()-i, r.right()-1-i, r.bottom()-i);
    }
}

void KThemeStyle::drawKMenuBar(QPainter *p, int x, int y, int w, int h,
                               const QColorGroup &g, QBrush *)
{
    const QColorGroup *cg = colorGroup(g, MenuBar);
    drawBaseButton(p, x, y, w, h, *cg, false, false, MenuBar);

}

void KThemeStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool active,
                                QMenuItem *mi, QBrush *)
{
    const QColorGroup *cg = colorGroup(g, MenuBar);
    QColor btext = cg->buttonText();
    if(active)
        qDrawShadePanel(p, x, y, w, h, *cg, false, 1);
    
    drawItem(p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
             *cg, mi->isEnabled(), mi->pixmap(), mi->text(),
             -1, &btext );;
}

void KThemeStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, QBrush *)
{
    const QColorGroup *cg = colorGroup(g, ProgressBar);
    drawBaseButton(p, x, y, w, h, *cg, false, false, ProgressBar);
}

void KThemeStyle::getKProgressBackground(const QColorGroup &g, QBrush &bg)
{
    const QColorGroup *cg = colorGroup(g, ProgressBg);
    bg.setColor(cg->color(QColorGroup::Background));
    if(isPixmap(ProgressBg))
        bg.setPixmap(*uncached(ProgressBg));
}

void KThemeStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap)
{
    QCommonStyle::tabbarMetrics( t, hframe, vframe, overlap );
}

void KThemeStyle::drawTab( QPainter* p, const QTabBar* tb, QTab* t ,
                           bool selected )
{
    QRect r(t->r);
    WidgetType widget = selected ? ActiveTab : InactiveTab;
    const QColorGroup *cg = colorGroup(tb->colorGroup(), widget);
    int i;
    int bWidth = borderWidth(widget);
    int hWidth = highlightWidth(widget);
    if(tb->shape() == QTabBar::RoundedAbove){
        if(!selected){
            p->fillRect(r.left(), r.top(), r.width(), 2,
                        tb->palette().normal().brush( QColorGroup::Background));
            r.setTop(r.top()+2);
        }
        p->setPen(cg->text());
        i=0;
        if(i < bWidth){
            p->drawLine(r.left(), r.top()+1, r.left(), r.bottom());
            p->drawLine(r.right(), r.top()+1, r.right(), r.bottom());
            p->drawLine(r.left()+1, r.top(), r.right()-1, r.top());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
                r.setBottom(r.bottom()-1);
            }
            r.setLeft(r.left()+1);
            r.setTop(r.top()+1);
            r.setRight(r.right()-1);
            ++i;
        }
        for(; i < bWidth; ++i){
            p->drawLine(r.left(), r.top(), r.left(), r.bottom());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
            p->drawLine(r.left(), r.top(), r.right(), r.top());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
                r.setBottom(r.bottom()-1);
            }
            r.setLeft(r.left()+1);
            r.setTop(r.top()+1);
            r.setRight(r.right()-1);
        }
        i=0;
        if(i < hWidth && bWidth == 0){
            p->setPen(cg->light());
            p->drawLine(r.left(), r.top()+1, r.left(), r.bottom());
            p->drawLine(r.left()+1, r.top(), r.right()-1, r.top());
            p->setPen(cg->dark());
            p->drawLine(r.right(), r.top()+1, r.right(), r.bottom());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
                r.setBottom(r.bottom()-1);
            }
            r.setLeft(r.left()+1);
            r.setTop(r.top()+1);
            r.setRight(r.right()-1);
            ++i;
        }
        for(; i < hWidth; ++i){
            p->setPen(cg->light());
            p->drawLine(r.left(), r.top(), r.left(), r.bottom());
            p->drawLine(r.left(), r.top(), r.right(), r.top());
            p->setPen(cg->dark());
            p->drawLine(r.right(), r.top()+1, r.right(), r.bottom());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
                r.setBottom(r.bottom()-1);
            }
            r.setLeft(r.left()+1);
            r.setTop(r.top()+1);
            r.setRight(r.right()-1);
        }
        if(isPixmap(widget))
            p->drawTiledPixmap(r.left(), r.top(), r.width(), r.height(),
                               *scalePixmap(r.width(), r.height(),
                                            widget));
        else
            p->fillRect(r.left(), r.top(), r.width(), r.height(),
                        cg->background());
    }
    else if(tb->shape()  == QTabBar::RoundedBelow){
        if(!selected){
            p->fillRect(r.left(), r.bottom()-2, r.width(), 2,
                        tb->palette().normal().brush( QColorGroup::Background));
            r.setBottom(r.bottom()-2);
        }
        p->setPen(cg->text());
        i=0;
        if(i < bWidth){
            p->drawLine(r.left(), r.top(), r.left(), r.bottom()-1);
            p->drawLine(r.right(), r.top(), r.right(), r.bottom()-1);
            p->drawLine(r.left()+1, r.bottom(), r.right()-1, r.bottom());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.top(), r.right(), r.top());
                r.setTop(r.top()+1);
            }
            r.setLeft(r.left()+1);
            r.setBottom(r.bottom()-1);
            r.setRight(r.right()-1);
            ++i;
        }
        for(; i < bWidth; ++i){
            p->drawLine(r.left(), r.top(), r.left(), r.bottom());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
            p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.top(), r.right(), r.top());
                r.setTop(r.top()+1);
            }
            r.setLeft(r.left()+1);
            r.setBottom(r.bottom()-1);
            r.setRight(r.right()-1);
            ++i;
        }
        i=0;
        if(i < hWidth && bWidth == 0){
            p->setPen(cg->dark());
            p->drawLine(r.left()+1, r.bottom(), r.right()-1, r.bottom());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom()-1);
            p->setPen(cg->light());
            p->drawLine(r.left(), r.top(), r.left(), r.bottom()-1);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.top(), r.right(), r.top());
                r.setTop(r.top()+1);
            }
            r.setLeft(r.left()+1);
            r.setBottom(r.bottom()-1);
            r.setRight(r.right()-1);
            ++i;
        }
        for(; i < hWidth; ++i){
            p->setPen(cg->dark());
            p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
            p->setPen(cg->light());
            p->drawLine(r.left(), r.top(), r.left(), r.bottom());
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(r.left(), r.top(), r.right(), r.top());
                r.setTop(r.top()+1);
            }
            r.setLeft(r.left()+1);
            r.setBottom(r.bottom()-1);
            r.setRight(r.right()-1);
            ++i;
        }
        if(isPixmap(widget))
            p->drawTiledPixmap(r.left(), r.top(), r.width(), r.height(),
                               *scalePixmap(r.width(), r.height(),
                                            widget));
        else
            p->fillRect(r.left(), r.top(), r.width(), r.height(),
                        cg->background());
    }
    else 
        QCommonStyle::drawTab(p, tb, t, selected );
}

void KThemeStyle::drawTabMask(QPainter* p,  const  QTabBar* tb, QTab* t,
                              bool selected )
{
    QRect r(t->r);

    if(tb->shape() == QTabBar::RoundedAbove){
        if(!selected)
            r.setTop(r.top()+2);
        p->drawLine(r.left()+1, r.top(), r.right()-1, r.top());
        QBrush b(color1, SolidPattern);
        p->fillRect(r.left(), r.top()+1, r.width(), r.height()-1, b);
    }
    else if(tb->shape()  == QTabBar::RoundedBelow){
        if(!selected)
            r.setBottom(r.bottom()-2);
        p->drawLine(r.left()+1, r.bottom(), r.right()-1, r.bottom());
        QBrush b(color1, SolidPattern);
        p->fillRect(r.left(), r.top(), r.width(), r.height()-1, b);
    } else
        QCommonStyle::drawTabMask(p, tb, t, selected );
    
}


#include "kthemestyle.moc"

