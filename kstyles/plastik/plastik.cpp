/* Plastik widget style for KDE 3
   Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>

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

#include <qimage.h>
#include <qstylefactory.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtabbar.h>
#include <qprogressbar.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcleanuphandler.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qscrollbar.h>
#include <qstyleplugin.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qvariant.h>
#include <qradiobutton.h>
#include <qregion.h>
#include <qslider.h>
#include <qsettings.h>
#include <kpixmap.h>

#include "plastik.h"
#include "plastik.moc"
#include "misc.h"

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

// popupmenu item constants...
static const int itemHMargin = 6;
static const int itemFrame = 2;
static const int arrowHMargin = 6;
static const int rightBorder = 12;

// -- Style Plugin Interface -------------------------
class PlastikStylePlugin : public QStylePlugin
{
    public:
        PlastikStylePlugin() {}
        ~PlastikStylePlugin() {}

        QStringList keys() const {
            return QStringList() << "Plastik";
        }

        QStyle* create( const QString& key ) {
            if (key.lower() == "plastik")
                return new PlastikStyle;
            return 0;
        }
};

KDE_Q_EXPORT_PLUGIN( PlastikStylePlugin )
// -- end --

PlastikStyle::PlastikStyle() : KStyle( AllowMenuTransparency, ThreeButtonScrollBar),
    kickerMode(false),
    kornMode(false),
    flatMode(false)
{
    hoverWidget = 0;
    hoverTab = 0;

    horizontalDots = 0;
    verticalDots = 0;

    horizontalLine = 0;
    verticalLine = 0;

    QSettings settings;
    _contrast = settings.readNumEntry("/Qt/KDE/contrast", 6);
    settings.beginGroup("/plastikstyle/Settings");
    _scrollBarLines = settings.readBoolEntry("/scrollBarLines", false);
    _animateProgressBar = settings.readBoolEntry("/animateProgressBar", false);
    _drawToolBarSeparator = settings.readBoolEntry("/drawToolBarSeparator", true);
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

    // setup pixmap cache...
    pixmapCache = new QIntCache<CacheEntry>(150000, 499);
    pixmapCache->setAutoDelete(true);

    if ( _animateProgressBar )
    {
        animationTimer = new QTimer( this );
        connect( animationTimer, SIGNAL(timeout()), this, SLOT(updateProgressPos()) );
    }
}


void PlastikStyle::updateProgressPos()
{
    QProgressBar* pb;
    //Update the registered progressbars.
    QMap<QWidget*, int>::iterator iter;
    bool visible = false;
    for (iter = progAnimWidgets.begin(); iter != progAnimWidgets.end(); ++iter)
    {   
        if ( !::qt_cast<QProgressBar*>(iter.key()) )
            continue;
        
        pb = dynamic_cast<QProgressBar*>(iter.key());
        if ( iter.key() -> isEnabled() && 
             pb -> progress() != pb->totalSteps() )
        {
            // update animation Offset of the current Widget
            iter.data() = (iter.data() + 1) % 20;
            iter.key()->update();
        }
        if (iter.key()->isVisible())
            visible = true;
    }
    if (!visible)
        animationTimer->stop();
}


PlastikStyle::~PlastikStyle()
{
    delete pixmapCache;
    delete horizontalDots;
    delete verticalDots;
    delete horizontalLine;
    delete verticalLine;
}

void PlastikStyle::polish(QApplication* app)
{
    if (!qstrcmp(app->argv()[0], "kicker"))
        kickerMode = true;
    else if (!qstrcmp(app->argv()[0], "korn"))
        kornMode = true;
}

void PlastikStyle::polish(QWidget* widget)
{
    if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
        khtmlWidgets[widget] = true;
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(khtmlWidgetDestroyed(QObject*)));
    }

    // use qt_cast where possible to check if the widget inheits one of the classes. might improve
    // performance compared to QObject::inherits()
    if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QComboBox*>(widget) ||
            ::qt_cast<QSpinWidget*>(widget) || ::qt_cast<QSlider*>(widget) ||
            ::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) ||
            ::qt_cast<QToolButton*>(widget) || widget->inherits("QSplitterHandle") )
    {
//         widget->setBackgroundMode(PaletteBackground);
        widget->installEventFilter(this);
    } else if (::qt_cast<QLineEdit*>(widget)) {
        widget->installEventFilter(this);
    } else if (::qt_cast<QTabBar*>(widget)) {
        widget->setMouseTracking(true);
        widget->installEventFilter(this);
    } else if (::qt_cast<QPopupMenu*>(widget)) {
        widget->setBackgroundMode( NoBackground );
    } else if ( !qstrcmp(widget->name(), "kde toolbar widget") ) {
        widget->installEventFilter(this);
    }

    if( _animateProgressBar && ::qt_cast<QProgressBar*>(widget) )
    {
        widget->installEventFilter(this);
        progAnimWidgets[widget] = 0;
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(progressBarDestroyed(QObject*)));
        if (!animationTimer->isActive())
            animationTimer->start( 50, false );
    }

    KStyle::polish(widget);
}

void PlastikStyle::unPolish(QWidget* widget)
{
    if( !strcmp(widget->name(), "__khtml") ) { // is it a khtml widget...?
        khtmlWidgets.remove(widget);
    }

    // use qt_cast to check if the widget inheits one of the classes.
    if ( ::qt_cast<QPushButton*>(widget) || ::qt_cast<QComboBox*>(widget) ||
            ::qt_cast<QSpinWidget*>(widget) || ::qt_cast<QSlider*>(widget) ||
            ::qt_cast<QCheckBox*>(widget) || ::qt_cast<QRadioButton*>(widget) ||
            ::qt_cast<QToolButton*>(widget) || ::qt_cast<QLineEdit*>(widget) ||
            widget->inherits("QSplitterHandle") )
    {
        widget->removeEventFilter(this);
    }
    else if (::qt_cast<QTabBar*>(widget)) {
        widget->setMouseTracking(false);
        widget->removeEventFilter(this);
    } else if (::qt_cast<QPopupMenu*>(widget)) {
        widget->setBackgroundMode( PaletteBackground );
    } else if ( !qstrcmp(widget->name(), "kde toolbar widget") ) {
        widget->removeEventFilter(this);
    }

    if ( ::qt_cast<QProgressBar*>(widget) )
    {
        progAnimWidgets.remove(widget);
    }

    KStyle::unPolish(widget);
}

void PlastikStyle::khtmlWidgetDestroyed(QObject* obj)
{
    khtmlWidgets.remove(static_cast<QWidget*>(obj));
}

void PlastikStyle::progressBarDestroyed(QObject* obj)
{
    progAnimWidgets.remove(static_cast<QWidget*>(obj));
}

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

void PlastikStyle::renderMask(QPainter *p,
                              const QRect &r,
                              const QColor &color,
                              const uint flags) const
{
    if((r.width() <= 0)||(r.height() <= 0))
        return;

    const bool roundUpperLeft = flags&Round_UpperLeft;
    const bool roundUpperRight = flags&Round_UpperRight;
    const bool roundBottomLeft = flags&Round_BottomLeft;
    const bool roundBottomRight = flags&Round_BottomRight;


    p->fillRect (QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2) , color);

    p->setPen(color);
    // sides
    p->drawLine(roundUpperLeft?r.x()+1:r.x(), r.y(),
                roundUpperRight?r.right()-1:r.right(), r.y() );
    p->drawLine(roundBottomLeft?r.x()+1:r.x(), r.bottom(),
                roundBottomRight?r.right()-1:r.right(), r.bottom() );
    p->drawLine(r.x(), roundUpperLeft?r.y()+1:r.y(),
                r.x(), roundBottomLeft?r.bottom()-1:r.bottom() );
    p->drawLine(r.right(), roundUpperLeft?r.y()+1:r.y(),
                r.right(), roundBottomLeft?r.bottom()-1:r.bottom() );
}

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
        if( (cacheEntry = pixmapCache->find(key)) ) {
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
                               const QColorGroup &g,
                               bool sunken,
                               bool mouseOver,
                               bool horizontal,
                               bool enabled,
                               bool khtmlMode) const
{
    // small fix for the kicker buttons...
    if(kickerMode) enabled = true;

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

        renderContour(p, r, g.background(), getColor(g,ButtonContour),
                    contourFlags);
        renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                    g.background(), g.button(), getColor(g,MouseOverHighlight), _contrast, surfaceFlags);
    } else {
        renderContour(p, r, g.background(), g.button().dark(105+_contrast*3),
                      contourFlags);
        renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                      g.background(), g.button(), getColor(g,MouseOverHighlight), _contrast/2, surfaceFlags);

        flatMode = false;
    }

    p->setPen(oldPen);
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
    if( (cacheEntry = pixmapCache->find(key)) ) {
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
                              const QColorGroup &g,
                              const bool pseudo3d,
                              const bool sunken) const
{
    int x, x2, y, y2, w, h;
    r.rect(&x,&y,&w,&h);
    r.coords(&x, &y, &x2, &y2);

    if (kickerMode &&
            p->device() && p->device()->devType() == QInternal::Widget &&
            QCString(static_cast<QWidget*>(p->device())->className()) == "FittsLawFrame") {
    //  Stolen wholesale from Keramik. I don't like it, but oh well.
        if (sunken) {
            const QCOORD corners[] = { x2, y, x2, y2, x, y2, x, y };
            p->setPen(g.background().dark());
            p->drawConvexPolygon(QPointArray(4, corners));
            p->setPen(g.background().light());
            p->drawPolyline(QPointArray(4, corners), 0, 3);
        } else {
            const QCOORD corners[] = { x, y2, x, y, x2, y, x2, y2 };
            p->setPen(g.background().dark());
            p->drawPolygon(QPointArray(4, corners));
            p->setPen(g.background().light());
            p->drawPolyline(QPointArray(4, corners), 0, 3);
        }
    } else {
        renderContour(p, r, g.background(), getColor(g, PanelContour) );

        if(pseudo3d) {
            if (sunken) {
                p->setPen(getColor(g, PanelDark) );
            } else {
                p->setPen(getColor(g, PanelLight) );
            }
            p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1);
            p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2);
            if (sunken) {
                p->setPen(getColor(g, PanelLight) );
            } else {
                p->setPen(getColor(g, PanelDark) );
            }
            p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1);
            p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);
        }
    }
}

void PlastikStyle::renderMenuBlendPixmap( KPixmap &pix, const QColorGroup &cg,
    const QPopupMenu* /* popup */ ) const
{
    pix.fill( cg.background().light(105) );
}

void PlastikStyle::renderTab(QPainter *p,
                            const QRect &r,
                            const QColorGroup &g,
                            bool mouseOver,
                            const bool selected,
                            const bool bottom,
                            const TabPosition pos,
                            const bool triangular,
                            const bool cornerWidget) const
{
    const bool reverseLayout = QApplication::reverseLayout();

    const bool isFirst = (pos == First) || (pos == Single);
    const bool isLast = (pos == Last);
    const bool isSingle = (pos == Single);

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
                      g.background(), getColor(g,PanelContour),
                      contourFlags);

        // surface
        if(!bottom) {
            p->setPen(getColor(g,PanelLight) );
            p->drawLine(Rs.x()+1, Rs.y(), Rs.right()-1, Rs.y() );
            renderGradient(p, QRect(Rs.x(), Rs.y()+1, 1, Rs.height()-1),
                           getColor(g,PanelLight), getColor(g,PanelLight2));
            renderGradient(p, QRect(Rs.right(), Rs.y()+1, 1, Rs.height()-1),
                            getColor(g,PanelDark), getColor(g,PanelDark2));
        } else {
            p->setPen(alphaBlendColors(g.background(), g.background().dark(160), 100) );
            p->drawLine(Rs.x()+1, Rs.bottom(), Rs.right()-1, Rs.bottom() );
            renderGradient(p, QRect(Rs.x(), Rs.y(), 1, Rs.height()-1),
                            getColor(g,PanelLight), getColor(g,PanelLight2));
            renderGradient(p, QRect(Rs.right(), Rs.y(), 1, Rs.height()-1),
                            getColor(g,PanelDark), getColor(g,PanelDark2));
        }

    // some "position specific" paintings...
        // draw parts of the inactive tabs around...
        if(!isSingle) {
            p->setPen(alphaBlendColors(g.background(), getColor(g, ButtonContour), 50) );
            if( (!isFirst&&!reverseLayout) || (!isLast&&reverseLayout) ) {
                p->drawPoint(r.left(), bottom?(triangular?r.bottom()-2:r.bottom()-3):(triangular?r.top()+2:r.top()+3) );
                renderSurface(p, QRect(r.left(), bottom?r.top()+3:(triangular?r.top()+3:r.top()+4), 1, (triangular?r.height()-6:r.height()-7) ),
                            g.background(), g.button(), getColor(g,MouseOverHighlight), _contrast,
                            Draw_Top|Draw_Bottom|Is_Horizontal);
            }
            if( (!isLast&&!reverseLayout) || (!isFirst&&reverseLayout) ) {
                p->drawPoint(r.right(), bottom?(triangular?r.bottom()-2:r.bottom()-3):(triangular?r.top()+2:r.top()+3) );
                renderSurface(p, QRect(r.right(), bottom?r.top()+3:(triangular?r.top()+3:r.top()+4), 1, (triangular?r.height()-6:r.height()-7) ),
                            g.background(), g.button(), getColor(g,MouseOverHighlight), _contrast,
                            Draw_Top|Draw_Bottom|Is_Horizontal);
            }
        }
        // left connection from the panel border to the tab. :)
        if(isFirst && !reverseLayout && !cornerWidget) {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.y(), Rb.x(), Rb.bottom() );
            p->setPen(getColor(g,PanelLight) );
            p->drawLine(Rb.x()+1, Rb.y(), Rb.x()+1, Rb.bottom() );
        } else if(isFirst && reverseLayout && !cornerWidget) {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawLine(Rb.right(), Rb.y(), Rb.right(), Rb.bottom() );
            p->setPen(getColor(g,PanelDark) );
            p->drawLine(Rb.right()-1, Rb.y(), Rb.right()-1, Rb.bottom() );
        }
        // rounded connections to the panel...
        if(!bottom) {
            // left
            if( (!isFirst && !reverseLayout) || (reverseLayout) || (isFirst && !reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.x(), Rb.y());
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.y()+1);
                p->drawPoint(Rb.x()+1, Rb.y());
            }
            // right
            if( (!reverseLayout) || (!isFirst && reverseLayout) || (isFirst && reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.right(), Rb.y());
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.y()+1);
                p->drawPoint(Rb.right()-1, Rb.y());
            }
        } else {
            // left
            if( (!isFirst && !reverseLayout) || (reverseLayout) || (isFirst && !reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.x(), Rb.bottom());
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.bottom()-1);
                p->drawPoint(Rb.x()+1, Rb.bottom());
            }
            // right
            if( (!reverseLayout) || (!isFirst && reverseLayout) || (isFirst && reverseLayout && cornerWidget) ) {
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.right(), Rb.bottom());
                p->setPen( alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
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
                        g.background(), getColor(g, ButtonContour),
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
                        g.background(), g.button(), getColor(g,MouseOverHighlight), _contrast,
                        surfaceFlags);

    // some "position specific" paintings...
        // fake parts of the panel border
        if(!bottom) {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.y(), ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.y());
            p->setPen(getColor(g,PanelLight) );
            p->drawLine(Rb.x(), Rb.y()+1, ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.y()+1 );
        } else {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawLine(Rb.x(), Rb.bottom(), ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.bottom());
            p->setPen(getColor(g,PanelDark) );
            p->drawLine(Rb.x(), Rb.bottom()-1, ((isLast&&!reverseLayout)||(isFirst&&reverseLayout&&cornerWidget))?Rb.right():Rb.right()-1, Rb.bottom()-1 );
        }
        // fake the panel border edge for tabs which are aligned left-most
        // (i.e. only if there is no widget in the corner of the tabwidget!)
        if(isFirst&&!reverseLayout&&!cornerWidget)
        // normal layout
        {
            if (!bottom) {
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.x()+1, Rb.y()+1 );
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.y()+1 );
                p->setPen(g.background() );
                p->drawPoint(Rb.x(), Rb.y() );
                p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x()+1, Rb.y() );
            } else {
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.x()+1, Rb.bottom()-1 );
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x(), Rb.bottom()-1 );
                p->setPen(g.background() );
                p->drawPoint(Rb.x(), Rb.bottom() );
                p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.x()+1, Rb.bottom() );
            }
        } else if(isFirst&&reverseLayout&&!cornerWidget)
        // reverse layout
        {
            if (!bottom) {
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.right()-1, Rb.y()+1 );
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.y()+1 );
                p->setPen(g.background() );
                p->drawPoint(Rb.right(), Rb.y() );
                p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.right()-1, Rb.y() );
            } else {
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
                p->drawPoint(Rb.right()-1, Rb.bottom()-1 );
                p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.right(), Rb.bottom()-1 );
                p->setPen(g.background() );
                p->drawPoint(Rb.right(), Rb.bottom() );
                p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
                p->drawPoint(Rb.right()-1, Rb.bottom() );
            }
        }
    }
}

void PlastikStyle::drawKStylePrimitive(KStylePrimitive kpe,
                                      QPainter *p,
                                      const QWidget* widget,
                                      const QRect &r,
                                      const QColorGroup &cg,
                                      SFlags flags,
                                      const QStyleOption& opt) const
{
    // some "global" vars...
    const bool enabled = (flags & Style_Enabled);

//  SLIDER
//  ------
    switch( kpe ) {
        case KPE_SliderGroove: {
            const QSlider* slider = (const QSlider*)widget;
            bool horizontal = slider->orientation() == Horizontal;

            if (horizontal) {
                int center = r.y()+r.height()/2;
                renderContour(p, QRect(r.left(), center-2, r.width(), 4),
                              cg.background(), cg.background().dark(enabled?150:130),
                              Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
            } else {
                int center = r.x()+r.width()/2;
                renderContour(p, QRect(center-2, r.top(), 4, r.height()),
                              cg.background(), cg.background().dark(enabled?150:130),
                              Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
            }
            break;
        }

        case KPE_SliderHandle: {
                const QSlider* slider = (const QSlider*)widget;
                bool horizontal = slider->orientation() == Horizontal;

                const bool pressed = (flags&Style_Active);
                const WidgetState s = enabled?(pressed?IsPressed:IsEnabled):IsDisabled;
                const QColor contour = getColor(cg,DragButtonContour,s),
                             surface = getColor(cg,DragButtonSurface,s);

                int xcenter = (r.left()+r.right()) / 2;
                int ycenter = (r.top()+r.bottom()) / 2;

                if (horizontal) {
                    renderContour(p, QRect(xcenter-5, ycenter-6, 11, 10),
                                cg.background(), contour,
                                Draw_Left|Draw_Right|Draw_Top|Round_UpperLeft|Round_UpperRight);

                    // manual contour: vertex
                    p->setPen(alphaBlendColors(cg.background(), contour, 50) );
                    p->drawPoint(xcenter-5+1, ycenter+4);
                    p->drawPoint(xcenter+5-1, ycenter+4);
                    p->drawPoint(xcenter-5+2, ycenter+5);
                    p->drawPoint(xcenter+5-2, ycenter+5);
                    p->drawPoint(xcenter-5+3, ycenter+6);
                    p->drawPoint(xcenter+5-3, ycenter+6);
                    p->drawPoint(xcenter-5+4, ycenter+7);
                    p->drawPoint(xcenter+5-4, ycenter+7);
                    // anti-aliasing of the contour... sort of. :)
                    p->setPen(alphaBlendColors(cg.background(), contour, 80) );
                    p->drawPoint(xcenter, ycenter+8);
                    p->setPen(alphaBlendColors(cg.background(), contour, 150) );
                    p->drawPoint(xcenter-5, ycenter+4);
                    p->drawPoint(xcenter+5, ycenter+4);
                    p->drawPoint(xcenter-5+1, ycenter+5);
                    p->drawPoint(xcenter+5-1, ycenter+5);
                    p->drawPoint(xcenter-5+2, ycenter+6);
                    p->drawPoint(xcenter+5-2, ycenter+6);
                    p->drawPoint(xcenter-5+3, ycenter+7);
                    p->drawPoint(xcenter+5-3, ycenter+7);
                    p->setPen(alphaBlendColors(cg.background(), contour, 190) );
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
                                cg.background(), surface, getColor(cg,MouseOverHighlight),
                                _contrast+3, surfaceFlags);
                    renderDot(p, QPoint(xcenter-3, ycenter-3), surface, false, true );
                    renderDot(p, QPoint(xcenter+2,   ycenter-3), surface, false, true );
                    p->setClipping(false);
                } else {
                    renderContour(p, QRect(xcenter-6, ycenter-5, 10, 11),
                                cg.background(), contour,
                                Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_BottomLeft);

                    // manual contour: vertex
                    p->setPen(alphaBlendColors(cg.background(), contour, 50) );
                    p->drawPoint(xcenter+4, ycenter-5+1);
                    p->drawPoint(xcenter+4, ycenter+5-1);
                    p->drawPoint(xcenter+5, ycenter-5+2);
                    p->drawPoint(xcenter+5, ycenter+5-2);
                    p->drawPoint(xcenter+6, ycenter-5+3);
                    p->drawPoint(xcenter+6, ycenter+5-3);
                    p->drawPoint(xcenter+7, ycenter-5+4);
                    p->drawPoint(xcenter+7, ycenter+5-4);
                    // anti-aliasing. ...sort of :)
                    p->setPen(alphaBlendColors(cg.background(), contour, 80) );
                    p->drawPoint(xcenter+8, ycenter);
                    p->setPen(alphaBlendColors(cg.background(), contour, 150) );
                    p->drawPoint(xcenter+4, ycenter-5);
                    p->drawPoint(xcenter+4, ycenter+5);
                    p->drawPoint(xcenter+5, ycenter-5+1);
                    p->drawPoint(xcenter+5, ycenter+5-1);
                    p->drawPoint(xcenter+6, ycenter-5+2);
                    p->drawPoint(xcenter+6, ycenter+5-2);
                    p->drawPoint(xcenter+7, ycenter-5+3);
                    p->drawPoint(xcenter+7, ycenter+5-3);
                    p->setPen(alphaBlendColors(cg.background(), contour, 190) );
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
                                cg.background(), surface, getColor(cg,MouseOverHighlight),
                                _contrast+3, surfaceFlags);
                    renderDot(p, QPoint(xcenter-3, ycenter-3), surface, false, true );
                    renderDot(p, QPoint(xcenter-3,   ycenter+2), surface, false, true );
                    p->setClipping(false);
                }

                break;
            }

        case KPE_ListViewExpander: {
            int radius = (r.width() - 4) / 2;
            int centerx = r.x() + r.width()/2;
            int centery = r.y() + r.height()/2;

            renderContour(p, r, cg.base(), cg.dark(), Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight );

            p->setPen( cg.text() );
            if(!_drawTriangularExpander)
            {
                // plus or minus
                p->drawLine( centerx - radius, centery, centerx + radius, centery );
                if ( flags & Style_On ) // Collapsed = On
                    p->drawLine( centerx, centery - radius, centerx, centery + radius );
            } else if(_drawTriangularExpander) {
              if( flags & Style_On )
                drawPrimitive(PE_ArrowRight, p, QRect(r.x()+1,r.y()+1,r.width(),r.height()), cg,ButtonContour, flags);
              if( flags & Style_Off )
                drawPrimitive(PE_ArrowDown, p, QRect(r.x()+1,r.y()+1,r.width(),r.height()), cg,ButtonContour, flags);
            }

            break;
        }

    // copied and slightly modified from KStyle.
    case KPE_ListViewBranch: {
        // Typical Windows style listview branch element (dotted line).

        // Create the dotline pixmaps if not already created
        if ( !verticalLine )
        {
            // make 128*1 and 1*128 bitmaps that can be used for
            // drawing the right sort of lines.
            verticalLine   = new QBitmap( 1, 129, true );
            horizontalLine = new QBitmap( 128, 1, true );
            QPointArray a( 64 );
            QPainter p2;
            p2.begin( verticalLine );

            int i;
            for( i=0; i < 64; i++ )
                a.setPoint( i, 0, i*2+1 );
            p2.setPen( color1 );
            p2.drawPoints( a );
            p2.end();
            QApplication::flushX();
            verticalLine->setMask( *verticalLine );

            p2.begin( horizontalLine );
            for( i=0; i < 64; i++ )
                a.setPoint( i, i*2+1, 0 );
            p2.setPen( color1 );
            p2.drawPoints( a );
            p2.end();
            QApplication::flushX();
            horizontalLine->setMask( *horizontalLine );
        }

        p->setPen( cg.mid() );

        if (flags & Style_Horizontal)
        {
            int point = r.x();
            int other = r.y();
            int end = r.x()+r.width();
            int thickness = r.height();

            while( point < end )
            {
                int i = 128;
                if ( i+point > end )
                    i = end-point;
                p->drawPixmap( point, other, *horizontalLine, 0, 0, i, thickness );
                point += i;
            }

        } else {
            int point = r.y();
            int other = r.x();
            int end = r.y()+r.height();
            int thickness = r.width();
            int pixmapoffset = (flags & Style_NoChange) ? 0 : 1;	// ### Hackish

            while( point < end )
            {
                int i = 128;
                if ( i+point > end )
                    i = end-point;
                p->drawPixmap( other, point, *verticalLine, 0, pixmapoffset, thickness, i );
                point += i;
            }
        }

        break;
    }

        default:
            KStyle::drawKStylePrimitive(kpe, p, widget, r, cg, flags, opt);
    }
}


void PlastikStyle::drawPrimitive(PrimitiveElement pe,
                                QPainter *p,
                                const QRect &r,
                                const QColorGroup &cg,
                                SFlags flags,
                                const QStyleOption &opt ) const
{
    bool down   = flags & Style_Down;
    bool on     = flags & Style_On;
    bool sunken = flags & Style_Sunken;
    bool horiz  = flags & Style_Horizontal;
    const bool enabled = flags & Style_Enabled;
    const bool mouseOver = flags & Style_MouseOver;
    
    bool hasFocus = flags & Style_HasFocus;

    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();

    int x2, y2;
    r.coords(&x, &y, &x2, &y2);

    switch(pe) {
    // BUTTONS
    // -------
        case PE_FocusRect: {
            if(_drawFocusRect)
                p->drawWinFocusRect( r );
            break;
        }

        case PE_HeaderSection: {
            // the taskbar buttons seems to be painted with PE_HeaderSection but I
            // want them look like normal buttons (at least for now. :) )
            if(!kickerMode) {
                // detect if this is the left most header item
                bool isFirst = false;
                QHeader *header = dynamic_cast<QHeader*>(p->device() );
                if (header) {
                    isFirst = header->mapToIndex(header->sectionAt(r.x() ) ) == 0;
                }

                uint contourFlags = Draw_Right|Draw_Top|Draw_Bottom;
                if (isFirst)
                    contourFlags |= Draw_Left;
                if(!enabled) contourFlags|=Is_Disabled;
                renderContour(p, r, cg.background(), getColor(cg,ButtonContour),
                                contourFlags);

                uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                if(!enabled) surfaceFlags|=Is_Disabled;
                else {
                    if(on||down) surfaceFlags|=Is_Sunken;
                    else {
                        if(mouseOver) {
                            surfaceFlags|=Is_Highlight|Highlight_Top|Highlight_Bottom;
                        }
                    }
                }
                renderSurface(p, QRect(isFirst?r.left()+1:r.left(), r.top()+1, isFirst?r.width()-2:r.width()-1, r.height()-2),
                                cg.background(), cg.button(), getColor(cg,MouseOverHighlight), _contrast,
                                surfaceFlags);

                break;
            }
        }
        case PE_ButtonBevel:
        case PE_ButtonTool:
        case PE_ButtonDropDown:
        case PE_ButtonCommand: {
            bool khtmlMode = opt.isDefault() ? false : khtmlWidgets.contains(opt.widget());
            renderButton(p, r, cg, (on||down), mouseOver, true, enabled, khtmlMode );
            break;
        }

        case PE_ButtonDefault: {
            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                    Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
            if(!enabled) contourFlags|=Is_Disabled;
            renderContour(p, r, cg.background(), cg.background().dark(120),
                    contourFlags);
            break;
        }

        case PE_SpinWidgetPlus:
        case PE_SpinWidgetMinus: {
            p->setPen( cg.buttonText() );

            int l = QMIN( w-2, h-2 );
            // make the length even so that we get a nice symmetric plus...
            if(l%2 != 0)
                --l;
            QPoint c = r.center();

            p->drawLine( c.x()-l/2, c.y(), c.x()+l/2, c.y() );
            if ( pe == PE_SpinWidgetPlus ) {
                p->drawLine( c.x(), c.y()-l/2, c.x(), c.y()+l/2 );
            }
            break;
        }

        case PE_ScrollBarSlider: {
            const WidgetState s = enabled?(down?IsPressed:IsEnabled):IsDisabled;
            const QColor surface = getColor(cg, DragButtonSurface, s);

            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            if(!enabled) contourFlags|=Is_Disabled;
            renderContour(p, r, cg.background(), getColor(cg, DragButtonContour, s),
                    contourFlags);

            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            if(horiz) surfaceFlags|=Is_Horizontal;
            if(!enabled) surfaceFlags|=Is_Disabled;
            if(r.height() >= 4)
                renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                        cg.background(), surface, cg.background(),
                        _contrast+3, surfaceFlags);

            // set contour-like color for the case _scrollBarLines is set and we paint lines instead of dots.
            p->setPen(alphaBlendColors(cg.background(), surface.dark(enabled?140:120), 50) );

            const int d = 4;
            int n = ((horiz?r.width():r.height())-8)/d;
            if(n>5) n=5;
            if(!horiz) {
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

            break;
        }

        case PE_ScrollBarAddPage:
        case PE_ScrollBarSubPage: {
            // draw double buffered to avoid flicker...
            QPixmap buffer;
            if(flags & Style_Horizontal) {
                buffer.resize(2, r.height() );
            } else {
                buffer.resize(r.width(), 2 );
            }
            QRect br(buffer.rect() );
            QPainter bp(&buffer);

            if (on || down) {
                bp.fillRect(br, QBrush(cg.mid().dark()));
            } else {
                if(flags & Style_Horizontal) {
                    bp.setPen(cg.background().dark(106));
                    bp.drawLine(br.left(), br.top(), br.right(), br.top());
                    bp.setPen(cg.background().light(106));
                    bp.drawLine(br.left(), br.bottom(), br.right(), br.bottom());
                    bp.fillRect(br.left(), br.top()+1, br.width(), br.height()-2,cg.background());
                } else {
                    bp.setPen(cg.background().dark(106));
                    bp.drawLine(br.left(), br.top(), br.left(), br.bottom());
                    bp.setPen(cg.background().light(106));
                    bp.drawLine(br.right(), br.top(), br.right(), br.bottom());
                    bp.fillRect(br.left()+1, br.top(), br.width()-2, br.height(),cg.background());
                }
            }

            bp.fillRect(br, QBrush(cg.background().light(), Dense4Pattern));

            bp.end();

            p->drawTiledPixmap(r, buffer, QPoint(0, r.top()%2) );
            break;
        }

    // SCROLLBAR BUTTONS
    // -----------------
        case PE_ScrollBarSubLine: {
            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            if(down) surfaceFlags|=Is_Sunken;
            if(!enabled) {
                contourFlags|=Is_Disabled;
                surfaceFlags|=Is_Disabled;
            }
            if(horiz) {
                contourFlags |= Round_UpperLeft|Round_BottomLeft|Is_Horizontal;
                surfaceFlags |= Round_UpperLeft|Round_BottomLeft|Is_Horizontal;
            } else {
                contourFlags |= Round_UpperLeft|Round_UpperRight;
                surfaceFlags |= Round_UpperLeft|Round_UpperRight;
            }
            renderContour(p, r, cg.background(), getColor(cg, ButtonContour),
                    contourFlags);
            renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                    cg.background(), cg.button(), getColor(cg,MouseOverHighlight), _contrast+3,
                    surfaceFlags);

            p->setPen(cg.foreground());
            drawPrimitive((horiz ? PE_ArrowLeft : PE_ArrowUp), p, r, cg, flags);
            break;
        }

        case PE_ScrollBarAddLine: {
            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            if(down) surfaceFlags|=Is_Sunken;
            if(!enabled) {
                contourFlags|=Is_Disabled;
                surfaceFlags|=Is_Disabled;
            }
            if(horiz) {
                contourFlags |= Round_UpperRight|Round_BottomRight|Is_Horizontal;
                surfaceFlags |= Round_UpperRight|Round_BottomRight|Is_Horizontal;
            } else {
                contourFlags |= Round_BottomLeft|Round_BottomRight;
                surfaceFlags |= Round_BottomLeft|Round_BottomRight;
            }
            renderContour(p, r, cg.background(), getColor(cg, ButtonContour),
                    contourFlags);
            renderSurface(p, QRect(r.left()+1, r.top()+1, r.width()-2, r.height()-2),
                    cg.background(), cg.button(), getColor(cg,MouseOverHighlight), _contrast+3,
                    surfaceFlags);

            p->setPen(cg.foreground());
            drawPrimitive((horiz ? PE_ArrowRight : PE_ArrowDown), p, r, cg, flags);
            break;
        }

    // CHECKBOXES
    // ----------
        case PE_Indicator: {
            QColor contentColor = enabled?cg.base():cg.background();

            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
            if(!enabled) {
                contourFlags |= Is_Disabled;
            }
            renderContour(p, r, cg.background(), getColor(cg, ButtonContour),
                    contourFlags);

            // surface
            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if(!enabled) {
                surfaceFlags |= Is_Disabled;
            } else if(mouseOver) {
                contentColor = alphaBlendColors(contentColor, getColor(cg,MouseOverHighlight), 240);
                surfaceFlags |= Is_Highlight;
                surfaceFlags |= Highlight_Left|Highlight_Right|
                        Highlight_Top|Highlight_Bottom;
            }
            renderSurface(p, QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2),
                    cg.background(), contentColor, getColor(cg,MouseOverHighlight), enabled?_contrast+3:(_contrast/2), surfaceFlags);

            drawPrimitive(PE_CheckMark, p, r, cg, flags);

            break;
        }

        case PE_IndicatorMask: {
            p->fillRect (r, color1);
            break;
        }

    // RADIOBUTTONS
    // ------------
        case PE_ExclusiveIndicator: {
            const QColor contourColor = getColor(cg, ButtonContour, enabled);
            QColor contentColor = enabled?cg.base():cg.background();

            QBitmap bmp;
            bmp = QBitmap(13, 13, radiobutton_mask_bits, true);
            // first the surface...
            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if(!enabled) {
                surfaceFlags |= Is_Disabled;
            } else if (mouseOver) {
                contentColor = alphaBlendColors(contentColor, getColor(cg,MouseOverHighlight), 240);
            }
            p->setClipRegion(bmp);
            renderSurface(p, r,
                    cg.background(), contentColor, getColor(cg,MouseOverHighlight), enabled?_contrast+3:(_contrast/2), surfaceFlags);
            p->setClipping(false);

            // ...then contour, anti-alias, mouseOver...
            // contour
            bmp = QBitmap(13, 13, radiobutton_contour_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(cg.background(), contourColor, 50) );
            p->drawPixmap(x, y, bmp);
            // anti-alias outside
            bmp = QBitmap(13, 13, radiobutton_aa_outside_bits, true);
            bmp.setMask(bmp);
            p->setPen(alphaBlendColors(cg.background(), contourColor, 150) );
            p->drawPixmap(x, y, bmp);
            // highlighting...
            if(mouseOver) {
                bmp = QBitmap(13, 13, radiobutton_highlight1_bits, true);
                bmp.setMask(bmp);
                p->setPen(alphaBlendColors(contentColor, getColor(cg,MouseOverHighlight), 80) );
                p->drawPixmap(x, y, bmp);
                bmp = QBitmap(13, 13, radiobutton_highlight2_bits, true);
                bmp.setMask(bmp);
                p->setPen(alphaBlendColors(contentColor, getColor(cg,MouseOverHighlight), 150) );
                p->drawPixmap(x, y, bmp);
            }
            // anti-alias inside, "above" the higlighting!
            bmp = QBitmap(13, 13, radiobutton_aa_inside_bits, true);
            bmp.setMask(bmp);
            if(mouseOver) {
                p->setPen(alphaBlendColors(getColor(cg,MouseOverHighlight), contourColor, 180) );
            } else {
                p->setPen(alphaBlendColors(contentColor, contourColor, 180) );
            }
            p->drawPixmap(x, y, bmp);


            QColor checkmarkColor = enabled?getColor(cg,CheckMark):cg.background();
            if(flags & Style_Down) {
                checkmarkColor = alphaBlendColors(contentColor, checkmarkColor, 150);
            }

            // draw the radio mark
            if (flags & Style_On || flags & Style_Down) {
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
            }

            break;
        }

        case PE_ExclusiveIndicatorMask: {
            p->fillRect(r, color0);

            QBitmap bmp;
            bmp = QBitmap(13, 13, radiobutton_mask_bits, true);
            bmp.setMask(bmp);
            p->setPen(color1);
            p->drawPixmap(x, y, bmp);

            break;
        }

    // GENERAL PANELS
    // --------------
        case PE_Splitter: {
            // highlight on mouse over
            QColor color = (hoverWidget == p->device())?cg.background().light(100+_contrast):cg.background();
            p->fillRect(r, color);
            if (w > h) {
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

                break;
        }

        case PE_PanelGroupBox:
        case PE_GroupBoxFrame: {
            if ( opt.isDefault() || opt.lineWidth() <= 0 )
                break;
            renderPanel(p, r, cg, false);

            break;
        }

        case PE_WindowFrame:
        case PE_Panel: {
            if ( opt.isDefault() || opt.lineWidth() <= 0 )
                break;
            renderPanel(p, r, cg, true, sunken);
            break;
        }

        case PE_PanelLineEdit: {
            bool isReadOnly = false;
            bool isEnabled = true;
            // panel is highlighted by default if it has focus, but if we have access to the
            // widget itself we can try to avoid highlighting in case it's readOnly or disabled.
            if (p->device() && dynamic_cast<QLineEdit*>(p->device()))
            {
                QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(p->device());
                isReadOnly = lineEdit->isReadOnly();
                isEnabled = lineEdit->isEnabled();
            }

            uint contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                    Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;

            // HACK!!
            //
            // In order to draw nice edges in khtml, we need to paint alpha-blended.
            // On the other hand, we can't paint alpha-blended in normal widgets.
            //
            // In this place there is no reliable way to detect if we are in khtml; the
            // only thing we know is that khtml buffers its widgets into a pixmap. So
            // when the paint device is a QPixmap, chances are high that we are in khtml.
            // It's possible that this breaks other things, so let's see how it works...
            if (p->device() && dynamic_cast<QPixmap*>(p->device() ) ) {
                contourFlags += Draw_AlphaBlend;
            }

            if ( _inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
            {
                renderContour(p, r, cg.background(),
                              getColor(cg,FocusHighlight,enabled), contourFlags );
            }
            else
            {
                renderContour(p, r, cg.background(),
                              getColor(cg, ButtonContour, enabled), contourFlags );
            }
            const QColor contentColor = enabled?cg.base():cg.background();
            if (_inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
            {
                p->setPen( getColor(cg,FocusHighlight).dark(130) );
            }
            else
            {
                p->setPen(contentColor.dark(130) );
            }
            p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2 );
            p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
            if (_inputFocusHighlight && hasFocus && !isReadOnly && isEnabled)
            {
                p->setPen( getColor(cg,FocusHighlight).light(130) );
            }
            else
            {
              p->setPen(contentColor.light(130) );
            }
            p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
            p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2 );
            break;
        }

        case PE_StatusBarSection: {
            renderContour(p, r, cg.background(), cg.background().dark(160),
                          Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
            break;
        }

        case PE_TabBarBase: // Still not sure what this one does
        case PE_PanelTabWidget: {
            renderPanel(p, r, cg, true, sunken);
            break;
        }

        case PE_PanelPopup: {
            renderContour(p, r, cg.background(), cg.background().dark(200),
                          Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
            break;
        }

    // MENU / TOOLBAR PANEL
    // --------------------
        case PE_PanelMenuBar:
        case PE_PanelDockWindow: {
            // fix for toolbar lag (from Mosfet Liquid) 
            QWidget* w = dynamic_cast<QWidget*>(p->device());
            if(w && w->backgroundMode() == PaletteButton) 
                w->setBackgroundMode(PaletteBackground);
            p->fillRect(r, cg.brush(QColorGroup::Background));

            if ( _drawToolBarSeparator ) {
                if ( r.width() > r.height() ) {
                    p->setPen( getColor(cg, PanelLight) );
                    p->drawLine( r.left(), r.top(), r.right(), r.top() );
                    p->setPen( getColor(cg, PanelDark) );
                    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
                }
                else {
                    p->setPen( getColor(cg, PanelLight) );
                    p->drawLine( r.left(), r.top(), r.left(), r.bottom() );
                    p->setPen( getColor(cg, PanelDark) );
                    p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
                }
            }

            break;
        }

    // TOOLBAR/DOCK WINDOW HANDLE
    // --------------------------
        case PE_DockWindowResizeHandle: {
            renderButton(p, r, cg);
            break;
        }

        case PE_DockWindowHandle: {

            int counter = 1;

            if(horiz) {
                int center = r.left()+r.width()/2;
                for(int j = r.top()+2; j <= r.bottom()-3; j+=3) {
                    if(counter%2 == 0) {
                        renderDot(p, QPoint(center+1, j), cg.background(), true, true);
                    } else {
                        renderDot(p, QPoint(center-2, j), cg.background(), true, true);
                    }
                    counter++;
                }
            } else {
                int center = r.top()+r.height()/2;
                for(int j = r.left()+2; j <= r.right()-3; j+=3) {
                    if(counter%2 == 0) {
                        renderDot(p, QPoint(j, center+1), cg.background(), true, true);
                    } else {
                        renderDot(p, QPoint(j, center-2), cg.background(), true, true);
                    }
                    counter++;
                }
            }

            break;
        }

    // TOOLBAR SEPARATOR
    // -----------------
        case PE_DockWindowSeparator: {
            p->fillRect(r, cg.background());

            if(_drawToolBarItemSeparator) {
                if(horiz) {
                    int center = r.left()+r.width()/2;
                    p->setPen( getColor(cg, PanelDark) );
                    p->drawLine( center-1, r.top()+3, center-1, r.bottom()-3 );
                    p->setPen( getColor(cg, PanelLight) );
                    p->drawLine( center, r.top()+3, center, r.bottom()-3 );
                } else {
                    int center = r.top()+r.height()/2;
                    p->setPen( getColor(cg, PanelDark) );
                    p->drawLine( r.x()+3, center-1, r.right()-3, center-1 );
                    p->setPen( getColor(cg, PanelLight) );
                    p->drawLine( r.x()+3, center, r.right()-3, center );
                }
            }
            break;
        }

        case PE_CheckMark: {
            const QColor contentColor = enabled?cg.base():cg.background();
            QColor checkmarkColor = enabled?getColor(cg,CheckMark):cg.background();
            if(flags & Style_Down) {
                checkmarkColor = alphaBlendColors(contentColor, checkmarkColor, 150);
            }

            int x = r.center().x() - 4, y = r.center().y() - 4;
            QBitmap bmp;
            if( flags & Style_On ) {
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
            } else if ( flags & Style_Off ) {
                // empty
            } else { // tristate
                bmp = QBitmap(CHECKMARKSIZE, CHECKMARKSIZE, checkmark_tristate_bits, true);
                bmp.setMask(bmp);
                p->setPen(alphaBlendColors(contentColor, checkmarkColor.dark(150), 50) );
                p->drawPixmap(x, y, bmp);
            }

            break;
        }

        case PE_SpinWidgetUp:
        case PE_SpinWidgetDown:
        case PE_HeaderArrow:
        case PE_ArrowUp:
        case PE_ArrowDown:
        case PE_ArrowLeft:
        case PE_ArrowRight: {
            QPointArray a;

            switch (pe) {
                case PE_SpinWidgetUp:
                case PE_ArrowUp: {
                    a.setPoints(7, u_arrow);
                    break;
                }
                case PE_SpinWidgetDown:
                case PE_ArrowDown: {
                    a.setPoints(7, d_arrow);
                    break;
                }
                case PE_ArrowLeft: {
                    a.setPoints(7, l_arrow);
                    break;
                }
                case PE_ArrowRight: {
                    a.setPoints(7, r_arrow);
                    break;
                }
                default: {
                    if (flags & Style_Up) {
                        a.setPoints(7, u_arrow);
                    } else {
                        a.setPoints(7, d_arrow);
                    }
                }
            }

            const QWMatrix oldMatrix( p->worldMatrix() );

            if (flags & Style_Down) {
                p->translate(pixelMetric(PM_ButtonShiftHorizontal),
                                pixelMetric(PM_ButtonShiftVertical));
            }

            a.translate((r.x()+r.width()/2), (r.y()+r.height()/2));
            // extra-pixel-shift, correcting some visual tics...
            switch(pe) {
                case PE_ArrowLeft:
                case PE_ArrowRight:
                    a.translate(0, -1);
                    break;
                case PE_SpinWidgetUp:
                case PE_SpinWidgetDown:
                    a.translate(+1, 0);
                    break;
                default:
                    a.translate(0, 0);
            }

            if (p->pen() == QPen::NoPen) {
                if (flags & Style_Enabled) {
                    p->setPen(cg.buttonText());
                } else {
                    p->setPen(cg.highlightedText());
                }
            }
            p->drawLineSegments(a, 0, 3);
            p->drawPoint(a[6]);

            p->setWorldMatrix( oldMatrix );

            break;
        }

        default: {
            return KStyle::drawPrimitive(pe, p, r, cg, flags, opt);
        }
    }
}


void PlastikStyle::drawControl(ControlElement element,
                              QPainter *p,
                              const QWidget *widget,
                              const QRect &r,
                              const QColorGroup &cg,
                              SFlags flags,
                              const QStyleOption& opt) const
{
    const bool reverseLayout = QApplication::reverseLayout();

    const bool enabled = (flags & Style_Enabled);

    switch (element) {

    // PROGRESSBAR
    // -----------
        case CE_ProgressBarGroove: {
            const QColor content = enabled?cg.base():cg.background();
            renderContour(p, r, cg.background(), getColor(cg, ButtonContour, enabled) );
            p->setPen(content.dark(105) );
            p->drawLine(r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
            p->drawLine(r.left()+1, r.top()+2, r.left()+1, r.bottom()-2);
            p->setPen(content.light(105) );
            p->drawLine(r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
            p->drawLine(r.right()-1, r.top()+2, r.right()-1, r.bottom()-2);
            break;
        }

        case CE_ProgressBarContents: {
            const QProgressBar *pb = dynamic_cast<const QProgressBar*>(widget);
            int steps = pb->totalSteps();

            const QColor bg = enabled?cg.base():cg.background(); // background
            const QColor fg = enabled?cg.highlight():cg.background().dark(110); // foreground

            if( steps == 0 ) { // Busy indicator
                static const int barWidth = 10;
                int progress = pb->progress() % (2*(r.width()-barWidth));
                if( progress < 0)
                    progress = 0;
                if( progress > r.width()-barWidth )
                    progress = (r.width()-barWidth)-(progress-(r.width()-barWidth));
                p->fillRect( QRect( r.x(), r.y(), r.width(), r.height() ), bg );
                renderContour( p, QRect( r.x()+progress, r.y(), barWidth, r.height() ),
                                bg, fg.dark(160),
                                Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|
                                    Round_BottomRight|Round_UpperLeft|Round_BottomLeft );
                renderSurface(p, QRect( r.x()+progress+1, r.y()+1, barWidth-2, r.height()-2 ),
                                    bg, fg, cg.highlight(),
                                    2*(_contrast/3),
                                    Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                        Round_UpperRight|Round_BottomRight|
                                        Round_UpperLeft|Round_BottomLeft|Is_Horizontal);
            } else {
                double percent = static_cast<double>(pb->progress()) / static_cast<double>(steps);

                int w = static_cast<int>(r.width() * percent);
                // renderContour/renderSurface handle small sizes not very well, so set a minimal
                // progressbar width...
                if(w<4) w = 4;
                int w2 = r.width()-(r.width()-w);

                QRect Rempty(reverseLayout?r.left():r.left()+w-1, r.top(), r.width()-w+1, r.height() );
                QRect Rcontour(reverseLayout?r.right()-w2+1:r.left(), r.top(), w2, r.height() );
                QRect Rsurface(Rcontour.left()+1, Rcontour.top()+1, w2-2, Rcontour.height()-2);

                p->fillRect(Rempty, bg);

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
                                fg.light(105), fg, cg.highlight(), 2*(_contrast/3),
                                reverseLayout ? Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                        Round_UpperLeft|Round_BottomLeft|Is_Horizontal
                                : Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                        Round_UpperRight|Round_BottomRight|Is_Horizontal);
                // 2
                renderSurface(&surfacePainter,
                                QRect(10, 0, 11, tileHeight),
                                fg, fg.light(105), cg.highlight(), 2*(_contrast/3),
                                reverseLayout ? Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                        Round_UpperLeft|Round_BottomLeft|Is_Horizontal
                                : Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|
                                        Round_UpperRight|Round_BottomRight|Is_Horizontal);
                // 1
                renderSurface(&surfacePainter,
                                QRect(0, 0, 11, tileHeight),
                                fg.light(105), fg, cg.highlight(), 2*(_contrast/3),
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
                    // find the animation Offset for the current Widget
                    QWidget* nonConstWidget = const_cast<QWidget*>(widget);
                    QMapConstIterator<QWidget*, int> iter = progAnimWidgets.find(nonConstWidget);
                    if (iter != progAnimWidgets.end())
                        animShift = iter.data();
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
            }

            break;
        }

    // TABS
    // ----
        case CE_TabBarTab: {
            const QTabBar * tb = (const QTabBar *) widget;
            bool cornerWidget = false;
            if( ::qt_cast<QTabWidget*>(tb->parent()) ) {
                const QTabWidget *tw = (const QTabWidget*)tb->parent();
                // is there a corner widget in the (top) left edge?
                QWidget *cw = tw->cornerWidget(Qt::TopLeft);
                if(cw)
                    cornerWidget = true;
            }
            QTabBar::Shape tbs = tb->shape();
            bool selected = false;
            if (flags & Style_Selected) selected = true;
            TabPosition pos;
            if (tb->count() == 1) {
                pos = Single;
            } else if ((tb->indexOf(opt.tab()->identifier()) == 0)) {
                pos = First;
            } else if (tb->indexOf(opt.tab()->identifier()) == tb->count() - 1) {
                pos = Last;
            } else {
                pos = Middle;
            }

            bool mouseOver = false;
            if (opt.tab() == hoverTab) {
                mouseOver = true;
                flags |= Style_MouseOver;
            }

            switch (tbs) {
                case QTabBar::TriangularAbove:
//                     renderTriangularTab(p, r, cg, (flags & Style_MouseOver), selected, false, pos);
                    renderTab(p, r, cg, mouseOver, selected, false, pos, true, cornerWidget);
                    break;
                case QTabBar::RoundedAbove:
                    renderTab(p, r, cg, mouseOver, selected, false, pos, false, cornerWidget);
                    break;
                case QTabBar::TriangularBelow:
//                     renderTriangularTab(p, r, cg, (flags & Style_MouseOver), selected, true, pos);
                    renderTab(p, r, cg, mouseOver, selected, true, pos, true, cornerWidget);
                    break;
                case QTabBar::RoundedBelow:
                    renderTab(p, r, cg, mouseOver, selected, true, pos, false, cornerWidget);
                    break;
                    default:
                            KStyle::drawControl(element, p, widget, r, cg, flags, opt);
            }

            break;
        }

        case CE_PushButton: {
            QPushButton *button = (QPushButton *)widget;

            const bool isDefault = enabled && button->isDefault();

            if (button->isFlat() )
                flatMode = true;

            if (widget == hoverWidget)
                flags |= Style_MouseOver;

            QColorGroup g2 = cg;
            if (isDefault)
                g2.setColor(QColorGroup::Background, cg.background().dark(120) );
            drawPrimitive(PE_ButtonBevel, p,
                    isDefault?QRect(r.x()+1,r.y()+1,r.width()-2,r.height()-2):r,
                    g2, flags, QStyleOption(button) );

            if (isDefault ) {
                drawPrimitive(PE_ButtonDefault, p, r, cg, flags);
            }

            break;
        }

        case CE_PushButtonLabel:
        {
            int x, y, w, h;
            r.rect( &x, &y, &w, &h );

            const QPushButton* button = static_cast<const QPushButton *>( widget );
            bool active = button->isOn() || button->isDown();
            bool cornArrow = false;

            // Shift button contents if pushed.
            if ( active )
            {
                x += pixelMetric(PM_ButtonShiftHorizontal, widget);
                y += pixelMetric(PM_ButtonShiftVertical, widget);
                flags |= Style_Sunken;
            }

            // Does the button have a popup menu?
            if ( button->isMenuButton() )
            {
                int dx = pixelMetric( PM_MenuButtonIndicator, widget );
                if ( button->iconSet() && !button->iconSet()->isNull()  &&
                    (dx + button->iconSet()->pixmap (QIconSet::Small, QIconSet::Normal, QIconSet::Off ).width()) >= w )
                {
                    cornArrow = true; //To little room. Draw the arrow in the corner, don't adjust the widget
                }
                else
                {
		    p->setPen(cg.buttonText());
                    drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - dx - 8, y + 2, dx, h - 4), r ),
                                cg, flags, opt );
                    w -= dx;
                }
            }

            // Draw the icon if there is one
            if ( button->iconSet() && !button->iconSet()->isNull() )
            {
                QIconSet::Mode  mode  = QIconSet::Disabled;
                QIconSet::State state = QIconSet::Off;

                if (button->isEnabled())
                    mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
                if (button->isToggleButton() && button->isOn())
                    state = QIconSet::On;

                QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );

                if (button->text().isEmpty() && !button->pixmap())
                    p->drawPixmap( x + w/2 - pixmap.width()/2, y + h / 2 - pixmap.height() / 2,
                                    pixmap );
                else
                    p->drawPixmap( x + 4, y + h / 2 - pixmap.height() / 2, pixmap );

                if (cornArrow) //Draw over the icon
                    drawPrimitive( PE_ArrowDown, p, visualRect( QRect(x + w - 6, x + h - 6, 7, 7), r ),
                                cg, flags, opt );


                int  pw = pixmap.width();
                x += pw + 4;
                w -= pw + 4;
            }

            // Make the label indicate if the button is a default button or not
            drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, button->colorGroup(),
                        button->isEnabled(), button->pixmap(), button->text(), -1,
                        &button->colorGroup().buttonText() );


            if ( flags & Style_HasFocus )
                drawPrimitive( PE_FocusRect, p,
                                visualRect( subRect( SR_PushButtonFocusRect, widget ), widget ),
                                cg, flags );
            break;
        }

    // MENUBAR ITEM (sunken panel on mouse over)
    // -----------------------------------------
        case CE_MenuBarItem: {
            QMenuItem *mi = opt.menuItem();
            bool active  = flags & Style_Active;
            bool focused = flags & Style_HasFocus;
            bool down = flags & Style_Down;
            const int text_flags =
                AlignVCenter | AlignHCenter | ShowPrefix | DontClip | SingleLine;

            p->fillRect(r, cg.background());

            if (active && focused) {
                if (down) {
                    drawPrimitive(PE_ButtonTool, p, r, cg, flags|Style_Down, opt);
                } else {
                    drawPrimitive(PE_ButtonTool, p, r, cg, flags, opt);
                }
            }

            p->setPen(cg.foreground() );
            p->drawText(r, text_flags, mi->text());
            break;
        }

    // POPUPMENU ITEM (highlighted on mouseover)
    // ------------------------------------------
        case CE_PopupMenuItem: {
            const QPopupMenu *popupmenu = static_cast< const QPopupMenu * >( widget );
            QMenuItem *mi = opt.menuItem();

            if ( !mi )
            {
                // Don't leave blank holes if we set NoBackground for the QPopupMenu.
                // This only happens when the popupMenu spans more than one column.
                if (! ( widget->erasePixmap() && !widget->erasePixmap()->isNull() ) )
                    p->fillRect( r, cg.background().light( 105 ) );

                break;
            }
            int  tab        = opt.tabWidth();
            int  checkcol   = opt.maxIconWidth();
            bool enabled    = mi->isEnabled();
            bool checkable  = popupmenu->isCheckable();
            bool active     = flags & Style_Active;
            bool etchtext   = styleHint( SH_EtchDisabledText );
            bool reverse    = QApplication::reverseLayout();
            if ( checkable )
                checkcol = QMAX( checkcol, 20 );

            // Draw the menu item background
            if (active) {
                if (enabled) {
                    renderSurface(p, r, cg.background(), cg.highlight(), cg.highlight(),
                            _contrast+3, Draw_Top|Draw_Bottom|Is_Horizontal);
                }
                else {
                    if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
                        p->drawPixmap( r.topLeft(), *widget->erasePixmap(), r );
                    else p->fillRect( r, cg.background().light(105) );
                    if(_drawFocusRect)
                        p->drawWinFocusRect( r );
                }
            }
            // Draw the transparency pixmap
            else if ( widget->erasePixmap() && !widget->erasePixmap()->isNull() )
                p->drawPixmap( r.topLeft(), *widget->erasePixmap(), r );
            // Draw a solid background
            else
                p->fillRect( r, cg.background().light( 105 ) );
            // Are we a menu item separator?
            if ( mi->isSeparator() )
            {
                p->setPen( cg.mid() );
                p->drawLine( r.x()+5, r.y() + 1, r.right()-5, r.y() + 1 );
                p->setPen( cg.light() );
                p->drawLine( r.x()+5, r.y() + 2, r.right()-5 , r.y() + 2 );
                break;
            }

            QRect cr = visualRect( QRect( r.x() + 2, r.y() + 2, checkcol - 1, r.height() - 4 ), r );
            // Do we have an icon?
            if ( mi->iconSet() )
            {
                QIconSet::Mode mode;

                // Select the correct icon from the iconset
                if (active)
                    mode = enabled?QIconSet::Active:QIconSet::Disabled;
                else
                    mode = enabled?QIconSet::Normal:QIconSet::Disabled;

                // Do we have an icon and are checked at the same time?
                // Then draw a "pressed" background behind the icon
                if ( checkable && /*!active &&*/ mi->isChecked() )
                    qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(),
                                        cg, true, 1, &cg.brush(QColorGroup::Midlight) );
                // Draw the icon
                QPixmap pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
                QRect pmr( 0, 0, pixmap.width(), pixmap.height() );
                pmr.moveCenter( cr.center() );
                p->drawPixmap( pmr.topLeft(), pixmap );
            }

            // Are we checked? (This time without an icon)
            else if ( checkable && mi->isChecked() )
            {
                // We only have to draw the background if the menu item is inactive -
                // if it's active the "pressed" background is already drawn
            // if ( ! active )
                    qDrawShadePanel( p, cr.x(), cr.y(), cr.width(), cr.height(), cg, true, 1,
                                        &cg.brush(QColorGroup::Midlight) );

                // Draw the checkmark
                SFlags cflags = Style_On;
                if (enabled)
                    cflags |= Style_Enabled;
                drawPrimitive( PE_CheckMark, p, cr, cg, cflags );
            }

            // Time to draw the menu item label...
            int xm = 2 + checkcol + 2; // X position margin

            int xp = reverse ? // X position
                    r.x() + tab + rightBorder + itemHMargin + itemFrame - 1 :
                    r.x() + xm;

            int offset = reverse ? -1 : 1; // Shadow offset for etched text

            // Label width (minus the width of the accelerator portion)
            int tw = r.width() - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

            // Set the color for enabled and disabled text
            // (used for both active and inactive menu items)
            p->setPen( enabled ? cg.buttonText() : cg.mid() );

            // This color will be used instead of the above if the menu item
            // is active and disabled at the same time. (etched text)
            QColor discol = cg.mid();

            // Does the menu item draw it's own label?
            if ( mi->custom() ) {
                int m = 2;
                // Save the painter state in case the custom
                // paint method changes it in some way
                p->save();

                // Draw etched text if we're inactive and the menu item is disabled
                if ( etchtext && !enabled && !active ) {
                    p->setPen( cg.light() );
                    mi->custom()->paint( p, cg, active, enabled, xp+offset, r.y()+m+1, tw, r.height()-2*m );
                    p->setPen( discol );
                }
                mi->custom()->paint( p, cg, active, enabled, xp, r.y()+m, tw, r.height()-2*m );
                p->restore();
            }
            else {
                // The menu item doesn't draw it's own label
                QString s = mi->text();
                // Does the menu item have a text label?
                if ( !s.isNull() ) {
                    int t = s.find( '\t' );
                    int m = 2;
                    int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
                    text_flags |= reverse ? AlignRight : AlignLeft;

                    //QColor draw = cg.text();
                    QColor draw = (active && enabled) ? cg.highlightedText () : cg.foreground();
                    p->setPen(draw);


                    // Does the menu item have a tabstop? (for the accelerator text)
                    if ( t >= 0 ) {
                        int tabx = reverse ? r.x() + rightBorder + itemHMargin + itemFrame :
                            r.x() + r.width() - tab - rightBorder - itemHMargin - itemFrame;

                        // Draw the right part of the label (accelerator text)
                        if ( etchtext && !enabled ) {
                            // Draw etched text if we're inactive and the menu item is disabled
                            p->setPen( cg.light() );
                            p->drawText( tabx+offset, r.y()+m+1, tab, r.height()-2*m, text_flags, s.mid( t+1 ) );
                            p->setPen( discol );
                        }
                        p->drawText( tabx, r.y()+m, tab, r.height()-2*m, text_flags, s.mid( t+1 ) );
                        s = s.left( t );
                    }

                    // Draw the left part of the label (or the whole label
                    // if there's no accelerator)
                    if ( etchtext && !enabled ) {
                        // Etched text again for inactive disabled menu items...
                        p->setPen( cg.light() );
                        p->drawText( xp+offset, r.y()+m+1, tw, r.height()-2*m, text_flags, s, t );
                        p->setPen( discol );
                    }


                    p->drawText( xp, r.y()+m, tw, r.height()-2*m, text_flags, s, t );

                    p->setPen(cg.text());

                }

                // The menu item doesn't have a text label
                // Check if it has a pixmap instead
                else if ( mi->pixmap() ) {
                    QPixmap *pixmap = mi->pixmap();

                    // Draw the pixmap
                    if ( pixmap->depth() == 1 )
                        p->setBackgroundMode( OpaqueMode );

                    int diffw = ( ( r.width() - pixmap->width() ) / 2 )
                                    + ( ( r.width() - pixmap->width() ) % 2 );
                    p->drawPixmap( r.x()+diffw, r.y()+1, *pixmap );

                    if ( pixmap->depth() == 1 )
                        p->setBackgroundMode( TransparentMode );
                }
            }

            // Does the menu item have a submenu?
            if ( mi->popup() ) {
                PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
                int dim = pixelMetric(PM_MenuButtonIndicator) - 1;
                QRect vr = visualRect( QRect( r.x() + r.width() - 5 - 1 - dim,
                            r.y() + r.height() / 2 - dim / 2, dim, dim), r );

                // Draw an arrow at the far end of the menu item
                if ( active ) {
                    if ( enabled )
                        discol = cg.buttonText();

                    QColorGroup g2( discol, cg.highlight(), white, white,
                                    enabled ? white : discol, discol, white );

                    drawPrimitive( arrow, p, vr, g2, Style_Enabled );
                } else
                    drawPrimitive( arrow, p, vr, cg,
                            enabled ? Style_Enabled : Style_Default );
            }
            break;
        }

    // Menu and dockwindow empty space
    //
        case CE_DockWindowEmptyArea:
            p->fillRect(r, cg.background());
            break;

        case CE_MenuBarEmptyArea:
            p->fillRect(r, cg.background());

//             if ( _drawToolBarSeparator ) {
//                 p->setPen( getColor(cg, PanelDark) );
//                 p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
//             }

            break;

        default:
          KStyle::drawControl(element, p, widget, r, cg, flags, opt);
    }
}

void PlastikStyle::drawControlMask(ControlElement element,
                                  QPainter *p,
                                  const QWidget *w,
                                  const QRect &r,
                                  const QStyleOption &opt) const
{
    switch (element) {
        case CE_PushButton: {
                    p->fillRect (r, color0);
                    renderMask(p, r, color1,
                            Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight);
                    break;
        }

        default: {
            KStyle::drawControlMask (element, p, w, r, opt);
        }
    }
}

void PlastikStyle::drawComplexControlMask(ComplexControl c,
                                         QPainter *p,
                                         const QWidget *w,
                                         const QRect &r,
                                         const QStyleOption &o) const
{
    switch (c) {
        case CC_SpinWidget:
        case CC_ListView:
        case CC_ComboBox: {
                p->fillRect (r, color0);
                renderMask(p, r, color1,
                        Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight);
            break;
        }
        default: {
            KStyle::drawComplexControlMask (c, p, w, r, o);
        }
    }
}

void PlastikStyle::drawComplexControl(ComplexControl control,
                                     QPainter *p,
                                     const QWidget *widget,
                                     const QRect &r,
                                     const QColorGroup &cg,
                                     SFlags flags,
                                     SCFlags controls,
                                     SCFlags active,
                                     const QStyleOption& opt) const
{
    const bool reverseLayout = QApplication::reverseLayout();

    const bool enabled = (flags & Style_Enabled);

    switch(control) {
    // COMBOBOX
    // --------
        case CC_ComboBox: {
            static const unsigned int handleWidth = 15;

            const QComboBox *cb = dynamic_cast<const QComboBox *>(widget);
            // at the moment cb is only needed to check if the combo box is editable or not.
            // if cb doesn't exist, just assume false and the app (gideon! ;) ) at least doesn't crash.
            bool editable = false;
            bool hasFocus = false;
            if (cb) {
                editable = cb->editable();
                hasFocus = cb->hasFocus();
            }

            const QColor buttonColor = enabled?cg.button():cg.background();
            const QColor inputColor = enabled?(editable?cg.base():cg.button())
                                              :cg.background();

            uint contourFlags = 0;
            if( khtmlWidgets.contains(cb) )
                contourFlags |= Draw_AlphaBlend;
            
            if (_inputFocusHighlight && hasFocus && editable && enabled)
            {
                QRect editField = querySubControlMetrics(control, widget, SC_ComboBoxEditField);
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
                renderContour(p, editFrame, cg.background(),  getColor(cg,FocusHighlight,enabled), editFlags);
                renderContour(p, buttonFrame, cg.background(), 
                              getColor(cg, ButtonContour, enabled), buttonFlags); 
            }
            else
            {
                contourFlags |= Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|
                    Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
                renderContour(p, r, cg.background(), getColor(cg, ButtonContour, enabled), contourFlags);
            }
            //extend the contour: between input and handler...
            p->setPen(alphaBlendColors(cg.background(), getColor(cg, ButtonContour, enabled), 50) );
            if(reverseLayout) {
                p->drawLine(r.left()+1+handleWidth, r.top()+1, r.left()+1+handleWidth, r.bottom()-1);
            } else {
                p->drawLine(r.right()-handleWidth-1, r.top()+1, r.right()-handleWidth-1, r.bottom()-1);
            }

            const QRect RbuttonSurface(reverseLayout?r.left()+1:r.right()-handleWidth, r.top()+1,
                                        handleWidth, r.height()-2);
            const QRect RcontentSurface(reverseLayout?r.left()+1+handleWidth+1:r.left()+1, r.top()+1,
                                         r.width()-handleWidth-3, r.height()-2);

            // handler

            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if(reverseLayout) {
                surfaceFlags |= Round_UpperLeft|Round_BottomLeft;
            } else {
                surfaceFlags |= Round_UpperRight|Round_BottomRight;
            }

            if ((widget == hoverWidget) || (flags & Style_MouseOver)) {
                surfaceFlags |= Is_Highlight;
                if(editable) surfaceFlags |= Highlight_Left|Highlight_Right;
                surfaceFlags |= Highlight_Top|Highlight_Bottom;
            }
            renderSurface(p, RbuttonSurface,
                           cg.background(), buttonColor, getColor(cg,MouseOverHighlight), enabled?_contrast+3:(_contrast/2),
                           surfaceFlags);

            if(!editable) {
                surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
                if(reverseLayout) {
                    surfaceFlags |= Round_UpperRight|Round_BottomRight;
                } else {
                    surfaceFlags |= Round_UpperLeft|Round_BottomLeft;
                }

                if ((widget == hoverWidget) || (flags & Style_MouseOver)) {
                    surfaceFlags |= Is_Highlight;
                    surfaceFlags |= Highlight_Top|Highlight_Bottom;
                }
                renderSurface(p, RcontentSurface,
                               cg.background(), buttonColor, getColor(cg,MouseOverHighlight), enabled?_contrast+3:(_contrast/2),
                               surfaceFlags);
                if (hasFocus) {
                    drawPrimitive(PE_FocusRect, p,
                        QRect(RcontentSurface.x() + 2,
                            RcontentSurface.y() + 2,
                            RcontentSurface.width() - 4,
                            RcontentSurface.height() - 4), cg);
                }
            } else {
                // thin frame around the input area
                if (_inputFocusHighlight && hasFocus && editable && enabled)
                {
                  p->setPen( getColor(cg,FocusHighlight).dark(130) );
                }
                else
                {
                  p->setPen(inputColor.dark(130) );
                }
                p->drawLine(RcontentSurface.x(), reverseLayout?RcontentSurface.y():RcontentSurface.y()+1,
                        RcontentSurface.x(), reverseLayout?RcontentSurface.bottom():RcontentSurface.bottom()-1);
                p->drawLine(RcontentSurface.x()+1, RcontentSurface.y(),
                        reverseLayout?RcontentSurface.right()-1:RcontentSurface.right(), RcontentSurface.y() );
                if (_inputFocusHighlight && hasFocus && editable && enabled)
                {
                  p->setPen( getColor(cg,FocusHighlight).light(130) );
                }
                else
                {
                  p->setPen(inputColor.light(130) );
                }
                p->drawLine(reverseLayout?RcontentSurface.x():RcontentSurface.x()+1, RcontentSurface.bottom(),
                        reverseLayout?RcontentSurface.right()-1:RcontentSurface.right(), RcontentSurface.bottom() );
                p->drawLine(RcontentSurface.right(), RcontentSurface.top()+1,
                        RcontentSurface.right(), RcontentSurface.bottom()-1 );

                // input area
                p->fillRect(RcontentSurface.x()+1, RcontentSurface.y()+1,
                        RcontentSurface.width()-2, RcontentSurface.height()-2, inputColor );
            }

            p->setPen(cg.foreground());
            drawPrimitive(PE_SpinWidgetDown, p, RbuttonSurface, cg, Style_Default|Style_Enabled|Style_Raised);

            // QComboBox draws the text using cg.text(), we can override this
            // from here
            p->setPen( cg.buttonText() );
            p->setBackgroundColor( cg.button() );
            break;
        }

    // TOOLBUTTON
    // ----------
        case CC_ToolButton: {
            const QToolButton *tb = (const QToolButton *) widget;

            QRect button, menuarea;
            button   = querySubControlMetrics(control, widget, SC_ToolButton, opt);
            menuarea = querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt);

            SFlags bflags = flags,
                    mflags = flags;

            if (kornMode) {
                drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
                break;
            } else {
                // don't want to have the buttoncolor as the background...
                p->fillRect(r, cg.background());
                bflags &= ~Style_MouseOver;
            }

            if (active & SC_ToolButton)
                bflags |= Style_Down;

            if (active & SC_ToolButtonMenu)
                mflags |= Style_Down;

            if (controls & SC_ToolButton) {
            // If we're pressed, on, or raised...
                if (bflags & (Style_Down | Style_On | Style_Raised) || widget==hoverWidget ) {
                    drawPrimitive(PE_ButtonTool, p, button, cg, bflags, opt);
                } else if (tb->parentWidget() &&
                            tb->parentWidget()->backgroundPixmap() &&
                            !tb->parentWidget()->backgroundPixmap()->isNull()) {
                    QPixmap pixmap = *(tb->parentWidget()->backgroundPixmap());
                    p->drawTiledPixmap( r, pixmap, tb->pos() );
                }
            }

            // Draw a toolbutton menu indicator if required
            if (controls & SC_ToolButtonMenu) {
                if (mflags & (Style_Down | Style_On | Style_Raised)) {
                    drawPrimitive(PE_ButtonDropDown, p, menuarea, cg, mflags, opt);
                }
                drawPrimitive(PE_ArrowDown, p, menuarea, cg, mflags, opt);
            }

            if (tb->hasFocus() && !tb->focusProxy()) {
                QRect fr = tb->rect();
                fr.addCoords(2, 2, -2, -2);
                drawPrimitive(PE_FocusRect, p, fr, cg);
            }

            // Set the color for the ToolButton menu indicator
            p->setPen(cg.buttonText() );

            break;
        }

    // SPINWIDGETS
    // -----------
        case CC_SpinWidget: {
            static const unsigned int handleWidth = 15;

            const QSpinWidget *sw = dynamic_cast<const QSpinWidget *>(widget);
            SFlags sflags = flags;
            PrimitiveElement pe;

            bool hasFocus = false;
            if (sw)
                hasFocus = sw->hasFocus();

            const QColor buttonColor = enabled?cg.button():cg.background();
            const QColor inputColor = enabled?cg.base():cg.background();

            // contour
            const bool heightDividable = ((r.height()%2) == 0);
            if (_inputFocusHighlight && hasFocus && enabled)
            {
                QRect editField = querySubControlMetrics(control, widget, SC_SpinWidgetEditField);
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
                renderContour(p, editFrame, cg.background(), cg.highlight(), editFlags);
                renderContour(p, buttonFrame, cg.background(), 
                              getColor(cg, ButtonContour, enabled), buttonFlags);
            }
            else
            {
                renderContour(p, querySubControlMetrics(control, widget, SC_SpinWidgetFrame),
                              cg.background(), getColor(cg, ButtonContour, enabled) );
            }
            p->setPen(alphaBlendColors(cg.background(), getColor(cg, ButtonContour, enabled), 50) );
            p->drawLine(reverseLayout?r.left()+1+handleWidth:r.right()-handleWidth-1, r.top()+1,
                    reverseLayout?r.left()+1+handleWidth:r.right()-handleWidth-1, r.bottom()-1);
            p->drawLine(reverseLayout?r.left()+1:r.right()-handleWidth, r.top()+1+(r.height()-2)/2,
                    reverseLayout?r.left()+handleWidth:r.right()-1, r.top()+1+(r.height()-2)/2);
            if(heightDividable)
                p->drawLine(reverseLayout?r.left()+1:r.right()-handleWidth, r.top()+1+(r.height()-2)/2-1,
                        reverseLayout?r.left()+handleWidth:r.right()-1, r.top()+1+(r.height()-2)/2-1);

            // surface
            QRect upRect = QRect(reverseLayout?r.left()+1:r.right()-handleWidth, r.top()+1,
                    handleWidth, (r.height()-2)/2);
            QRect downRect = QRect(reverseLayout?r.left()+1:r.right()-handleWidth,
                    heightDividable?r.top()+1+((r.height()-2)/2):r.top()+1+((r.height()-2)/2)+1,
                    handleWidth, ((r.height()-2)/2) );
            if(heightDividable) {
                upRect = QRect(upRect.left(), upRect.top(), upRect.width(), upRect.height()-1 );
                downRect = QRect(downRect.left(), downRect.top()+1, downRect.width(), downRect.height()-1 );
            }

            uint surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if(reverseLayout) {
                surfaceFlags |= Round_UpperLeft;
            } else {
                surfaceFlags |= Round_UpperRight;
            }
            if ((widget == hoverWidget) || (sflags & Style_MouseOver)) {
                surfaceFlags |= Is_Highlight;
                surfaceFlags |= Highlight_Top|Highlight_Left|Highlight_Right;
            }
            if (active==SC_SpinWidgetUp) surfaceFlags|=Is_Sunken;
            if(!enabled) surfaceFlags|=Is_Disabled;
            renderSurface(p, upRect, cg.background(), buttonColor, getColor(cg,MouseOverHighlight),
                           _contrast, surfaceFlags);
            surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if(reverseLayout) {
                surfaceFlags |= Round_BottomLeft;
            } else {
                surfaceFlags |= Round_BottomRight;
            }
            if ((widget == hoverWidget) || (sflags & Style_MouseOver)) {
                surfaceFlags |= Is_Highlight;
                surfaceFlags |= Highlight_Bottom|Highlight_Left|Highlight_Right;
            }
            if (active==SC_SpinWidgetDown) surfaceFlags|=Is_Sunken;
            if(!enabled) surfaceFlags|=Is_Disabled;
            renderSurface(p, downRect, cg.background(), buttonColor, getColor(cg,MouseOverHighlight),
                           _contrast, surfaceFlags);

            // icons...
            sflags = Style_Default | Style_Enabled;
            if (active == SC_SpinWidgetUp) {
                sflags |= Style_On;
                sflags |= Style_Sunken;
            } else
                sflags |= Style_Raised;
            if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                pe = PE_SpinWidgetPlus;
            else
                pe = PE_SpinWidgetUp;
            p->setPen(cg.foreground());
            drawPrimitive(pe, p, upRect, cg, sflags);

            sflags = Style_Default | Style_Enabled;
            if (active == SC_SpinWidgetDown) {
                sflags |= Style_On;
                sflags |= Style_Sunken;
            } else
                sflags |= Style_Raised;
            if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                pe = PE_SpinWidgetMinus;
            else
                pe = PE_SpinWidgetDown;
            p->setPen(cg.foreground());
            drawPrimitive(pe, p, downRect, cg, sflags);

            // thin frame around the input area
            const QRect Rcontent = QRect(reverseLayout?r.left()+1+handleWidth+1:r.left()+1, r.top()+1,
                    r.width()-1-2-handleWidth, r.height()-2);
            if (_inputFocusHighlight && hasFocus && enabled)
            {
              p->setPen( getColor(cg,FocusHighlight).dark(130) );
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
              p->setPen( getColor(cg,FocusHighlight).light(130) );
            }
            else
            {
              p->setPen(inputColor.light(130) );
            }
            p->drawLine(Rcontent.left()+1, Rcontent.bottom(), Rcontent.right()-1, Rcontent.bottom() );
            p->drawLine(Rcontent.right(), Rcontent.top()+1,
                    Rcontent.right(), reverseLayout?Rcontent.bottom()-1:Rcontent.bottom() );

            break;
        }

        default:
            KStyle::drawComplexControl(control, p, widget,
                                        r, cg, flags, controls,
                                        active, opt);
            break;
    }
}


QRect PlastikStyle::subRect(SubRect r, const QWidget *widget) const
{
    switch (r) {
        case SR_ComboBoxFocusRect: {
            return querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxEditField );
        }

        // Don't use KStyles progressbar subrect
        // TODO:
        case SR_ProgressBarGroove: {
            return QRect(widget->rect());
        }
        case SR_ProgressBarContents:
        case SR_ProgressBarLabel: {
            QRect rw = widget->rect();
            return QRect(rw.left()+2, rw.top()+2, rw.width()-4, rw.height()-4 );
        }

        default: {
            return KStyle::subRect(r, widget);
        }
    }
}

QRect PlastikStyle::querySubControlMetrics(ComplexControl control,
                                          const QWidget *widget,
                                          SubControl subcontrol,
                                          const QStyleOption &opt) const
{
    if (!widget) {
        return QRect();
    }

    QRect r(widget->rect());
    switch (control) {
        case CC_ComboBox: {
            switch (subcontrol) {
                case SC_ComboBoxEditField: {
                    // TODO: is the handler width in pixelmetric?
                    return QRect(r.left()+2, r.top()+2, r.width()-4-15-1, r.height()-4);
                }
                default: {
                    return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
                }
            }
            break;
        }
        case CC_SpinWidget: {
            const int fw = 2; // Frame width...

            const bool heightDividable = ((r.height()%2) == 0);

            QSize bs;
            if(heightDividable) {
                bs.setHeight(QMAX(8, (r.height()-2)/2));
            } else {
                bs.setHeight(QMAX(8, (r.height()-2-1)/2));
            }
            bs.setWidth(15);

            const int buttonsLeft = /*reverseLayout?r.left()+1:*/r.right()-bs.width();

            switch (subcontrol) {
                case SC_SpinWidgetUp: {
                    return QRect(buttonsLeft, r.top()+1, bs.width(), bs.height() );
                }
                case SC_SpinWidgetDown: {
                    if(heightDividable) {
                        return QRect(buttonsLeft, r.top()+1+bs.height(),
                                bs.width(), r.height()-(bs.height()+2) );
                    } else {
                        return QRect(buttonsLeft, r.top()+1+bs.height()+1,
                                bs.width(), r.height()-(bs.height()+2+1) );
                    }
                }
                case SC_SpinWidgetFrame: {
                    return QRect(r.left(), r.top(), r.width(), r.height() );
                }
                case SC_SpinWidgetEditField: {
                    return QRect(r.left()+fw, r.top()+fw,
                            r.width()-(bs.width()+1+2*fw), r.height()-2*fw);
                }
                case SC_SpinWidgetButtonField: {
                    return QRect(buttonsLeft, r.top()+1, bs.width(), r.height()-2);
                }
                default: {
                    return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
                }
            }
            break;
        }
        default: {
            return KStyle::querySubControlMetrics(control, widget, subcontrol, opt);
        }
    }
}

int PlastikStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    switch(m) {
    // TABS
    // ----
        case PM_TabBarTabVSpace: {
            const QTabBar * tb = (const QTabBar *) widget;
            if (tb->shape() == QTabBar::RoundedAbove ||
                tb->shape() == QTabBar::RoundedBelow)
                return 12;
            else
                return 4;
        }

        case PM_TabBarTabOverlap: {
            return 1;
        }

    // extra space between menubar items
        case PM_MenuBarItemSpacing: {
            return 6;
        }

//     // extra space between toolbar items
//         case PM_ToolBarItemSpacing: {
//             return 4;
//         }

    // SCROLL BAR
        case PM_ScrollBarSliderMin: {
            return 21;
        }
        case PM_ScrollBarExtent: {
            return 16;
        }

        case PM_DockWindowSeparatorExtent:
            return 6;

    // SPLITTERS
    // ---------
        case PM_SplitterWidth: {
            return 6;
        }

    // PROGRESSBARS
    // ------------
        case PM_ProgressBarChunkWidth:
            return 10;

    // SLIDER
    // ------
        case PM_SliderLength:
            return 11;

    // MENU INDICATOR
    // --------------
        case PM_MenuButtonIndicator:
            return 8;

    // CHECKBOXES / RADIO BUTTONS
    // --------------------------
        case PM_ExclusiveIndicatorWidth:    // Radiobutton size
        case PM_ExclusiveIndicatorHeight:   // 13x13
        case PM_IndicatorWidth:             // Checkbox size
        case PM_IndicatorHeight:            // 13x13
            return 13;

    // FRAMES
    // ------
        case PM_SpinBoxFrameWidth:
            return 1;

        case PM_MenuBarFrameWidth:
            return 1;

        case PM_DefaultFrameWidth: {
            if(widget && ::qt_cast<QPopupMenu*>(widget))
                return 1;
            else
                return 2;
        }

        case PM_ButtonDefaultIndicator: {
            return 0;
        }

        case PM_ButtonMargin: {
            return 2;
        }

        case PM_ButtonShiftVertical:
        case PM_ButtonShiftHorizontal: {
            return 1;
        }

        default:
            return KStyle::pixelMetric(m, widget);
    }
}


QSize PlastikStyle::sizeFromContents(ContentsType t,
                                    const QWidget *widget,
                                    const QSize &s,
                                    const QStyleOption &opt) const
{
    switch (t) {
        case CT_PopupMenuItem: {
            if (!widget || opt.isDefault())
                return s;

            const QPopupMenu *popup = dynamic_cast<const QPopupMenu *>(widget);
            QMenuItem *mi = opt.menuItem();
            int maxpmw = opt.maxIconWidth();
            int w = s.width(), h = s.height();
            bool checkable = popup->isCheckable();

            if (mi->custom()) {
                w = mi->custom()->sizeHint().width();
                h = mi->custom()->sizeHint().height();
                if (!mi->custom()->fullSpan() )
                    h += 4;
            } else if (mi->widget()) {
            // don't change the size in this case.
            } else if (mi->isSeparator()) {
                w = 20;
                h = 2;
            } else {
                if (mi->pixmap()) {
                    h = QMAX(h, mi->pixmap()->height() + 2);
                } else {
                    h = QMAX(h, 16 + 2 );
                    h = QMAX(h, popup->fontMetrics().height() + 4 );
                }

                if (mi->iconSet()) {
                    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small, QIconSet::Normal).height() + 2);
                }
            }

            if (!mi->text().isNull() && (mi->text().find('\t') >= 0)) {
                w += itemHMargin + itemFrame*2 + 7;
            } else if (mi->popup()) {
                w += 2 * arrowHMargin;
            }

            if (maxpmw) {
                w += maxpmw + 6;
            }
            if (checkable && maxpmw < 20) {
                w += 20 - maxpmw;
            }
            if (checkable || maxpmw > 0) {
                w += 12;
            }

            w += rightBorder;

            return QSize(w, h);
        }

        case CT_PushButton:
        {
            const QPushButton* btn = static_cast<const QPushButton*>(widget);

            int w = s.width() + 2 * pixelMetric(PM_ButtonMargin, widget);
            int h = s.height() + 2 * pixelMetric(PM_ButtonMargin, widget);
            if ( btn->text().isEmpty() && s.width() < 32 ) return QSize(w, h);

            return QSize( w+25, h+5 );
        }

        case CT_ToolButton:
        {
            if(widget->parent() && ::qt_cast<QToolBar*>(widget->parent()) )
                return QSize( s.width()+2*4, s.height()+2*4 );
            else
                return KStyle::sizeFromContents (t, widget, s, opt);
        }

        default:
            return KStyle::sizeFromContents (t, widget, s, opt);
    }

    return KStyle::sizeFromContents (t, widget, s, opt);
}

int PlastikStyle::styleHint( StyleHint stylehint,
                                   const QWidget *widget,
                                   const QStyleOption &option,
                                   QStyleHintReturn* returnData ) const
{
    switch (stylehint) {
        case SH_PopupMenu_SubMenuPopupDelay:
            return 96; // Motif-like delay...

        default:
            return KStyle::styleHint(stylehint, widget, option, returnData);
    }
}

bool PlastikStyle::eventFilter(QObject *obj, QEvent *ev)
{
    if (KStyle::eventFilter(obj, ev) )
        return true;

    if (!obj->isWidgetType() ) return false;
 
    // focus highlight
    if ( ::qt_cast<QLineEdit*>(obj) ) {
        QWidget* widget = static_cast<QWidget*>(obj);

        if ( ::qt_cast<QSpinWidget*>(widget->parentWidget()) )
        {
            QWidget* spinbox = widget->parentWidget();
            if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
            {
                spinbox->repaint(false);
            }
            return false;
        }

        if ((ev->type() == QEvent::FocusIn) || (ev->type() == QEvent::FocusOut))
        {
            widget->repaint(false);
        }
        return false;
    }
    
    //Hover highlight... use qt_cast to check if the widget inheits one of the classes.
    if ( ::qt_cast<QPushButton*>(obj) || ::qt_cast<QComboBox*>(obj) ||
            ::qt_cast<QSpinWidget*>(obj) || ::qt_cast<QCheckBox*>(obj) ||
            ::qt_cast<QRadioButton*>(obj) || ::qt_cast<QToolButton*>(obj) || obj->inherits("QSplitterHandle") )
    {
        if ((ev->type() == QEvent::Enter) && static_cast<QWidget*>(obj)->isEnabled())
        {
            QWidget* button = static_cast<QWidget*>(obj);
            hoverWidget = button;
            button->repaint(false);
        }
        else if ((ev->type() == QEvent::Leave) && (obj == hoverWidget) )
        {
            QWidget* button = static_cast<QWidget*>(obj);
            hoverWidget = 0;
            button->repaint(false);
        }
        return false;
    }
    if ( ::qt_cast<QTabBar*>(obj) ) {
        if ((ev->type() == QEvent::Enter) && static_cast<QWidget*>(obj)->isEnabled())
        {
            QWidget* tabbar = static_cast<QWidget*>(obj);
            hoverWidget = tabbar;
            hoverTab = 0;
            tabbar->repaint(false);
        }
        else if (ev->type() == QEvent::MouseMove)
        {
            QTabBar *tabbar = dynamic_cast<QTabBar*>(obj);
            QMouseEvent *me = dynamic_cast<QMouseEvent*>(ev);

            if (tabbar && me) {
                // avoid unnecessary repaints (which otherwise would occour on every
                // MouseMove event causing high cpu load).

                bool repaint = true;

                QTab *tab = tabbar->selectTab(me->pos() );
                if (hoverTab == tab)
                    repaint = false;
                hoverTab = tab;

                if (repaint)
                    tabbar->repaint(false);
            }
        }
        else if (ev->type() == QEvent::Leave)
        {
            QWidget* tabbar = static_cast<QWidget*>(obj);
            hoverWidget = 0;
            hoverTab = 0;
            tabbar->repaint(false);
        }
        return false;
    }
    // Track show events for progress bars
    if ( _animateProgressBar && ::qt_cast<QProgressBar*>(obj) )
    {
        if ((ev->type() == QEvent::Show) && !animationTimer->isActive())
        {
            animationTimer->start( 50, false );
        }
    }
    if ( !qstrcmp(obj->name(), "kde toolbar widget") )
    {
        QWidget* lb = static_cast<QWidget*>(obj);
        if (lb->backgroundMode() == Qt::PaletteButton)
            lb->setBackgroundMode(Qt::PaletteBackground);
        lb->removeEventFilter(this);
    }

    return false;
}

QColor PlastikStyle::getColor(const QColorGroup &cg, const ColorType t, const bool enabled)const
{
    return getColor(cg, t, enabled?IsEnabled:IsDisabled);
}

QColor PlastikStyle::getColor(const QColorGroup &cg, const ColorType t, const WidgetState s)const
{
    const bool enabled = (s != IsDisabled) &&
            ((s == IsEnabled) || (s == IsPressed) || (s == IsHighlighted));
    const bool pressed = (s == IsPressed);
    const bool highlighted = (s == IsHighlighted);
    switch(t) {
        case ButtonContour:
            return enabled ? cg.button().dark(130+_contrast*8)
                           : cg.background().dark(120+_contrast*8);
        case DragButtonContour: {
            if(enabled) {
                if(pressed)
                    return cg.button().dark(130+_contrast*6); // bright
                else if(highlighted)
                    return cg.button().dark(130+_contrast*9); // dark
                else
                    return cg.button().dark(130+_contrast*8); // normal
            } else {
                return cg.background().dark(120+_contrast*8);
            }
        }
        case DragButtonSurface: {
            if(enabled) {
                if(pressed)
                    return cg.button().dark(100-_contrast);  // bright
                else if(highlighted)
                    return cg.button().light(100+_contrast); // dark
                else
                    return cg.button();                      // normal
            } else {
                return cg.background();
            }
        }
        case PanelContour:
            return cg.background().dark(160+_contrast*8);
        case PanelDark:
            return alphaBlendColors(cg.background(), cg.background().dark(120+_contrast*5), 110);
        case PanelDark2:
            return alphaBlendColors(cg.background(), cg.background().dark(110+_contrast*5), 110);
        case PanelLight:
            return alphaBlendColors(cg.background(), cg.background().light(120+_contrast*5), 110);
        case PanelLight2:
            return alphaBlendColors(cg.background(), cg.background().light(110+_contrast*5), 110);
        case MouseOverHighlight:
            if( _customOverHighlightColor )
                return _overHighlightColor;
            else
                return cg.highlight();
        case FocusHighlight:
            if( _customFocusHighlightColor )
                return _focusHighlightColor;
            else
                return cg.highlight();
        case CheckMark:
            if( _customCheckMarkColor )
                return _checkMarkColor;
            else
                return cg.foreground();
        default:
            return cg.background();
    }
}
