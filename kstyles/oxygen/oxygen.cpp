/* Oxygen widget style for KDE 4
   Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>

    based on the plastik style which is:

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

#include "oxygen.h"
#include "oxygen.moc"

#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QSettings>
#include <QtGui/QStyleOption>

#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QToolBar>
#include <QtGui/QScrollBar>
#include <QtGui/QGroupBox>

#include <kglobal.h>
#include <ksharedconfig.h>
#include <kcolorutils.h>
#include <kcolorscheme.h>

#include "helper.h"
#include "tileset.h"

K_EXPORT_STYLE("Oxygen", OxygenStyle)

K_GLOBAL_STATIC_WITH_ARGS(OxygenStyleHelper, globalHelper, ("OxygenStyle"))

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


OxygenStyle::OxygenStyle() :
//     kickerMode(false),
//     kornMode(false),
    flatMode(false),
    _helper(*globalHelper)
{
    _config = _helper.config();

    setWidgetLayoutProp(WT_Generic, Generic::DefaultFrameWidth, 2);

    // TODO: change this when double buttons are implemented
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::DoubleBotButton, true);
    setWidgetLayoutProp(WT_ScrollBar, ScrollBar::MinimumSliderHeight, 21);

    setWidgetLayoutProp(WT_PushButton, PushButton::DefaultIndicatorMargin, 1);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Left, 16);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Right, 16);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Top, 1);
    setWidgetLayoutProp(WT_PushButton, PushButton::ContentsMargin + Bot, 1);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin, 3);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Left, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Right, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Top, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::FocusMargin + Bot, 2);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftHorizontal, 0);
    setWidgetLayoutProp(WT_PushButton, PushButton::PressedShiftVertical,   0);

    setWidgetLayoutProp(WT_Splitter, Splitter::Width, 6);

    setWidgetLayoutProp(WT_CheckBox, CheckBox::Size, 23);
    setWidgetLayoutProp(WT_RadioButton, RadioButton::Size, 23);

    setWidgetLayoutProp(WT_MenuBar, MenuBar::ItemSpacing, 6);

    setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin, 3);
    setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin+Left, 3);
    setWidgetLayoutProp(WT_MenuBarItem, MenuBarItem::Margin+Right, 3);

    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckAlongsideIcon, 1);
    setWidgetLayoutProp(WT_MenuItem, MenuItem::CheckWidth, 13);

    setWidgetLayoutProp(WT_ProgressBar, ProgressBar::BusyIndicatorSize, 10);

    setWidgetLayoutProp(WT_TabBar, TabBar::TabOverlap, 1);
    setWidgetLayoutProp(WT_TabBar, TabBar::BaseOverlap, 8);

    setWidgetLayoutProp(WT_TabWidget, TabWidget::ContentsMargin, 6);

    setWidgetLayoutProp(WT_Slider, Slider::HandleThickness, 20/*15*/);
    setWidgetLayoutProp(WT_Slider, Slider::HandleLength, 11);

    setWidgetLayoutProp(WT_SpinBox, SpinBox::FrameWidth, 2);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonWidth, 2+16+1);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonSpacing, 0);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Left, 0);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Right, 3);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Top, 3);
    setWidgetLayoutProp(WT_SpinBox, SpinBox::ButtonMargin+Bot, 3);

    setWidgetLayoutProp(WT_ComboBox, ComboBox::FrameWidth, 3);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonWidth, 19);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Left, 0);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Right, 5);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Top, 4);
    setWidgetLayoutProp(WT_ComboBox, ComboBox::ButtonMargin+Bot, 2);

    setWidgetLayoutProp(WT_ToolBar, ToolBar::FrameWidth, 0);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemSpacing, 1);
    setWidgetLayoutProp(WT_ToolBar, ToolBar::ItemMargin, 0);

    setWidgetLayoutProp(WT_ToolButton, ToolButton::ContentsMargin, 6);
    setWidgetLayoutProp(WT_ToolButton, ToolButton::FocusMargin,    3);

    setWidgetLayoutProp(WT_GroupBox, GroupBox::FrameWidth, 5);

    QSettings settings;
    // TODO get from KGlobalSettings::contrastF or expose in OxygenHelper
    _contrast = settings.value("/Qt/KDE/contrast", 6).toInt();
    settings.beginGroup("/oxygenstyle/Settings");
    _scrollBarLines = settings.value("/scrollBarLines", false).toBool();
    _animateProgressBar = settings.value("/animateProgressBar", true).toBool();
    _drawToolBarItemSeparator = settings.value("/drawToolBarItemSeparator", true).toBool();
    _drawFocusRect = settings.value("/drawFocusRect", true).toBool();
    _drawTriangularExpander = settings.value("/drawTriangularExpander", false).toBool();
    _inputFocusHighlight = settings.value("/inputFocusHighlight", true).toBool();
    // FIXME below this line to be deleted (and can we not use QSettings? KConfig* is safe now)
    _customOverHighlightColor = true;
    _customFocusHighlightColor = true;
    // do next two lines in polish()?
    KColorScheme schemeView( KColorScheme::View, _config );
    _viewHoverColor = _overHighlightColor = schemeView.decoration( KColorScheme::HoverColor ).color();
    _viewFocusColor = _focusHighlightColor = schemeView.decoration( KColorScheme::FocusColor ).color();
    settings.endGroup();

    // setup pixmap cache...
    pixmapCache = new QCache<int, CacheEntry>(327680);

    if ( _animateProgressBar )
    {
        animationTimer = new QTimer( this );
        connect( animationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()) );
    }

}


void OxygenStyle::updateProgressPos()
{
    QProgressBar* pb;
    //Update the registered progressbars.
    QMap<QWidget*, int>::iterator iter;
    bool visible = false;
    for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); ++iter)
    {
        pb = dynamic_cast<QProgressBar*>(iter.key());

        if ( !pb )
            continue;

        if ( iter.key() -> isEnabled() &&
             pb->value() != pb->maximum() )
        {
            // update animation Offset of the current Widget
            iter.value() = (iter.value() + 1) % 20;
            iter.key()->update();
        }
        if (iter.key()->isVisible())
            visible = true;
    }
    if (!visible)
        animationTimer->stop();
}


OxygenStyle::~OxygenStyle()
{
    delete pixmapCache;
}


void OxygenStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
        case PE_Widget:
        {
            if (!widget)
                return;

            if (widget->isWindow()) {
            QColor color = option->palette.color(widget->backgroundRole());
            int splitY = qMin(300, 3*option->rect.height()/4);

            QRect upperRect = QRect(0, 0, option->rect.width(), splitY);
            QPixmap tile = _helper.verticalGradient(color, splitY);
            painter->drawTiledPixmap(upperRect, tile);

            QRect lowerRect = QRect(0,splitY, option->rect.width(), option->rect.height() - splitY);
            painter->fillRect(lowerRect, _helper.backgroundBottomColor(color));

            int radialW = qMin(600, option->rect.width());
            int frameH = widget->geometry().y() - widget->y();
            tile = _helper.radialGradient(color, radialW);
            QRect radialRect = QRect((option->rect.width() - radialW) / 2, 0, radialW, 64-frameH);
            painter->drawPixmap(radialRect, tile, QRect(0, frameH, radialW, 64-frameH));
            }

            if (qobject_cast<const QGroupBox*>(widget)) {
                //painter->fillRect(option->rect, QColor(Qt::blue));
            }

            break;
        }

        default:
            KStyle::drawPrimitive(element, option, painter, widget);
    }
}


void OxygenStyle::drawKStylePrimitive(WidgetType widgetType, int primitive,
                                       const QStyleOption* opt,
                                       const QRect &r, const QPalette &pal,
                                       State flags, QPainter* p,
                                       const QWidget* widget,
                                       KStyle::Option* kOpt) const
{
    const bool reverseLayout = opt->direction == Qt::RightToLeft;

    const bool enabled = flags & State_Enabled;
    const bool mouseOver(enabled && (flags & State_MouseOver));

    switch (widgetType)
    {

        case WT_PushButton:
        {
            switch (primitive)
            {
                case PushButton::Panel:
                {
                    bool sunken   = (flags & State_On) || (flags & State_Sunken);
                    bool hasFocus = flags & State_HasFocus;

                    renderSlab(p, r, sunken, hasFocus, mouseOver);
                    return;
                }

                case PushButton::DefaultButtonFrame:
                {
                    return;
                }
            }
        }
        break;

        case WT_ToolBoxTab:
        {
            switch (primitive)
            {
                case ToolBoxTab::Panel:
                {
                    bool sunken   = (flags & State_On) || (flags & State_Sunken) || (flags & State_Selected);

//                    renderButton(p, r, pal, sunken, mouseOver);

                    return;
                }
            }
        }
        break;

        case WT_ProgressBar:
        {
//             const Q3ProgressBar *pb = dynamic_cast<const Q3ProgressBar*>(widget);
//             int steps = pb->totalSteps();

            QColor bg = enabled?pal.color(QPalette::Base):pal.color(QPalette::Background); // background
            QColor fg = enabled?pal.color(QPalette::Highlight):pal.color(QPalette::Background).dark(110); // foreground


            switch (primitive)
            {
                case ProgressBar::Groove:
                {
                    QColor bg = enabled?pal.color(QPalette::Base):pal.color(QPalette::Background); // background

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
                    return;
                }

                case ProgressBar::Indicator:
                {
                    return;
                }
            }
        }
        break;

        case WT_MenuBar:
        {
            switch (primitive)
            {
                case MenuBar::EmptyArea:
                {
                    return;
                }
            }
        }
        break;

        case WT_MenuBarItem:
        {
            switch (primitive)
            {
                case MenuBarItem::Panel:
                {
                    bool active  = flags & State_Selected;
                    bool focused = flags & State_HasFocus;
                    bool down = flags & State_Sunken;

                    if (active && focused) {
//                        renderButton(p, r, pal, down, mouseOver, true);
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
                    //FIXME CBR renderContour(p, r, pal.color(QPalette::Background), pal.color(QPalette::Background).dark(200),                          Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
                    return;
                }

                case Menu::Background:
                {
                    p->fillRect( r, pal.color(QPalette::Background).light( 105 ) );
                    return;
                }

                case Menu::TearOff:
                {
                    // TODO: See Keramik...

                    return;
                }

                case Menu::Scroller:
                {
                    // TODO
                    return;
                }
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
                    p->setPen( pal.color( QPalette::Light ) );
                    p->drawLine( r.x()+5, r.y() + 1, r.right()-5 , r.y() + 1 );

                    return;
                }

                case MenuItem::ItemIndicator:
                {
                    if (enabled) {
                        p->fillRect(r, pal.color(QPalette::Highlight));
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
                    renderCheckBox(p, r, pal, enabled, false, mouseOver, CheckBox::CheckOn);
                    return;
                }

                case MenuItem::CheckOff:
                {
                    renderCheckBox(p, r, pal, enabled, false, mouseOver, CheckBox::CheckOff);
                    return;
                }

                case MenuItem::RadioOn:
                {
                    renderRadioButton(p, r, pal, enabled, mouseOver, RadioButton::RadioOn);
                    return;
                }

                case MenuItem::RadioOff:
                {
                    renderRadioButton(p, r, pal, enabled, mouseOver, RadioButton::RadioOff);
                    return;
                }

                case MenuItem::CheckIcon:
                {
                    // TODO
                    return;
                }
            }
        }
        break;

        case WT_DockWidget:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    //FIXME CBRrenderContour(p, r, pal.color( QPalette::Background ),                                  pal.color( QPalette::Background ).dark(160),                                  Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);

                    return;
                }

               case DockWidget::SeparatorHandle:
                    if (flags&State_Horizontal)
                        drawKStylePrimitive(WT_Splitter, Splitter::HandleVert, opt, r, pal, flags, p, widget);
                    else
                        drawKStylePrimitive(WT_Splitter, Splitter::HandleHor, opt, r, pal, flags, p, widget);
                    return;
            }
        }
        break;

        case WT_StatusBar:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    return;
                }
            }
        }
        break;

        case WT_CheckBox:
        {
            switch(primitive)
            {
                case CheckBox::CheckOn:
                case CheckBox::CheckOff:
                case CheckBox::CheckTriState:
                {
                    bool hasFocus = flags & State_HasFocus;

                    renderCheckBox(p, r, pal, enabled, hasFocus, mouseOver, primitive);
                    return;
                }
            }
        }
        break;

        case WT_RadioButton:
        {
            switch(primitive)
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

        case WT_ScrollBar:
        {
            switch (primitive)
            {
                case ScrollBar::DoubleButtonHor:
                case ScrollBar::DoubleButtonVert:
                break;

                case ScrollBar::SingleButtonHor:
                break;

                case ScrollBar::SingleButtonVert:
                    renderHole(p, QRect(0,r.bottom()+1,r.width(),3), false,false, TileSet::Top | TileSet::Left | TileSet::Right);
                break;

                case ScrollBar::GrooveAreaVertTop:
                {
                    renderHole(p, r.adjusted(0,3,0,3), false,false, TileSet::Left | TileSet::Right);
                    return;
                }

                case ScrollBar::GrooveAreaVertBottom:
                {
                    renderHole(p, r.adjusted(0,-3,0,0), false,false, TileSet::Left | TileSet::Bottom | TileSet::Right);
                    return;
                }

                case ScrollBar::GrooveAreaHorLeft:
                {
                    renderHole(p, r, false,false, TileSet::Top | TileSet::Left | TileSet::Bottom);
                    return;
                }

                case ScrollBar::GrooveAreaHorRight:
                {
                    renderHole(p, r, false,false, TileSet::Top | TileSet:: Right | TileSet::Bottom);
                    return;
                }

                case ScrollBar::SliderVert:
                {
                    renderHole(p, r.adjusted(0,3,0,-3), false,false, TileSet::Left | TileSet::Right);

                    int offset = r.top()/2; // divide by 2 to make the "lightplay" move half speed of the handle
                    if(r.height()-2 <= 32)
                        _helper.verticalScrollBar(QColor(0,116,0), r.width()-2,
                                    r.height()-2, offset)->render(r.adjusted(1,1,-1,-1 ), p, TileSet::Full);
                    else
                    {
                        // When the handle is 32 pixels high or more then we paint it in two parts
                        // This is needed since the middle part is tiled and that doesn't work nice with lightplay
                        // So to fix this we paint the bottom part with a modified offset
                        _helper.verticalScrollBar(QColor(0,116,0), r.width()-2,
                                    64, offset)->render(r.adjusted(1,1,-1,-1-16 ), p, TileSet::Top | TileSet::Left
                                    | TileSet::Right | TileSet::Center);
                        _helper.verticalScrollBar(QColor(0,116,0), r.width()-2, 64, offset+r.height())
                                    ->render(QRect(r.left()+1,r.bottom()-16,r.width()-2,16), p, TileSet::Left
                                    | TileSet::Right | TileSet::Bottom);
                    }
                    return;
                }

                case ScrollBar::SliderHor:
                {
                    renderHole(p, r, false,false, TileSet::Top | TileSet::Bottom);
                    p->fillRect(r.adjusted(2, 2, -2, -2), QColor(Qt::red));
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
                    if (!tabOpt) break;

                    QStyleOptionTab::TabPosition pos = tabOpt->position;
                    bool bottom = primitive == TabBar::SouthTab;
                    bool cornerWidget = reverseLayout ?
                            (tabOpt->cornerWidgets&QStyleOptionTab::LeftCornerWidget) :
                            (tabOpt->cornerWidgets&QStyleOptionTab::RightCornerWidget);

                    // TODO: tab painting needs a lot of work in order to handle east and west tabs.
                    renderTab(p, r, pal, mouseOver, flags&State_Selected, bottom, pos, false, cornerWidget, reverseLayout);

                    return;
                }
                case TabBar::BaseFrame:
                    //p->fillRect(r,QColor(Qt::red));
                    return;

                // TODO: TabBar::EastTab, TabBar::WestTab, TabBar::ScrollButton
            }

        }
        break;

        case WT_TabWidget:
        {
            switch (primitive)
            {
                case Generic::Frame:
                {
                    const QStyleOptionTabWidgetFrame* tabOpt = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(opt);
                    int w = tabOpt->tabBarSize.width();
                    int lw = tabOpt->leftCornerWidgetSize.width();
                    //int h = tabOpt->tabBarSize.height();
                    switch(tabOpt->shape)
                    {
                        case QTabBar::RoundedNorth:
                        case QTabBar::TriangularNorth:
                            renderSlab(p, r.adjusted(0,8,0,0), false, false, false,
                                    TileSet::Left | TileSet::Bottom | TileSet::Right);
                            if(reverseLayout)
                            {
                                // Left and right widgets are placed right and left when in reverse mode

                                if (w+lw >0)
                                    renderSlab(p, QRect(0, r.y(), r.width() - w - lw, 8), false, false, false,
                                        TileSet::Left | TileSet::Top);
                                else
                                    renderSlab(p, QRect(0, r.y(), r.width(), 8), false, false, false,
                                            TileSet::Left | TileSet::Top | TileSet::Right);

                                if (lw > 0)
                                    renderSlab(p, QRect(r.right() - lw, r.y(), lw, 8), false, false, false,
                                        TileSet::Top | TileSet::Right);
                            }
                            else
                            {
                                if (lw > 0)
                                    renderSlab(p, QRect(0, r.y(), lw, 8), false, false, false,
                                        TileSet::Left | TileSet::Top);

                                if (w+lw >0)
                                    renderSlab(p, QRect(w+lw, r.y(), r.width() - w - lw, 8), false, false, false,
                                            TileSet::Top | TileSet::Right);
                                else
                                    renderSlab(p, QRect(0, r.y(), r.width(), 8), false, false, false,
                                            TileSet::Left | TileSet::Top | TileSet::Right);

                            }
                            return;

                        default:
                            return;
                    }
                }

            }
        }
        break;

        case WT_Window:
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

        case WT_Splitter:
        {
            switch (primitive)
            {
                case Splitter::HandleHor:
                {
                    int h = r.height();
                    QColor color = pal.color(QPalette::Background);

                    int ngroups = qMax(1,h / 250);
                    int center = (h - (ngroups-1) * 250) /2 + r.top();
                    for(int k = 0; k < ngroups; k++, center += 250) {
                        renderDot(p, QPointF(r.left()+3, center-5), color);
                        renderDot(p, QPointF(r.left()+3, center), color);
                        renderDot(p, QPointF(r.left()+3, center+5), color);
                    }
                    return;
                }
                case Splitter::HandleVert:
                {
                    int w = r.width();
                    QColor color = pal.color(QPalette::Background);

                    int ngroups = qMax(1, w / 250);
                    int center = (w - (ngroups-1) * 250) /2 + r.left();
                    for(int k = 0; k < ngroups; k++, center += 250) {
                        renderDot(p, QPointF(center-5, r.top()+3), color);
                        renderDot(p, QPointF(center, r.top()+3), color);
                        renderDot(p, QPointF(center+5, r.top()+3), color);
                    }
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
                    renderSlab(p, r);
                    return;
                }

                case Slider::GrooveHor:
                case Slider::GrooveVert:
                {

                    bool horizontal = primitive == Slider::GrooveHor;

                    if (horizontal) {
                        int center = r.y()+r.height()/2;
                        renderHole(p, QRect(r.left(), center-2, r.width(), 4));
                    } else {
                        int center = r.x()+r.width()/2;
                        renderHole(p, QRect(center-2, r.top(), 4, r.height()));
                    }

                    return;
                }
            }

        }
        break;

        case WT_SpinBox:
        {
            bool hasFocus = flags & State_HasFocus;

            const QColor inputColor = enabled?pal.color(QPalette::Base):pal.color(QPalette::Background);

            switch (primitive)
            {
                case Generic::Frame:
                {
                    p->fillRect(opt->rect.adjusted(1,1,-1,-1), inputColor );
                    renderHole(p, r, hasFocus, mouseOver);
                    return;
                }
                case SpinBox::EditField:
                case SpinBox::ButtonArea:
                case SpinBox::UpButton:
                case SpinBox::DownButton:
                {
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

            const QColor buttonColor = enabled?pal.color(QPalette::Button):pal.color(QPalette::Background);
            const QColor inputColor = enabled ? pal.color(QPalette::Base) : pal.color(QPalette::Background);
            QRect editField = subControlRect(CC_ComboBox, qstyleoption_cast<const QStyleOptionComplex*>(opt), SC_ComboBoxEditField, widget);

            switch (primitive)
            {
                case Generic::Frame:
                {
                    // TODO: pressed state
                    if(!editable) {
                        renderSlab(p, r, false, hasFocus, mouseOver);
                    } else {
                        // input area
                        p->fillRect(r.adjusted(1,1,-1,-1), inputColor );

                        if (_inputFocusHighlight && hasFocus && enabled)
                        {
                            renderHole(p, r, true, mouseOver);
                        }
                        else
                        {
                            renderHole(p, r);
                        }
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

                        p->setPen(pal.color(QPalette::Button));
                        p->drawRect(QRect(isFirst?r.left()+1:r.left(), r.top()+1, isFirst?r.width()-2:r.width()-1, r.height()-2));
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
                    const bool isReadOnly = flags & State_ReadOnly;
                    const bool isEnabled = flags & State_Enabled;
                    const bool hasFocus = flags & State_HasFocus;

                    if ( _inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
                    {
                        renderHole(p, r, true, mouseOver);
                    }
                    else
                    {
                        renderHole(p, r, false, mouseOver);
                    }
                    return;
                }

                case LineEdit::Panel:
                {
                    const QColor inputColor =
                                enabled?pal.color(QPalette::Base):pal.color(QPalette::Background);

                    if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame*>(opt))
                    {
                        const int lineWidth(panel->lineWidth);

                        if (lineWidth > 0)
                        {
                            p->fillRect(r.adjusted(lineWidth,lineWidth,-lineWidth,-lineWidth), inputColor);
                            drawPrimitive(PE_FrameLineEdit, panel, p, widget);
                        }
                        else
                            p->fillRect(r.adjusted(2,2,-2,-2), inputColor);
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
                    renderSlab(p, r.adjusted(0,0,0,-8), false, false, false,
                            TileSet::Left | TileSet::Top | TileSet::Right);
                    p->fillRect(QRect(5, r.bottom()-7, r.width()-10, 1), QColor(0,0,0,2));
                    p->fillRect(QRect(5, r.bottom()-6, r.width()-10, 6), QColor(0,0,0,5));
                    p->fillRect(QRect(5, r.bottom(), r.width()-10, 1), QColor(0,0,0,2));
                    _helper.slope(QColor(255,255,255))->render(QRect(0, r.bottom()-7, r.width(), 8), p, TileSet::Left | TileSet::Right);
                    return;
                }
            }

        }
        break;

        case WT_ToolBar:
        {
            switch (primitive)
            {
                case ToolBar::HandleHor:
                {
                    int counter = 1;

                        int center = r.left()+r.width()/2;
                        for(int j = r.top()+2; j <= r.bottom()-3; j+=3) {
                            if(counter%2 == 0) {
                                renderDot(p, QPoint(center+1, j), pal.color(QPalette::Background));
                            } else {
                                renderDot(p, QPoint(center-2, j), pal.color(QPalette::Background));
                            }
                            counter++;
                        }
                    return;
                }
                case ToolBar::HandleVert:
                {
                    int counter = 1;

                        int center = r.top()+r.height()/2;
                        for(int j = r.left()+2; j <= r.right()-3; j+=3) {
                            if(counter%2 == 0) {
                                renderDot(p, QPoint(j, center+1), pal.color(QPalette::Background));
                            } else {
                                renderDot(p, QPoint(j, center-2), pal.color(QPalette::Background));
                            }
                            counter++;
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
                        } else {
                            int center = r.top()+r.height()/2;
                            p->setPen( getColor(pal, PanelDark) );
                            p->drawLine( r.x()+3, center-1, r.right()-3, center-1 );
                        }
                    }

                    return;
                }
            }
        }
        break;

        case WT_ToolButton:
        {
            switch (primitive)
            {
                case ToolButton::Panel:
                {
//                    renderButton(p, r, pal, flags&State_Sunken||flags&State_On,
 //                                false, true, flags&State_Enabled);

                    return;
                }
            }

        }
        break;

    }


    // Arrows
    if (primitive >= Generic::ArrowUp && primitive <= Generic::ArrowLeft) {
        QPolygonF a;

        switch (primitive) {
            case Generic::ArrowUp: {
                a.clear();
                a << QPointF(0, -3.5) << QPointF(4.5, 3.5) << QPointF(-4.5, 3.5);
                break;
            }
            case Generic::ArrowDown: {
                a.clear();
                a << QPointF(0, 3.5) << QPointF(4.5, -3.5) << QPointF(-4.5, -3.5);
                break;
            }
            case Generic::ArrowLeft: {
                a.clear();
                a << QPointF(-3.5, 0) << QPointF(3.5, -4.5) << QPointF(3.5, 4.5);
                break;
            }
            case Generic::ArrowRight: {
                a.clear();
                a << QPointF(3.5, 0) << QPointF(-3.5, -4.5) << QPointF(-3.5, 4.5);
                break;
            }
        }

        a.translate((r.x()+r.width()/2), (r.y()+r.height()/2));
        KStyle::ColorOption* colorOpt   = extractOption<KStyle::ColorOption*>(kOpt);
        QColor               arrowColor = colorOpt->color.color(pal);

        QPen oldPen(p->pen()); // important to save the pen as combobox assumes we don't touch
        p->setPen(Qt::NoPen);
        p->setBrush(arrowColor);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawPolygon(a);
        p->setRenderHint(QPainter::Antialiasing, false);
        p->setPen(oldPen);
        return;
    }

    switch (primitive)
    {
        case Generic::Frame:
        {
            // WT_Generic and other fallen-through frames...
            // QFrame, Qt item views, etc.: sunken..
            bool focusHighlight = flags&State_HasFocus/* && flags&State_Enabled*/;
            if (flags & State_Sunken) {
                renderHole(p, r, focusHighlight);
            } else if (flags & State_Raised) {
                renderPanel(p, r, pal, true, false, focusHighlight);
            } else {
                renderPanel(p, r, pal, false);
            }

            return;
        }

        case Generic::FocusIndicator:
            // we don't want the stippled focus indicator in oxygen
            return;

        default:
            break;
    }

    // default fallback
    KStyle::drawKStylePrimitive(widgetType, primitive, opt,
                                r, pal, flags, p, widget, kOpt);
}

void OxygenStyle::polish(QWidget* widget)
{
    if (widget->isWindow())
        widget->setAttribute(Qt::WA_StyledBackground);

    if (qobject_cast<const QGroupBox*>(widget))
        widget->setAttribute(Qt::WA_StyledBackground);

    if( _animateProgressBar && qobject_cast<QProgressBar*>(widget) )
    {
        widget->installEventFilter(this);
        progAnimWidgets[widget] = 0;
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
        if (!animationTimer->isActive()) {
            animationTimer->setSingleShot( false );
            animationTimer->start( 50 );
        }
    }

    if (qobject_cast<QPushButton*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QCheckBox*>(widget)
        || qobject_cast<QRadioButton*>(widget)
        || qobject_cast<QTabBar*>(widget)
        ) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (qobject_cast<QMenuBar*>(widget)
        || widget->inherits("Q3ToolBar")
        || qobject_cast<QToolBar*>(widget)
        || (widget && qobject_cast<QToolBar *>(widget->parent())) )
    {
        widget->setBackgroundRole(QPalette::Background);
    }

    if (qobject_cast<QScrollBar*>(widget))
    {
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

    KStyle::polish(widget);
}

void OxygenStyle::unpolish(QWidget* widget)
{
    if ( qobject_cast<QProgressBar*>(widget) )
    {
        progAnimWidgets.remove(widget);
    }

    if (qobject_cast<QPushButton*>(widget)
        || qobject_cast<QComboBox*>(widget)
        || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QCheckBox*>(widget)
        || qobject_cast<QRadioButton*>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if (qobject_cast<QMenuBar*>(widget)
        || (widget && widget->inherits("Q3ToolBar"))
        || qobject_cast<QToolBar*>(widget)
        || (widget && qobject_cast<QToolBar *>(widget->parent())) )
    {
        widget->setBackgroundRole(QPalette::Button);
    }

    if (qobject_cast<QScrollBar*>(widget))
    {
        widget->setAttribute(Qt::WA_OpaquePaintEvent);
    }

    KStyle::unpolish(widget);
}

void OxygenStyle::progressBarDestroyed(QObject* obj)
{
    progAnimWidgets.remove(static_cast<QWidget*>(obj));
}

void OxygenStyle::renderSlab(QPainter *p, const QRect &r, bool sunken, bool focus, bool hover, TileSet::PosFlags posFlags) const
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    TileSet *tile;
    QColor base = QColor(Qt::white);
    // for slabs, hover takes precedence over focus (other way around for holes)
    // but in any case if the button is sunken we don't show focus nor hover
    if(sunken)
        tile = _helper.slabSunken(base);
    else if (hover)
        tile = _helper.slabFocused(base, _viewHoverColor);
    else if (focus)
        tile = _helper.slabFocused(base, _viewFocusColor);
    else
        tile = _helper.slab(base);
    tile->render(r, p, posFlags);
}

void OxygenStyle::renderHole(QPainter *p, const QRect &r, bool focus, bool hover, TileSet::PosFlags posFlags) const
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    TileSet *tile;
    QColor base = QColor(Qt::white);
    // for holes, focus takes precedence over hover (other way around for buttons)
    if (focus)
        tile = _helper.holeFocused(base, _viewFocusColor);
    else if (hover)
        tile = _helper.holeFocused(base, _viewHoverColor);
    else
        tile = _helper.hole(base);
    tile->render(r, p, posFlags);
}

void OxygenStyle::renderCheckBox(QPainter *p, const QRect &rect, const QPalette &pal,
                                  bool enabled, bool hasFocus, bool mouseOver, int primitive) const
{
    QColor contentColor = enabled?pal.color(QPalette::Base):pal.color(QPalette::Background);

    int s = qMin(rect.width(), rect.height());
    QRect r = centerRect(rect, s, s);

    renderSlab(p, r, false, hasFocus, mouseOver);

    // check mark
    double x = r.center().x() - 3.5, y = r.center().y() - 2.5;

    QPen pen(pal.color(QPalette::Text), 2.0);
    if (primitive == CheckBox::CheckTriState) {
        QVector<qreal> dashes;
        dashes << 1.0 << 2;
        pen.setWidthF(1.3);
        pen.setDashPattern(dashes);
    }

    if (primitive != CheckBox::CheckOff)
    {
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(pen);
        p->drawLine(QPointF(x+9, y), QPointF(x+3,y+7));
        p->drawLine(QPointF(x, y+4), QPointF(x+3,y+7));
        p->setRenderHint(QPainter::Antialiasing, false);
    }
}

void OxygenStyle::renderRadioButton(QPainter *p, const QRect &r, const QPalette &pal,
                                        bool enabled, bool mouseOver, int prim) const
{
    int x = r.x();
    int y = r.y();
    int s = qMin(r.width(), r.height());

    p->drawPixmap(x, y, _helper.roundButton(pal.color(QPalette::Button), s));

    // highlighting...
    if(mouseOver) {
    }

    // draw the radio mark
    switch (prim)
    {
        case RadioButton::RadioOn:
        {
            return;
        }
        case RadioButton::RadioOff:
        {
            // empty
            return;
        }

        default:
            // StateTristate, shouldn't happen...
            return;
    }
}

void OxygenStyle::renderDot(QPainter *p, const QPointF &point, const QColor &baseColor) const
{
    Q_UNUSED(baseColor)
    const qreal diameter = 2.5;
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0, 0, 0, 66));
    p->drawEllipse(QRectF(point.x()-diameter/2+0.5, point.y()-diameter/2+0.5, diameter, diameter));
    p->setRenderHint(QPainter::Antialiasing, false);
}

void OxygenStyle::renderPanel(QPainter *p,
                              const QRect &r,
                              const QPalette &pal,
                              const bool pseudo3d,
                              const bool sunken,
                              const bool focusHighlight) const
{
    int x, x2, y, y2, w, h;
    r.getRect(&x,&y,&w,&h);
    r.getCoords(&x, &y, &x2, &y2);

        if(pseudo3d) {
            QColor dark = focusHighlight ?
                    getColor(pal,FocusHighlight).dark(130) : getColor(pal, PanelDark);
            QColor light = focusHighlight ?
                    getColor(pal,FocusHighlight).light(130) : getColor(pal, PanelLight);
            if (sunken) {
                p->setPen(dark);
            } else {
                p->setPen(light);
            }
            p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1);
            p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2);
            if (sunken) {
                p->setPen(light);
            } else {
                p->setPen(dark);
            }
            p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1);
            p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);
        }
}


void OxygenStyle::renderTab(QPainter *p,
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

    // the top part of the tab which is nearly the same for all positions
    QRect Rc = r.adjusted(0,0,0,-8); // contour

    // the area where the fake border should appear
    const QRect Rb(r.x(), bottom?r.top():Rc.bottom()+1, r.width(), r.height()-Rc.height() );

    if (selected) {
        renderSlab(p, Rc, false, false, false, TileSet::Left | TileSet::Top | TileSet::Right);

        // some "position specific" paintings...
        // First draw the left connection from the panel border to the tab
        if(isFirst && !reverseLayout && !cornerWidget) {
            renderSlab(p, Rb, false, false, false, TileSet::Left);
        } else
            renderHole(p, QRect(Rb.left(), Rb.top(),4,5), false, false, TileSet::Right | TileSet::Bottom);

        // Now draw the right connection from the panel border to the tab
        if(isFirst && reverseLayout && !cornerWidget) {
            renderSlab(p, Rb, false, false, false, TileSet::Right);
        } else
            renderHole(p, QRect(Rb.right()-3, Rb.top(),3,5), false, false, TileSet::Left | TileSet::Bottom);

    } else {
    // inactive tabs
        uint posFlag = TileSet::Top;
        if(isFirst && !reverseLayout && !cornerWidget)
            posFlag |= TileSet::Left;
        if(isFirst && reverseLayout && !cornerWidget)
            posFlag |= TileSet::Right;
        renderSlab(p, QRect(Rb.left(), Rb.y(), Rb.width(), 8), false, false, mouseOver, posFlag);
    }
}

int OxygenStyle::styleHint(StyleHint hint, const QStyleOption * option,
                            const QWidget * widget, QStyleHintReturn * returnData) const
{
    switch (hint) {
        case SH_Menu_SubMenuPopupDelay:
            return 96; // Motif-like delay...

        case SH_ScrollView_FrameOnlyAroundContents:
            return true;

        default:
            return KStyle::styleHint(hint, option, widget, returnData);
    }
}

QRect OxygenStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option,
                                SubControl subControl, const QWidget* widget) const
{
    QRect r = option->rect;

    switch (control)
    {
        case CC_GroupBox:
            switch (subControl)
            {
                case SC_GroupBoxFrame:
                    return r;

                case SC_GroupBoxLabel:
                    return r.adjusted(0,8,0,0);

                default:
                    break;
            }

        case CC_ComboBox:
            switch (subControl)
            {
                case SC_ComboBoxEditField:
                    if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option))
                        if(!cb->editable)
                            return KStyle::subControlRect(control, option, subControl, widget).adjusted(3,0,0,0);
                    break;

                default:
                    break;
            }
        default:
            break;
    }

    return KStyle::subControlRect(control, option, subControl, widget);
}

bool OxygenStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if (KStyle::eventFilter(obj, ev) )
        return true;

    // Track show events for progress bars
    if ( _animateProgressBar && qobject_cast<QProgressBar*>(obj) )
    {
        if ((ev->type() == QEvent::Show) && !animationTimer->isActive())
        {
            animationTimer->start( 50 );
        }
    }

    return false;
}

QColor OxygenStyle::getColor(const QPalette &pal, const ColorType t, const bool enabled)const
{
    return getColor(pal, t, enabled?IsEnabled:IsDisabled);
}

QColor OxygenStyle::getColor(const QPalette &pal, const ColorType t, const WidgetState s)const
{
    const bool enabled = (s != IsDisabled) &&
            ((s == IsEnabled) || (s == IsPressed) || (s == IsHighlighted));
    const bool pressed = (s == IsPressed);
    const bool highlighted = (s == IsHighlighted);
    switch(t) {
        case ButtonContour:
            return enabled ? pal.color(QPalette::Button).dark(130+_contrast*8)
            : pal.color(QPalette::Background).dark(120+_contrast*8);
        case DragButtonContour: {
            if(enabled) {
                if(pressed)
                    return pal.color(QPalette::Button).dark(130+_contrast*6); // bright
                else if(highlighted)
                    return pal.color(QPalette::Button).dark(130+_contrast*9); // dark
                else
                    return pal.color(QPalette::Button).dark(130+_contrast*8); // normal
            } else {
                return pal.color(QPalette::Background).dark(120+_contrast*8);
            }
        }
        case DragButtonSurface: {
            if(enabled) {
                if(pressed)
                    return pal.color(QPalette::Button).dark(100-_contrast);  // bright
                else if(highlighted)
                    return pal.color(QPalette::Button).light(100+_contrast); // dark
                else
                    return pal.color(QPalette::Button);                      // normal
            } else {
                return pal.color(QPalette::Background);
            }
        }
        case PanelContour:
            return pal.color(QPalette::Background).dark(160+_contrast*8);
        case PanelDark:
            return alphaBlendColors(pal.color(QPalette::Background), pal.color(QPalette::Background).dark(120+_contrast*5), 110);
        case PanelDark2:
            return alphaBlendColors(pal.color(QPalette::Background), pal.color(QPalette::Background).dark(110+_contrast*5), 110);
        case PanelLight:
            return alphaBlendColors(pal.color(QPalette::Background), pal.color(QPalette::Background).light(120+_contrast*5), 110);
        case PanelLight2:
            return alphaBlendColors(pal.color(QPalette::Background), pal.color(QPalette::Background).light(110+_contrast*5), 110);
        case MouseOverHighlight:
            if( _customOverHighlightColor )
                return _overHighlightColor;
            else
                return pal.color( QPalette::Highlight );
        case FocusHighlight:
            if( _customFocusHighlightColor )
                return _focusHighlightColor;
            else
                return pal.color( QPalette::Highlight );
        case CheckMark:
            return pal.color( QPalette::Foreground );
        default:
            return pal.color(QPalette::Background);
    }
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

