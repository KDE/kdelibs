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

void KThemeStyle::polish(QApplication *app)
{
  oldPalette = app->palette();
  if(isPixmap(Background)){
    QBrush bgBrush(oldPalette.color(QPalette::Normal,
					QColorGroup::Background),
		   *uncached(Background));
    oldPalette.setBrush(QPalette::Normal, QColorGroup::Background,
			    bgBrush);
    bgBrush.setColor(oldPalette.color(QPalette::Disabled,
					  QColorGroup::Background));
    oldPalette.setBrush(QPalette::Disabled, QColorGroup::Background,
			  bgBrush);
    bgBrush.setColor(oldPalette.color(QPalette::Active,
					QColorGroup::Background));
    oldPalette.setBrush(QPalette::Active, QColorGroup::Background,
			  bgBrush);
    app->setPalette(oldPalette);
  }
}

void KThemeStyle::unPolish(QApplication *app)
{
    app->setPalette(oldPalette, TRUE);
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
    
    drawShade(p, x, y, w, h, *colorGroup(g, type), sunken, rounded,
              highlightWidth(type), borderWidth(type), shade());
    
    // This stinks. I wanted to assign a color and optional pixmap to a
    // brush with pixmapBrush and fillRect that, but the pixmap in a brush
    // doesn't necessarily paint at 0,0 in the pixmap. It depends on where
    // you start painting :( Faster to just do this...
    if((w-offset*2) > 0 && (h-offset*2) > 0){
        if(isPixmap(type))
            p->drawTiledPixmap(x+offset, y+offset, w-offset*2, h-offset*2,
                               *scalePixmap(w-offset*2, h-offset*2, type));
        else
            p->fillRect(x+offset, y+offset, w-offset*2, h-offset*2,
                        colorGroup(g, type)->brush(QColorGroup::Button));
    }
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
    p->setPen(color1);
    if(round && borderWidth(PushButton) == 1){
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->drawLine(x+1, y, x2-1, y);
        p->drawLine(x+1, y2, x2-1, y2);
        p->drawLine(x, y+1, x, y2-1);
        p->drawLine(x2, y+1, x2, y2-1);
        p->fillRect(x+1, y+1, w-2, h-2, QBrush(color1, SolidPattern));
    }
    else if(round){
        int x2 = x+w-1;
        int y2 = y+h-1;
        p->drawLine(x+2, y, x2-2, y);
        p->drawLine(x+2, y2, x2-2, y2);
        p->drawLine(x, y+2, x, y2-2);
        p->drawLine(x2, y+2, x2, y2-2);

        p->drawLine(x+1, y+1, x2-1, y+1);
        p->drawLine(x+1, y2-1, x2-1, y2-1);
        p->drawLine(x+1, y+1, x+1, y2-1);
        p->drawLine(x2-1, y+1, x2-1, y2-1);
        p->fillRect(x+2, y+2, w-4, h-4, QBrush(color1, SolidPattern));

    }
    else
        p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
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
                                     KToolButtonType type, const QString btext,
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
    drawBaseButton(p, x, y, w, h, g, sunken, false, ComboBox);
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

    if((controls & SubPage)){
        drawScrollBarGroove(p, sb, horizontal, subPage, g);
    }
    if((controls & AddPage)){
        drawScrollBarGroove(p, sb, horizontal, addPage, g);
    }
    if(controls & AddLine){
        drawBaseButton(p, add.x(), add.y(), add.width(), add.height(), g,
                       activeControl == AddLine, ScrollButton);
        drawArrow(p, (horizontal) ? RightArrow : DownArrow, false,
                  add.x()+3, add.y()+3, add.width()-6,
                  add.height()-6, *colorGroup(g, ScrollButton));
    }
    if(controls & SubLine){
        p->setPen(g.dark());
        p->drawRect(sub);
        drawBaseButton(p, sub.x(), sub.y(), sub.width(), sub.height(), g,
                       activeControl == SubLine, ScrollButton);
        drawArrow(p, (horizontal)  ? LeftArrow : UpArrow, false, sub.x()+3,
                  sub.y()+3, sub.width()-6, sub.height()-6,
                  *colorGroup(g, ScrollButton));
    }
    if(controls & Slider){
        drawBaseButton(p, slider.x(), slider.y(), slider.width(),
                       slider.height(), g, false, ScrollBarSlider);
        if(isPixmap(ScrollDeco)){
            if(slider.width() > uncached(ScrollDeco)->width() &&
               slider.height() > uncached(ScrollDeco)->height()){
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
    sliderMax = sliderMin + maxlen - sliderLength;
    if (sliderLength < 12)
        sliderLength = 12;
    if (sliderLength > maxlen )
        sliderLength = maxlen;
}

QStyle::ScrollControl KThemeStyle::scrollBarPointOver(const QScrollBar *sb,
                                                      int sliderStart,
                                                      const QPoint &p)
{
    if(!sb->rect().contains(p))
        return(NoScroll);
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
        return AddLine;
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
        return AddLine;
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
    // Offset is off here...
    if(isPixmap(Slider)){
        const QBitmap *mask = uncached(Slider)->mask();
        if(mask){
            if(orient == Qt::Horizontal)
                p->drawPixmap(x, y+(h-uncached(Slider)->height())/2,
                              *mask);
            else
                p->drawPixmap(x+(w-uncached(Slider)->width())/2,
                              y, *mask);
        }
        else
            p->fillRect(x, y, w, h, QBrush(color1, SolidPattern));
    }
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
    if(arrowType() == MotifArrow)
        qDrawArrow(p, type, Qt::MotifStyle, down, x, y, w, h, g, enabled);
    else if(arrowType() == SmallArrow){
        QBrush oldBrush = g.brush(QColorGroup::Button);
        QColorGroup tmp(g);
        tmp.setBrush(QColorGroup::Button, QBrush(NoBrush));
        QPlatinumStyle::drawArrow(p, Qt::DownArrow, false, w-15, y, 15, h,
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

void KThemeStyle::drawShade(QPainter *p, int x, int y, int w, int h,
                            QColorGroup g, bool sunken, bool rounded,
                            int hWidth, int bWidth, ShadeStyle style)
{
    int i = 0;
    
    p->setPen(g.shadow());
    for(; i < bWidth && w > 2 && h > 2; ++i, ++x, ++y, w-=2, h-=2)
        p->drawRect(x, y , w, h);

    if(!hWidth)
        return;

    int x2 = x+w-1, y2 = y+h-1;
    QPen oldPen = p->pen();
    QPointArray highShade(hWidth*4);
    QPointArray lowShade(hWidth*4);
    QPointArray roundFill(4);  //small for a QPointArray, but that will change

    if(rounded && bWidth > 1)
        roundFill.putPoints(0, 4, x,y, x2,y, x,y2, x2,y2);

    for(i=0; i < hWidth; ++i, ++x, ++y, --x2, --y2){
        highShade.putPoints(i*4, 4, x,y, x2,y, x,y, x,y2);
        lowShade.putPoints(i*4, 4, x,y2, x2,y2, x2,y, x2,y2);
    }
    switch(style){
    case Motif:
        p->setPen((sunken) ? g.dark() : g.light());
        p->drawLineSegments(highShade);
        p->setPen((sunken) ? g.light() : g.dark());
        p->drawLineSegments(lowShade);
        break;
    case Windows:
        if(hWidth > 1){
            p->setPen((sunken) ? g.shadow() : g.light());
            p->drawLineSegments(highShade, 0, 2);
            p->setPen((sunken) ? g.light() : g.shadow());
            p->drawLineSegments(lowShade, 0, 2);

            p->setPen((sunken) ? g.dark() : g.mid());
            p->drawLineSegments(highShade, 4);
            p->setPen((sunken) ? g.mid() : g.dark());
            p->drawLineSegments(lowShade, 4);
        }
        else{
            p->setPen((sunken) ? g.shadow() : g.light());
            p->drawLineSegments(highShade);
            p->setPen((sunken) ? g.light() : g.shadow());
            p->drawLineSegments(lowShade);
        }
        break;
    case Next:
    default:
        p->setPen((sunken) ? g.shadow() : g.light());
        p->drawLineSegments(highShade);
        p->setPen((sunken) ? g.light() : g.shadow());
        p->drawLineSegments(lowShade);
        break;
    }

    if(rounded && bWidth > 1){
        p->setPen(g.shadow());
        p->drawPoints(roundFill);
    }
    p->setPen(oldPen);
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

void KThemeStyle::drawPopupMenuItem(QPainter *p, bool checkable, int /*maxpmw*/,
                                    int tab, QMenuItem *mi, const QPalette &pal,
                                    bool act, bool enabled, int x, int y, int w,
                                    int h)
{
    WidgetType widget = act && enabled ? MenuItemOn : MenuItemOff;
    int offset = decoWidth(widget);
    int pWidth = QMAX(isPixmap(CheckMark) ? uncached(CheckMark)->width()+2 : 0 ,
                      12);
    const QColorGroup cg = *colorGroup(!enabled ? pal.disabled() : act ?
                                       pal.active() : pal.normal(),
                                       widget);
    if(mi->isSeparator()){
        p->setPen(cg.dark());
        p->drawLine(x, y, x+w, y);
        p->setPen(cg.light());
        p->drawLine(x, y+1, x+w, y+1);
        return;
    }
    drawBaseButton(p, x, y, w,  h, cg, act && enabled, false, widget);
    /*
    if(mi->isChecked()){
        if (act && enabled){
            qDrawShadePanel(p, x+offset, y+offset, pWidth, h-offset*2, cg,
                            true, 1, &cg.brush(QColorGroup::Button));
        }
        else{
            qDrawShadePanel(p, x+offset, y+offset, pWidth, h-offset*2, cg,
                            true, 1, &cg.brush(QColorGroup::Midlight));
        }
    } */
    if(mi->iconSet()){
        QIconSet::Mode mode = enabled ? QIconSet::Normal : QIconSet::Disabled;
        if (act && enabled )
            mode = QIconSet::Active;
        QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
        int pixw = pixmap.width();
        int pixh = pixmap.height();
        QRect cr(x+offset+2, y+offset+2, pWidth, h-offset*2);
        QRect pmr(0, 0, pixw, pixh );
        pmr.moveCenter(cr.center());
        p->setPen(cg.text());
        p->drawPixmap(pmr.topLeft(), pixmap);

    }
    else if(checkable && mi->isChecked())
        drawCheckMark(p, x+offset, y+offset, pWidth, h-offset*2, cg, act,
                      !enabled);

    QString s = mi->text();
    p->setPen(enabled ? cg.text() : cg.light());
    if(!s.isNull()){
        int t = s.find( '\t' );
        const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
        if (t >= 0){
            p->drawText(x+w-tab-offset, y+offset, tab, h-offset*2,
                        text_flags, s.mid(t+1));
        }
        p->drawText(x+offset+pWidth+2, y+offset, w-pWidth-offset*2,
                     h-offset*2, text_flags, s, t);
    }
    else if(mi->pixmap()){ // Not sure if this is correct...
        QPixmap *pixmap = mi->pixmap();
        if (pixmap->depth() == 1)
            p->setBackgroundMode(OpaqueMode );
        p->drawPixmap( x+offset+2, y+offset+2, *pixmap );
        if ( pixmap->depth() == 1 )
            p->setBackgroundMode(TransparentMode);
    }
    if (mi->popup()){
        int dim = h-10;
        drawArrow(p, RightArrow, false, x+w-dim-offset-4,
                  y+5, dim, dim, cg, mi->isEnabled());
    }
}

#include "kthemestyle.moc"

