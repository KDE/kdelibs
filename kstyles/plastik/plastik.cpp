/* Plastik widget style for KDE 4
   Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>

   based on the KDE style "dotNET":

   Copyright (C) 2001-2002, Chris Lee <clee@kde.org>
                            Carsten Pfeiffer <pfeiffer@kde.org>
                            Karol Szwed <gallium@kde.org>
   Drawing routines completely reimplemented from KDE3 HighColor, which was
   originally based on some stuff from the KDE2 HighColor.

   based on drawing routines of the style "Keramik":

   Copyright (c) 2002 Malte Starostik <malte@kde.org>
             (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>
   based on the KDE3 HighColor Style
   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <klingens@kde.org>
   Progressbar code based on KStyle,
   Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

// #include <qimage.h>
// #include <qstylefactory.h>
// #include <q3pointarray.h>
#include <qpainter.h>
// #include <qtabbar.h>
// #include <q3progressbar.h>
#include <qcheckbox.h>
#include <qcombobox.h>
// #include <q3cleanuphandler.h>
// #include <q3header.h>
// #include <qlineedit.h>
// #include <q3listbox.h>
// #include <qscrollbar.h>
// #include <qstyleplugin.h>
#include <qpushbutton.h>
// #include <qtabwidget.h>
// #include <qtimer.h>
// #include <qtoolbutton.h>
// #include <q3toolbar.h>
// #include <qmenubar.h>
// #include <q3popupmenu.h>
// #include <qdrawutil.h>
// #include <qapplication.h>
// #include <qvariant.h>
#include <qradiobutton.h>
// #include <qregion.h>
// #include <qslider.h>
#include <qsettings.h>
#include <qsplitter.h>
// #include <kpixmap.h>
#include <QStyleOption>

#include "plastik.h"
// #include "plastik.moc"
#include "misc.h"

K_EXPORT_STYLE("Plastik4", PlastikStyle)

// some bitmaps for the radio button so it's easier to handle the circle stuff...
// 13x13
static const unsigned char radiobutton_mask_bits[] = {
   0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfc, 0x07,
   0xf8, 0x03};
static const unsigned char radiobutton_contour_bits[] = {
   0xf0, 0x01, 0x0c, 0x06, 0x02, 0x08, 0x02, 0x08, 0x01, 0x10, 0x01, 0x10,
   0x01, 0x10, 0x01, 0x10, 0x01, 0x10, 0x02, 0x08, 0x02, 0x08, 0x0c, 0x06,
   0xf0, 0x01};
static const unsigned char radiobutton_aa_inside_bits[] = {
   0x00, 0x00, 0x10, 0x01, 0x04, 0x04, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x04, 0x04, 0x10, 0x01,
   0x00, 0x00};
static const unsigned char radiobutton_aa_outside_bits[] = {
   0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x08, 0x02};
static const unsigned char radiobutton_highlight1_bits[] = {
   0x00, 0x00, 0xf0, 0x01, 0x1c, 0x07, 0x04, 0x04, 0x06, 0x0c, 0x02, 0x08,
   0x02, 0x08, 0x02, 0x08, 0x06, 0x0c, 0x04, 0x04, 0x1c, 0x07, 0xf0, 0x01,
   0x00, 0x00};
static const unsigned char radiobutton_highlight2_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x18, 0x03, 0x08, 0x02, 0x04, 0x04,
   0x04, 0x04, 0x04, 0x04, 0x08, 0x02, 0x18, 0x03, 0xe0, 0x00, 0x00, 0x00,
   0x00, 0x00};
// check mark
const uint CHECKMARKSIZE = 9; // 9x9
static const unsigned char checkmark_aa_bits[] = {
   0x45, 0x01, 0x28, 0x00, 0x11, 0x01, 0x82, 0x00, 0x44, 0x00, 0x82, 0x00,
   0x11, 0x01, 0x28, 0x00, 0x45, 0x01};
static const unsigned char checkmark_dark_bits[] = {
   0x82, 0x00, 0x45, 0x01, 0xaa, 0x00, 0x54, 0x00, 0x28, 0x00, 0x74, 0x00,
   0xea, 0x00, 0xc5, 0x01, 0x82, 0x00};
static const unsigned char checkmark_light_bits[] = {
   0x00, 0xfe, 0x82, 0xfe, 0x44, 0xfe, 0x28, 0xfe, 0x10, 0xfe, 0x08, 0xfe,
   0x04, 0xfe, 0x02, 0xfe, 0x00, 0xfe};
static const unsigned char checkmark_tristate_bits[] = {
   0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xff, 0x01,
   0x00, 0x00, 0xff, 0x01, 0x00, 0x00};
// radio mark
const uint RADIOMARKSIZE = 9; // 9x9
static const unsigned char radiomark_aa_bits[] = {
   0x00, 0x00, 0x44, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x82, 0x00, 0x44, 0x00, 0x00, 0x00};
static const unsigned char radiomark_dark_bits[] = {
   0x00, 0x00, 0x38, 0x00, 0x44, 0x00, 0xf2, 0x00, 0xfa, 0x00, 0xfa, 0x00,
   0x7c, 0x00, 0x38, 0x00, 0x00, 0x00};
static const unsigned char radiomark_light_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


PlastikStyle::PlastikStyle() :
//     kickerMode(false),
//     kornMode(false),
    flatMode(false)
{
    setWidgetLayoutProp(WT_Generic, Generic::DefaultFrameWidth, 2);

    // TODO: change this when double buttons are implemented
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, 0);

    setWidgetLayoutProp(WT_PushButton, PushButton::DefaultIndicatorMargin, 1);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Left, 4);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Right, 4);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin, 3);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Left, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Right, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Top, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Bot, 2);

    setWidgetLayoutProp(WT_CheckBox, CheckBox::Size, 13);
    setWidgetLayoutProp(WT_RadioButton, RadioButton::Size, 13);

    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckAlongsideIcon, 1);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, 13);

    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize, 10);

    setWidgetLayoutProp(WT_TabBar, TabBar::TabOverlap, 1);

    setWidgetLayoutProp(WT_TabWidget, TabWidget::FrameWidth, 2);

    setWidgetLayoutProp(WT_Slider, Slider::HandleThickness, 20/*15*/);
    setWidgetLayoutProp(WT_Slider, Slider::HandleLength, 11);

    setWidgetLayoutProp(WT_SpinBox, SpinBox::FrameWidth, 2);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonWidth, 2+16+1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonSpacing, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Left, 2);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Right, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Top, 1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Bot, 1);

    setWidgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, 2);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonWidth, 2+16+1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Left, 2);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Right, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Top, 1);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Bot, 1);

    setWidgetLayoutProp(WT_ToolBar, ToolBar::PanelFrameWidth, 1);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemSpacing, 1);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemMargin, 0);

    setWidgetLayoutProp(WT_ToolButton, ToolButton::ContentsMargin, 4);
    setWidgetLayoutProp(WT_ToolButton, ToolButton::FocusMargin,    3);

//     hoverTab = 0;
// 
//     horizontalDots = 0;
//     verticalDots = 0;
// 
//     horizontalLine = 0;
//     verticalLine = 0;
// 
    QSettings settings;
    _contrast = settings.readNumEntry("/Qt/KDE/contrast", 6);
//     settings.beginGroup("/plastikstyle/Settings");
    _scrollBarLines = settings.readBoolEntry("/scrollBarLines", false);
    _animateProgressBar = settings.readBoolEntry("/animateProgressBar", false);
//     _drawToolBarSeparator = settings.readBoolEntry("/drawToolBarSeparator", true);
    _drawToolBarItemSeparator = settings.readBoolEntry("/drawToolBarItemSeparator", true);
    _drawFocusRect = settings.readBoolEntry("/drawFocusRect", true);
    _drawTriangularExpander = settings.readBoolEntry("/drawTriangularExpander", false);
    _inputFocusHighlight = settings.readBoolEntry("/inputFocusHighlight", true);
    _customOverHighlightColor = settings.readBoolEntry("/customOverHighlightColor", false);
    _overHighlightColor.setNamedColor( settings.readEntry("/overHighlightColor", "black") );
    _customFocusHighlightColor = settings.readBoolEntry("/customFocusHighlightColor", false);
    _focusHighlightColor.setNamedColor( settings.readEntry("/focusHighlightColor", "black") );
    _customCheckMarkColor = settings.readBoolEntry("/customCheckMarkColor", false);
    _checkMarkColor.setNamedColor( settings.readEntry("/checkMarkColor", "black") );
    settings.endGroup();
// 
//     // setup pixmap cache...
    pixmapCache = new QCache<int, CacheEntry>(327680);
// 
//     if ( _animateProgressBar )
//     {
//         animationTimer = new QTimer( this );
//         connect( animationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()) );
//     }
}

// 
// 
// void PlastikStyle::updateProgressPos()
// {
//     Q3ProgressBar* pb;
//     //Update the registered progressbars.
//     QMap<QWidget*, int>::iterator iter;
//     bool visible = false;
//     for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); ++iter)
//     {   
//         if ( !qobject_cast<Q3ProgressBar>(iter.key()) )
//             continue;
//         
//         pb = dynamic_cast<Q3ProgressBar*>(iter.key());
//         if ( iter.key() -> isEnabled() && 
//              pb -> progress() != pb->totalSteps() )
//         {
//             // update animation Offset of the current Widget
//             iter.data() = (iter.data() + 1) % 20;
//             iter.key()->update();
//         }
//         if (iter.key()->isVisible())
//             visible = true;
//     }
//     if (!visible)
//         animationTimer->stop();
// }
// 
// 
PlastikStyle::~PlastikStyle()
{
    delete pixmapCache;
//     delete horizontalDots;
//     delete verticalDots;
//     delete horizontalLine;
//     delete verticalLine;
}


void PlastikStyle::drawKStylePrimitive(WidgetType widgetType, int primitive,
                                       const QStyleOption* opt,
                                       QRect r, QPalette pal, State flags,
                                       QPainter* p,
                                       const QWidget* widget,
                                       KStyle::Option* kOpt) const
{
    bool reverseLayout = opt->direction == Qt::RightToLeft;

    bool enabled = flags & State_Enabled;

    const bool mouseOver(enabled && (flags & State_MouseOver));

    switch (widgetType)
    {
        case WT_PushButton:
        {
            switch (primitive)
            {
                case Generic::Bevel:
                {
                    bool sunken   = (flags & State_On) || (flags & State_Sunken);

                    // TODO: set different background color for default buttons (Bevel is drawn on top of DefaultButtonBevel)
//                     const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(opt);
//                     if (bOpt &&  //### helper function in KStyle?
//                         (bOpt->features & QStyleOptionButton::DefaultButton)) {
//             QColorGroup g2 = cg;
//             if (isDefault)
//                 g2.setColor(QColorGroup::Background, cg.background().dark(120) );
//                     }

                    renderButton(p, r, pal, sunken,
                                 mouseOver/*,
                                 bool horizontal,
                                 bool enabled,
                            bool khtmlMode*/);

                    return;
                }

                case PushButton::DefaultButtonBevel:
                {
                    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                            Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
                    if(!enabled) contourFlags|=Is_Disabled;
                    renderContour(p, r, pal.background().color(), pal.background().color().dark(120), contourFlags);

                    return;
                }
            }
        }
        break;

        case WT_Splitter:
        {
            if (primitive == Generic::Bevel)
            {
                int w = r.width();
                int h = r.height();

                QColor color = (mouseOver)?pal.background().color().light(100+_contrast):pal.background().color();
                p->fillRect(r, color);
                if (w > h) {
                    // TODO: check if horizontal/vertical orientation is set in some QStyleOption
                    if (h > 4) {
                        int ycenter = r.height()/2;
                        for(int k = 2*r.width()/10; k < 8*r.width()/10; k+=5) {
                            renderDot(p, QPoint(k, ycenter-1), color, false, true);
                        }
                    }
                } else {
                    if (w > 4) {
                        int xcenter = r.width()/2;
                        for(int k = 2*r.height()/10; k < 8*r.height()/10; k+=5) {
                            renderDot(p, QPoint(xcenter-1, k), color, false, true);
                        }
                    }
                }

                return;
            }
        }
        break;

        case WT_CheckBox:
        {
            switch (primitive)
            {
                case CheckBox::CheckOn:
                case CheckBox::CheckOff:
                case CheckBox::CheckTriState:
                {
                    renderCheckBox(p, r, pal, enabled, mouseOver, primitive);
                    return;
                }
            }
        }
        break;

        case WT_RadioButton:
        {
            switch (primitive)
            {
                case RadioButton::RadioOn:
                case RadioButton::RadioOff:
                {
                    renderRadioButton(p, r, pal, enabled, mouseOver, primitive);
                    return;
                }
            }

        }
        break;

        case WT_ProgressBar:
        {
//             const Q3ProgressBar *pb = dynamic_cast<const Q3ProgressBar*>(widget);
//             int steps = pb->totalSteps();

            const QColor bg = enabled?pal.base().color():pal.background().color(); // background
            const QColor fg = enabled?pal.highlight().color():pal.background().color().dark(110); // foreground


            switch (primitive)
            {
                case Generic::Bevel:
                {
                    renderContour(p, r, pal.background().color(), getColor(pal, ButtonContour, enabled) );
                    p->setPen(bg.dark(105) );
                    p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
                    p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2);
                    p->setPen(bg.light(105) );
                    p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
                    p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);

                    // fill background
                    p->fillRect(r.adjusted(2,2,-2,-2), bg );

                    return;
                }

                case ProgressBar::BusyIndicator:
                {
                    renderContour( p, r/*QRect( r.x()+progress, r.y(), barWidth, r.height() )*/,
                                   bg, fg.dark(160),
                                   Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|
                                           Round_BottomRight|Round_UpperLeft|Round_BottomLeft );
                    renderSurface(p, QRect( r.x()+/*progress+*/1, r.y()+1, r.width()-2, r.height()-2 ),
                                  bg, fg, pal.highlight().color(),
                                  2*(_contrast/3),
                                  Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                          Round_UpperRight|Round_BottomRight|
                                          Round_UpperLeft|Round_BottomLeft|Is_Horizontal);

                    return;
                }

                case ProgressBar::Indicator:
                {
                    QRect Rcontour = r;
                    QRect Rsurface(Rcontour.left()+1, Rcontour.top()+1, Rcontour.width()-2, Rcontour.height()-2);

                    renderContour(p, Rcontour,
                                  bg, fg.dark(160),
                                  reverseLayout ? Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft
                        : Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight);

                    QRegion mask(Rsurface);
                    if(reverseLayout) {
                        mask -= QRegion(Rsurface.left(), Rsurface.top(), 1, 1);
                        mask -= QRegion(Rsurface.left(), Rsurface.bottom(), 1, 1);
                    } else {
                        mask -= QRegion(Rsurface.right(), Rsurface.top(), 1, 1);
                        mask -= QRegion(Rsurface.right(), Rsurface.bottom(), 1, 1);
                    }
                    p->setClipRegion(mask);
                    int counter = 0;
                    QPixmap surfaceTile(21, r.height()-2);
                    QPainter surfacePainter(&surfaceTile);
                    // - 21 pixel -
                    //  __________
                    // |    `    `| <- 3
                    // | 1   | 2  |
                    // |____,____,| <- 3
                    // 1 = light, 11 pixel, 1 pixel overlapping with 2
                    // 2 = dark, 11 pixel, 1 pixel overlapping with 3
                    // 3 = light edges
                    const int tileHeight = surfaceTile.height();
                    // 3
                    renderSurface(&surfacePainter,
                                    QRect(20, 0, 11, tileHeight),
                                    fg.light(105), fg, pal.highlight().color(), 2*(_contrast/3),
                                    reverseLayout ? Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperLeft|Round_BottomLeft|Is_Horizontal
                                    : Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperRight|Round_BottomRight|Is_Horizontal);
                    // 2
                    renderSurface(&surfacePainter,
                                    QRect(10, 0, 11, tileHeight),
                                    fg, fg.light(105), pal.highlight().color(), 2*(_contrast/3),
                                    reverseLayout ? Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperLeft|Round_BottomLeft|Is_Horizontal
                                    : Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperRight|Round_BottomRight|Is_Horizontal);
                    // 1
                    renderSurface(&surfacePainter,
                                    QRect(0, 0, 11, tileHeight),
                                    fg.light(105), fg, pal.highlight().color(), 2*(_contrast/3),
                                    reverseLayout ? Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperLeft|Round_BottomLeft|Is_Horizontal
                                    : Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                            Round_UpperRight|Round_BottomRight|Is_Horizontal);

                    surfacePainter.end();
                    int staticShift = 0;
                    int animShift = 0;
                    if (!_animateProgressBar) {
                        staticShift = (reverseLayout ? Rsurface.left() : Rsurface.right()) % 40 - 40;
                    } else {
                        // TODO
//                         // find the animation Offset for the current Widget
//                         QWidget* nonConstWidget = const_cast<QWidget*>(widget);
//                         QMapConstIterator<QWidget*, int> iter = progAnimWidgets.find(nonConstWidget);
//                         if (iter != progAnimWidgets.end())
//                             animShift = iter.data();
                        animShift = 0;
                    }
                    while((counter*10) < (Rsurface.width()+20)) {
                        counter++;
                        if (reverseLayout) {
                            // from right to left, overlap 1 pixel with the previously drawn tile
                            p->drawPixmap(Rsurface.right()-counter*20-animShift+40+staticShift, r.top()+1,
                                        surfaceTile);
                        } else {
                            // from left to right, overlap 1 pixel with the previously drawn tile
                            p->drawPixmap(Rsurface.left()+counter*20+animShift-40+staticShift, r.top()+1,
                                        surfaceTile);
                        }
                    }

                    p->setClipping(false);

                    return;
                }
            }
        }
        break;

        case WT_MenuBar:
        {
            // TODO
        }
        break;

        case WT_MenuBarItem:
        {
            switch (primitive)
            {
                case Generic::Bevel:
                {
                    bool active  = flags & State_Selected;
                    bool focused = flags & State_HasFocus;
                    bool down = flags & State_Sunken;

                    // TODO: set Background palette instead...
                    p->fillRect(r, pal.background().color() );

                    if (active && focused) {
                        renderButton(p, r, pal, down, mouseOver, true);
                    }

                    return;
                }
            }
        }
        break;

        case WT_Menu:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    renderContour(p, r, pal.background().color(), pal.background().color().dark(200),
                          Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
                    return;
                }

                case Menu::Background:
                {
                    p->fillRect( r, pal.background().color().light( 105 ) );
                    return;
                }

                case Menu::TearOff:
                {
                    // TODO: See Keramik...
                    return;
                }

                //TODO:scroller
            }
        }
        break;

        case WT_MenuItem:
        {
            switch (primitive)
            {
                case MenuItem::Separator:
                {
                    p->setPen( pal.mid().color() );
                    p->drawLine( r.x()+5, r.y() /*+ 1*/, r.right()-5, r.y() );
                    p->setPen( pal.light().color() );
                    p->drawLine( r.x()+5, r.y() + 1, r.right()-5 , r.y() + 1 );

                    return;
                }

                case MenuItem::ItemIndicator:
                {
                    if (enabled) {
                        renderSurface(p, r, pal.background().color(), pal.highlight().color(), pal.highlight().color(),
                                _contrast+3, Draw_Top|Draw_Bottom|Is_Horizontal);
                    }
                    else {
                        drawKStylePrimitive(WT_Generic, Generic::FocusIndicator, opt, r, pal, flags, p, widget, kOpt);
                    }

                    return;
                }

                case MenuItem::CheckColumn:
                {
                    // empty
                    return;
                }

                case MenuItem::CheckOn:
                {
                    renderCheckBox(p, r, pal, true /*TODO enabled*/, mouseOver, CheckBox::CheckOn);
                    return;
                }

                case MenuItem::CheckOff:
                {
                    renderCheckBox(p, r, pal, true /*TODO enabled*/, mouseOver, CheckBox::CheckOff);
                    return;
                }

                case MenuItem::RadioOn:
                {
                    renderRadioButton(p, r, pal, true /*TODO enabled*/, mouseOver, RadioButton::RadioOn);
                    return;
                }

                case MenuItem::RadioOff:
                {
                    renderRadioButton(p, r, pal, true /*TODO enabled*/, mouseOver, RadioButton::RadioOff);
                    return;
                }

                case MenuItem::CheckIcon:
                {
                    // TODO
                    renderButton(p, r, pal, true /*sunken*/);
                    return;
                }
            }
        }
        break;

        case WT_ScrollBar:
        {
            bool down = (flags & State_Sunken);

            switch (primitive)
            {
                case ScrollBar::SliderVert:
                case ScrollBar::SliderHor:
                {
                    bool horizontal = (primitive == ScrollBar::SliderHor);

                    const WidgetState s = enabled?(down?IsPressed:IsEnabled):IsDisabled;
                    const QColor surface = getColor(pal, DragButtonSurface, s);

                    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
                    if(!enabled) contourFlags|=Is_Disabled;
                    renderContour(p, r, pal.background().color(), getColor(pal, DragButtonContour, s),
                            contourFlags);

                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
                    if(horizontal) surfaceFlags|=Is_Horizontal;
                    if(!enabled) surfaceFlags|=Is_Disabled;
                    if(r.height() >= 4)
                        renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                                      pal.background().color(), surface, pal.background(),
                                      _contrast+3, surfaceFlags);

                    // set contour-like color for the case _scrollBarLines is set and we paint lines instead of dots.
                    p->setPen(alphaBlendColors(pal.background().color(), surface.dark(enabled?140:120), 50) );

                    const int d = 4;
                    int n = ((horizontal?r.width():r.height())-8)/d;
                    if(n>5) n=5;
                    if(!horizontal) {
                        for(int j = 0; j < n; j++) {
                            int yPos = r.center().y()-(n*d)/2+d*j+1;
                            if(_scrollBarLines)
                                p->drawLine(r.x()+1, yPos, r.right()-1, yPos);
                            else
                            {
                                for(int k = 3; k <= 13; k+=4) {
                                    renderDot(p, QPoint(k, yPos), surface, false, true );
                                }
                            }
                        }
                    } else {
                        for(int j = 0; j < n; j++) {
                            int xPos = r.center().x()-(n*d)/2+d*j+1;
                            if(_scrollBarLines)
                                p->drawLine(xPos, r.y()+1, xPos, r.bottom()-1);
                            else
                            {
                                for(int k = 3; k <= 13; k+=4) {
                                    renderDot(p, QPoint(xPos, k), surface, false, true );
                                }
                            }
                        }
                    }

                    return;
                }

                case ScrollBar::DoubleButtonHor:
                {
                    // TODO
                }
                break;

                case ScrollBar::DoubleButtonVert:
                {
                    // TODO
                }
                break;

                case ScrollBar::SingleButtonHor:
                {
                    bool down = flags&State_Sunken;

                    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;

                    if(down) surfaceFlags|=Is_Sunken;
                    if(!enabled) {
                        contourFlags|=Is_Disabled;
                        surfaceFlags|=Is_Disabled;
                    }

                    // TODO: round buttons. need to find out if it's an addPage or subPage button for that...
                    contourFlags |= /*Round_UpperLeft|Round_BottomLeft|*/Is_Horizontal;
                    surfaceFlags |= /*Round_UpperLeft|Round_BottomLeft|*/Is_Horizontal;

                    renderContour(p, r, pal.background(), getColor(pal, ButtonContour),
                            contourFlags);
                    renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                                  pal.background(), pal.button().color(), getColor(pal,MouseOverHighlight), _contrast+3,
                            surfaceFlags);
                    return;
                }
                break;

                case ScrollBar::SingleButtonVert:
                {
                    bool down = flags&State_Sunken;

                    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;

                    if(down) surfaceFlags|=Is_Sunken;
                    if(!enabled) {
                        contourFlags|=Is_Disabled;
                        surfaceFlags|=Is_Disabled;
                    }

                    // TODO: round buttons. need to find out if it's an addPage or subPage button for that...
//                     contourFlags |= Round_BottomLeft|Round_BottomRight;
//                     surfaceFlags |= Round_BottomLeft|Round_BottomRight;

                    renderContour(p, r, pal.background(), getColor(pal, ButtonContour),
                                  contourFlags);
                    renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                                  pal.background(), pal.button().color(), getColor(pal,MouseOverHighlight), _contrast+3,
                                  surfaceFlags);
                    return;
                }
                break;

                case ScrollBar::GrooveAreaVert:
                case ScrollBar::GrooveAreaHor:
                {
                    bool hor = primitive==ScrollBar::GrooveAreaHor;
                    bool on = flags&State_On;
                    bool down = flags&State_Sunken;

                    // TODO: double buffering still needed?
                    // draw double buffered to avoid flickr...
                    QPixmap buffer;
                    if(hor) {
                        buffer.resize(2, r.height() );
                    } else {
                        buffer.resize(r.width(), 2 );
                    }
                    QRect br(buffer.rect() );
                    QPainter bp(&buffer);

                    if (on || down) {
                        bp.fillRect(br, QBrush(pal.mid().color().dark()));
                    } else {
                        if(hor) {
                            bp.setPen(pal.background().color().dark(106));
                            bp.drawLine(br.left(), br.top(), br.right(), br.top());
                            bp.setPen(pal.background().color().light(106));
                            bp.drawLine(br.left(), br.bottom(), br.right(), br.bottom());
                            bp.fillRect(br.left(), br.top()+1, br.width(), br.height()-2,pal.background());
                        } else {
                            bp.setPen(pal.background().color().dark(106));
                            bp.drawLine(br.left(), br.top(), br.left(), br.bottom());
                            bp.setPen(pal.background().color().light(106));
                            bp.drawLine(br.right(), br.top(), br.right(), br.bottom());
                            bp.fillRect(br.left()+1, br.top(), br.width()-2, br.height(),pal.background());
                        }
                    }

                    bp.fillRect(br, QBrush(pal.background().color().light(), Qt::Dense4Pattern));

                    bp.end();

                    p->drawTiledPixmap(r, buffer, QPoint(0, r.top()%2));

                    return;
                }
            }

        }
        break;

        case WT_TabBar:
        {
            const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(opt);

            switch (primitive)
            {
                case TabBar::NorthTab:
                case TabBar::SouthTab:
                {
                    // TODO: check if tabOpt != 0...
                    QStyleOptionTab::TabPosition pos = tabOpt->position;

                    // TODO: tab painting needs a lot of work in order to handle east and west tabs.
                    // TODO: handle triangular...
                    // TODO: either overlap tabs 1 pixel, or adapt painting.
                    renderTab(p, r, pal, mouseOver, flags&State_Selected, false, pos, true /*triangular*/, false/*cornerWidget*/, reverseLayout);

                    return;

                }

                // TODO: TabBar::EastTab, TabBar::WestTab, TabBar::BaseFrame, TabBar::ScrollButton

            }

        }
        break;

        case WT_TabWidget:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    renderPanel(p, r, pal, true, flags&State_Sunken);
                    return;
                }

            }

        }
        break;

        case WT_Slider:
        {
            switch (primitive)
            {
                case Slider::HandleHor:
                case Slider::HandleVert:
                {

                    bool horizontal = primitive == Slider::HandleHor;

                    const bool pressed = (flags&State_Sunken);
                    const WidgetState s = enabled?(pressed?IsPressed:IsEnabled):IsDisabled;
                    const QColor contour = getColor(pal,DragButtonContour,s),
                                surface = getColor(pal,DragButtonSurface,s);

                    int xcenter = (r.left()+r.right()) / 2;
                    int ycenter = (r.top()+r.bottom()) / 2;

                    if (horizontal) {
                        renderContour(p, QRect(xcenter-5, ycenter-6, 11, 10),
                                      pal.background().color(), contour,
                                    Draw_Left|Draw_Right|Draw_Top|Round_UpperLeft|Round_UpperRight);

                        // manual contour: vertex
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 50) );
                        p->drawPoint(xcenter-5+1, ycenter+4);
                        p->drawPoint(xcenter+5-1, ycenter+4);
                        p->drawPoint(xcenter-5+2, ycenter+5);
                        p->drawPoint(xcenter+5-2, ycenter+5);
                        p->drawPoint(xcenter-5+3, ycenter+6);
                        p->drawPoint(xcenter+5-3, ycenter+6);
                        p->drawPoint(xcenter-5+4, ycenter+7);
                        p->drawPoint(xcenter+5-4, ycenter+7);
                        // anti-aliasing of the contour... sort of. :)
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 80) );
                        p->drawPoint(xcenter, ycenter+8);
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 150) );
                        p->drawPoint(xcenter-5, ycenter+4);
                        p->drawPoint(xcenter+5, ycenter+4);
                        p->drawPoint(xcenter-5+1, ycenter+5);
                        p->drawPoint(xcenter+5-1, ycenter+5);
                        p->drawPoint(xcenter-5+2, ycenter+6);
                        p->drawPoint(xcenter+5-2, ycenter+6);
                        p->drawPoint(xcenter-5+3, ycenter+7);
                        p->drawPoint(xcenter+5-3, ycenter+7);
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 190) );
                        p->drawPoint(xcenter-5+4, ycenter+8);
                        p->drawPoint(xcenter+5-4, ycenter+8);


                        QRegion mask(xcenter-4, ycenter-5, 9, 13);
                        mask -= QRegion(xcenter-4, ycenter+4, 1, 4);
                        mask -= QRegion(xcenter-3, ycenter+5, 1, 3);
                        mask -= QRegion(xcenter-2, ycenter+6, 1, 2);
                        mask -= QRegion(xcenter-1, ycenter+7, 1, 1);
                        mask -= QRegion(xcenter+1, ycenter+7, 1, 1);
                        mask -= QRegion(xcenter+2, ycenter+6, 1, 2);
                        mask -= QRegion(xcenter+3, ycenter+5, 1, 3);
                        mask -= QRegion(xcenter+4, ycenter+4, 1, 4);
                        p->setClipRegion(mask);
                        uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Round_UpperLeft|Round_UpperRight|Is_Horizontal;
                        if(!enabled)
                            surfaceFlags |= Is_Disabled;
                        renderSurface(p, QRect(xcenter-4, ycenter-5, 9, 13),
                                      pal.background().color(), surface, getColor(pal,MouseOverHighlight),
                                    _contrast+3, surfaceFlags);
                        renderDot(p, QPoint(xcenter-3, ycenter-3), surface, false, true );
                        renderDot(p, QPoint(xcenter+2,   ycenter-3), surface, false, true );
                        p->setClipping(false);
                    } else {
                        renderContour(p, QRect(xcenter-6, ycenter-5, 10, 11),
                                      pal.background().color(), contour,
                                    Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft);

                        // manual contour: vertex
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 50) );
                        p->drawPoint(xcenter+4, ycenter-5+1);
                        p->drawPoint(xcenter+4, ycenter+5-1);
                        p->drawPoint(xcenter+5, ycenter-5+2);
                        p->drawPoint(xcenter+5, ycenter+5-2);
                        p->drawPoint(xcenter+6, ycenter-5+3);
                        p->drawPoint(xcenter+6, ycenter+5-3);
                        p->drawPoint(xcenter+7, ycenter-5+4);
                        p->drawPoint(xcenter+7, ycenter+5-4);
                        // anti-aliasing. ...sort of :)
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 80) );
                        p->drawPoint(xcenter+8, ycenter);
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 150) );
                        p->drawPoint(xcenter+4, ycenter-5);
                        p->drawPoint(xcenter+4, ycenter+5);
                        p->drawPoint(xcenter+5, ycenter-5+1);
                        p->drawPoint(xcenter+5, ycenter+5-1);
                        p->drawPoint(xcenter+6, ycenter-5+2);
                        p->drawPoint(xcenter+6, ycenter+5-2);
                        p->drawPoint(xcenter+7, ycenter-5+3);
                        p->drawPoint(xcenter+7, ycenter+5-3);
                        p->setPen(alphaBlendColors(pal.background().color(), contour, 190) );
                        p->drawPoint(xcenter+8, ycenter-5+4);
                        p->drawPoint(xcenter+8, ycenter+5-4);

                        QRegion mask(xcenter-5, ycenter-4, 13, 9);
                        mask -= QRegion(xcenter+4, ycenter-4, 4, 1);
                        mask -= QRegion(xcenter+5, ycenter-3, 3, 1);
                        mask -= QRegion(xcenter+6, ycenter-2, 2, 1);
                        mask -= QRegion(xcenter+7, ycenter-1, 1, 1);
                        mask -= QRegion(xcenter+7, ycenter+1, 1, 1);
                        mask -= QRegion(xcenter+6, ycenter+2, 2, 1);
                        mask -= QRegion(xcenter+5, ycenter+3, 3, 1);
                        mask -= QRegion(xcenter+4, ycenter+4, 4, 1);
                        p->setClipRegion(mask);
                        uint surfaceFlags = Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft|
                                        Round_UpperRight|Is_Horizontal;
                        if(!enabled)
                            surfaceFlags |= Is_Disabled;
                        renderSurface(p, QRect(xcenter-5, ycenter-4, 13, 9),
                                      pal.background().color(), surface, getColor(pal,MouseOverHighlight),
                                    _contrast+3, surfaceFlags);
                        renderDot(p, QPoint(xcenter-3, ycenter-3), surface, false, true );
                        renderDot(p, QPoint(xcenter-3,   ycenter+2), surface, false, true );
                        p->setClipping(false);
                    }

                    return;
                }

                case Slider::GrooveHor:
                case Slider::GrooveVert:
                {

                    bool horizontal = primitive == Slider::GrooveHor;

                    if (horizontal) {
                        int center = r.y()+r.height()/2;
                        renderContour(p, QRect(r.left(), center-2, r.width(), 4),
                                      pal.background(), pal.background().color().dark(enabled?150:130),
                                    Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
                    } else {
                        int center = r.x()+r.width()/2;
                        renderContour(p, QRect(center-2, r.top(), 4, r.height()),
                                      pal.background(), pal.background().color().dark(enabled?150:130),
                                    Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
                    }

                    return;
                }
            }

        }
        break;

        case WT_SpinBox:
        {
//             const Q3SpinWidget *sw = dynamic_cast<const Q3SpinWidget *>(widget);
//             SFlags sflags = flags;
//             PrimitiveElement pe;

            bool hasFocus = flags & State_HasFocus;

            const QColor buttonColor = enabled?pal.button().color():pal.background().color();
            const QColor inputColor = enabled?pal.base().color():pal.background().color();

            switch (primitive)
            {
                case Generic::Frame:
                {
                    QRect editField = subControlRect(CC_SpinBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_SpinBoxEditField, widget);

                    // contour
                    const bool heightDividable = ((r.height()%2) == 0);
                    if (_inputFocusHighlight && hasFocus && enabled)
                    {
                        QRect editFrame = r;
                        QRect buttonFrame = r;

                        uint editFlags = 0;
                        uint buttonFlags = 0;

                        // Hightlight only the part of the contour next to the control buttons
                        if (reverseLayout)
                        {
                            // querySubControlMetrics doesn't work right for reverse Layout
                            int dx = r.right() - editField.right();
                            editFrame.setLeft(editFrame.left() + dx);
                            buttonFrame.setRight(editFrame.left() - 1);
                            editFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight;
                            buttonFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft;
                        }
                        else
                        {
                            editFrame.setRight(editField.right());
                            buttonFrame.setLeft(editField.right() + 1);

                            editFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft;
                            buttonFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight;
                        }
                        renderContour(p, editFrame, pal.background().color(), pal.highlight().color(), editFlags);
                        renderContour(p, buttonFrame, pal.background().color(),
                                    getColor(pal, ButtonContour, enabled), buttonFlags);
                    }
                    else
                    {
                        renderContour(p, subControlRect(CC_SpinBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_SpinBoxFrame, widget),
                                      pal.background().color(), getColor(pal, ButtonContour, enabled) );
                    }
                    p->setPen(alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour, enabled), 50) );
                    p->drawLine(reverseLayout?editField.left()-2:editField.right()+2, r.top()+1,
                                reverseLayout?editField.left()-2:editField.right()+2, r.bottom()-1);
                    p->drawLine(reverseLayout?r.left()+1:editField.right()+2+1, r.top()+1+(r.height()-2)/2,
                                reverseLayout?editField.right()-2-1:r.right()-1, r.top()+1+(r.height()-2)/2);
                    if(heightDividable)
                        p->drawLine(reverseLayout?r.left()+1:editField.right()+2+1, r.top()+1+(r.height()-2)/2-1,
                                    reverseLayout?editField.right()-2-1:r.right()-1, r.top()+1+(r.height()-2)/2-1);

                    // thin frame around the input area
                    const QRect Rcontent = subControlRect(CC_SpinBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_SpinBoxEditField, widget).adjusted(-1,-1,1,1);
                    if (_inputFocusHighlight && hasFocus && enabled)
                    {
                        p->setPen( getColor(pal,FocusHighlight).dark(130) );
                    }
                    else
                    {
                        p->setPen(inputColor.dark(130) );
                    }
                    p->drawLine(Rcontent.left(), reverseLayout?Rcontent.top():Rcontent.top()+1,
                            Rcontent.left(), reverseLayout?Rcontent.bottom():Rcontent.bottom()-1 );
                    p->drawLine(Rcontent.left()+1, Rcontent.top(),
                            reverseLayout?Rcontent.right()-1:Rcontent.right(), Rcontent.top() );
                    if (_inputFocusHighlight && hasFocus && enabled)
                    {
                        p->setPen( getColor(pal,FocusHighlight).light(130) );
                    }
                    else
                    {
                        p->setPen(inputColor.light(130) );
                    }
                    p->drawLine(Rcontent.left()+1, Rcontent.bottom(), Rcontent.right()-1, Rcontent.bottom() );
                    p->drawLine(Rcontent.right(), Rcontent.top()+1,
                            Rcontent.right(), reverseLayout?Rcontent.bottom()-1:Rcontent.bottom() );

                    return;
                }

                case SpinBox::UpButton:
                {
                    QRect upRect = subControlRect(CC_SpinBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_SpinBoxUp, widget);

                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                    if(reverseLayout) {
                        surfaceFlags |= Round_UpperLeft;
                    } else {
                        surfaceFlags |= Round_UpperRight;
                    }
                    if (mouseOver) {
                        surfaceFlags |= Is_Highlight;
                        surfaceFlags |= Highlight_Top|Highlight_Left|Highlight_Right;
                    }
                    if (flags & State_Sunken) surfaceFlags|=Is_Sunken;
                    if(!enabled) surfaceFlags|=Is_Disabled;
                    renderSurface(p, upRect, pal.background().color(), buttonColor, getColor(pal,MouseOverHighlight),
                                _contrast, surfaceFlags);

                    return;
                }

                case SpinBox::DownButton:
                {
                    QRect downRect = subControlRect(CC_SpinBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_SpinBoxDown, widget);

                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                    if(reverseLayout) {
                        surfaceFlags |= Round_BottomLeft;
                    } else {
                        surfaceFlags |= Round_BottomRight;
                    }
                    if (mouseOver) {
                        surfaceFlags |= Is_Highlight;
                        surfaceFlags |= Highlight_Bottom|Highlight_Left|Highlight_Right;
                    }
                    if (flags & State_Sunken) surfaceFlags|=Is_Sunken;
                    if(!enabled) surfaceFlags|=Is_Disabled;
                    renderSurface(p, downRect, pal.background().color(), buttonColor, getColor(pal,MouseOverHighlight),
                                  _contrast, surfaceFlags);

                    return;
                }
            }

        }
        break;

        case WT_ComboBox:
        {
            bool editable = false;
            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt) )
                editable = cb->editable;

            bool hasFocus = flags & State_HasFocus;

            const QColor buttonColor = enabled?pal.button().color():pal.background().color();
            const QColor inputColor = enabled?(editable?pal.base().color():pal.button().color() ):pal.background().color();
            QRect editField = subControlRect(CC_ComboBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_ComboBoxEditField, widget);

            switch (primitive)
            {
                case Generic::Frame:
                {
                    // TODO: pressed state

                    uint contourFlags = 0;
//                     if( khtmlWidgets.contains(cb) )
//                         contourFlags |= Draw_AlphaBlend;

                    if (_inputFocusHighlight && hasFocus && editable && enabled)
                    {
                        QRect editFrame = r;
                        QRect buttonFrame = r;

                        uint editFlags = contourFlags;
                        uint buttonFlags = contourFlags;

                        // Hightlight only the part of the contour next to the control button
                        if (reverseLayout)
                        {
                            // querySubControlMetrics doesn't work right for reverse Layout
                            int dx = r.right() - editField.right();
                            editFrame.setLeft(editFrame.left() + dx);
                            buttonFrame.setRight(editFrame.left() - 1);
                            editFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight;
                            buttonFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft;
                        }
                        else
                        {
                            editFrame.setRight(editField.right());
                            buttonFrame.setLeft(editField.right() + 1);

                            editFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft;
                            buttonFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight;
                        }
                        renderContour(p, editFrame, pal.background().color(),  getColor(pal,FocusHighlight,enabled), editFlags);
                        renderContour(p, buttonFrame, pal.background().color(),
                                    getColor(pal, ButtonContour, enabled), buttonFlags);
                    }
                    else
                    {
                        contourFlags |= Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                            Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
                        renderContour(p, r, pal.background().color(), getColor(pal, ButtonContour, enabled), contourFlags);
                    }
                    //extend the contour: between input and handler...
                    p->setPen(alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour, enabled), 50) );
                    if(reverseLayout) {
                        p->drawLine(editField.left()-2, r.top()+1, editField.left()-2, r.bottom()-1);
                    } else {
                        p->drawLine(editField.right()+2, r.top()+1, editField.right()+2, r.bottom()-1);
                    }

                    QRect Rsurface = editField.adjusted(-1,-1,1,1);
                    if(!editable) {
                        int surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                        if(reverseLayout) {
                            surfaceFlags |= Round_UpperRight|Round_BottomRight;
                        } else {
                            surfaceFlags |= Round_UpperLeft|Round_BottomLeft;
                        }

                        if (mouseOver) {
                            surfaceFlags |= Is_Highlight;
                            surfaceFlags |= Highlight_Top|Highlight_Bottom;
                        }
                        renderSurface(p, Rsurface,
                                    pal.background().color(), buttonColor, getColor(pal,MouseOverHighlight), enabled?_contrast+3:(_contrast/2),
                                    surfaceFlags);
                    } else {
                        // thin frame around the input area
                        if (_inputFocusHighlight && hasFocus && editable && enabled)
                        {
                            p->setPen( getColor(pal,FocusHighlight).dark(130) );
                        }
                        else
                        {
                            p->setPen(inputColor.dark(130) );
                        }
                        p->drawLine(Rsurface.x(), reverseLayout?Rsurface.y():Rsurface.y()+1,
                                Rsurface.x(), reverseLayout?Rsurface.bottom():Rsurface.bottom()-1);
                        p->drawLine(Rsurface.x()+1, Rsurface.y(),
                                reverseLayout?Rsurface.right()-1:Rsurface.right(), Rsurface.y() );
                        if (_inputFocusHighlight && hasFocus && editable && enabled)
                        {
                            p->setPen( getColor(pal,FocusHighlight).light(130) );
                        }
                        else
                        {
                            p->setPen(inputColor.light(130) );
                        }
                        p->drawLine(reverseLayout?Rsurface.x():Rsurface.x()+1, Rsurface.bottom(),
                                reverseLayout?Rsurface.right()-1:Rsurface.right(), Rsurface.bottom() );
                        p->drawLine(Rsurface.right(), Rsurface.top()+1,
                                Rsurface.right(), Rsurface.bottom()-1 );

                        // input area
                        p->fillRect(editField, inputColor );
                    }

                    return;
                }

                case ComboBox::EditField:
                {
                    // empty
                    return;
                }

                case ComboBox::Button:
                {
                    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                    if(reverseLayout) {
                        surfaceFlags |= Round_UpperLeft|Round_BottomLeft;
                    } else {
                        surfaceFlags |= Round_UpperRight|Round_BottomRight;
                    }

                    if (mouseOver) {
                        surfaceFlags |= Is_Highlight;
                        if(editable) surfaceFlags |= Highlight_Left|Highlight_Right;
                        surfaceFlags |= Highlight_Top|Highlight_Bottom;
                    }
                    renderSurface(p, r,
                                pal.background().color(), buttonColor, getColor(pal,MouseOverHighlight), enabled?_contrast+3:(_contrast/2),
                                surfaceFlags);

                    return;
                }
            }

        }
        break;

        case WT_Header:
        {
            switch (primitive)
            {
                case Header::SectionHor:
                case Header::SectionVert:
                {
                    if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
                        bool isFirst = (primitive==Header::SectionHor)&&(header->position == QStyleOptionHeader::Beginning);

                        uint contourFlags = Draw_Right|Draw_Top|Draw_Bottom;
                        if (isFirst)
                            contourFlags |= Draw_Left;
                        if(!enabled) contourFlags|=Is_Disabled;
                        renderContour(p, r, pal.background().color(), getColor(pal,ButtonContour),
                                        contourFlags);

                        uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                        if(!enabled) surfaceFlags|=Is_Disabled;
                        else {
                            if(flags&State_On || flags&State_Sunken) surfaceFlags|=Is_Sunken;
                            else {
                                if(mouseOver) {
                                    surfaceFlags|=Is_Highlight|Highlight_Top|Highlight_Bottom;
                                }
                            }
                        }
                        renderSurface(p, QRect(isFirst?r.left()+1:r.left(), r.top()+1, isFirst?r.width()-2:r.width()-1, r.height()-2),
                                    pal.background().color(), pal.button().color(), getColor(pal,MouseOverHighlight), _contrast,
                                        surfaceFlags);
                    }

                    return;
                }
            }
        }
        break;

        case WT_Tree:
        {
            switch (primitive)
            {
                case Tree::VerticalBranch:
                case Tree::HorizontalBranch:
                {
                //### FIXME: set sane color.
                    QBrush brush(Qt::Dense4Pattern);
                    brush.setColor(pal.mid().color() );
                    p->fillRect(r, brush);
                    return;
                }
                case Tree::ExpanderOpen:
                case Tree::ExpanderClosed:
                {
                    int radius = (r.width() - 4) / 2;
                    int centerx = r.x() + r.width()/2;
                    int centery = r.y() + r.height()/2;

                    renderContour(p, r, pal.base().color(), pal.dark().color(), Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight );

                    p->setPen( pal.text().color() );
                    if(!_drawTriangularExpander)
                    {
                        // plus or minus
                        p->drawLine( centerx - radius, centery, centerx + radius, centery );
                        if (primitive == Tree::ExpanderClosed) // Collapsed = On
                            p->drawLine( centerx, centery - radius, centerx, centery + radius );
                    } else {
                        if(primitive == Tree::ExpanderClosed)
                            drawKStylePrimitive(WT_Generic, Generic::ArrowRight, opt, QRect(r.x()+1,r.y()+1,r.width(),r.height()), pal, flags, p, widget);
                        else
                            drawKStylePrimitive(WT_Generic, Generic::ArrowDown, opt, QRect(r.x()+1,r.y()+1,r.width(),r.height()), pal, flags, p, widget);
                    }

                    return;
                }
                default:
                    break;
            }
        }
        break;

        case WT_LineEdit:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    bool isReadOnly = false;
                    bool isEnabled = flags & State_Enabled;
                    bool hasFocus = flags & State_HasFocus;

// TODO: fixme...
//                     // panel is highlighted by default if it has focus, but if we have access to the
//                     // widget itself we can try to avoid highlighting in case it's readOnly or disabled.
//                     if (p->device() && dynamic_cast<QLineEdit*>(p->device()))
//                     {
//                         QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(p->device());
//                         isReadOnly = lineEdit->isReadOnly();
//                     }

                    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                            Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;

                    if ( _inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
                    {
                        renderContour(p, r, pal.background().color(),
                                    getColor(pal,FocusHighlight,enabled), contourFlags );
                    }
                    else
                    {
                        renderContour(p, r, pal.background().color(),
                                    getColor(pal, ButtonContour, enabled), contourFlags );
                    }
                    const QColor contentColor = enabled?pal.base().color():pal.background().color();
                    if (_inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
                    {
                        p->setPen( getColor(pal,FocusHighlight).dark(130) );
                    }
                    else
                    {
                        p->setPen(contentColor.dark(130) );
                    }
                    p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2 );
                    p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
                    if (_inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
                    {
                        p->setPen( getColor(pal,FocusHighlight).light(130) );
                    }
                    else
                    {
                    p->setPen(contentColor.light(130) );
                    }
                    p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
                    p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2 );

                    return;
                }

                case LineEdit::Panel:
                {
                    if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame*>(opt))
                    {
                        const int lineWidth(panel->lineWidth);
                        p->fillRect(r.adjusted(lineWidth, lineWidth, -lineWidth, -lineWidth),
                                    pal.brush(QPalette::Base));

                        if (lineWidth > 0)
                            drawPrimitive(PE_FrameLineEdit, panel, p, widget);
                    }
                }
            }

        }
        break;

        case WT_GroupBox:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    renderPanel(p, r, pal, false);

                    return;
                }
            }

        }
        break;

        case WT_ToolBar:
        {
            switch (primitive)
            {
                case ToolBar::Handle:
                {
                    int counter = 1;

                    if(flags & State_Horizontal) {
                        int center = r.left()+r.width()/2;
                        for(int j = r.top()+2; j <= r.bottom()-3; j+=3) {
                            if(counter%2 == 0) {
                                renderDot(p, QPoint(center+1, j), pal.background().color(), true, true);
                            } else {
                                renderDot(p, QPoint(center-2, j), pal.background().color(), true, true);
                            }
                            counter++;
                        }
                    } else {
                        int center = r.top()+r.height()/2;
                        for(int j = r.left()+2; j <= r.right()-3; j+=3) {
                            if(counter%2 == 0) {
                                renderDot(p, QPoint(j, center+1), pal.background().color(), true, true);
                            } else {
                                renderDot(p, QPoint(j, center-2), pal.background().color(), true, true);
                            }
                            counter++;
                        }
                    }

                    return;
                }

                case ToolBar::Separator:
                {
                    if(_drawToolBarItemSeparator) {
                        if(flags & State_Horizontal) {
                            int center = r.left()+r.width()/2;
                            p->setPen( getColor(pal, PanelDark) );
                            p->drawLine( center-1, r.top()+3, center-1, r.bottom()-3 );
                            p->setPen( getColor(pal, PanelLight) );
                            p->drawLine( center, r.top()+3, center, r.bottom()-3 );
                        } else {
                            int center = r.top()+r.height()/2;
                            p->setPen( getColor(pal, PanelDark) );
                            p->drawLine( r.x()+3, center-1, r.right()-3, center-1 );
                            p->setPen( getColor(pal, PanelLight) );
                            p->drawLine( r.x()+3, center, r.right()-3, center );
                        }
                    }

                    return;
                }

                case ToolBar::Panel:
                {
                    return;
                }
            }
        }
        break;

        case WT_ToolButton:
        {
            switch (primitive)
            {
                case Generic::Bevel:
                {
                    if (flags&State_Enabled)
                        renderButton(p, r, pal, flags&State_Sunken||flags&State_On);

                    return;
                }
            }
        }
        break;
    }


    // Arrows
    if (primitive >= Generic::ArrowUp && primitive <= Generic::ArrowLeft) {
        QPolygon a;

        switch (primitive) {
            case Generic::ArrowUp: {
                a.setPoints(7, u_arrow);
                break;
            }
            case Generic::ArrowDown: {
                a.setPoints(7, d_arrow);
                break;
            }
            case Generic::ArrowLeft: {
                a.setPoints(7, l_arrow);
                break;
            }
            case Generic::ArrowRight: {
                a.setPoints(7, r_arrow);
                break;
            }
//                         default: {
//                             if (flags & Style_Up) {
//                                 a.setPoints(7, u_arrow);
//                             } else {
//                                 a.setPoints(7, d_arrow);
//                             }
//                         }
        }

                const QMatrix oldMatrix( p->worldMatrix() );

//                     if (flags & Style_Down) {
//                         p->translate(pixelMetric(PM_ButtonShiftHorizontal),
//                                         pixelMetric(PM_ButtonShiftVertical));
//                     }

        a.translate((r.x()+r.width()/2), (r.y()+r.height()/2));
//                     // extra-pixel-shift, correcting some visual tics...
//                     switch(pe) {
//                         case Generic::ArrowLeft:
//                         case Generic::ArrowRight:
//                             a.translate(0, -1);
//                             break;
//                         case PE_SpinWidgetUp:
//                         case PE_SpinWidgetDown:
//                             a.translate(+1, 0);
//                             break;
//                         default:
//                             a.translate(0, 0);
//                     }

        KStyle::ColorOption* colorOpt   = extractOption<KStyle::ColorOption*>(kOpt);
        QColor               arrowColor = colorOpt->color.color(pal);

        p->setPen(arrowColor);

        p->drawLineSegments(a, 0, 3);
        p->drawPoint(a[6]);

        p->setWorldMatrix( oldMatrix );

        return;
    }

    // default fallback
    KStyle::drawKStylePrimitive(widgetType, primitive, opt,
                                r, pal, flags, p, widget, kOpt);
}

// 
// void PlastikStyle::polish(QApplication* app)
// {
//     if (!qstrcmp(app->argv()[0], "kicker"))
//         kickerMode = true;
//     else if (!qstrcmp(app->argv()[0], "korn"))
//         kornMode = true;
// }
// 
void PlastikStyle::polish(QWidget* widget)
{
//     if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
//         khtmlWidgets[widget] = true;
//         connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
//     }
// 
//     // use qt_cast where possible to check if the widget inheits one of the classes. might improve
//     // performance compared to QObject::inherits()
//     if ( qobject_cast<QPushButton>(widget) || qobject_cast<QComboBox>(widget) ||
//             qobject_cast<Q3SpinWidget>(widget) || qobject_cast<QSlider>(widget) ||
//             qobject_cast<QCheckBox>(widget) || qobject_cast<QRadioButton>(widget) ||
//             qobject_cast<QToolButton>(widget) || widget->inherits("QSplitterHandle") )
//     {
// //         widget->setBackgroundMode(PaletteBackground);
//         widget->installEventFilter(this);
//     } else if (qobject_cast<QLineEdit>(widget)) {
//         widget->installEventFilter(this);
//     } else if (qobject_cast<QTabBar>(widget)) {
//         widget->setMouseTracking(true);
//         widget->installEventFilter(this);
//     } else if (qobject_cast<Q3PopupMenu>(widget)) {
//         widget->setBackgroundMode( Qt::NoBackground );
//     } else if ( !qstrcmp(widget->name(), "kde toolbar widget") ) {
//         widget->installEventFilter(this);
//     }
// 
//     if( _animateProgressBar && qobject_cast<Q3ProgressBar>(widget) )
//     {
//         widget->installEventFilter(this);
//         progAnimWidgets[widget] = 0;
//         connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
//         if (!animationTimer->isActive())
//             animationTimer->start( 50, false );
//     }
// 
    if (qobject_cast<QPushButton*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QCheckBox*>(widget)
        || qobject_cast<QRadioButton*>(widget)
        || qobject_cast<QSplitterHandle*>(widget)
        || qobject_cast<QTabBar*>(widget)
        ) {
        widget->setAttribute(Qt::WA_Hover);
    }

    KStyle::polish(widget);
}

void PlastikStyle::unPolish(QWidget* widget)
{
//     if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
//         khtmlWidgets.remove(widget);
//     }
// 
//     // use qt_cast to check if the widget inheits one of the classes.
//     if ( qobject_cast<QPushButton>(widget) || qobject_cast<QComboBox>(widget) ||
//             qobject_cast<Q3SpinWidget>(widget) || qobject_cast<QSlider>(widget) ||
//             qobject_cast<QCheckBox>(widget) || qobject_cast<QRadioButton>(widget) ||
//             qobject_cast<QToolButton>(widget) || qobject_cast<QLineEdit>(widget) ||
//             widget->inherits("QSplitterHandle") )
//     {
//         widget->removeEventFilter(this);
//     }
//     else if (qobject_cast<QTabBar>(widget)) {
//         widget->setMouseTracking(false);
//         widget->removeEventFilter(this);
//     } else if (qobject_cast<Q3PopupMenu>(widget)) {
//         widget->setBackgroundMode( Qt::PaletteBackground );
//     } else if ( !qstrcmp(widget->name(), "kde toolbar widget") ) {
//         widget->removeEventFilter(this);
//     }
// 
//     if ( qobject_cast<Q3ProgressBar>(widget) )
//     {
//         progAnimWidgets.remove(widget);
//     }

    if (qobject_cast<QPushButton*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QCheckBox*>(widget)
        || qobject_cast<QSplitterHandle*>(widget)
        || qobject_cast<QRadioButton*>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

//     KStyle::unPolish(widget);
}
// 
// void PlastikStyle::khtmlWidgetDestroyed(QObject* obj)
// {
//     khtmlWidgets.remove(static_cast<QWidget*>(obj));
// }
// 
// void PlastikStyle::progressBarDestroyed(QObject* obj)
// {
//     progAnimWidgets.remove(static_cast<QWidget*>(obj));
// }
// 
void PlastikStyle::renderContour(QPainter *p,
                                const QRect &r,
                                const QColor &backgroundColor,
                                const QColor &contour,
                                const uint flags) const
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    const bool drawLeft = flags&Draw_Left;
    const bool drawRight = flags&Draw_Right;
    const bool drawTop = flags&Draw_Top;
    const bool drawBottom = flags&Draw_Bottom;
    const bool disabled = flags&Is_Disabled;
    const bool alphaBlend = flags&Draw_AlphaBlend;

    QColor contourColor;
    if (disabled) {
        contourColor = backgroundColor.dark(150);
    } else {
        contourColor = contour;
    }

// sides
    p->setPen( alphaBlendColors(backgroundColor, contourColor, 50) );
    if(drawLeft)
        p->drawLine(r.left(), drawTop?r.top()+2:r.top(), r.left(), drawBottom?r.bottom()-2:r.bottom());
    if(drawRight)
        p->drawLine(r.right(), drawTop?r.top()+2:r.top(), r.right(), drawBottom?r.bottom()-2:r.bottom());
    if(drawTop)
        p->drawLine(drawLeft?r.left()+2:r.left(), r.top(), drawRight?r.right()-2:r.right(), r.top());
    if(drawBottom)
        p->drawLine(drawLeft?r.left()+2:r.left(), r.bottom(), drawRight?r.right()-2:r.right(), r.bottom());

// edges
    const int alphaAA = 110; // the alpha value for anti-aliasing...

    // first part...
    p->setPen(alphaBlendColors(backgroundColor, contourColor, 50) );
    if(drawLeft && drawTop) {
        switch(flags&Round_UpperLeft) {
            case false:
                p->drawPoint(r.left()+1, r.top());
                p->drawPoint(r.left(), r.top()+1);
                break;
            default:
                p->drawPoint(r.left()+1, r.top()+1);
        }
    }
    if(drawLeft && drawBottom) {
        switch(flags&Round_BottomLeft) {
            case false:
                p->drawPoint(r.left()+1, r.bottom());
                p->drawPoint(r.left(), r.bottom()-1);
                break;
            default:
                p->drawPoint(r.left()+1, r.bottom()-1);
        }
    }
    if(drawRight && drawTop) {
        switch(flags&Round_UpperRight) {
            case false:
                p->drawPoint(r.right()-1, r.top());
                p->drawPoint(r.right(), r.top()+1);
                break;
            default:
                p->drawPoint(r.right()-1, r.top()+1);
        }
    }
    if(drawRight && drawBottom) {
        switch(flags&Round_BottomRight) {
            case false:
                p->drawPoint(r.right()-1, r.bottom());
                p->drawPoint(r.right(), r.bottom()-1);
                break;
            default:
                p->drawPoint(r.right()-1, r.bottom()-1);
        }
    }

    // second part... fill edges in case we don't paint alpha-blended
    p->setPen( backgroundColor );
    if (!alphaBlend) {
        if(flags&Round_UpperLeft && drawLeft && drawTop) {
            p->drawPoint( r.x(), r.y() );
        }
        if(flags&Round_BottomLeft && drawLeft && drawBottom) {
            p->drawPoint( r.x(), r.bottom() );
        }
        if(flags&Round_UpperRight && drawRight && drawTop) {
            p->drawPoint( r.right(), r.y() );
        }
        if(flags&Round_BottomRight && drawRight && drawBottom) {
            p->drawPoint( r.right(), r.bottom() );
        }
    }

    // third part... anti-aliasing...
    if(drawLeft && drawTop) {
        switch(flags&Round_UpperLeft) {
            case false:
                renderPixel(p,QPoint(r.left(),r.top()),alphaAA,contourColor,backgroundColor,alphaBlend);
                break;
            default:
                renderPixel(p,QPoint(r.left()+1,r.top()),alphaAA,contourColor,backgroundColor,alphaBlend);
                renderPixel(p,QPoint(r.left(),r.top()+1),alphaAA,contourColor,backgroundColor,alphaBlend);
        }
    }
    if(drawLeft && drawBottom) {
        switch(flags&Round_BottomLeft) {
            case false:
                renderPixel(p,QPoint(r.left(),r.bottom()),alphaAA,contourColor,backgroundColor,alphaBlend);
                break;
            default:
                renderPixel(p,QPoint(r.left()+1,r.bottom()),alphaAA,contourColor,backgroundColor,alphaBlend);
                renderPixel(p,QPoint(r.left(),r.bottom()-1),alphaAA,contourColor,backgroundColor,alphaBlend);
        }
    }
    if(drawRight && drawTop) {
        switch(flags&Round_UpperRight) {
            case false:
                renderPixel(p,QPoint(r.right(),r.top()),alphaAA,contourColor,backgroundColor,alphaBlend);
                break;
            default:
                renderPixel(p,QPoint(r.right()-1,r.top()),alphaAA,contourColor,backgroundColor,alphaBlend);
                renderPixel(p,QPoint(r.right(),r.top()+1),alphaAA,contourColor,backgroundColor,alphaBlend);
        }
    }
    if(drawRight && drawBottom) {
        switch(flags&Round_BottomRight) {
            case false:
                renderPixel(p,QPoint(r.right(),r.bottom()),alphaAA,contourColor,backgroundColor,alphaBlend);
                break;
            default:
                renderPixel(p,QPoint(r.right()-1,r.bottom()),alphaAA,contourColor,backgroundColor,alphaBlend);
                renderPixel(p,QPoint(r.right(),r.bottom()-1),alphaAA,contourColor,backgroundColor,alphaBlend);
        }
    }

}
// 
// void PlastikStyle::renderMask(QPainter *p,
//                               const QRect &r,
//                               const QColor &color,
//                               const uint flags) const
// {
//     if((r.width() <= 0)||(r.height() <= 0))
//         return;
// 
//     const bool roundUpperLeft = flags&Round_UpperLeft;
//     const bool roundUpperRight = flags&Round_UpperRight;
//     const bool roundBottomLeft = flags&Round_BottomLeft;
//     const bool roundBottomRight = flags&Round_BottomRight;
// 
// 
//     p->fillRect (QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2) , color);
// 
//     p->setPen(color);
//     // sides
//     p->drawLine(roundUpperLeft?r.x()+1:r.x(), r.y(),
//                 roundUpperRight?r.right()-1:r.right(), r.y() );
//     p->drawLine(roundBottomLeft?r.x()+1:r.x(), r.bottom(),
//                 roundBottomRight?r.right()-1:r.right(), r.bottom() );
//     p->drawLine(r.x(), roundUpperLeft?r.y()+1:r.y(),
//                 r.x(), roundBottomLeft?r.bottom()-1:r.bottom() );
//     p->drawLine(r.right(), roundUpperLeft?r.y()+1:r.y(),
//                 r.right(), roundBottomLeft?r.bottom()-1:r.bottom() );
// }
// 
void PlastikStyle::renderSurface(QPainter *p,
                                 const QRect &r,
                                 const QColor &backgroundColor,
                                 const QColor &buttonColor,
                                 const QColor &highlightColor,
                                 int intensity,
                                 const uint flags) const
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    const bool disabled = flags&Is_Disabled;

    const bool drawLeft = flags&Draw_Left;
    const bool drawRight = flags&Draw_Right;
    const bool drawTop = flags&Draw_Top;
    const bool drawBottom = flags&Draw_Bottom;
    const bool roundUpperLeft = flags&Round_UpperLeft;
    const bool roundUpperRight = flags&Round_UpperRight;
    const bool roundBottomLeft = flags&Round_BottomLeft;
    const bool roundBottomRight = flags&Round_BottomRight;
    const bool sunken = flags&Is_Sunken;
    const bool horizontal = flags&Is_Horizontal;
    bool highlight = false,
        highlightLeft = false,
        highlightRight = false,
        highlightTop = false,
        highlightBottom = false;
    // only highlight if not sunken & not disabled...
    if(!sunken && !disabled) {
        highlight = (flags&Is_Highlight);
        highlightLeft = (flags&Highlight_Left);
        highlightRight = (flags&Highlight_Right);
        highlightTop = (flags&Highlight_Top);
        highlightBottom = (flags&Highlight_Bottom);
    }

    QColor baseColor = alphaBlendColors(backgroundColor, disabled?backgroundColor:buttonColor, 10);
    if (disabled) {
        intensity = 2;
    } else if (highlight) {
        // blend this _slightly_ with the background
        baseColor = alphaBlendColors(baseColor, highlightColor, 240);
    } else if (sunken) {
        // enforce a common sunken-style...
        baseColor = baseColor.dark(110+intensity);
        intensity = _contrast/2;
    }
// some often needed colors...
    // 1 more intensive than 2 and 3.
    const QColor colorTop1 = alphaBlendColors(baseColor,
                    sunken?baseColor.dark(100+intensity*2):baseColor.light(100+intensity*2), 80);
    const QColor colorTop2 = alphaBlendColors(baseColor,
                    sunken?baseColor.dark(100+intensity):baseColor.light(100+intensity), 80);
    const QColor colorBottom1 = alphaBlendColors(baseColor,
                        sunken?baseColor.light(100+intensity*2):baseColor.dark(100+intensity*2), 80);
    const QColor colorBottom2 = alphaBlendColors(baseColor,
                        sunken?baseColor.light(100+intensity):baseColor.dark(100+intensity), 80);

// sides
    if (drawLeft) {
        if (horizontal) {
            int height = r.height();
            if (roundUpperLeft || !drawTop) height--;
            if (roundBottomLeft || !drawBottom) height--;
            renderGradient(p, QRect(r.left(), (roundUpperLeft&&drawTop)?r.top()+1:r.top(), 1, height),
                            colorTop1, baseColor);
        } else {
            p->setPen(colorTop1 );
            p->drawLine(r.left(), (roundUpperLeft&&drawTop)?r.top()+1:r.top(),
                        r.left(), (roundBottomLeft&&drawBottom)?r.bottom()-1:r.bottom() );
        }
    }
    if (drawRight) {
        if (horizontal) {
            int height = r.height();
            // TODO: there's still a bogus in it: when edge4 is Thick
            //       and we don't whant to draw the Top, we have a unpainted area
            if (roundUpperRight || !drawTop) height--;
            if (roundBottomRight || !drawBottom) height--;
            renderGradient(p, QRect(r.right(), (roundUpperRight&&drawTop)?r.top()+1:r.top(), 1, height),
                            baseColor, colorBottom1);
        } else {
            p->setPen(colorBottom1 );
            p->drawLine(r.right(), (roundUpperRight&&drawTop)?r.top()+1:r.top(),
                        r.right(), (roundBottomRight&&drawBottom)?r.bottom()-1:r.bottom() );
        }
    }
    if (drawTop) {
        if (horizontal) {
            p->setPen(colorTop1 );
            p->drawLine((roundUpperLeft&&drawLeft)?r.left()+1:r.left(), r.top(),
                        (roundUpperRight&&drawRight)?r.right()-1:r.right(), r.top() );
        } else {
            int width = r.width();
            if (roundUpperLeft || !drawLeft) width--;
            if (roundUpperRight || !drawRight) width--;
            renderGradient(p, QRect((roundUpperLeft&&drawLeft)?r.left()+1:r.left(), r.top(), width, 1),
                            colorTop1, colorTop2);
        }
    }
    if (drawBottom) {
        if (horizontal) {
            p->setPen(colorBottom1 );
            p->drawLine((roundBottomLeft&&drawLeft)?r.left()+1:r.left(), r.bottom(),
                        (roundBottomRight&&drawRight)?r.right()-1:r.right(), r.bottom() );
        } else {
            int width = r.width();
            if (roundBottomLeft || !drawLeft) width--;
            if (roundBottomRight || !drawRight) width--;
            renderGradient(p, QRect((roundBottomLeft&&drawLeft)?r.left()+1:r.left(), r.bottom(), width, 1),
                            colorBottom2, colorBottom1);
        }
    }

// button area...
    int width = r.width();
    int height = r.height();
    if (drawLeft) width--;
    if (drawRight) width--;
    if (drawTop) height--;
    if (drawBottom) height--;
    renderGradient(p, QRect(drawLeft?r.left()+1:r.left(), drawTop?r.top()+1:r.top(), width, height),
                    colorTop2, colorBottom2, horizontal);


// highlighting...
    if(highlightTop) {
        p->setPen(alphaBlendColors(colorTop1 , highlightColor, 80) );
        p->drawLine((roundUpperLeft&&drawLeft)?r.left()+1:r.left(), r.top(),
                    (roundUpperRight&&drawRight)?r.right()-1:r.right(), r.top() );
        p->setPen(alphaBlendColors(colorTop2 , highlightColor, 150) );
        p->drawLine(highlightLeft?r.left()+1:r.left(), r.top()+1,
                    highlightRight?r.right()-1:r.right(), r.top()+1 );
    }
    if(highlightBottom) {
        p->setPen(alphaBlendColors(colorBottom1 , highlightColor, 80) );
        p->drawLine((roundBottomLeft&&drawLeft)?r.left()+1:r.left(), r.bottom(),
                    (roundBottomRight&&drawRight)?r.right()-1:r.right(), r.bottom() );
        p->setPen(alphaBlendColors(colorBottom2 , highlightColor, 150) );
        p->drawLine(highlightLeft?r.left()+1:r.left(), r.bottom()-1,
                    highlightRight?r.right()-1:r.right(), r.bottom()-1 );
    }
    if(highlightLeft) {
        p->setPen(alphaBlendColors(colorTop1 , highlightColor, 80) );
        p->drawLine(r.left(), (roundUpperLeft&&drawTop)?r.top()+1:r.top(),
                    r.left(), (roundBottomLeft&&drawBottom)?r.bottom()-1:r.bottom() );
        p->setPen(alphaBlendColors(colorTop2 , highlightColor, 150) );
        p->drawLine(r.left()+1, highlightTop?r.top()+1:r.top(),
                    r.left()+1, highlightBottom?r.bottom()-1:r.bottom() );
    }
    if(highlightRight) {
        p->setPen(alphaBlendColors(colorBottom1 , highlightColor, 80) );
        p->drawLine(r.right(), (roundUpperRight&&drawTop)?r.top()+1:r.top(),
                    r.right(), (roundBottomRight&&drawBottom)?r.bottom()-1:r.bottom() );
        p->setPen(alphaBlendColors(colorBottom2 , highlightColor, 150) );
        p->drawLine(r.right()-1, highlightTop?r.top()+1:r.top(),
                    r.right()-1, highlightBottom?r.bottom()-1:r.bottom() );
    }
}

void PlastikStyle::renderPixel(QPainter *p,
            const QPoint &pos,
            const int alpha,
            const QColor &color,
            const QColor &background,
            bool fullAlphaBlend) const
{
    if(fullAlphaBlend)
    // full alpha blend: paint into an image with alpha buffer and convert to a pixmap ...
    {
        QRgb rgb = color.rgb();
        // generate a quite unique key -- use the unused width field to store the alpha value.
        CacheEntry search(cAlphaDot, alpha, 0, rgb);
        int key = search.key();

        CacheEntry *cacheEntry;
        if( (cacheEntry = pixmapCache->take(key)) ) {
            if( search == *cacheEntry ) { // match! we can draw now...
                if(cacheEntry->pixmap)
                    p->drawPixmap(pos, *(cacheEntry->pixmap) );
                return;
            } else { //Remove old entry in case of a conflict!
                pixmapCache->remove( key );
            }
        }


        QImage aImg(1,1,32); // 1x1
        aImg.setAlphaBuffer(true);
        aImg.setPixel(0,0,qRgba(qRed(rgb),qGreen(rgb),qBlue(rgb),alpha));
        QPixmap *result = new QPixmap(aImg);

        p->drawPixmap(pos, *result);

        // add to the cache...
        CacheEntry *toAdd = new CacheEntry(search);
        toAdd->pixmap = result;
        bool insertOk = pixmapCache->insert( key, toAdd, result->depth()/8);
        if(!insertOk)
            delete result;
    } else
    // don't use an alpha buffer: calculate the resulting color from the alpha value, the fg- and the bg-color.
    {
        QRgb rgb_a = color.rgb();
        QRgb rgb_b = background.rgb();
        int a = alpha;
        if(a>255) a = 255;
        if(a<0) a = 0;
        int a_inv = 255 - a;

        QColor res  = QColor( qRgb(qRed(rgb_b)*a_inv/255 + qRed(rgb_a)*a/255,
                              qGreen(rgb_b)*a_inv/255 + qGreen(rgb_a)*a/255,
                              qBlue(rgb_b)*a_inv/255 + qBlue(rgb_a)*a/255) );
        p->setPen(res);
        p->drawPoint(pos);
    }
}

void PlastikStyle::renderButton(QPainter *p,
                               const QRect &r,
                               const QPalette &pal,
                               bool sunken,
                               bool mouseOver,
                               bool horizontal,
                               bool enabled,
                               bool khtmlMode) const
{
    const QPen oldPen( p->pen() );

    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
    if(!enabled) contourFlags|=Is_Disabled;
    if(khtmlMode) contourFlags|=Draw_AlphaBlend;

    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
    if(horizontal) surfaceFlags|=Is_Horizontal;
    if(!enabled) surfaceFlags|=Is_Disabled;
    else {
        if(sunken) surfaceFlags|=Is_Sunken;
        else {
            if(mouseOver) {
                surfaceFlags|=Is_Highlight;
                if(horizontal) {
                    surfaceFlags|=Highlight_Top;
                    surfaceFlags|=Highlight_Bottom;
                } else {
                    surfaceFlags|=Highlight_Left;
                    surfaceFlags|=Highlight_Right;
                }
            }
        }
    }

    if (!flatMode) {
        contourFlags |= Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
        surfaceFlags |= Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;

        renderContour(p, r, pal.background().color(), getColor(pal,ButtonContour),
                    contourFlags);
        renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                      pal.background().color(), pal.button().color(), getColor(pal,MouseOverHighlight), _contrast, surfaceFlags);
    } else {
        renderContour(p, r, pal.background().color(), pal.button().color().dark(105+_contrast*3),
                      contourFlags);
        renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                      pal.background().color(), pal.button().color(), getColor(pal,MouseOverHighlight), _contrast/2, surfaceFlags);

        flatMode = false;
    }

    p->setPen(oldPen);
}

void PlastikStyle::renderCheckBox(QPainter *p, const QRect &rect, const QPalette &pal,
                                  bool enabled, bool mouseOver, int primitive) const
{
    QColor contentColor = enabled?pal.base().color():pal.background().color();

    int s = QMIN(rect.width(), rect.height());
    QRect r = centerRect(rect, s, s);

    uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
    if(!enabled) {
        contourFlags |= Is_Disabled;
    }
    renderContour(p, r, pal.background().color(), getColor(pal, ButtonContour),
                  contourFlags);

                // surface
    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
    if(!enabled) {
        surfaceFlags |= Is_Disabled;
    } else if(mouseOver) {
        contentColor = alphaBlendColors(contentColor, getColor(pal,MouseOverHighlight), 240);
        surfaceFlags |= Is_Highlight;
        surfaceFlags |= Highlight_Left|Highlight_Right|
                Highlight_Top|Highlight_Bottom;
    }
    renderSurface(p, QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2),
                  pal.background().color(), contentColor, getColor(pal,MouseOverHighlight), enabled?_contrast+3:(_contrast/2), surfaceFlags);

            // check mark
    QColor checkmarkColor = enabled?getColor(pal,CheckMark):pal.background().color();
            // TODO: check mouse pressed Style_Down equivalent for kstyle4
    if(false/*flags & Style_Down*/) {
        checkmarkColor = alphaBlendColors(contentColor, checkmarkColor, 150);
    }
    int x = r.center().x() - 4, y = r.center().y() - 4;

    QBitmap bmp;

    switch (primitive)
    {
        case CheckBox::CheckOn:
        {
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, checkmark_dark_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 50) );
            p->drawPixmap(x, y, bmp);
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, checkmark_light_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(125), 50) );
            p->drawPixmap(x, y, bmp);
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, checkmark_aa_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 150) );
            p->drawPixmap(x, y, bmp);

            return;
        }

        case CheckBox::CheckOff:
        {
                    // empty
            return;
        }

        case CheckBox::CheckTriState:
        {
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, checkmark_tristate_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 50) );
            p->drawPixmap(x, y, bmp);

            return;
        }
    }
}

void PlastikStyle::renderRadioButton(QPainter *p, const QRect &r, const QPalette &pal,
                                        bool enabled, bool mouseOver, int primitive) const
{


    int x = r.x();
    int y = r.y();

    const QColor contourColor = getColor(pal, ButtonContour, enabled);
    QColor contentColor = enabled?pal.base().color():pal.background().color();

    QBitmap bmp;
    bmp = QBitmap(13, 13, radiobutton_mask_bits, true);
            // first the surface...
    uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
    if(!enabled) {
        surfaceFlags |= Is_Disabled;
    } else if (mouseOver) {
        contentColor = alphaBlendColors(contentColor, getColor(pal,MouseOverHighlight), 240);
    }
    p->setClipRegion(bmp);
    renderSurface(p, r,
                  pal.background().color(), contentColor, getColor(pal,MouseOverHighlight), enabled?_contrast+3:(_contrast/2), surfaceFlags);
    p->setClipping(false);

            // ...then contour, anti-alias, mouseOver...
            // contour
    bmp = QBitmap(13, 13, radiobutton_contour_bits, true);
    bmp.setMask(bmp);
    p->setPen(alphaBlendColors(pal.background().color(), contourColor, 50) );
    p->drawPixmap(x, y, bmp);
            // anti-alias outside
    bmp = QBitmap(13, 13, radiobutton_aa_outside_bits, true);
    bmp.setMask(bmp);
    p->setPen(alphaBlendColors(pal.background().color(), contourColor, 150) );
    p->drawPixmap(x, y, bmp);
            // highlighting...
    if(mouseOver) {
        bmp = QBitmap(13, 13, radiobutton_highlight1_bits, true);
        bmp.setMask(bmp);
        p->setPen(alphaBlendColors(contentColor, getColor(pal,MouseOverHighlight), 80) );
        p->drawPixmap(x, y, bmp);
        bmp = QBitmap(13, 13, radiobutton_highlight2_bits, true);
        bmp.setMask(bmp);
        p->setPen(alphaBlendColors(contentColor, getColor(pal,MouseOverHighlight), 150) );
        p->drawPixmap(x, y, bmp);
    }
            // anti-alias inside, "above" the higlighting!
    bmp = QBitmap(13, 13, radiobutton_aa_inside_bits, true);
    bmp.setMask(bmp);
    if(mouseOver) {
        p->setPen(alphaBlendColors(getColor(pal,MouseOverHighlight), contourColor, 180) );
    } else {
        p->setPen(alphaBlendColors(contentColor, contourColor, 180) );
    }
    p->drawPixmap(x, y, bmp);


    QColor checkmarkColor = enabled?getColor(pal,CheckMark):pal.background().color();
            // TODO: implement pressed state with Style_Down equivalent
    if(false /*flags & Style_Down*/) {
        checkmarkColor = alphaBlendColors(contentColor, checkmarkColor, 150);
    }

            // draw the radio mark
    switch (primitive)
    {
        case RadioButton::RadioOn:
        {
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, radiomark_dark_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 50) );
            p->drawPixmap(x+2, y+2, bmp);
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, radiomark_light_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(125), 50) );
            p->drawPixmap(x+2, y+2, bmp);
            bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, radiomark_aa_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 150) );
            p->drawPixmap(x+2, y+2, bmp);

            return;
        }
        case RadioButton::RadioOff:
        {
                // empty
            return;
        }
    }
}

void PlastikStyle::renderDot(QPainter *p,
                             const QPoint &point,
                             const QColor &baseColor,
                             const bool thick,
                             const bool sunken) const
{
    const QColor topColor = alphaBlendColors(baseColor, sunken?baseColor.dark(130):baseColor.light(150), 70);
    const QColor bottomColor = alphaBlendColors(baseColor, sunken?baseColor.light(150):baseColor.dark(130), 70);
    p->setPen(topColor );
    p->drawLine(point.x(), point.y(), point.x()+1, point.y());
    p->drawPoint(point.x(), point.y()+1);
    p->setPen(bottomColor );
    if(thick) {
        p->drawLine(point.x()+1, point.y()+2, point.x()+2, point.y()+2);
        p->drawPoint(point.x()+2, point.y()+1);
    } else {
        p->drawPoint(point.x()+1, point.y()+1);
    }
}

void PlastikStyle::renderGradient(QPainter *painter,
                                  const QRect &rect,
                                  const QColor &c1,
                                  const QColor &c2,
                                  bool horizontal) const
{
    if((rect.width() <= 0)||(rect.height() <= 0))
        return;

    // generate a quite unique key for this surface.
    CacheEntry search(cGradientTile,
                      horizontal ? 0 : rect.width(),
                      horizontal ? rect.height() : 0,
                      c1.rgb(), c2.rgb(), horizontal );
    int key = search.key();

    CacheEntry *cacheEntry;
    if( (cacheEntry = pixmapCache->take(key)) ) {
        if( search == *cacheEntry ) { // match! we can draw now...
            if(cacheEntry->pixmap) {
                painter->drawTiledPixmap(rect, *(cacheEntry->pixmap) );
            }
            return;
        } else {
            // Remove old entry in case of a conflict!
            // This shouldn't happen very often, see comment in CacheEntry.
            pixmapCache->remove(key);
        }
    }

    // there wasn't anything matching in the cache, create the pixmap now...
    QPixmap *result = new QPixmap(horizontal ? 10 : rect.width(),
                                  horizontal ? rect.height() : 10);
    QPainter p(result);

    int r_w = result->rect().width();
    int r_h = result->rect().height();
    int r_x, r_y, r_x2, r_y2;
    result->rect().coords(&r_x, &r_y, &r_x2, &r_y2);

    int rDiff, gDiff, bDiff;
    int rc, gc, bc;

    register int x, y;

    rDiff = ( c2.red())   - (rc = c1.red());
    gDiff = ( c2.green()) - (gc = c1.green());
    bDiff = ( c2.blue())  - (bc = c1.blue());

    register int rl = rc << 16;
    register int gl = gc << 16;
    register int bl = bc << 16;

    int rdelta = ((1<<16) / (horizontal ? r_h : r_w)) * rDiff;
    int gdelta = ((1<<16) / (horizontal ? r_h : r_w)) * gDiff;
    int bdelta = ((1<<16) / (horizontal ? r_h : r_w)) * bDiff;

    // these for-loops could be merged, but the if's in the inner loop
    // would make it slow
    if(horizontal) {
        for ( y = 0; y < r_h; y++ ) {
            rl += rdelta;
            gl += gdelta;
            bl += bdelta;

            p.setPen(QColor(rl>>16, gl>>16, bl>>16));
            p.drawLine(r_x, r_y+y, r_x2, r_y+y);
        }
    } else {
        for( x = 0; x < r_w; x++) {
            rl += rdelta;
            gl += gdelta;
            bl += bdelta;

            p.setPen(QColor(rl>>16, gl>>16, bl>>16));
            p.drawLine(r_x+x, r_y, r_x+x, r_y2);
        }
    }

    p.end();

    // draw the result...
    painter->drawTiledPixmap(rect, *result);

    // insert into cache using the previously created key.
    CacheEntry *toAdd = new CacheEntry(search);
    toAdd->pixmap = result;
    bool insertOk = pixmapCache->insert( key, toAdd, result->width()*result->height()*result->depth()/8 );

    if(!insertOk)
        delete result;
}

void PlastikStyle::renderPanel(QPainter *p,
                              const QRect &r,
                              const QPalette &pal,
                              const bool pseudo3d,
                              const bool sunken) const
{
    int x, x2, y, y2, w, h;
    r.rect(&x,&y,&w,&h);
    r.coords(&x, &y, &x2, &y2);

//     if (kickerMode &&
//             p->device() && p->device()->devType() == QInternal::Widget &&
//             QByteArray(static_cast<QWidget*>(p->device())->className()) == "FittsLawFrame") {
//     //  Stolen wholesale from Keramik. I don't like it, but oh well.
//         if (sunken) {
//             const QCOORD corners[] = { x2, y, x2, y2, x, y2, x, y };
//             p->setPen(pal.background().color().dark());
//             p->drawConvexPolygon(Q3PointArray(4, corners));
//             p->setPen(pal.background().color().light());
//             p->drawPolyline(Q3PointArray(4, corners), 0, 3);
//         } else {
//             const QCOORD corners[] = { x, y2, x, y, x2, y, x2, y2 };
//             p->setPen(pal.background().color().dark());
//             p->drawPolygon(Q3PointArray(4, corners));
//             p->setPen(pal.background().color().light());
//             p->drawPolyline(Q3PointArray(4, corners), 0, 3);
//         }
//     } else {
        renderContour(p, r, pal.background().color(), getColor(pal, PanelContour) );

        if(pseudo3d) {
            if (sunken) {
                p->setPen(getColor(pal, PanelDark) );
            } else {
                p->setPen(getColor(pal, PanelLight) );
            }
            p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1);
            p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2);
            if (sunken) {
                p->setPen(getColor(pal, PanelLight) );
            } else {
                p->setPen(getColor(pal, PanelDark) );
            }
            p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1);
            p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);
        }
//     }
}

// void PlastikStyle::renderMenuBlendPixmap( KPixmap &pix, const QColorGroup &cg,
//     const Q3PopupMenu* /* popup */ ) const
// {
//     pix.fill( cg.background().light(105) );
// }
// 
void PlastikStyle::renderTab(QPainter *p,
                            const QRect &r,
                            const QPalette &pal,
                            bool mouseOver,
                            const bool selected,
                            const bool bottom,
                            const QStyleOptionTab::TabPosition pos/*const TabPosition pos*/,
                            const bool triangular,
                            const bool cornerWidget,
                            const bool reverseLayout) const
{
    const bool isFirst = pos == QStyleOptionTab::Beginning || pos == QStyleOptionTab::OnlyOneTab/* (pos == First) || (pos == Single)*/;
    const bool isLast = pos == QStyleOptionTab::End /*(pos == Last)*/;
    const bool isSingle = pos == QStyleOptionTab::OnlyOneTab /*(pos == Single)*/;

    if (selected) {
    // is selected

    // the top part of the tab which is nearly the same for all positions
        QRect Rc; // contour
        if (!bottom) {
            if (isFirst && !cornerWidget && !reverseLayout) {
                Rc = QRect(r.x(), r.y(), r.width()-1, r.height()-3);
            } else if (isFirst && !cornerWidget && reverseLayout) {
                Rc = QRect(r.x()+1, r.y(), r.width()-1, r.height()-3);
            } else {
                Rc = QRect(r.x()+1, r.y(), r.width()-2, r.height()-3);
            }
        } else {
            if (isFirst && !cornerWidget && !reverseLayout) {
                Rc = QRect(r.x(), r.y()+3, r.width()-1, r.height()-3);
            } else if (isFirst && !cornerWidget && reverseLayout) {
                Rc = QRect(r.x()+1, r.y()+3, r.width()-1, r.height()-3);
            } else {
                Rc = QRect(r.x()+1, r.y()+3, r.width()-2, r.height()-3);
            }
        }
        const QRect Rs(Rc.x()+1, bottom?Rc.y():Rc.y()+1, Rc.width()-2, Rc.height()-1); // the resulting surface
        // the area where the fake border shoudl appear
        const QRect Rb(r.x(), bottom?r.top():Rc.bottom()+1, r.width(), r.height()-Rc.height() );

        uint contourFlags = Draw_Left|Draw_Right;
        if(!bottom) {
            contourFlags |= Draw_Top|Round_UpperLeft|Round_UpperRight;
        } else {
            contourFlags |= Draw_Bottom|Round_BottomLeft|Round_BottomRight;
        }
        renderContour(p, Rc,
                      pal.background().color(), getColor(pal,PanelContour),
                      contourFlags);

        // surface
        if(!bottom) {
            p->setPen(getColor(pal,PanelLight) );
            p->drawLine(Rs.x()+1, Rs.y(), Rs.right()-1, Rs.y() );
            renderGradient(p, QRect(Rs.x(), Rs.y()+1, 1, Rs.height()-1),
                           getColor(pal,PanelLight), getColor(pal,PanelLight2));
            renderGradient(p, QRect(Rs.right(), Rs.y()+1, 1, Rs.height()-1),
                            getColor(pal,PanelDark), getColor(pal,PanelDark2));
        } else {
            p->setPen(alphaBlendColors(pal.background().color(), pal.background().color().dark(160), 100) );
            p->drawLine(Rs.x()+1, Rs.bottom(), Rs.right()-1, Rs.bottom() );
            renderGradient(p, QRect(Rs.x(), Rs.y(), 1, Rs.height()-1),
                            getColor(pal,PanelLight), getColor(pal,PanelLight2));
            renderGradient(p, QRect(Rs.right(), Rs.y(), 1, Rs.height()-1),
                            getColor(pal,PanelDark), getColor(pal,PanelDark2));
        }

    // some "position specific" paintings...
        // draw parts of the inactive tabs around...
        if(!isSingle) {
            p->setPen(alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour), 50) );
            if( (!isFirst&&!reverseLayout) || (!isLast&&reverseLayout) ) {
                p->drawPoint(r.left(), bottom?(triangular?r.bottom()-2:r.bottom()-3):(triangular?r.top()+2:r.top()+3) );
                renderSurface(p, QRect(r.left(), bottom?r.top()+3:(triangular?r.top()+3:r.top()+4), 1, (triangular?r.height()-6:r.height()-7) ),
                            pal.background().color(), pal.button(), getColor(pal,MouseOverHighlight), _contrast,
                            Draw_Top|Draw_Bottom|Is_Horizontal);
            }
            if( (!isLast&&!reverseLayout) || (!isFirst&&reverseLayout) ) {
                p->drawPoint(r.right(), bottom?(triangular?r.bottom()-2:r.bottom()-3):(triangular?r.top()+2:r.top()+3) );
                renderSurface(p, QRect(r.right(), bottom?r.top()+3:(triangular?r.top()+3:r.top()+4), 1, (triangular?r.height()-6:r.height()-7) ),
                            pal.background().color(), pal.button(), getColor(pal,MouseOverHighlight), _contrast,
                            Draw_Top|Draw_Bottom|Is_Horizontal);
            }
        }
        // left connection from the panel border to the tab. :)
        if(isFirst && !reverseLayout && !cornerWidget) {
            p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.y(), Rb.x(), Rb.bottom() );
            p->setPen(getColor(pal,PanelLight) );
            p->drawLine(Rb.x()+1, Rb.y(), Rb.x()+1, Rb.bottom() );
        } else if(isFirst && reverseLayout && !cornerWidget) {
            p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
            p->drawLine(Rb.right(), Rb.y(), Rb.right(), Rb.bottom() );
            p->setPen(getColor(pal,PanelDark) );
            p->drawLine(Rb.right()-1, Rb.y(), Rb.right()-1, Rb.bottom() );
        }
        // rounded connections to the panel...
        if(!bottom) {
            // left
            if( (!isFirst && !reverseLayout) || (reverseLayout) || (isFirst && !reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.x(), Rb.y());
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.y()+1);
                p->drawPoint(Rb.x()+1, Rb.y());
            }
            // right
            if( (!reverseLayout) || (!isFirst && reverseLayout) || (isFirst && reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.right(), Rb.y());
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.y()+1);
                p->drawPoint(Rb.right()-1, Rb.y());
            }
        } else {
            // left
            if( (!isFirst && !reverseLayout) || (reverseLayout) || (isFirst && !reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.x(), Rb.bottom());
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.bottom()-1);
                p->drawPoint(Rb.x()+1, Rb.bottom());
            }
            // right
            if( (!reverseLayout) || (!isFirst && reverseLayout) || (isFirst && reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.right(), Rb.bottom());
                p->setPen( alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.bottom()-1);
                p->drawPoint(Rb.right()-1, Rb.bottom());
            }
        }

    } else {
    // inactive tabs

    // the top part of the tab which is nearly the same for all positions
        QRect Rc; // contour
        if (isFirst&&reverseLayout ) {
            Rc = QRect(r.x()+1, (bottom?r.y()+2:(triangular?r.y()+2:r.y()+3)), r.width()-2, (triangular?r.height()-4:r.height()-5) );
        } else {
            Rc = QRect(r.x()+1, (bottom?r.y()+2:(triangular?r.y()+2:r.y()+3)), r.width()-1, (triangular?r.height()-4:r.height()-5) );
        }
        QRect Rs; // the resulting surface
        if ( (isFirst&&!reverseLayout) || (isLast&&reverseLayout) ) {
            Rs = QRect(Rc.x()+1, bottom?Rc.y():Rc.y()+1, Rc.width()-2, Rc.height()-1);
        } else {
            Rs = QRect(Rc.x(), bottom?Rc.y():Rc.y()+1, Rc.width()-1, Rc.height()-1);
        }
        // the area where the fake border shoudl appear
        const QRect Rb(r.x(), bottom?r.y():Rc.bottom()+1, r.width(), 2 );

        uint contourFlags;
        if(!bottom) {
            if ( (isFirst&&!reverseLayout) || (isLast&&reverseLayout) ) {
                contourFlags = Draw_Left|Draw_Right|Draw_Top|Round_UpperLeft;
            } else if ( (isLast&&!reverseLayout) || (isFirst&&reverseLayout) ) {
                contourFlags = Draw_Right|Draw_Top|Round_UpperRight;
            } else {
                contourFlags = Draw_Right|Draw_Top;
            }
        } else {
            if ( (isFirst&&!reverseLayout) || (isLast&&reverseLayout) ) {
                contourFlags = Draw_Left|Draw_Right|Draw_Bottom|Round_BottomLeft;
            } else if ( (isLast&&!reverseLayout) || (isFirst&&reverseLayout) ) {
                contourFlags = Draw_Right|Draw_Bottom|Round_BottomRight;
            } else {
                contourFlags = Draw_Right|Draw_Bottom;
            }
        }
        renderContour(p, Rc,
                        pal.background().color(), getColor(pal, ButtonContour),
                        contourFlags);

        uint surfaceFlags = Is_Horizontal;
        if(mouseOver) {
            surfaceFlags |= (bottom?Highlight_Bottom:Highlight_Top);
            surfaceFlags |= Is_Highlight;
        }
        if ( (isFirst&&!reverseLayout) || (isLast&&reverseLayout) ) {
            if(!bottom)
                surfaceFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft;
            else
                surfaceFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_BottomLeft;
        } else if ( (isLast&&!reverseLayout) || (isFirst&&reverseLayout) ) {
            if(!bottom)
                surfaceFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight;
            else
                surfaceFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_BottomRight;
        } else {
            surfaceFlags |= Draw_Top|Draw_Bottom;
        }
        renderSurface(p, Rs,
                        pal.background().color(), pal.button(), getColor(pal,MouseOverHighlight), _contrast,
                        surfaceFlags);

    // some "position specific" paintings...
        // fake parts of the panel border
        if(!bottom) {
            p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.y(), ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.y());
            p->setPen(getColor(pal,PanelLight) );
            p->drawLine(Rb.x(), Rb.y()+1, ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.y()+1 );
        } else {
            p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.bottom(), ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.bottom());
            p->setPen(getColor(pal,PanelDark) );
            p->drawLine(Rb.x(), Rb.bottom()-1, ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.bottom()-1 );
        }
        // fake the panel border edge for tabs which are aligned left-most
        // (i.e. only if there is no widget in the corner of the tabwidget!)
        if(isFirst&&!reverseLayout&&!cornerWidget)
        // normal layout
        {
            if (!bottom) {
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.x()+1, Rb.y()+1 );
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.y()+1 );
                p->setPen(pal.background().color() );
                p->drawPoint(Rb.x(), Rb.y() );
                p->setPen(alphaBlendColors( alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour), 50), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x()+1, Rb.y() );
            } else {
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.x()+1, Rb.bottom()-1 );
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.bottom()-1 );
                p->setPen(pal.background().color() );
                p->drawPoint(Rb.x(), Rb.bottom() );
                p->setPen(alphaBlendColors( alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour), 50), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.x()+1, Rb.bottom() );
            }
        } else if(isFirst&&reverseLayout&&!cornerWidget)
        // reverse layout
        {
            if (!bottom) {
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.right()-1, Rb.y()+1 );
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.y()+1 );
                p->setPen(pal.background().color() );
                p->drawPoint(Rb.right(), Rb.y() );
                p->setPen(alphaBlendColors( alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour), 50), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right()-1, Rb.y() );
            } else {
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 50) );
                p->drawPoint(Rb.right()-1, Rb.bottom()-1 );
                p->setPen(alphaBlendColors(pal.background().color(), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.bottom()-1 );
                p->setPen(pal.background().color() );
                p->drawPoint(Rb.right(), Rb.bottom() );
                p->setPen(alphaBlendColors( alphaBlendColors(pal.background().color(), getColor(pal, ButtonContour), 50), getColor(pal,PanelContour), 150) );
                p->drawPoint(Rb.right()-1, Rb.bottom() );
            }
        }
    }
}

// void PlastikStyle::drawPrimitive(PrimitiveElement pe,
//                                 QPainter *p,
//                                 const QRect &r,
//                                 const QColorGroup &cg,
//                                 SFlags flags,
//                                 const QStyleOption &opt ) const
// {
//     bool down   = flags & Style_Down;
//     bool on     = flags & Style_On;
//     bool sunken = flags & Style_Sunken;
//     bool horiz  = flags & Style_Horizontal;
//     const bool enabled = flags & Style_Enabled;
//     const bool mouseOver = flags & Style_MouseOver;
//     
//     bool hasFocus = flags & Style_HasFocus;
// 
//     int x = r.x();
//     int y = r.y();
//     int w = r.width();
//     int h = r.height();
// 
//     int x2, y2;
//     r.coords(&x, &y, &x2, &y2);

// 
//     // CHECKBOXES
//     // ----------
// 
//         case PE_IndicatorMask: {
//             p->fillRect (r, Qt::color1);
//             break;
//         }
// 
//     // RADIOBUTTONS
//     // ------------

// 
//         case PE_ExclusiveIndicatorMask: {
//             p->fillRect(r, Qt::color0);
// 
//             QBitmap bmp;
//             bmp = QBitmap(13, 13, radiobutton_mask_bits, true);
//             bmp.setMask(bmp);
//             p->setPen(Qt::color1);
//             p->drawPixmap(x, y, bmp);
// 
//             break;
//         }

//         case PE_WindowFrame:
//         case PE_Panel: {
//             if ( opt.isDefault() || opt.lineWidth() <= 0 )
//                 break;
//             renderPanel(p, r, cg, true, sunken);
//             break;
//         }

//         case PE_StatusBarSection: {
//             renderContour(p, r, cg.background(), cg.background().dark(160),
//                           Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
//             break;
//         }

//     // MENU / TOOLBAR PANEL
//     // --------------------
//         case PE_PanelMenuBar:
//         case PE_PanelDockWindow: {
//             // fix for toolbar lag (from Mosfet Liquid) 
//             QWidget* w = dynamic_cast<QWidget*>(p->device());
//             if(w && w->backgroundMode() == Qt::PaletteButton) 
//                 w->setBackgroundMode(Qt::PaletteBackground);
//             p->fillRect(r, cg.brush(QColorGroup::Background));
// 
//             if ( _drawToolBarSeparator ) {
//                 if ( r.width() > r.height() ) {
//                     p->setPen( getColor(cg, PanelLight) );
//                     p->drawLine( r.left(), r.top(), r.right(), r.top() );
//                     p->setPen( getColor(cg, PanelDark) );
//                     p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
//                 }
//                 else {
//                     p->setPen( getColor(cg, PanelLight) );
//                     p->drawLine( r.left(), r.top(), r.left(), r.bottom() );
//                     p->setPen( getColor(cg, PanelDark) );
//                     p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
//                 }
//             }
// 
//             break;
//         }

// 
//         default: {
//             return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
//         }
//     }
// }
// 
// 
// void PlastikStyle::drawControl(ControlElement element,
//                               QPainter *p,
//                               const QWidget *widget,
//                               const QRect &r,
//                               const QColorGroup &cg,
//                               SFlags flags,
//                               const QStyleOption& opt) const
// {
//     const bool reverseLayout = QApplication::isRightToLeft();
// 
//     const bool enabled = (flags & Style_Enabled);
// 
//     switch (element) {
// 


//     // Menu and dockwindow empty space
//     //
//         case CE_DockWindowEmptyArea:
//             p->fillRect(r, cg.background());
//             break;
// 
//         case CE_MenuBarEmptyArea:
//             p->fillRect(r, cg.background());
// 
// //             if ( _drawToolBarSeparator ) {
// //                 p->setPen( getColor(cg, PanelDark) );
// //                 p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
// //             }
// 
//             break;
// 
//         default:
//           KStyle::drawControl(element, p, widget, r, cg, flags, opt);
//     }
// }
// 
// void PlastikStyle::drawControlMask(ControlElement element,
//                                   QPainter *p,
//                                   const QWidget *w,
//                                   const QRect &r,
//                                   const QStyleOption &opt) const
// {
//     switch (element) {
//         case CE_PushButton: {
//                     p->fillRect (r, Qt::color0);
//                     renderMask(p, r, Qt::color1,
//                             Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight);
//                     break;
//         }
// 
//         default: {
//             KStyle::drawControlMask (element, p, w, r, opt);
//         }
//     }
// }
// 
// void PlastikStyle::drawComplexControlMask(ComplexControl c,
//                                          QPainter *p,
//                                          const QWidget *w,
//                                          const QRect &r,
//                                          const QStyleOption &o) const
// {
//     switch (c) {
//         case CC_SpinWidget:
//         case CC_ListView:
//         case CC_ComboBox: {
//                 p->fillRect (r, Qt::color0);
//                 renderMask(p, r, Qt::color1,
//                         Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight);
//             break;
//         }
//         default: {
//             KStyle::drawComplexControlMask (c, p, w, r, o);
//         }
//     }
// }

// int PlastikStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
// {
//     switch(m) {
//     // TABS
//     // ----
//         case PM_TabBarTabVSpace: {
//             const QTabBar * tb = (const QTabBar *) widget;
//             if (tb->shape() == QTabBar::RoundedNorth ||
//                 tb->shape() == QTabBar:: RoundedSouth)
//                 return 12;
//             else
//                 return 4;
//         }
// 
//         case PM_TabBarTabOverlap: {
//             return 1;
//         }
// 
//     // extra space between menubar items
//         case PM_MenuBarItemSpacing: {
//             return 6;
//         }

//     // SCROLL BAR
//         case PM_ScrollBarSliderMin: {
//             return 21;
//         }
//         case PM_ScrollBarExtent: {
//             return 16;
//         }
// 
//         case PM_DockWindowSeparatorExtent:
//             return 6;
// 
//     // SPLITTERS
//     // ---------
//         case PM_SplitterWidth: {
//             return 6;
//         }
// 
//     // PROGRESSBARS
//     // ------------
//         case PM_ProgressBarChunkWidth:
//             return 10;
//
//     // MENU INDICATOR
//     // --------------
//         case PM_MenuButtonIndicator:
//             return 8;

//     // FRAMES
//     // ------

//         case PM_MenuBarFrameWidth:
//             return 1;
// 
//         case PM_DefaultFrameWidth: {
//             if(widget && qobject_cast<Q3PopupMenu>(widget))
//                 return 1;
//             else
//                 return 2;
//         }
// 
//         case PM_ButtonDefaultIndicator: {
//             return 0;
//         }
// 
//         case PM_ButtonMargin: {
//             return 2;
//         }
// 
//         case PM_ButtonShiftVertical:
//         case PM_ButtonShiftHorizontal: {
//             return 1;
//         }
// 
//         default:
//             return KStyle::pixelMetric(m, widget);
//     }
// }

// int PlastikStyle::styleHint( StyleHint stylehint,
//                                    const QWidget *widget,
//                                    const QStyleOption &option,
//                                    QStyleHintReturn* returnData ) const
// {
//     switch (stylehint) {
//         case SH_PopupMenu_SubMenuPopupDelay:
//             return 96; // Motif-like delay...
// 
//         default:
//             return KStyle::styleHint(stylehint, widget, option, returnData);
//     }
// }
// 
// bool PlastikStyle::eventFilter(QObject *obj, QEvent *ev)
// {
//     if (KStyle::eventFilter(obj, ev) )
//         return true;
// 
//     if (!obj->isWidgetType() ) return false;
//  
//     // focus highlight
//     if ( qobject_cast<QLineEdit>(obj) ) {
//         QWidget* widget = static_cast<QWidget*>(obj);
// 
//         if ( qobject_cast<Q3SpinWidget>(widget->parentWidget()) )
//         {
//             QWidget* spinbox = widget->parentWidget();
//             if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
//             {
//                 spinbox->repaint(false);
//             }
//             return false;
//         }
// 
//         if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
//         {
//             widget->repaint(false);
//         }
//         return false;
//     }
//     
//     if ( qobject_cast<QTabBar>(obj) ) {
//         if ((ev->type() == QEvent::Enter) && static_cast<QWidget*>(obj)->isEnabled())
//         {
//             QWidget* tabbar = static_cast<QWidget*>(obj);
//             hoverWidget = tabbar;
//             hoverTab = 0;
//             tabbar->repaint(false);
//         }
//         else if (ev->type() == QEvent::MouseMove)
//         {
//             QTabBar *tabbar = dynamic_cast<QTabBar*>(obj);
//             QMouseEvent *me = dynamic_cast<QMouseEvent*>(ev);
// 
//             if (tabbar && me) {
//                 // avoid unnecessary repaints (which otherwise would occour on every
//                 // MouseMove event causing high cpu load).
// 
//                 bool repaint = true;
// 
//                 QTab *tab = tabbar->selectTab(me->pos() );
//                 if (hoverTab == tab)
//                     repaint = false;
//                 hoverTab = tab;
// 
//                 if (repaint)
//                     tabbar->repaint(false);
//             }
//         }
//         else if (ev->type() == QEvent::Leave)
//         {
//             QWidget* tabbar = static_cast<QWidget*>(obj);
//             hoverWidget = 0;
//             hoverTab = 0;
//             tabbar->repaint(false);
//         }
//         return false;
//     }
//     // Track show events for progress bars
//     if ( _animateProgressBar && qobject_cast<Q3ProgressBar>(obj) )
//     {
//         if ((ev->type() == QEvent::Show) && !animationTimer->isActive())
//         {
//             animationTimer->start( 50, false );
//         }
//     }
//     if ( !qstrcmp(obj->name(), "kde toolbar widget") )
//     {
//         QWidget* lb = static_cast<QWidget*>(obj);
//         if (lb->backgroundMode() == Qt::PaletteButton)
//             lb->setBackgroundMode(Qt::PaletteBackground);
//         lb->removeEventFilter(this);
//     }
// 
//     return false;
// }
// 
QColor PlastikStyle::getColor(const QPalette &pal, const ColorType t, const bool enabled)const
{
    return getColor(pal, t, enabled?IsEnabled:IsDisabled);
}

QColor PlastikStyle::getColor(const QPalette &pal, const ColorType t, const WidgetState s)const
{
    const bool enabled = (s != IsDisabled) &&
            ((s == IsEnabled) || (s == IsPressed) || (s == IsHighlighted));
    const bool pressed = (s == IsPressed);
    const bool highlighted = (s == IsHighlighted);
    switch(t) {
        case ButtonContour:
            return enabled ? pal.button().color().dark(130+_contrast*8)
            : pal.background().color().dark(120+_contrast*8);
        case DragButtonContour: {
            if(enabled) {
                if(pressed)
                    return pal.button().color().dark(130+_contrast*6); // bright
                else if(highlighted)
                    return pal.button().color().dark(130+_contrast*9); // dark
                else
                    return pal.button().color().dark(130+_contrast*8); // normal
            } else {
                return pal.background().color().dark(120+_contrast*8);
            }
        }
        case DragButtonSurface: {
            if(enabled) {
                if(pressed)
                    return pal.button().color().dark(100-_contrast);  // bright
                else if(highlighted)
                    return pal.button().color().light(100+_contrast); // dark
                else
                    return pal.button().color();                      // normal
            } else {
                return pal.background().color();
            }
        }
        case PanelContour:
            return pal.background().color().dark(160+_contrast*8);
        case PanelDark:
            return alphaBlendColors(pal.background().color(), pal.background().color().dark(120+_contrast*5), 110);
        case PanelDark2:
            return alphaBlendColors(pal.background().color(), pal.background().color().dark(110+_contrast*5), 110);
        case PanelLight:
            return alphaBlendColors(pal.background().color(), pal.background().color().light(120+_contrast*5), 110);
        case PanelLight2:
            return alphaBlendColors(pal.background().color(), pal.background().color().light(110+_contrast*5), 110);
        case MouseOverHighlight:
            if( _customOverHighlightColor )
                return _overHighlightColor;
            else
                return pal.highlight();
        case FocusHighlight:
            if( _customFocusHighlightColor )
                return _focusHighlightColor;
            else
                return pal.highlight();
        case CheckMark:
            if( _customCheckMarkColor )
                return _checkMarkColor;
            else
                return pal.foreground().color();
        default:
            return pal.background().color();
    }
}
