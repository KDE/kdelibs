/* This file is part of the KDE libraries
   Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qglobal.h>

#if QT_VERSION < 300

#include <kthemestyle.h>
#include <kthemebase.h>
#include <kapp.h>
#include <kdebug.h>
#include <qbitmap.h>
#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#include <qpopupmenu.h>
#include <qtabbar.h>
#include <qglobal.h>
#include <kglobalsettings.h>
#include <kdrawutil.h>

#include <limits.h>

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

KThemeStyle::KThemeStyle(const QString &configFile )
    :  KThemeBase(configFile)
{
    setScrollBarExtent(getSBExtent(), getSBExtent());
    setButtonDefaultIndicatorWidth(0); // We REALLY should support one, see drawPushButton() below!
}

KThemeStyle::~KThemeStyle(){
}

void KThemeStyle::polish(QApplication */*app*/)
{
}

void KThemeStyle::polish(QPalette &p)
{
    oldPalette = p;
    if(isPixmap(Background) || isColor(Background)){
        if(isColor(Background)){
            p.setColor(QColorGroup::Background,
                                colorGroup(oldPalette.normal(), Background)
                                ->background());
        }
        if(isPixmap(Background)){
            QBrush bgBrush(oldPalette.color(QPalette::Normal,
                                            QColorGroup::Background),
                           *uncached(Background));
            p.setBrush(QColorGroup::Background, bgBrush);
        }
        p.setColor(QColorGroup::ButtonText,
                   colorGroup(oldPalette.normal(), PushButton)->foreground());
    }
}


void KThemeStyle::unPolish(QApplication *app)
{
    app->setPalette(oldPalette, true);
}

void KThemeStyle::polish(QWidget *w)
{
    if ( !w->isTopLevel() ) {
	if ( w->inherits("QGroupBox")
	     || w->inherits("QTabWidget") ) {
	    w->setAutoMask( TRUE );
	    return;
	}
 	if (w->inherits("QLabel")
 	    || w->inherits("QSlider")
 	    || w->inherits("QButton")
	    || w->inherits("QProgressBar")
	    ){
	    w->setBackgroundOrigin( QWidget::ParentOrigin );
 	}
    }
    if(w->inherits("QPopupMenu")){
        popupPalette = w->palette();
        if(isColor(MenuItem) || isColor(MenuItemDown)){
            QPalette newPal(w->palette());
            w->setPalettePropagation(QWidget::SamePalette);
            if(isColor(MenuItem)){
                newPal.setNormal(*colorGroup(newPal.normal(), MenuItem));
                newPal.setDisabled(*colorGroup(newPal.normal(), MenuItem));
            }
            if(isColor(MenuItemDown))
                newPal.setActive(*colorGroup(newPal.active(), MenuItemDown));
            w->setPalette(newPal);
        }
    }
    if(w->inherits("QCheckBox")){
        if(isColor(IndicatorOff) || isColor(IndicatorOn)){
            QPalette newPal(w->palette());
            w->setPalettePropagation(QWidget::SamePalette);
            if(isColor(IndicatorOff)){
                newPal.setNormal(*colorGroup(newPal.normal(), IndicatorOff));
                newPal.setDisabled(*colorGroup(newPal.normal(), IndicatorOff));
            }
            if(isColor(IndicatorOn))
                newPal.setActive(*colorGroup(newPal.active(), IndicatorOn));
            w->setPalette(newPal);
        }
    }
    if(w->inherits("QRadioButton")){
        if(isColor(ExIndicatorOff) || isColor(ExIndicatorOn)){
            QPalette newPal(w->palette());
            w->setPalettePropagation(QWidget::SamePalette);
            if(isColor(ExIndicatorOff)){
                newPal.setNormal(*colorGroup(newPal.normal(), ExIndicatorOff));
                newPal.setDisabled(*colorGroup(newPal.normal(),
                                               ExIndicatorOff));
            }
            if(isColor(ExIndicatorOn))
                newPal.setActive(*colorGroup(newPal.active(), ExIndicatorOn));
            w->setPalette(newPal);
        }
    }
}

void KThemeStyle::unPolish(QWidget* w)
{
    if ( !w->isTopLevel() ) {
	if ( w->inherits("QGroupBox")
	     || w->inherits("QTabWidget") ) {
	    w->setAutoMask( FALSE );
	    return;
	}
 	if (w->inherits("QLabel")
 	    || w->inherits("QSlider")
 	    || w->inherits("QButton")
	    || w->inherits("QProgressBar")
	    ){
	    w->setBackgroundOrigin( QWidget::WidgetOrigin );
 	}
    }
    if(w->inherits("QPopupMenu"))
        w->unsetPalette();
    if(w->inherits("QCheckBox"))
        w->unsetPalette();
    if(w->inherits("QRadioButton"))
        w->unsetPalette();
}

void KThemeStyle::drawBaseButton(QPainter *p, int x, int y, int w, int h,
                                 const QColorGroup &g, bool sunken, bool
                                 rounded, WidgetType type, const QBrush *)
{
    int offset = borderPixmap(type) ? 0 : decoWidth(type);
    QPen oldPen = p->pen();

    // handle reverse bevel here since it uses decowidth differently
    if(gradientHint(type) == GrReverseBevel){
        int i;
        bitBlt(p->device(), x, y, scalePixmap(w, h, type), 0, 0, w, h,
               Qt::CopyROP, true);
        p->setPen(g.text());
        for(i=0; i < borderWidth(type); ++i, ++x, ++y, w-=2, h-=2)
            p->drawRect(x, y, w, h);
    }
    // same with KDE style borders
    else if(!borderPixmap(type) && shade() == KDE){
        kDrawBeButton(p, x, y, w, h, g, sunken);
        if(isPixmap(type))
            p->drawTiledPixmap(x+4, y+4, w-6, h-6,
                               *scalePixmap(w-6, h-6,
                                            type));
            else
                p->fillRect(x+4, y+4, w-6, h-offset*6,
                            g.brush(QColorGroup::Button));

    }
    else{
        if((w-offset*2) > 0 && (h-offset*2) > 0){
            if(isPixmap(type))
                if(rounded)
                    p->drawTiledPixmap(x, y, w, h, *scalePixmap(w, h, type));
                else
                    p->drawTiledPixmap(x+offset, y+offset, w-offset*2,
                                       h-offset*2,
                                       *scalePixmap(w-offset*2, h-offset*2,
                                                    type));
            else
                p->fillRect(x+offset, y+offset, w-offset*2, h-offset*2,
                            g.brush(QColorGroup::Button));
        }
        if(borderPixmap(type))
            bitBlt(p->device(), x, y, scaleBorder(w, h, type), 0, 0, w, h,
                   Qt::CopyROP, false);
        else
            drawShade(p, x, y, w, h, g, sunken, rounded,
                      highlightWidth(type), borderWidth(type), shade());
    }
    p->setPen(oldPen);
}

void KThemeStyle::drawButton(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken,
                             const QBrush *fill)
{
    drawBaseButton(p, x, y, w, h, g, sunken, roundButton(), sunken ?
                   PushButtonDown : PushButton, fill);
}

void KThemeStyle::drawPushButton(QPushButton* btn, QPainter *p)
{
    bool sunken = btn->isOn() || btn->isDown();
    int diw = buttonDefaultIndicatorWidth();
    drawBaseButton(p, diw, diw, btn->width()-2*diw, btn->height()-2*diw,
                   *colorGroup(btn->colorGroup(), sunken ? PushButtonDown :
                               PushButton), sunken, roundButton(),
                   sunken ? PushButtonDown : PushButton, NULL);
    // TODO if diw, draw fancy default button indicator
}

void KThemeStyle::drawBaseMask(QPainter *p, int x, int y, int w, int h,
                               bool round)
{
    // round edge fills
    static const QCOORD btm_left_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,1,
    1,2,2,2,3,2,4,2,2,3,3,3,4,3,3,4,4,4 };

    static const QCOORD btm_right_fill[]={ 0,0,1,0,2,0,3,0,4,0,0,1,1,1,2,1,3,1,4,
    1,0,2,1,2,2,2,3,2,0,3,1,3,2,3,0,4,1,4 };

    static const QCOORD top_left_fill[]={ 3,0,4,0,2,1,3,1,4,1,1,2,2,2,3,2,4,2,0,3,
    1,3,2,3,3,3,4,3,0,4,1,4,2,4,3,4,4,4 };

    static const QCOORD top_right_fill[]={ 0,0,1,0,0,1,1,1,2,1,0,2,1,2,2,2,3,2,0,
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
    drawBaseMask(p, x, y, w, h, roundComboBox());
}

void KThemeStyle::drawBevelButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken,
                                  const QBrush *)
{
    WidgetType type = sunken ? BevelDown : Bevel;
    drawBaseButton(p, x, y, w, h, *colorGroup(g, type), sunken, false, type);
}

void KThemeStyle::drawKToolBarButton(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, bool sunken,
                                     bool raised, bool enabled, bool popup,
                                     KToolButtonType type, const QString &btext,
                                     const QPixmap *pixmap, QFont *font,
                                     QWidget *)
{
    QFont tmp_font(QString::fromLatin1("Helvetica"), 10);
    if (font)
        tmp_font = *font;
    QFontMetrics fm(tmp_font);
    WidgetType widget = sunken ? ToolButtonDown : ToolButton;

    drawBaseButton(p, x, y, w, h, *colorGroup(g, widget), sunken, false,
                   widget);
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
    else if (type == Text){ // only text, even if there is a icon
        if (!btext.isNull()){
            int tf = AlignTop|AlignLeft;
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
    else if (type == IconTextBottom){
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
            dx= (w - fm.width(btext)) / 2;
            dy= h - fm.lineSpacing() - 4;
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
                                 const QColorGroup &g, KToolBarPos, QBrush *)
{
    if(w > h)
        drawBaseButton(p, x, y, w, h, *colorGroup(g, HBarHandle), false, false,
                       HBarHandle);
    else
        drawBaseButton(p, x, y, w, h, *colorGroup(g, VBarHandle), false, false,
                       VBarHandle);
}

void KThemeStyle::drawKToolBar(QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, KToolBarPos, QBrush *)
{
    drawBaseButton(p, x, y, w, h, *colorGroup(g, ToolBar), false, false,
                   ToolBar);
}

QRect KThemeStyle::buttonRect(int x, int y, int w, int h)
{
    int spacing = decoWidth(PushButton) > decoWidth(PushButtonDown) ?
        decoWidth(PushButton) : decoWidth(PushButtonDown);
    return(QRect(x+spacing, y+spacing, w-(spacing*2), h-(spacing*2)));
}

void KThemeStyle::drawComboButton(QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken, bool,
                                  bool, const QBrush *)
{
    WidgetType widget = sunken ? ComboBoxDown : ComboBox;
    drawBaseButton(p, x, y, w, h, *colorGroup(g, widget), sunken,
                   roundComboBox(), widget);
    if(!sunken && isPixmap(ComboDeco))
        p->drawPixmap(w - uncached(ComboDeco)->width()-
                      decoWidth(ComboBox)-2,
                      y+(h - uncached(ComboDeco)->
                         height())/2, *uncached(ComboDeco));
    else if(sunken && isPixmap(ComboDecoDown))
        p->drawPixmap(w - uncached(ComboDecoDown)->width()-
                      decoWidth(ComboBoxDown)-2,
                      y +(h - uncached(ComboDecoDown)->
                          height())/2, *uncached(ComboDecoDown));
    else{
        qDrawArrow(p, Qt::DownArrow, Qt::MotifStyle, false, w-15, y+6, 10,
                   h-15, *colorGroup(g, widget), true);
        qDrawShadeRect(p, w-14, y+7+(h-15), 10, 3, *colorGroup(g, widget));
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

    bool active;
    if((controls & QStyle::SubPage)){
        drawScrollBarGroove(p, sb, horizontal, subPage, g);
    }
    if((controls & QStyle::AddPage)){
        drawScrollBarGroove(p, sb, horizontal, addPage, g);
    }
    if(controls & QStyle::AddLine){
        active = activeControl == QStyle::AddLine;
        drawBaseButton(p, add.x(), add.y(), add.width(), add.height(),
                       *colorGroup(g, active ? ScrollButtonDown : ScrollButton),
                       active, false, active ? ScrollButtonDown : ScrollButton);
        drawArrow(p, (horizontal) ? RightArrow : DownArrow, active, add.x()+3,
                  add.y()+3, add.width()-6, add.height()-6,
                  *colorGroup(g, active ? ScrollButtonDown : ScrollButton));
    }
    if(controls & QStyle::SubLine){
        active = activeControl == QStyle::SubLine;
        p->setPen(g.dark());
        p->drawRect(sub);
        drawBaseButton(p, sub.x(), sub.y(), sub.width(), sub.height(),
                       *colorGroup(g, active ? ScrollButtonDown : ScrollButton),
                       active, false, active ? ScrollButtonDown : ScrollButton);
        drawArrow(p, (horizontal)  ? LeftArrow : UpArrow, active, sub.x()+3,
                  sub.y()+3, sub.width()-6, sub.height()-6,
                  *colorGroup(g, active ? ScrollButtonDown : ScrollButton));
    }
    if(controls & QStyle::Slider){
        active = activeControl == QStyle::Slider;
        WidgetType widget = horizontal ?
            active ? HScrollBarSliderDown : HScrollBarSlider :
            active ? VScrollBarSliderDown : VScrollBarSlider;

        drawBaseButton(p, slider.x(), slider.y(), slider.width(),
                       slider.height(), *colorGroup(g, widget), active, false,
                       widget);
        int spaceW = horizontal ? slider.width()-decoWidth(widget)-4 :
            slider.width();
        int spaceH = horizontal ? slider.height() :
            slider.height()-decoWidth(widget)-4;
        widget = active ? horizontal ? HScrollDecoDown : VScrollDecoDown :
            horizontal ? HScrollDeco : VScrollDeco;
        if(isPixmap(widget)){
            if(spaceW >= uncached(widget)->width() &&
               spaceH >= uncached(widget)->height()){
                p->drawPixmap(slider.x()+(slider.width() -
                                          uncached(widget)->width())/2,
                              slider.y()+(slider.height() -
                                          uncached(widget)->height())/2,
                              *uncached(widget));
            }
        }
    }
}

void KThemeStyle::drawScrollBarGroove(QPainter *p, const QScrollBar *sb,
                                      bool horizontal, QRect r, QColorGroup g)

{
    WidgetType widget = (horizontal) ? HScrollGroove : VScrollGroove;
    if(!isPixmap(widget)){
        p->fillRect(r, colorGroup(g, widget)->brush(QColorGroup::Background));
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
    int offset = decoWidth(horizontal ? HScrollGroove : VScrollGroove);
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
        QPlatinumStyle::drawSliderGroove(p, x, y, w, h,
                                         *colorGroup(g, SliderGroove),
                                         c, orient);
    else
        drawBaseButton(p, x, y, w, h, *colorGroup(g, SliderGroove), true,
                       false, SliderGroove);
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
    WidgetType widget;
    switch(type){
    case UpArrow:
        widget = enabled ? down ? SunkenArrowUp : ArrowUp : DisArrowUp;
        break;
    case DownArrow:
        widget = enabled ? down ? SunkenArrowDown : ArrowDown : DisArrowDown;
        break;
    case LeftArrow:
        widget = enabled ? down ? SunkenArrowLeft : ArrowLeft : DisArrowLeft;
        break;
    case RightArrow:
    default:
        widget = enabled ? down ? SunkenArrowRight : ArrowRight : DisArrowRight;
        break;
    }
    if(isPixmap(widget)){
        p->drawPixmap(x+(w-uncached(widget)->width())/2,
                      y+(h-uncached(widget)->height())/2,
                      *uncached(widget));
        return;
    }
    const QColorGroup *cg = colorGroup(g, widget);
    // Standard arrow types
    if(arrowType() == MotifArrow)
        qDrawArrow(p, type, Qt::MotifStyle, down, x, y, w, h, *cg, enabled);
    else if(arrowType() == SmallArrow){
        QColorGroup tmp(*cg);
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
        p->setBrush(cg->brush(QColorGroup::Shadow));
        p->setPen(cg->shadow());
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
    WidgetType widget = btn->isDown() || btn->isOn() ? PushButtonDown :
        PushButton;
    const QColorGroup *cg = colorGroup(btn->colorGroup(), widget);
    int x, y, w, h;

    QRect r = btn->rect();
    r.rect(&x, &y, &w, &h);
    x+=decoWidth(widget);
    y+=decoWidth(widget);
    w-=decoWidth(widget)*2;
    h-=decoWidth(widget)*2;
    bool act = btn->isOn() || btn->isDown();

    // If this is a button with an associated popup menu, draw an arrow first
    if ( btn->popup() )
    {
	int dx = menuButtonIndicatorWidth( btn->height() );

	QColorGroup g( btn->colorGroup() );
	int xx = x + w - dx - 4;
	int yy = y - 3;
	int hh = h + 6;

	if ( !act )
	{
	    p->setPen( g.light() );
	    p->drawLine( xx, yy + 3, xx, yy + hh - 4 );
	}
	else
	{
	    p->setPen( g.button() );
	    p->drawLine( xx, yy + 4, xx, yy + hh - 4 );
	}
	drawArrow( p, DownArrow, FALSE,
		   x + w - dx - 2, y + 2, dx, h - 4,
		   btn->colorGroup(),
		   btn->isEnabled() );
	w -= dx;
    }

    // Next, draw iconset, if any
    if ( btn->iconSet() && !btn->iconSet()->isNull() )
    {
        QIconSet::Mode mode = btn->isEnabled()
			      ? QIconSet::Normal : QIconSet::Disabled;
        if ( mode == QIconSet::Normal && btn->hasFocus() )
            mode = QIconSet::Active;
        QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
        int pixw = pixmap.width();
        int pixh = pixmap.height();

        p->drawPixmap( x+6, y+h/2-pixh/2, pixmap );
        x += pixw + 8;
        w -= pixw + 8;
    }
    
    if(widget == PushButtonDown){
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
    drawBaseButton(p, x, y, w, h, *colorGroup(g, Splitter), false, false,
                   Splitter);
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
    int offset = QMAX(decoWidth(MenuItemDown), decoWidth(MenuItem)) + 4;

    if(mi->isSeparator())
        return(2);
    if(mi->isChecked())
        h = isPixmap(CheckMark) ? uncached(CheckMark)->height()+offset :
            offset+16;
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

void KThemeStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
                                     int tab, QMenuItem* mi,
                                     const QPalette& pal, bool act,
                                     bool enabled, int x, int y, int w, int h)
{
// I changed the following method to be based from Qt's instead of my own
// wacky code. Works much better now :P (mosfet)
static const int motifItemFrame         = 2;    // menu item frame width
static const int motifItemHMargin       = 5;    // menu item hor text margin
static const int motifItemVMargin       = 4;    // menu item ver text margin

static const int motifArrowHMargin      = 6;    // arrow horizontal margin
static const int windowsRightBorder     = 12;       // right border on windowsstatic const int windowsCheckMarkWidth = 12;       // checkmarks width on windows
    bool dis = !enabled;
    const QColorGroup &g = dis ? *colorGroup(pal.normal(), MenuItem) :
        *colorGroup(pal.normal(), MenuItemDown);

    QColorGroup itemg = dis ? *colorGroup(pal.disabled(), MenuItem)
        : act ? *colorGroup(pal.active(), MenuItemDown)
        : *colorGroup(pal.normal(), MenuItem);

    maxpmw = QMAX( maxpmw, 20 );
    int checkcol = maxpmw;

    if ( mi && mi->isSeparator()){
        p->setPen(g.dark());
	p->drawLine(x, y, x+w, y);
	p->setPen(g.light());
	p->drawLine(x, y+1, x+w, y+1);
	return;
    }
    if(act){
        drawBaseButton(p, x, y, w, h, g, true, false, MenuItemDown);
    }
    else{
        drawShade(p, x, y, w, h, *colorGroup(g, MenuItem), false, false,
                  highlightWidth(MenuItem), borderWidth(MenuItem),
                  shade());
        int dw = decoWidth(MenuItem);
        if(!isPixmap(MenuItem))
            p->fillRect(x+dw, y+dw, w-dw*2, h-dw*2,
                        colorGroup(g, MenuItem)->
                        brush(QColorGroup::Background));
        else{
            // process inactive item pixmaps as one large item
            p->drawTiledPixmap(x+dw, y+dw, w-dw*2, h-dw*2, *scalePixmap
                               (w, p->window().height(), MenuItem),
                               x, y);
        }
        
        if( checkable && mi && mi->isChecked() )
	{
	    // draw 'pressed' border around checkable items
            // This is extremely important for items that have an iconset
            // because the checkmark isn't drawn in that case
            // An alternative would be superimposing the checkmark over
            // the iconset instead or not drawing the iconset at all.
            int mw = checkcol + motifItemFrame;
            drawShade(p, x, y, mw, h, g, true, false,
                      highlightWidth(MenuItemDown),
                      borderWidth(MenuItemDown), shade());
	}
    }
    if ( !mi )
	return;
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

    p->setPen(colorGroup(g, act ? MenuItemDown : MenuItem)->text());

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
                discol = colorGroup(g, MenuItemDown)->text();
		//discol = white;
            QColorGroup g2(discol, g.highlight(),
                           white, white,
                           dis ? discol : white,
                           discol, white);
	    drawArrow(p, RightArrow, true,
                      x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
                      dim, dim, g2, TRUE);
	} else {
	    drawArrow(p, RightArrow,
                      false,
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
                               const QColorGroup &g, bool, QBrush *)
{
    drawBaseButton(p, x, y, w, h, *colorGroup(g, MenuBar), false, false,
                   MenuBar);
}

void KThemeStyle::drawKMenuItem(QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool active,
                                QMenuItem *mi, QBrush *)
{
    const QColorGroup *cg = colorGroup(g, active ? MenuBarItem : MenuBar);
    QColor btext = cg->buttonText();
    if(active)
        drawBaseButton(p, x, y, w, h, *cg, false, false, MenuBarItem);
        //qDrawShadePanel(p, x, y, w, h, *cg, false, 1);

    drawItem(p, x, y, w, h, AlignCenter|ShowPrefix|DontClip|SingleLine,
             *cg, mi->isEnabled(), mi->pixmap(), mi->text(),
             -1, &btext );;
}

void KThemeStyle::drawKProgressBlock(QPainter *p, int x, int y, int w, int h,
                                     const QColorGroup &g, QBrush *)
{
    drawBaseButton(p, x, y, w, h, *colorGroup(g, ProgressBar), false, false,
                   ProgressBar);
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
    WidgetType widget = selected ? ActiveTab : InactiveTab;
    const QColorGroup *cg = colorGroup(tb->colorGroup(), widget);
    int i;
    int x = t->r.x(), y = t->r.y();
    int x2 = t->r.right(), y2 = t->r.bottom();
    int bWidth = borderWidth(widget);
    int hWidth = highlightWidth(widget);
    if(tb->shape() == QTabBar::RoundedAbove){
        if(!selected){
            p->fillRect(x, y, x2-x+1, 2,
                        tb->palette().normal().brush( QColorGroup::Background));
            y+=2;
        }
        p->setPen(cg->text());
        i=0;
        if(i < bWidth){
            p->drawLine(x, y+1, x, y2);
            p->drawLine(x2, y+1, x2, y2);
            p->drawLine(x+1, y, x2-1, y);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y2, x2, y2);
                --y2;
            }
            ++i, ++x, ++y, --x2;
        }
        for(; i < bWidth; ++i, ++x, ++y, --x2){
            p->drawLine(x, y, x, y2);
            p->drawLine(x2, y, x2, y2);
            p->drawLine(x, y, x2, y);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y2, x2, y2);
                --y2;
            }
        }
        i=0;
        if(i < hWidth && bWidth == 0){
            p->setPen(cg->light());
            p->drawLine(x, y+1, x, y2);
            p->drawLine(x+1, y, x2-1, y);
            p->setPen(cg->dark());
            p->drawLine(x2, y+1, x2, y2);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y2, x2, y2);
                --y2;
            }
            ++i, ++x, ++y, --x2;
        }
        for(; i < hWidth; ++i, ++x, ++y, --x2){
            p->setPen(cg->light());
            p->drawLine(x, y, x, y2);
            p->drawLine(x, y, x2, y);
            p->setPen(cg->dark());
            p->drawLine(x2, y+1, x2, y2);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y2, x2, y2);
                --y2;
            }
        }
        if(isPixmap(widget))
            p->drawTiledPixmap(x, y, x2-x+1, y2-y+1,
                               *scalePixmap(x2-x+1, y2-y+1, widget));
        else
            p->fillRect(x, y, x2-x+1, y2-y+1, cg->background());
    }
    else if(tb->shape()  == QTabBar::RoundedBelow){
        if(!selected){
            p->fillRect(x, y2-2, x2-x+1, 2,
                        tb->palette().normal().brush( QColorGroup::Background));
            y2-=2;
        }
        p->setPen(cg->text());
        i=0;
        if(i < bWidth){
            p->drawLine(x, y, x, y2-1);
            p->drawLine(x2, y, x2, y2-1);
            p->drawLine(x+1, y2, x2-1, y2);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y, x2, y);
                ++y;
            }
        }
        for(; i < bWidth; ++i, ++x, --x2, --y2){
            p->drawLine(x, y, x, y2);
            p->drawLine(x2, y, x2, y2);
            p->drawLine(x, y2, x2, y2);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y, x2, y);
                ++y;
            }
        }
        i=0;
        if(i < hWidth && bWidth == 0){
            p->setPen(cg->dark());
            p->drawLine(x+1, y2, x2-1, y2);
            p->drawLine(x2, y, x2, y2-1);
            p->setPen(cg->light());
            p->drawLine(x, y, x, y2-1);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y, x2, y);
                ++y;
            }
            ++i, ++x, --x2, --y2;
        }
        for(; i < hWidth; ++i, ++x, --x2, --y2){
            p->setPen(cg->dark());
            p->drawLine(x, y2, x2, y2);
            p->drawLine(x2, y, x2, y2);
            p->setPen(cg->light());
            p->drawLine(x, y, x, y2);
            if(selected ? activeTabLine() : inactiveTabLine()){
                p->drawLine(x, y, x2, y);
                ++y;
            }
        }
        if(isPixmap(widget))
            p->drawTiledPixmap(x, y, x2-x+1, y2-y+1,
                               *scalePixmap(x2-x+1, y2-y+1, widget));
        else
            p->fillRect(x, y, x2-x+1, y2-y+1, cg->background());
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

#endif // QT_VERSION < 300

