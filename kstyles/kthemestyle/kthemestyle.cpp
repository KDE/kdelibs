/*
 $Id$

 This file is part of the KDE libraries
 Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

 KDE3 port (C) 2001-2002 Maksim Orlovich <mo002j@mail.rochester.edu>
Port version 0.9.6

 Includes code portions from the dotNET style, and the KDE HighColor style.

 dotNET Style
  Copyright (C) 2001, Chris Lee        <lee@azsites.com>
                    Carsten Pfeiffer <pfeiffer@kde.org>

 KDE3 HighColor Style
	Copyright (C) 2001 Karol Szwed       <gallium@kde.org>
	(C) 2001 Fredrik Höglund   <fredrik@kde.org>

	Drawing routines adapted from the KDE2 HCStyle,
		Copyright (C) 2000 Daniel M. Duley   <mosfet@kde.org>
		(C) 2000 Dirk Mueller      <mueller@kde.org>
		(C) 2001 Martijn Klingens  <klingens@kde.org>


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


#include <kthemestyle.h>
#include <kthemebase.h>
#include <qstyleplugin.h>
#include <qstylefactory.h>
#include <kimageeffect.h>

#include <qbitmap.h>
#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#include <qpopupmenu.h>
#include <qpalette.h>
#include <qtabbar.h>
#include <qtoolbutton.h>
#include <qglobal.h>
#include <kglobalsettings.h>
#include <kdrawutil.h>
#include <qdrawutil.h>
#include <qprogressbar.h>
#include <qdir.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qrangecontrol.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qobjectlist.h>
#include "kstyledirs.h"

#include <qimage.h>

#include <limits.h>

#ifdef __GLIBC__
#include <dlfcn.h>
#endif

static const QCOORD u_arrow[] = { -1, -3, 0, -3, -2, -2, 1, -2, -3, -1, 2, -1, -4, 0, 3, 0, -4, 1, 3, 1};
static const QCOORD d_arrow[] = { -4, -2, 3, -2, -4, -1, 3, -1, -3, 0, 2, 0, -2, 1, 1, 1, -1, 2, 0, 2};
static const QCOORD l_arrow[] = { -3, -1, -3, 0, -2, -2, -2, 1, -1, -3, -1, 2, 0, -4, 0, 3, 1, -4, 1, 3};
static const QCOORD r_arrow[] = { -2, -4, -2, 3, -1, -4, -1, 3, 0, -3, 0, 2, 1, -2, 1, 1, 2, -1, 2, 0};

const QCOORD win_style_u_arrow[] = { 0, -2, 0, -2, -1, -1, 1, -1, -2, 0, 2, 0, -3, 1, 3, 1 };
const QCOORD win_style_d_arrow[] = { -3, -2, 3, -2, -2, -1, 2, -1, -1, 0, 1, 0, 0, 1, 0, 1 };
const QCOORD win_style_l_arrow[] = { 1, -3, 1, -3, 0, -2, 1, -2, -1, -1, 1, -1, -2, 0, 1, 0, -1, 1, 1, 1, 0, 2, 1, 2, 1, 3, 1, 3 };
const QCOORD win_style_r_arrow[] = { -2, -3, -2, -3, -2, -2, -1, -2, -2, -1, 0, -1, -2, 0, 1, 0, -2, 1, 0, 1, -2, 2, -1, 2, -2, 3, -2, 3 };


#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)


static const int itemFrame = 2;
static const int itemHMargin = 3;
static const int itemVMargin = 1;
static const int arrowHMargin = 6;
static const int rightBorder = 12;


/*
BUGS:
Sliders flash a bit -- anything else?

TODO:
Nicer disabled buttons.
Sliders are not disabled properly
*/


class KThemeStylePlugin : public QStylePlugin
{
public:

    KThemeStylePlugin()
    {
#ifdef __GLIBC__
        dlopen("kthemestyle.so",RTLD_LAZY);
	//####### Keep reference count up so kdecore w. fast-malloc doesn't get unloaded
	//####### (Fixes exit crashes with qt-only apps that occur on Linux)
	//####### This should be rethought after 3.0,
	//####### as it relies on the implementation-specific behavior
	//####### of the glibc libdl (finding already loaded libraries based on the
	//####### soname)
#endif
    }

    ~KThemeStylePlugin()
    {}

    QStringList keys() const
    {
        QSettings cfg;
        KStyleDirs::dirs()->addToSearch( "config", cfg );

        QStringList keys;
        bool ok;

        keys = cfg.readListEntry( "/kthemestyle/themes", &ok);
        if ( !ok )
            qWarning( "KThemeStyle cache seems corrupt!\n" ); //Too bad one can't i18n this :-(
	    
        return keys;
    }

    QStyle* create( const QString& key )
    {
        QSettings cfg;
        KStyleDirs::dirs()->addToSearch( "config", cfg );

        QString file = cfg.readEntry( "/kthemestyle/" + key + "/file" );
        if ( !key.isEmpty() )
        {
            QFileInfo fi( file );
            return new KThemeStyle( fi.dirPath(), fi.fileName() );
        }

        return 0;
    }
};

Q_EXPORT_PLUGIN( KThemeStylePlugin )


void kDrawWindowsArrow ( QPainter *p, const QStyle* style, QStyle::PrimitiveElement pe, bool down,
                         int x, int y, int w, int h,
                         const QColorGroup &cg, bool enabled )
{
    QPointArray a;
    switch ( pe )
    {
        case QStyle::PE_ArrowUp:
            a.setPoints( QCOORDARRLEN( win_style_u_arrow ), win_style_u_arrow );
            break;

        case QStyle::PE_ArrowDown:
            a.setPoints( QCOORDARRLEN( win_style_d_arrow ), win_style_d_arrow );
            break;

        case QStyle::PE_ArrowLeft:
            a.setPoints( QCOORDARRLEN( win_style_l_arrow ), win_style_l_arrow );
            break;
        default:
            a.setPoints( QCOORDARRLEN( win_style_r_arrow ), win_style_r_arrow );
    }

    p->save();
    if ( down )
    {
        p->translate( style->pixelMetric( QStyle::PM_ButtonShiftHorizontal ),
                      style->pixelMetric( QStyle::PM_ButtonShiftVertical ) );
    }

    if ( enabled )
    {
        a.translate( x + w / 2, y + h / 2 );
        p->setPen( cg.buttonText() );
        p->drawLineSegments( a );
    }
    else
    {
        a.translate( x + w / 2 + 1, y + h / 2 + 1 );
        p->setPen( cg.light() );
        p->drawLineSegments( a );
        a.translate( -1, -1 );
        p->setPen( cg.mid() );
        p->drawLineSegments( a );
    }

    p->restore();

}



QSize KThemeStyle::sizeFromContents( ContentsType contents,
                                     const QWidget* widget,
                                     const QSize &contentSize,
                                     const QStyleOption& opt ) const
{
    switch ( contents )
    {
            // PUSHBUTTON SIZE
            // ------------------------------------------------------------------
        case CT_PushButton:
            {
                const QPushButton * button = ( const QPushButton* ) widget;
                int w = contentSize.width();
                int h = contentSize.height();
                int bm = pixelMetric( PM_ButtonMargin, widget );
                int fw = pixelMetric( PM_DefaultFrameWidth, widget ) * 2;

                w += bm + fw + 6; // ### Add 6 to make way for bold font.
                h += bm + fw;

                // Ensure we stick to standard width and heights.
                if ( button->isDefault() || button->autoDefault() )
                {
                    if ( w < 80 && !button->pixmap() )
                        w = 80;
                }

                if ( h < 22 )
                    h = 22;

                return QSize( w, h );
            }

            // POPUPMENU ITEM SIZE
            // -----------------------------------------------------------------
        case CT_PopupMenuItem:
            {
                if ( ! widget || opt.isDefault() )
                    return contentSize;

                const QPopupMenu *popup = ( const QPopupMenu * ) widget;
                bool checkable = popup->isCheckable();
                QMenuItem *mi = opt.menuItem();
                int maxpmw = opt.maxIconWidth();
                int w = contentSize.width(), h = contentSize.height();

                if ( mi->custom() )
                {
                    w = mi->custom() ->sizeHint().width();
                    h = mi->custom() ->sizeHint().height();
                    if ( ! mi->custom() ->fullSpan() )
                        h += 2 * itemVMargin + 2 * itemFrame;
                }
                else if ( mi->widget() )
                {}
                else if ( mi->isSeparator() )
                {
                    w = 10; // Arbitrary
                    h = 2;
                }
                else
                {
                    if ( mi->pixmap() )
                        h = QMAX( h, mi->pixmap() ->height() + 2 * itemFrame );
                    else
                        h = QMAX( h, popup->fontMetrics().height()
                                  + 2 * itemVMargin + 2 * itemFrame );

                    if ( mi->iconSet() )
                        h = QMAX( h, mi->iconSet() ->pixmap(
                                      QIconSet::Small, QIconSet::Normal ).height() +
                                  2 * itemFrame );
                }

                if ( ! mi->text().isNull() && mi->text().find( '\t' ) >= 0 )
                    w += 12;
                else if ( mi->popup() )
                    w += 2 * arrowHMargin;

                if ( maxpmw )
                    w += maxpmw + 6;
                if ( checkable && maxpmw < 20 )
                    w += 20 - maxpmw;
                if ( checkable || maxpmw > 0 )
                    w += 12;

                w += rightBorder;

                return QSize( w, h );
            }

        default:
            return KThemeBase::sizeFromContents( contents, widget, contentSize, opt );
    }
}


int KThemeStyle::pixelMetric ( PixelMetric metric, const QWidget * widget ) const
{
    switch ( metric )
    {
        case PM_MenuBarFrameWidth:
            return 1;

        case PM_DefaultFrameWidth:
            return ( frameWidth() );

        case PM_ButtonMargin:
            return decoWidth( PushButton ) > decoWidth( PushButtonDown ) ?
                   3 + decoWidth( PushButton ) : 3 + decoWidth( PushButtonDown );

        case PM_ScrollBarExtent:
        case PM_SliderThickness:  //Should this be 16 allways?
            return getSBExtent();

        case PM_ButtonDefaultIndicator:
            return 0;

        case PM_ButtonShiftHorizontal:
            return buttonXShift();

        case PM_ButtonShiftVertical:
            return buttonYShift();

        case PM_ExclusiveIndicatorWidth:
            if ( isPixmap( ExIndicatorOn ) )
                return ( uncached( ExIndicatorOn ) ->size().width() );
            else
                return KThemeBase::pixelMetric ( metric, widget );

        case PM_ExclusiveIndicatorHeight:
            if ( isPixmap( ExIndicatorOn ) )
                return ( uncached( ExIndicatorOn ) ->size().height() );
            else
                return KThemeBase::pixelMetric ( metric, widget );


        case PM_IndicatorWidth:
            if ( isPixmap( IndicatorOn ) )
                return ( uncached( IndicatorOn ) ->size().width() );
            else
                return KThemeBase::pixelMetric ( metric, widget );

        case PM_IndicatorHeight:
            if ( isPixmap( IndicatorOn ) )
                return ( uncached( IndicatorOn ) ->size().height() );
            else
                return KThemeBase::pixelMetric ( metric, widget );

        case PM_SliderLength:
            return ( sliderButtonLength() );

        case PM_SplitterWidth:
            return ( splitWidth() );

        default:
            return KThemeBase::pixelMetric ( metric, widget );
    }
};



KThemeStyle::KThemeStyle( const QString& configDir, const QString &configFile )
        : KThemeBase( configDir, configFile ), paletteSaved( false ), polishLock( false ), menuCache( 0 ), vsliderCache( 0 ),
         brushHandle( 0 ), brushHandleSet( false )
{
    mtfstyle = QStyleFactory::create( "Motif" );
    if ( !mtfstyle )
        mtfstyle = QStyleFactory::create( *( QStyleFactory::keys().begin() ) );
}

KThemeStyle::~KThemeStyle()
{
    delete vsliderCache;
    delete menuCache;

}


void KThemeStyle::polish( QApplication * /*app*/ )
{}


void KThemeStyle::polish( QPalette &p )
{
    if ( polishLock )
    {
        return ; //Palette polishing disabled ...
    }



    if ( !paletteSaved )
    {
        oldPalette = p;
        paletteSaved = true;
    }

    p = overridePalette( p );

    if ( isPixmap( Background ) )
    {
        QBrush bgBrush( p.color( QPalette::Normal,
                                QColorGroup::Background ),
                                *uncached( Background ) );
        brushHandle = uncached( Background )->handle();
        brushHandleSet = true;
        p.setBrush( QColorGroup::Background, bgBrush );
    }

}

void KThemeStyle::paletteChanged()
{
    QPalette p = QApplication::palette();
    polish( p );
    QApplication::setPalette( p );
}


void KThemeStyle::unPolish( QApplication *app )
{
    app->setPalette( oldPalette, true );
}

bool KThemeStyle::eventFilter( QObject* object, QEvent* event )
{
    if( object->inherits("KActiveLabel"))
    {
        if(event->type() == QEvent::Move || event->type() == QEvent::Resize ||
            event->type() == QEvent::Show)
        {
            QWidget *w = static_cast<QWidget*>(object);
            QPoint pos(0, 0);
            pos = w->mapTo(w->topLevelWidget(), pos);
            QPixmap pix(uncached( Background )->size());
            QPainter p;
            p.begin(&pix);
            p.drawTiledPixmap(0, 0,
                            uncached( Background )->width(),
                            uncached( Background )->height() ,
                            *uncached( Background ),
                            pos.x(), pos.y());
            p.end();
            QPalette pal(w->palette());
            QBrush brush( pal.color( QPalette::Normal,
                                                    QColorGroup::Background),
                                pix );
            pal.setBrush(QColorGroup::Base, brush);
            w->setPalette(pal);
        }
    }

    return KStyle::eventFilter(object, event);
}

void KThemeStyle::polish( QWidget *w )
{
    if (w->inherits("QStatusBar"))
        w->setPaletteBackgroundColor(QApplication::palette().color(QPalette::Normal, QColorGroup::Background));

    if (w->backgroundPixmap() && !w->isTopLevel())
    {
        //The brushHandle check verifies that the bg pixmap is actually the brush..
        if (!brushHandleSet || brushHandle == w->backgroundPixmap()->handle())
        {
            w->setBackgroundOrigin( QWidget::WindowOrigin );
        }
    }

    if (w->inherits("KActiveLabel"))
    {
        if (uncached( Background ))
            w->installEventFilter(this);
    }

    if ( w->inherits( "QTipLabel" ) )
    {
        polishLock = true;

        QColorGroup clrGroup( Qt::black, QColor( 255, 255, 220 ),
                              QColor( 96, 96, 96 ), Qt::black, Qt::black,
                              Qt::black, QColor( 255, 255, 220 ) );
        QPalette toolTip ( clrGroup, clrGroup, clrGroup );

        QToolTip::setPalette( toolTip );
        polishLock = false;
    }

    if ( w->inherits( "KonqIconViewWidget" ) )   //Konqueror background hack/workaround
    {
        w->setPalette( oldPalette );
        return ;
    }

    if ( w->inherits( "QMenuBar" ) || w->inherits( "QScrollBar" ) || w->inherits( "QToolBar" ) || w->inherits ("QToolButton") )
    {
        w->setBackgroundMode( QWidget::NoBackground );
    }
    else if ( w->inherits( "KToolBarSeparator" ) || w->inherits( "QToolBarSeparator" ) )
    {
        w->setBackgroundMode( QWidget::PaletteBackground );
    }
    else if ( w->inherits( "QMenuItem" ) || w->inherits( "QPopupMenu" ) )
    {
        popupPalette = w->palette();
        if ( isColor( MenuItem ) || isColor( MenuItemDown ) )
        {
            QPalette newPal( w->palette() );
            if ( isColor( MenuItem ) )
            {
                newPal.setActive( *colorGroup( newPal.active(), MenuItem ) );
                newPal.setDisabled( *colorGroup( newPal.active(), MenuItem ) );
            }
            if ( isColor( MenuItemDown ) )
            {
                newPal.setActive( *colorGroup( newPal.active(), MenuItemDown ) );
            }
            w->setPalette( newPal );
        }

        if ( w->inherits( "QPopupMenu" ) )
            w->setBackgroundMode( QWidget::NoBackground );
    }
    else if ( w->inherits( "QCheckBox" ) )
    {
        if ( isColor( IndicatorOff ) || isColor( IndicatorOn ) )
        {
            QPalette newPal( w->palette() );
            if ( isColor( IndicatorOff ) )
            {
                newPal.setActive( *colorGroup( newPal.active(), IndicatorOff ) );
                newPal.setDisabled( *colorGroup( newPal.active(), IndicatorOff ) );
            }
            if ( isColor( IndicatorOn ) )
                newPal.setActive( *colorGroup( newPal.active(), IndicatorOn ) );
            w->setPalette( newPal );
        }
    }
    else if ( w->inherits( "QRadioButton" ) )
    {
        if ( isColor( ExIndicatorOff ) || isColor( ExIndicatorOn ) )
        {
            QPalette newPal( w->palette() );
            if ( isColor( ExIndicatorOff ) )
            {
                newPal.setActive( *colorGroup( newPal.active(), ExIndicatorOff ) );
                newPal.setDisabled( *colorGroup( newPal.active(),
                                                 ExIndicatorOff ) );
            }
            if ( isColor( ExIndicatorOn ) )
                newPal.setActive( *colorGroup( newPal.active(), ExIndicatorOn ) );
            w->setPalette( newPal );
        }
    }

    KStyle::polish( w );
}

void KThemeStyle::unPolish( QWidget* w )
{
    if (w->backgroundPixmap() && !w->isTopLevel())
    {
        //The brushHandle check verifies that the bg pixmap is actually the brush..
        if (!brushHandleSet || brushHandle ==w->backgroundPixmap()->handle())
        {
            w->setBackgroundOrigin( QWidget::WidgetOrigin );
        }
    }
    if ( w->inherits( "QMenuBar" ) || w->inherits( "QPopupMenu" ) || w->inherits( "QMenuItem" ) ||
            w->inherits( "QScrollBar" ) || w->inherits( "QToolBar" )  || w->inherits ("QToolButton") )
        w->setBackgroundMode( QWidget::PaletteBackground );
    if ( w->inherits( "QPopupMenu" )  || w->inherits( "QMenuItem" ) )
        w->unsetPalette();
    else if ( w->inherits( "QCheckBox" ) )
        w->unsetPalette();
    else if ( w->inherits( "QRadioButton" ) )
        w->unsetPalette();
    KStyle::unPolish( w );
}


void KThemeStyle::drawBaseButton( QPainter *p, int x, int y, int w, int h,
                                  const QColorGroup &g, bool sunken, bool
                                  rounded, WidgetType type ) const
{
    int offset = borderPixmap( type ) ? 0 : decoWidth( type ) ; //##### This is wrong, but the code relies on it..
    QPen oldPen = p->pen();

    // handle reverse bevel here since it uses decowidth differently
    if ( gradientHint( type ) == GrReverseBevel )
    {
        int i;
        bitBlt( p->device(), x, y, scalePixmap( w, h, type ), 0, 0, w, h,
                Qt::CopyROP, true );
        p->setPen( g.text() );
        for ( i = 0; i < borderWidth( type ); ++i, ++x, ++y, w -= 2, h -= 2 )
            p->drawRect( x, y, w, h );
    }
    // same with KDE style borders
    else if ( !borderPixmap( type ) && shade() == KDE )
    {
        kDrawBeButton( p, x, y, w, h, g, sunken );
        if ( isPixmap( type ) )
            p->drawTiledPixmap( x + 4, y + 4, w - 6, h - 6,
                                *scalePixmap( w - 6, h - 6,
                                              type ) );
        else
            p->fillRect( x + 4, y + 4, w - 6, h - offset * 6,
                         g.brush( QColorGroup::Button ) );

    }
    else
    {
        if ( ( w - offset * 2 ) > 0 && ( h - offset * 2 ) > 0 )
        {
            if ( isPixmap( type ) )
                if ( rounded )
                    p->drawTiledPixmap( x, y, w, h, *scalePixmap( w, h, type ) );
                else
                    p->drawTiledPixmap( x + offset, y + offset, w - offset * 2,
                                        h - offset * 2,
                                        *scalePixmap( w - offset * 2, h - offset * 2,
                                                      type ) );
            else if ( 1 )  //##### TODO - Get this optimization working... !borderPixmap( type ) || (( w - decoWidth(type) * 2 ) > 0 && ( h - decoWidth(type) * 2 ) > 0) )
                //Sometimes border covers the whole thing - in that case, avoid drawing the base.
            {
                p->fillRect( x + offset, y + offset, w - offset * 2, h - offset * 2,
                             g.brush( QColorGroup::Button ) );
            }
        }
        if ( borderPixmap( type ) )
        {
            bitBlt( p->device(), x, y, scaleBorder( w, h, type ), 0, 0, w, h,
                    Qt::CopyROP, false );
        }
        else
            drawShade( p, x, y, w, h, g, sunken, rounded,
                       highlightWidth( type ), borderWidth( type ), shade() );
    }
    p->setPen( oldPen );
}

void KThemeStyle::drawPrimitive ( PrimitiveElement pe, QPainter * p, const QRect & r, const QColorGroup & g_base,
                                  SFlags flags, const QStyleOption & opt ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    bool sunken = ( flags & Style_Sunken );
    bool enabled = ( flags & Style_Enabled );
    bool down = ( flags & Style_Down );
    bool on = flags & Style_On;
    QColorGroup g = g_base;
    

    switch ( pe )
    {
        case PE_ArrowUp:
        case PE_ArrowDown:
        case PE_ArrowRight:
        case PE_ArrowLeft:
            {
                QRect r( x, y, w, h );
                if ( r.width() > 12 )
                {
                    r.setRect( r.x() + ( r.width() - 12 ) / 2, r.y(), 12, r.height() );
                }
                if ( r.height() > 12 )
                {
                    r.setRect( r.x(), r.y() + ( r.height() - 12 ) / 2, r.width(), 12 );
                }
                r.rect( &x, &y, &w, &h );
                // Handles pixmapped arrows. A little inefficent because you can specify
                // some as pixmaps and some as default types.
                WidgetType widget;
                switch ( pe )
                {
                    case PE_ArrowUp:
                    widget = enabled ? down ? SunkenArrowUp : ArrowUp : DisArrowUp;
                        break;
                    case PE_ArrowDown:
                    widget = enabled ? down ? SunkenArrowDown : ArrowDown : DisArrowDown;
                        break;
                    case PE_ArrowLeft:
                    widget = enabled ? down ? SunkenArrowLeft : ArrowLeft : DisArrowLeft;
                        break;
                    case PE_ArrowRight:
                    default:
                    widget = enabled ? down ? SunkenArrowRight : ArrowRight : DisArrowRight;
                        break;
                }
                if ( isPixmap( widget ) )
                {
                    bitBlt( p->device(), x + ( w - uncached( widget ) ->width() ) / 2,
                            y + ( h - uncached( widget ) ->height() ) / 2,
                            uncached( widget ) );

                    return ;
                }
                const QColorGroup *cg = colorGroup( g, widget );
                // Standard arrow types
                if ( arrowType() == MotifArrow )
                {
                    mtfstyle->drawPrimitive( pe, p, r, g, flags, opt );

                    handled = true;
                }
                else if ( arrowType() == SmallArrow )
                {
                    // #### FIXME: This should be like the Platinum style - uses HighColor look for now
                    QPointArray a;

                    switch ( pe )
                    {
                        case PE_ArrowUp:
                            a.setPoints( QCOORDARRLEN( u_arrow ), u_arrow );
                            break;

                        case PE_ArrowDown:
                            a.setPoints( QCOORDARRLEN( d_arrow ), d_arrow );
                            break;

                        case PE_ArrowLeft:
                            a.setPoints( QCOORDARRLEN( l_arrow ), l_arrow );
                            break;

                        default:
                            a.setPoints( QCOORDARRLEN( r_arrow ), r_arrow );
                    }

                    p->save();

                    if ( flags & Style_Down )
                        p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
                                      pixelMetric( PM_ButtonShiftVertical ) );

                    if ( flags & Style_Enabled )
                    {
                        a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
                        p->setPen( cg->buttonText() );
                        p->drawLineSegments( a );
                    }
                    else
                    {
                        a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
                        p->setPen( cg->mid() );
                        p->drawLineSegments( a );
                    }
                    p->restore();
                }
                else
                {
                    QPointArray a;
                    int x2 = x + w - 1, y2 = y + h - 1;
                    switch ( pe )
                    {
                        case PE_ArrowUp:
                            a.setPoints( 4, x, y2, x2, y2, x + w / 2, y, x, y2 );
                            break;
                        case PE_ArrowDown:
                            a.setPoints( 4, x, y, x2, y, x + w / 2, y2, x, y );
                            break;
                        case PE_ArrowLeft:
                            a.setPoints( 4, x2, y, x2, y2, x, y + h / 2, x2, y );
                            break;
                        default:
                            a.setPoints( 4, x, y, x, y2, x2, y + h / 2, x, y );
                            break;
                    }
                    QBrush oldBrush = p->brush();
                    QPen oldPen = p->pen();
                    p->setBrush( cg->brush( QColorGroup::Shadow ) );
                    p->setPen( cg->shadow() );
                    p->drawPolygon( a );
                    p->setBrush( oldBrush );
                    p->setPen( oldPen );
                    handled = true;
                }
                break;

            }
        case PE_HeaderSection:
            {
                sunken = false; //Never mind this one
            }
        case PE_ButtonBevel:
            {
                WidgetType type = ( sunken || on || down ) ? BevelDown : Bevel;
                drawBaseButton( p, x, y, w, h, *colorGroup( g, type ), ( sunken || on || down ), false, type );
                handled = true;
                break;
            }
        case PE_ButtonCommand:
            {
                drawBaseButton( p, x, y, w, h, g, ( sunken || on || down ), roundButton(), ( sunken || on || down ) ?
                                PushButtonDown : PushButton );
                handled = true;
                break;
            }
        case PE_PanelDockWindow:
            {
                drawBaseButton( p, x, y, w, h, *colorGroup( g, ToolBar ), false, false,
                                ToolBar );
                handled = true;
                break;
            }
        case PE_CheckMark:
            {
                if ( isPixmap( CheckMark ) )
                {
                    if ( flags & Style_Enabled || flags & Style_On )
                        bitBlt( p->device(), x + ( w - uncached( CheckMark ) ->width() ) / 2,
                                y + ( h - uncached( CheckMark ) ->height() ) / 2,
                                uncached( CheckMark ) );
                    handled = true;
                }
                else //Small hack to ensure the checkmark gets painter proper color..
                {
                    g.setColor( QColorGroup::Text, g.buttonText() );
                }
                break;
            }
        case PE_ExclusiveIndicator:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) )
                {
                    bitBlt( p->device(), x, y, uncached( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn :
                                                         ExIndicatorOff ) );
                    handled = true;
                }

                break;
            }
        case PE_ExclusiveIndicatorMask:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) )
                {
                    const QBitmap * mask = uncached( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) ->mask();
                    if ( mask )
                    {
                        p->setPen( Qt::color1 );
                        p->drawPixmap( x, y, *mask );
                    }
                    else
                        p->fillRect( x, y, w, h, QBrush( color1, SolidPattern ) );
                    handled = true;
                }
                break;
            }

        case PE_IndicatorMask:
            {
                if ( isPixmap( ( flags & Style_On ) ? IndicatorOn : IndicatorOff ) )
                {
                    const QBitmap * mask = uncached( ( flags & Style_On ) ? IndicatorOn :
                                                     IndicatorOff ) ->mask();
                    if ( mask )
                    {
                        p->setPen( Qt::color1 );
                        p->drawPixmap( x, y, *mask );
                    }
                    else
                        p->fillRect( x, y, w, h, QBrush( color1, SolidPattern ) );
                    handled = true;
                }
                break;
            }
        case PE_Indicator:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ?
                               IndicatorOn : IndicatorOff ) )
                {
                    bitBlt( p->device(), x, y, uncached( ( flags & Style_On || flags & Style_Down ) ?
                                                         IndicatorOn : IndicatorOff ) );
                    handled = true;
                }
                break;
            }
        case PE_PanelPopup:
            {
                if (h==8 && w==50) //Used by Qt when no items..  #### WORKAROUND/HACK
                    p->fillRect(r, g.brush(QColorGroup::Background) );
                break;
            }
        case PE_Splitter:
            {
                drawBaseButton( p, x, y, w, h, *colorGroup( g, Splitter ), false, false,
                                Splitter );
                handled = true;
                break;
            }
        case PE_FocusRect:
            {
                p->setPen( g.dark() );
                if ( is3DFocus() )
                {
                    int i = focusOffset();
                    p->drawLine( r.x() + i, r.y() + 1 + i, r.x() + i, r.bottom() - 1 - i );
                    p->drawLine( r.x() + 1 + i, r.y() + i, r.right() - 1 - i, r.y() + i );
                    p->setPen( g.light() );
                    p->drawLine( r.right() - i, r.y() + 1 + i, r.right() - i, r.bottom() - 1 - i );
                    p->drawLine( r.x() + 1 + i, r.bottom() - i, r.right() - 1 - i, r.bottom() - i );
                    handled = true;
                }
                break;
            }
        case PE_PanelMenuBar:
            {
                QPixmap* cache = makeMenuBarCache(w, h);
                p->drawPixmap( x, y, *cache);
                handled = true;
                break;
            }
        case PE_ScrollBarAddPage:
        case PE_ScrollBarSubPage:
            {
                WidgetType widget = ( flags & Style_Horizontal ) ? HScrollGroove : VScrollGroove;

                if ( !isPixmap( widget ) )
                {
                    p->fillRect( r, colorGroup( g, widget ) ->brush( QColorGroup::Background ) );
                    // Do the borders and frame
                    drawShade( p, r.x(), r.y(), r.width(),
                               r.height(), *colorGroup( g, widget ), true, false,
                               highlightWidth( widget ), borderWidth( widget ), shade() );
                }
                else
                {
                    // If the groove is pixmapped we make a full-sized image (it gets
                    // cached) then bitBlt it to the appropriate rect.
                    p->drawTiledPixmap( r.x(), r.y(), r.width(), r.height(),
                                        *scalePixmap( r.width(), r.height(),
                                                      widget ) );
                    drawShade( p, r.x(), r.y(), r.width(),
                               r.height(), *colorGroup( g, widget ), true, false,
                               highlightWidth( widget ), borderWidth( widget ), shade() );
                }

                handled = true;
                break;
            }
        case PE_ScrollBarAddLine:
            {
                bool horizontal = ( flags & Style_Horizontal );
                drawBaseButton( p, r.x(), r.y(), r.width(), r.height(),
                                *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                                down, false, down ? ScrollButtonDown : ScrollButton );

                drawPrimitive( ( horizontal ) ? PE_ArrowRight : PE_ArrowDown, p ,
                               QRect( r.x() + 3, r.y() + 3, r.width() - 6, r.height() - 6 ),
                               *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                               flags );

                handled = true;
                break;
            }
        case PE_ScrollBarSubLine:
            {
                bool horizontal = ( flags & Style_Horizontal );
                drawBaseButton( p, r.x(), r.y(), r.width(), r.height(),
                                *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                                down, false, down ? ScrollButtonDown : ScrollButton );

                drawPrimitive( ( horizontal ) ? PE_ArrowLeft : PE_ArrowUp, p ,
                               QRect( r.x() + 3, r.y() + 3, r.width() - 6, r.height() - 6 ),
                               *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                               flags );
                handled = true;
                break;
            }
        case PE_ScrollBarSlider:
            {
                bool active = ( flags & Style_Active ) || ( flags & Style_Down ); //activeControl == QStyle::AddLine;
                bool horizontal = ( flags & Style_Horizontal );
                int offsetH = horizontal ? 0: decoWidth(VScrollGroove) ;
                int offsetV = horizontal ? decoWidth(HScrollGroove):0;

                WidgetType widget = horizontal ?
                                active ? HScrollBarSliderDown : HScrollBarSlider :
                                    active ? VScrollBarSliderDown : VScrollBarSlider;
                drawBaseButton( p, r.x()+offsetH, r.y()+offsetV, r.width()-2*offsetH,
                                r.height()-2*offsetV, *colorGroup( g, widget ), active, false,
                                widget );

                int spaceW = horizontal ? r.width() - decoWidth( widget ) - 4 :
                             r.width();
                int spaceH = horizontal ? r.height() :
                             r.height() - decoWidth( widget ) - 4;

                widget = active ? horizontal ? HScrollDecoDown : VScrollDecoDown :
                         horizontal ? HScrollDeco : VScrollDeco;
                if ( isPixmap( widget ) )
                {
                    if ( spaceW >= uncached( widget ) ->width() &&
                            spaceH >= uncached( widget ) ->height() )
                    {
                        bitBlt( p->device(),
                                r.x() + ( r.width() - uncached( widget ) ->width() ) / 2,
                                r.y() + ( r.height() - uncached( widget ) ->height() ) / 2,
                                uncached( widget ) );
                    }
                }
                handled = true;
                break;

            }
        default:
            handled = false;
    }

    if ( !handled )
        KThemeBase::drawPrimitive ( pe, p, r, g,
                                    flags, opt );
}



QPixmap* KThemeStyle::makeMenuBarCache(int w, int h) const
{
    if (menuCache)
    {
        if (menuCache->width() != w || menuCache->height() != h )
        {
            delete menuCache;
        }
        else
            return menuCache;
    }

    const QColorGroup *g = colorGroup( QApplication::palette().active(), MenuBar);

    menuCache = new QPixmap ( w, h );
    QPainter p(menuCache);
    drawBaseButton( &p, 0, 0, w, h, *g, false, false, MenuBar );
    p.end();
    return menuCache;
}


void KThemeStyle::drawControl( ControlElement element,
                               QPainter *p,
                               const QWidget *widget,
                               const QRect &r,
                               const QColorGroup &cg,
                               SFlags how ,
                               const QStyleOption& opt ) const
{
    bool handled = false;
    bool sunken = ( how & Style_Sunken );
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );


    switch ( element )
    {
        case CE_PushButton:
            {
                const QPushButton * btn = ( const QPushButton* ) widget;
                bool sunken = btn->isOn() || btn->isDown();
                int diw = pixelMetric( PM_ButtonDefaultIndicator, btn );
                drawBaseButton( p, diw, diw, btn->width() - 2 * diw, btn->height() - 2 * diw,
                                *colorGroup( btn->colorGroup(), sunken ? PushButtonDown :
                                             PushButton ), sunken, roundButton(),
                                sunken ? PushButtonDown : PushButton );
                // TODO if diw, draw fancy default button indicator
                handled = true;
                break;
            }
        case CE_PushButtonLabel:
            {
                const QPushButton* button = ( const QPushButton* ) widget;
                bool active = button->isOn() || button->isDown();
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );

                // Shift button contents if pushed.
                if ( active )
                {
                    x += pixelMetric( PM_ButtonShiftHorizontal, widget );
                    y += pixelMetric( PM_ButtonShiftVertical, widget );
                    how |= Style_Sunken;
                }

                // Does the button have a popup menu?
                if ( button->isMenuButton() )
                {
                    int dx = pixelMetric( PM_MenuButtonIndicator, widget );
                    drawPrimitive( PE_ArrowDown, p, QRect( x + w - dx - 2, y + 2, dx, h - 4 ),
                                   cg, how, opt );
                    w -= dx;
                }

                // Draw the icon if there is one
                if ( button->iconSet() && !button->iconSet() ->isNull() )
                {
                    QIconSet::Mode mode = QIconSet::Disabled;
                    QIconSet::State state = QIconSet::Off;

                    if ( button->isEnabled() )
                        mode = button->hasFocus() ? QIconSet::Active : QIconSet::Normal;
                    if ( button->isToggleButton() && button->isOn() )
                        state = QIconSet::On;

                    QPixmap pixmap = button->iconSet() ->pixmap( QIconSet::Small, mode, state );
                    p->drawPixmap( x + 4, y + h / 2 - pixmap.height() / 2, pixmap );
                    int pw = pixmap.width();
                    x += pw + 4;
                    w -= pw + 4;
                }

                // Make the label indicate if the button is a default button or not
                if ( active || button->isDefault() && button->isEnabled() )
                {
                    // Draw "fake" bold text  - this enables the font metrics to remain
                    // the same as computed in QPushButton::sizeHint(), but gives
                    // a reasonable bold effect.
                    int i;

                    // Text shadow
                    for ( i = 0; i < 2; i++ )
                        drawItem( p, QRect( x + i + 1, y + 1, w, h ), AlignCenter | ShowPrefix,
                                  button->colorGroup(), button->isEnabled(), button->pixmap(),
                                  button->text(), -1,
                                  active ? &button->colorGroup().dark() : &button->colorGroup().mid() );

                    // Normal Text
                    for ( i = 0; i < 2; i++ )
                        drawItem( p, QRect( x + i, y, w, h ), AlignCenter | ShowPrefix,
                                  button->colorGroup(), true, button->pixmap(),
                                  button->text(), -1,
                                  active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
                }
                else
                {
                    if ( button->isEnabled() )
                    {
                        drawItem( p, QRect( x, y, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
                    }
                    else
                    {
                        //TODO: Handle reversed
                        drawItem( p, QRect( x + 1, y + 1, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  &button->colorGroup().light() );

                        drawItem( p, QRect( x, y, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  &button->colorGroup().buttonText() );
                    }
                }

                // Draw a focus rect if the button has focus
                if ( how & Style_HasFocus )
                    drawPrimitive( PE_FocusRect, p,
                                   QStyle::visualRect( subRect( SR_PushButtonFocusRect, widget ), widget ),
                                   cg, how );
                handled = true;
                break;
            }

        case CE_TabBarTab:
            {
                const QTabBar* tb = ( const QTabBar* ) widget;
                QTabBar::Shape tbs = tb->shape();
                bool selected = how & Style_Selected;
                WidgetType widget = selected ? ActiveTab : InactiveTab;
                const QColorGroup *cg = colorGroup( tb->colorGroup(), widget );
                int i;
                int x2 = x + w - 1, y2 = y + h - 1;
                int bWidth = borderWidth( widget );
                int hWidth = highlightWidth( widget );
                handled = true;
                if ( tbs == QTabBar::RoundedAbove )
                {
                    if ( !selected )
                    {
                        p->fillRect( x, y, x2 - x + 1, 2,
                                     tb->palette().active().brush( QColorGroup::Background ) );
                        y += 2;
                    }
                    p->setPen( cg->text() );
                    i = 0;
                    if ( i < bWidth )
                    {
                        p->drawLine( x, y + 1, x, y2 );
                        p->drawLine( x2, y + 1, x2, y2 );
                        p->drawLine( x + 1, y, x2 - 1, y );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                        ++i, ++x, ++y, --x2;
                    }
                    for ( ; i < bWidth; ++i, ++x, ++y, --x2 )
                    {
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->drawLine( x, y, x2, y );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                    }
                    i = 0;
                    if ( i < hWidth && bWidth == 0 )
                    {
                        p->setPen( cg->light() );
                        p->drawLine( x, y + 1, x, y2 );
                        p->drawLine( x + 1, y, x2 - 1, y );
                        p->setPen( cg->dark() );
                        p->drawLine( x2, y + 1, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                        ++i, ++x, ++y, --x2;
                    }
                    for ( ; i < hWidth; ++i, ++x, ++y, --x2 )
                    {
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x, y, x2, y );
                        p->setPen( cg->dark() );
                        p->drawLine( x2, y + 1, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                    }
                    if ( isPixmap( widget ) )
                        p->drawTiledPixmap( x, y, x2 - x + 1, y2 - y + 1,
                                            *scalePixmap( x2 - x + 1, y2 - y + 1, widget ) );
                    else
                        p->fillRect( x, y, x2 - x + 1, y2 - y + 1, cg->background() );
                }
                else if ( tb->shape() == QTabBar::RoundedBelow )
                {
                    if ( !selected )
                    {
                        p->fillRect( x, y2 - 2, x2 - x + 1, 2,
                                     tb->palette().active().brush( QColorGroup::Background ) );
                        y2 -= 2;
                    }
                    p->setPen( cg->text() );
                    i = 0;
                    if ( i < bWidth )
                    {
                        p->drawLine( x, y, x, y2 - 1 );
                        p->drawLine( x2, y, x2, y2 - 1 );
                        p->drawLine( x + 1, y2, x2 - 1, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    for ( ; i < bWidth; ++i, ++x, --x2, --y2 )
                    {
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->drawLine( x, y2, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    i = 0;
                    if ( i < hWidth && bWidth == 0 )
                    {
                        p->setPen( cg->dark() );
                        p->drawLine( x + 1, y2, x2 - 1, y2 );
                        p->drawLine( x2, y, x2, y2 - 1 );
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 - 1 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                        ++i, ++x, --x2, --y2;
                    }
                    for ( ; i < hWidth; ++i, ++x, --x2, --y2 )
                    {
                        p->setPen( cg->dark() );
                        p->drawLine( x, y2, x2, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    if ( isPixmap( widget ) )
                        p->drawTiledPixmap( x, y, x2 - x + 1, y2 - y + 1,
                                            *scalePixmap( x2 - x + 1, y2 - y + 1, widget ) );
                    else
                        p->fillRect( x, y, x2 - x + 1, y2 - y + 1, cg->background() );
                }
                break;
            }
        case CE_MenuBarItem:
            {

                r.rect( &x, &y, &w, &h );
                QMenuItem *mi = opt.menuItem();
                QMenuBar *mb = ( QMenuBar* ) widget;
                QRect pr = mb->rect();
                bool active = how & Style_Active;
                //bool focused = how & Style_HasFocus;
                const QColorGroup *g = colorGroup( cg, active ? MenuBarItem : MenuBar );
                QColor btext = g->buttonText();

                QPixmap* cache = makeMenuBarCache(pr.width(), pr.height());

                QPixmap buf( w, pr.height() );

                bitBlt(&buf, 0, 0, cache, x, y, w, pr.height());
                QPainter p2( &buf );

                if ( active )
                {
                    drawBaseButton( &p2, 0, 0, w, h, *g, false, false, MenuBarItem );
                }
                
                p2.end();
                p->drawPixmap( x, y, buf, 0, 0, w, h );
                                
                drawItem( p, QRect(x,y,w,h), AlignCenter | AlignVCenter | ShowPrefix | DontClip | SingleLine,
                          *g, mi->isEnabled(), mi->pixmap(), mi->text(),
                          -1, &btext );
                handled = true;
                break;
            }
        case CE_PopupMenuItem:
            {
                bool separator = false;
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );

                const QPopupMenu *popupmenu = ( const QPopupMenu * ) widget;
                QMenuItem *mi = opt.menuItem();
                if ( mi )
                {
                    separator = mi->isSeparator();
                }

                int tab = opt.tabWidth();
                int checkcol = opt.maxIconWidth();
                bool enabled = (mi? mi->isEnabled():true);
                bool checkable = popupmenu->isCheckable();
                bool active = how & Style_Active;
                bool etchtext = styleHint( SH_EtchDisabledText, 0 );
                bool reverse = QApplication::reverseLayout();

                const QColorGroup& cg_ours = *colorGroup( cg, active ? MenuItemDown : MenuItem );
                //QColor btext = cg_ours.buttonText();


                if ( checkable )
                    checkcol = QMAX( checkcol, 20 );

                // Are we a menu item separator?
                if ( separator )
                {
                    p->setPen( cg_ours.dark() );
                    p->drawLine( x, y, x + w, y );
                    p->setPen( cg_ours.light() );
                    p->drawLine( x, y + 1, x + w, y + 1 );
                    break;
                }

                // Draw the menu item background
                if ( active )
                    drawBaseButton( p, x, y, w, h, cg_ours, true, false, MenuItemDown );
                else
                {
                    drawShade( p, x, y, w, h, *colorGroup( cg_ours, MenuItem ), false, false,
                               highlightWidth( MenuItem ), borderWidth( MenuItem ),
                               shade() );
                    int dw = decoWidth( MenuItem );
                    if ( !isPixmap( MenuItem ) )
                    {
                        p->fillRect(
                            x + dw, y + dw, w - dw * 2, h - dw * 2,
                            cg_ours.brush( QColorGroup::Background ) );
                        //cg.brush( QColorGroup::Background ));
                        //colorGroup( cg_ours, MenuItem ) ->brush( QColorGroup::Background ) );
                    }
                    else
                    {
                        // process inactive item pixmaps as one large item
                        p->drawTiledPixmap( x + dw, y + dw, w - dw * 2, h - dw * 2, *scalePixmap
                                            ( w, p->window().height(), MenuItem ),
                                            x, y );
                    }
                }

                if (!mi)
                    break;

                // Do we have an icon?
                if ( mi->iconSet() )
                {
                    QIconSet::Mode mode;
                    QRect cr = visualRect( QRect( x, y, checkcol, h ), r );

                    // Select the correct icon from the iconset
                    if ( active )
                        mode = enabled ? QIconSet::Active : QIconSet::Disabled;
                    else
                        mode = enabled ? QIconSet::Normal : QIconSet::Disabled;

                    // Do we have an icon and are checked at the same time?
                    // Then draw a "pressed" background behind the icon
                    if ( checkable && mi->isChecked() )  //!active && -- ??
                        drawBaseButton( p, cr.x(), cr.y(), cr.width(), cr.height(), *colorGroup( cg_ours, BevelDown ), true, false, BevelDown );

                    // Draw the icon
                    QPixmap pixmap = mi->iconSet() ->pixmap( QIconSet::Small, mode );
                    int pixw = pixmap.width();
                    int pixh = pixmap.height();
                    QRect pmr( 0, 0, pixw, pixh );
                    pmr.moveCenter( cr.center() );
                    p->setPen( cg_ours.highlightedText() );
                    p->drawPixmap( pmr.topLeft(), pixmap );
                }

                // Are we checked? (This time without an icon)
                else if ( checkable && mi->isChecked() )
                {
                    int cx = reverse ? x + w - checkcol : x;

                    // We only have to draw the background if the menu item is inactive -
                    // if it's active the "pressed" background is already drawn
                    //if ( ! active )
                    // qDrawShadePanel( p, cx, y, checkcol, h, cg_ours, true, 1,
                    //     &cg_ours.brush(QColorGroup::Midlight) );

                    // Draw the checkmark
                    SFlags cflags = Style_Default;
                    cflags |= active ? Style_Enabled : Style_On;

                    drawPrimitive( PE_CheckMark, p, QRect( cx + itemFrame, y + itemFrame,
                                                           checkcol - itemFrame * 2, h - itemFrame * 2 ), cg_ours, cflags );
                }

                // Time to draw the menu item label...
                int xm = itemFrame + checkcol + itemHMargin; // X position margin

                int xp = reverse ?  // X position
                         x + tab + rightBorder + itemHMargin + itemFrame - 1 :
                         x + xm;

                int offset = reverse ? -1 : 1; // Shadow offset for etched text

                // Label width (minus the width of the accelerator portion)
                int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

                // Set the color for enabled and disabled text
                // (used for both active and inactive menu items)
                p->setPen( enabled ? cg_ours.buttonText() : cg_ours.mid() );

                // This color will be used instead of the above if the menu item
                // is active and disabled at the same time. (etched text)
                QColor discol = cg_ours.mid();

                // Does the menu item draw it's own label?
                if ( mi->custom() )
                {
                    int m = itemVMargin;
                    // Save the painter state in case the custom
                    // paint method changes it in some way
                    p->save();

                    // Draw etched text if we're inactive and the menu item is disabled
                    if ( etchtext && !enabled && !active )
                    {
                        p->setPen( cg_ours.light() );
                        mi->custom() ->paint( p, cg_ours, active, enabled, xp + offset, y + m + 1, tw, h - 2 * m );
                        p->setPen( discol );
                    }
                    mi->custom() ->paint( p, cg_ours, active, enabled, xp, y + m, tw, h - 2 * m );
                    p->restore();
                }
                else
                {
                    // The menu item doesn't draw it's own label
                    QString s = mi->text();

                    // Does the menu item have a text label?
                    if ( !s.isNull() )
                    {
                        int t = s.find( '\t' );
                        int m = itemVMargin;
                        int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
                        text_flags |= reverse ? AlignRight : AlignLeft;

                        // Does the menu item have a tabstop? (for the accelerator text)
                        if ( t >= 0 )
                        {
                            int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
                                       x + w - tab - rightBorder - itemHMargin - itemFrame;


                            // Draw the right part of the label (accelerator text)
                            if ( etchtext && !enabled && !active )
                            {
                                // Draw etched text if we're inactive and the menu item is disabled
                                p->setPen( cg_ours.light() );
                                p->drawText( tabx + offset, y + m + 1, tab, h - 2 * m, text_flags, s.mid( t + 1 ) );
                                p->setPen( discol );
                            }
                            p->drawText( tabx, y + m, tab, h - 2 * m, text_flags, s.mid( t + 1 ) );
                            s = s.left( t );
                        }


                        // Draw the left part of the label (or the whole label
                        // if there's no accelerator)
                        if ( etchtext && !enabled && !active )
                        {
                            // Etched text again for inactive disabled menu items...
                            p->setPen( cg_ours.light() );
                            p->drawText( xp + offset, y + m + 1, tw, h - 2 * m, text_flags, s, t );
                            p->setPen( discol );
                        }

                        p->drawText( xp, y + m, tw, h - 2 * m, text_flags, s, t );

                    }

                    // The menu item doesn't have a text label
                    // Check if it has a pixmap instead
                    else if ( mi->pixmap() )
                    {
                        QPixmap * pixmap = mi->pixmap();

                        // Draw the pixmap
                        if ( pixmap->depth() == 1 )
                            p->setBackgroundMode( OpaqueMode );

                        int diffw = ( ( w - pixmap->width() ) / 2 )
                                    + ( ( w - pixmap->width() ) % 2 );
                        p->drawPixmap( x + diffw, y + itemFrame, *pixmap );

                        if ( pixmap->depth() == 1 )
                            p->setBackgroundMode( TransparentMode );
                    }
                }

                // Does the menu item have a submenu?
                if ( mi->popup() )
                {
                    PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
                    int dim = 10 -  itemFrame; //We're not very useful to inherit off, so just hardcode..
                    QRect vr = visualRect( QRect( x + w - arrowHMargin - itemFrame - dim,
                                                  y + h / 2 - dim / 2, dim, dim ), r );

                    // Draw an arrow at the far end of the menu item
                    if ( active )
                    {
                        if ( enabled )
                            discol = cg_ours.buttonText();

                        QColorGroup g2( discol, cg_ours.highlight(), white, white,
                                        enabled ? white : discol, discol, white );

                        drawPrimitive( arrow, p, vr, g2, Style_Enabled | Style_Down );
                    }
                    else
                        drawPrimitive( arrow, p, vr, cg_ours,
                                       enabled ? Style_Enabled : Style_Default );
                }
                handled = true;
                break;
            }
        case CE_ProgressBarGroove:
            {
                QBrush bg;
                const QColorGroup * cg2 = colorGroup( cg, ProgressBg );
                qDrawWinPanel( p, r, *cg2, TRUE );
                bg.setColor( cg2->color( QColorGroup::Background ) );
                if ( isPixmap( ProgressBg ) )
                    bg.setPixmap( *uncached( ProgressBg ) );
                p->fillRect( x + 2, y + 2, w - 4, h - 4, bg );

                handled = true;
                break;
            }
        case CE_ProgressBarContents:
            {
                const QProgressBar* pb = (const QProgressBar*)widget;
                QRect cr = subRect(SR_ProgressBarContents, widget);
                double progress = pb->progress();
                bool reverse = QApplication::reverseLayout();
                int steps = pb->totalSteps();
                
                int pstep = 0;

                if (!cr.isValid())
                        return;

                // Draw progress bar
                if (progress > 0 || steps == 0)
                {
                        double pg = (steps == 0) ? 0.1 : progress / steps;
                        int width = QMIN(cr.width(), (int)(pg * cr.width()));
                        if (steps == 0)
                        { //Busy indicator

                                if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless

                                int remWidth = cr.width() - width; //Never disappear completely
                                if (remWidth <= 0) remWidth = 1; //Do something non-crashy when too small...                                       

                                pstep =  int(progress) % ( 2 *  remWidth ); 

                                if ( pstep > remWidth )
                                {
                                        //Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...                                           
                                        // - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
                                        pstep = - (pstep - 2 * remWidth );                                                                                      
                                }
                        }
                                                                           
                        if ( !reverse )
                                drawBaseButton( p, x + pstep, y, width, h, *colorGroup( cg, ProgressBar ), false, false, ProgressBar );
                        else
                        {
                                //TODO:Optimize
                                QPixmap buf( width, h );
                                QPainter p2( &buf );
                                drawBaseButton( &p2, 0, 0, width, h, *colorGroup( cg, ProgressBar ), false, false, ProgressBar );
                                p2.end();
                                QPixmap mirroredPix = QPixmap( buf.convertToImage().mirror( true, false ) );
                                bitBlt( p->device(), x + w - width - pstep, y, &mirroredPix );
                        }
                }

                handled = true;
                break;
            }
        default:
            handled = false;
    };

    if ( !handled )
        KThemeBase::drawControl( element,
                                 p, widget, r, cg, how, opt );
}


void KThemeStyle::drawControlMask( ControlElement element,
                                   QPainter *p,
                                   const QWidget *widget,
                                   const QRect &r,
                                   const QStyleOption& opt ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    switch ( element )
    {
        case CE_PushButton:
            {
                //Is this correct?
                drawBaseMask( p, x, y, w, h, roundButton() );
                handled = true;
                break;
            }
        default:
            handled = false;
    };

    if ( !handled )
        KThemeBase::drawControlMask( element,
                                     p, widget, r, opt );

}


void KThemeStyle::drawKStylePrimitive( KStylePrimitive kpe,
                                       QPainter* p,
                                       const QWidget* widget,
                                       const QRect &r,
                                       const QColorGroup &cg,
                                       SFlags flags,
                                       const QStyleOption& opt ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    switch ( kpe )
    {
        case KPE_SliderGroove:
            {
                if ( !roundSlider() )
                {
                    const QSlider * slider = ( const QSlider* ) widget;
                    bool horizontal = slider->orientation() == Horizontal;
                    if ( horizontal )
                    {
                        drawBaseButton( p, x, y, w, h, *colorGroup( cg, SliderGroove ), true,
                                        false, SliderGroove );
                    }
                    else
                    {
                        drawBaseButton( p, x, y, w, h, *colorGroup( cg, RotSliderGroove ), true,
                                        false, RotSliderGroove );
                    }
                }
                else
                {
                    //This code is from HighColorDefault..
                    const QSlider* slider = ( const QSlider* ) widget;
                    bool horizontal = slider->orientation() == Horizontal;
                    int gcenter = ( horizontal ? r.height() : r.width() ) / 2;

                    QRect gr;
                    if ( horizontal )
                        gr = QRect( r.x(), r.y() + gcenter - 3, r.width(), 7 );
                    else
                        gr = QRect( r.x() + gcenter - 3, r.y(), 7, r.height() );

                    int x, y, w, h;
                    gr.rect( &x, &y, &w, &h );
                    int x2 = x + w - 1;
                    int y2 = y + h - 1;

                    // Draw the slider groove.
                    p->setPen( cg.dark() );
                    p->drawLine( x + 2, y, x2 - 2, y );
                    p->drawLine( x, y + 2, x, y2 - 2 );
                    p->fillRect( x + 2, y + 2, w - 4, h - 4,
                                 slider->isEnabled() ? cg.dark() : cg.mid() );
                    p->setPen( cg.shadow() );
                    p->drawRect( x + 1, y + 1, w - 2, h - 2 );
                    p->setPen( cg.light() );
                    p->drawPoint( x + 1, y2 - 1 );
                    p->drawPoint( x2 - 1, y2 - 1 );
                    p->drawLine( x2, y + 2, x2, y2 - 2 );
                    p->drawLine( x + 2, y2, x2 - 2, y2 );
                }
                handled = true;
                break;
            }
        case KPE_SliderHandle:
            {
                if ( isPixmap( Slider ) )
                {
                    const QSlider * slider = ( const QSlider* ) widget;
                    bool horizontal = slider->orientation() == Horizontal;
                    if ( horizontal )
                    {
                        bitBlt( p->device(), x, y + ( h - uncached( Slider ) ->height() ) / 2,
                                uncached( Slider ) );
                    }
                    else
                    {
                        if ( !vsliderCache )
                        {
                            QWMatrix r270;
                            r270.rotate( 270 );
                            vsliderCache = new QPixmap( uncached( Slider ) ->xForm( r270 ) );
                            if ( uncached( Slider ) ->mask() )
                                vsliderCache->setMask( uncached( Slider ) ->mask() ->xForm( r270 ) );
                        }
                        bitBlt( p->device(), x + ( w - vsliderCache->width() ) / 2, y,
                                vsliderCache );
                    }
                }
                else
                {
                    //This code again from HighColor..
                    //...except sans the gradient..
                    const QSlider* slider = ( const QSlider* ) widget;
                    bool horizontal = slider->orientation() == Horizontal;
                    int x, y, w, h;
                    r.rect( &x, &y, &w, &h );
                    int x2 = x + w - 1;
                    int y2 = y + h - 1;

                    p->setPen( cg.mid() );
                    p->drawLine( x + 1, y, x2 - 1, y );
                    p->drawLine( x, y + 1, x, y2 - 1 );
                    p->setPen( cg.shadow() );
                    p->drawLine( x + 1, y2, x2 - 1, y2 );
                    p->drawLine( x2, y + 1, x2, y2 - 1 );

                    p->setPen( cg.light() );
                    p->drawLine( x + 1, y + 1, x2 - 1, y + 1 );
                    p->drawLine( x + 1, y + 1, x + 1, y2 - 1 );
                    p->setPen( cg.dark() );
                    p->drawLine( x + 2, y2 - 1, x2 - 1, y2 - 1 );
                    p->drawLine( x2 - 1, y + 2, x2 - 1, y2 - 1 );
                    p->setPen( cg.midlight() );
                    p->drawLine( x + 2, y + 2, x2 - 2, y + 2 );
                    p->drawLine( x + 2, y + 2, x + 2, y2 - 2 );
                    p->setPen( cg.mid() );
                    p->drawLine( x + 3, y2 - 2, x2 - 2, y2 - 2 );
                    p->drawLine( x2 - 2, y + 3, x2 - 2, y2 - 2 );
                    p->fillRect( QRect( x + 3, y + 3, w - 6, h - 6 ),
                                 cg.button() );

                    // Paint riffles
                    if ( horizontal )
                    {
                        p->setPen( cg.light() );
                        p->drawLine( x + 5, y + 4, x + 5, y2 - 4 );
                        p->drawLine( x + 8, y + 4, x + 8, y2 - 4 );
                        p->drawLine( x + 11, y + 4, x + 11, y2 - 4 );
                        p->setPen( slider->isEnabled() ? cg.shadow() : cg.mid() );
                        p->drawLine( x + 6, y + 4, x + 6, y2 - 4 );
                        p->drawLine( x + 9, y + 4, x + 9, y2 - 4 );
                        p->drawLine( x + 12, y + 4, x + 12, y2 - 4 );
                    }
                    else
                    {
                        p->setPen( cg.light() );
                        p->drawLine( x + 4, y + 5, x2 - 4, y + 5 );
                        p->drawLine( x + 4, y + 8, x2 - 4, y + 8 );
                        p->drawLine( x + 4, y + 11, x2 - 4, y + 11 );
                        p->setPen( slider->isEnabled() ? cg.shadow() : cg.mid() );
                        p->drawLine( x + 4, y + 6, x2 - 4, y + 6 );
                        p->drawLine( x + 4, y + 9, x2 - 4, y + 9 );
                        p->drawLine( x + 4, y + 12, x2 - 4, y + 12 );
                    }
                }
                handled = true;
                break;
            }
            //case KPE_DockWindowHandle:
        case KPE_ToolBarHandle:
        case KPE_GeneralHandle:
            {
                if ( w > h )
                    drawBaseButton( p, x, y, w, h, *colorGroup( cg, HBarHandle ), false, false,
                                    HBarHandle );
                else
                    drawBaseButton( p, x, y, w, h, *colorGroup( cg, VBarHandle ), false, false,
                                    VBarHandle );

                handled = true;
                break;
            }
        default:
            handled = false;

    }

    if ( !handled )
    {
        KThemeBase::drawKStylePrimitive( kpe, p, widget,
                                         r, cg, flags, opt );
    }

}




void KThemeStyle::drawComplexControl ( ComplexControl control, QPainter * p, const QWidget * widget,
                                       const QRect & r, const QColorGroup & g, SFlags how ,
                                       SCFlags controls, SCFlags active,
                                       const QStyleOption & opt ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    bool down = how & Style_Down;
    bool on = how & Style_On;

    // bool enabled = ( how & Style_Enabled );

    switch ( control )
    {
        case CC_ToolButton:
            {
                const QToolButton * toolbutton = ( const QToolButton * ) widget;
                QRect button, menu;
                button = querySubControlMetrics( control, widget, SC_ToolButton, opt );
                menu = querySubControlMetrics( control, widget, SC_ToolButtonMenu, opt );


                if ( controls & SC_ToolButton )
                {
                    WidgetType widget = ( down || on ) ? ToolButtonDown : ToolButton;

                    drawBaseButton( p, button.x(), button.y(), button.width(), button.height(), *colorGroup( g, widget ), down || on, false,
                                    widget );

                    // int m = decoWidth( widget );
                }

                if ( controls & SC_ToolButtonMenu )
                {
                    drawPrimitive( PE_ArrowDown, p, menu, g, how );
                    /*                if ( enabled )
                         kDrawWindowsArrow(p, this, PE_ArrowDown, false, menu.x(), menu.y(), menu.width(), menu.height(),
                                                     g, true );
                                    else
                         kDrawWindowsArrow(p, this, PE_ArrowDown, false, menu.x(), menu.y(), menu.width(), menu.height(),
                                                     g, false );*/
                }

                if ( toolbutton->hasFocus() && !toolbutton->focusProxy() )
                {
                    QRect fr = toolbutton->rect();
                    fr.addCoords( 3, 3, -3, -3 );
                    drawPrimitive( PE_FocusRect, p, fr, g );
                }

                handled = true;
                break;
            }

        case CC_ComboBox:
            {
                if ( controls & SC_ComboBoxFrame )
                {
                    //TODO: Anyway of detecting when the popup is there -- would look nicer if sunken then too..
                    bool sunken = ( active == SC_ComboBoxArrow );
                    //No frame, edit box and button for now?
                    WidgetType widget = sunken ? ComboBoxDown : ComboBox;
                    drawBaseButton( p, x, y, w, h, *colorGroup( g, widget ), sunken,
                                    roundComboBox(), widget );

                    controls ^= SC_ComboBoxFrame;
                }

                if ( controls & SC_ComboBoxArrow )
                {
                    bool sunken = ( active == SC_ComboBoxArrow );
                    QRect ar = QStyle::visualRect(
                                   querySubControlMetrics( CC_ComboBox, widget, SC_ComboBoxArrow ),
                                   widget );
                    ar.rect( &x, &y, &w, &h );
                    WidgetType widget = sunken ? ComboBoxDown : ComboBox;

                    if ( !sunken && isPixmap( ComboDeco ) )
                        bitBlt( p->device(),
                                x + ( w - uncached( ComboDeco ) ->width() - decoWidth( ComboBox ) / 2 ),
                                y + ( h - uncached( ComboDeco ) ->height() ) / 2,
                                uncached( ComboDeco ) );
                    else if ( sunken && isPixmap( ComboDecoDown ) )
                        bitBlt( p->device(),
                                x + ( w - uncached( ComboDecoDown ) ->width() - decoWidth( ComboBoxDown ) ) / 2,
                                y + ( h - uncached( ComboDecoDown ) ->height() ) / 2,
                                uncached( ComboDecoDown ) );
                    else
                    {

                        mtfstyle->drawPrimitive( PE_ArrowDown, p, QRect( x, y, w, h ), *colorGroup( g, widget ), sunken ? ( how | Style_Sunken ) : how, opt );
                        qDrawShadeRect( p, x, y, w, h, *colorGroup( g, widget ) ); //w-14, y+7+(h-15), 10, 3,
                    }
                    controls ^= SC_ComboBoxArrow;
                }
                break;
            }
        case CC_ScrollBar:
            {
                const QScrollBar *sb = ( const QScrollBar* ) widget;
                bool maxedOut = ( sb->minValue() == sb->maxValue() );
                bool horizontal = ( sb->orientation() == Qt::Horizontal );
                SFlags sflags = ( ( horizontal ? Style_Horizontal : Style_Default ) |
                                  ( maxedOut ? Style_Default : Style_Enabled ) );

                //Here, we don't do add page, subpage, etc.,
                QRect addline, subline, subline2, groove, slider;
                subline = querySubControlMetrics( control, widget, SC_ScrollBarSubLine, opt );
                addline = querySubControlMetrics( control, widget, SC_ScrollBarAddLine, opt );
                groove = querySubControlMetrics( control, widget, SC_ScrollBarGroove, opt );

                slider = querySubControlMetrics( control, widget, SC_ScrollBarSlider, opt );
                subline2 = addline;

                QPixmap buf( sb->width(), sb->height() );
                QPainter p2( &buf );

                if ( groove.isValid() )
                {
                    p2.fillRect( groove, QColor( 255, 0, 0 ) );
                    drawPrimitive( PE_ScrollBarSubPage, &p2, groove, g,
                                   sflags | ( ( active == SC_ScrollBarSubPage ) ?
                                              Style_Down : Style_Default ) );
                }


                // Draw the up/left button set
                if ( subline.isValid() )
                {
                    drawPrimitive( PE_ScrollBarSubLine, &p2, subline, g,
                                   sflags | ( active == SC_ScrollBarSubLine ?
                                              Style_Down : Style_Default ) );
                }

                if ( addline.isValid() )
                    drawPrimitive( PE_ScrollBarAddLine, &p2, addline, g,
                                   sflags | ( ( active == SC_ScrollBarAddLine ) ?
                                              Style_Down : Style_Default ) );

                if ( slider.isValid() )
                { //(controls & SC_ScrollBarSlider) &&
                    drawPrimitive( PE_ScrollBarSlider, &p2, slider, g,
                                   sflags | ( ( active == SC_ScrollBarSlider ) ?
                                              Style_Down : Style_Default ) );
                    // Draw focus rect
                    if ( sb->hasFocus() )
                    {
                        QRect fr( slider.x() + 2, slider.y() + 2,
                                  slider.width() - 5, slider.height() - 5 );
                        drawPrimitive( PE_FocusRect, &p2, fr, g, Style_Default );
                    }
                    p2.end();
                    bitBlt( p->device(), x, y, &buf );
                    handled = true;

                }
                break;
            }
        default:
            handled = false;
    }

    if ( !handled )
    {
        KThemeBase::drawComplexControl ( control, p, widget,
                                         r, g, how ,
                                         controls, active,
                                         opt );
    }

}


void KThemeStyle::drawBaseMask( QPainter *p, int x, int y, int w, int h,
                                bool round ) const
{
    // round edge fills
    static const QCOORD btm_left_fill[] =
        {
            0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1,
            1, 2, 2, 2, 3, 2, 4, 2, 2, 3, 3, 3, 4, 3, 3, 4, 4, 4
        };

    static const QCOORD btm_right_fill[] =
        {
            0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 1, 1, 1, 2, 1, 3, 1, 4,
            1, 0, 2, 1, 2, 2, 2, 3, 2, 0, 3, 1, 3, 2, 3, 0, 4, 1, 4
        };

    static const QCOORD top_left_fill[] =
        {
            3, 0, 4, 0, 2, 1, 3, 1, 4, 1, 1, 2, 2, 2, 3, 2, 4, 2, 0, 3,
            1, 3, 2, 3, 3, 3, 4, 3, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4
        };

    static const QCOORD top_right_fill[] =
        {
            0, 0, 1, 0, 0, 1, 1, 1, 2, 1, 0, 2, 1, 2, 2, 2, 3, 2, 0,
            3, 1, 3, 2, 3, 3, 3, 4, 3, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4
        };

    QBrush fillBrush( color1, SolidPattern );
    p->setPen( color1 );
    if ( round && w > 19 && h > 19 )
    {
        int x2 = x + w - 1;
        int y2 = y + h - 1;
        QPointArray a( QCOORDARRLEN( top_left_fill ), top_left_fill );
        a.translate( 1, 1 );
        p->drawPoints( a );
        a.setPoints( QCOORDARRLEN( btm_left_fill ), btm_left_fill );
        a.translate( 1, h - 6 );
        p->drawPoints( a );
        a.setPoints( QCOORDARRLEN( top_right_fill ), top_right_fill );
        a.translate( w - 6, 1 );
        p->drawPoints( a );
        a.setPoints( QCOORDARRLEN( btm_right_fill ), btm_right_fill );
        a.translate( w - 6, h - 6 );
        p->drawPoints( a );

        p->fillRect( x + 6, y, w - 12, h, fillBrush );
        p->fillRect( x, y + 6, x + 6, h - 12, fillBrush );
        p->fillRect( x2 - 6, y + 6, x2, h - 12, fillBrush );
        p->drawLine( x + 6, y, x2 - 6, y );
        p->drawLine( x + 6, y2, x2 - 6, y2 );
        p->drawLine( x, y + 6, x, y2 - 6 );
        p->drawLine( x2, y + 6, x2, y2 - 6 );

    }
    else
        p->fillRect( x, y, w, h, fillBrush );
}

int KThemeStyle::styleHint( StyleHint sh, const QWidget *w, const QStyleOption &opt, QStyleHintReturn *shr ) const
{
    switch ( sh )
    {
        case SH_EtchDisabledText:
        case SH_Slider_SnapToValue:
        case SH_PrintDialog_RightAlignButtons:
        case SH_FontDialog_SelectAssociatedText:
        case SH_PopupMenu_AllowActiveAndDisabled:
        case SH_MenuBar_AltKeyNavigation:
        case SH_MenuBar_MouseTracking:
        case SH_PopupMenu_MouseTracking:
        case SH_ComboBox_ListMouseTracking:
            return 1;

        case SH_GUIStyle:
            return WindowsStyle;

        case SH_PopupMenu_SubMenuPopupDelay:
            return 128;

        default:
            return KThemeBase::styleHint( sh, w, opt, shr );
    };
}



/* This is where we draw the borders and highlights. The new round button
 * code is a pain in the arse. We don't want to be calculating arcs so
 * use a whole lotta QPointArray's ;-) The code is made a lot more complex
 * because you can have variable width border and highlights...
 * I may want to cache this if round buttons are used, but am concerned
 * about excessive cache misses. This is a memory/speed tradeoff that I
 * have to test.
 */
void KThemeStyle::drawShade( QPainter *p, int x, int y, int w, int h,
                             const QColorGroup &g, bool sunken, bool rounded,
                             int hWidth, int bWidth, ShadeStyle style ) const
{
    int i, sc, bc, x2, y2;
    QPen highPen, lowPen;

    if ( style == Motif )
    {
        highPen.setColor( sunken ? g.dark() : g.light() );
        lowPen.setColor( sunken ? g.light() : g.dark() );
    }
    else
    {
        highPen.setColor( sunken ? g.shadow() : g.light() );
        lowPen.setColor( sunken ? g.light() : g.shadow() );
    }

    // Advanced round buttons
    if ( rounded && w > 19 && h > 19 )
    {
        x2 = x + w - 1, y2 = y + h - 1;
        QPointArray bPntArray, hPntArray, lPntArray;
        QPointArray bLineArray, hLineArray, lLineArray;
        // borders
        for ( i = 0, bc = 0; i < bWidth; ++i )
        {
            bPntArray.putPoints( bc, 24, x + 4, y + 1, x + 5, y + 1, x + 3, y + 2, x + 2, y + 3,
                                 x + 1, y + 4, x + 1, y + 5, x + 1, y2 - 5, x + 1, y2 - 4, x + 2, y2 - 3,
                                 x2 - 5, y + 1, x2 - 4, y + 1, x2 - 3, y + 2, x2 - 5, y2 - 1,
                                 x2 - 4, y2 - 1, x2 - 3, y2 - 2, x2 - 2, y2 - 3, x2 - 1, y2 - 5,
                                 x2 - 1, y2 - 4, x + 3, y2 - 2, x + 4, y2 - 1, x + 5, y2 - 1,
                                 x2 - 2, y + 3, x2 - 1, y + 4, x2 - 1, y + 5 );
            bc += 24;
            // ellispe edges don't match exactly, so fill in blanks
            if ( i < bWidth - 1 || hWidth != 0 )
            {
                bPntArray.putPoints( bc, 20, x + 6, y + 1, x + 4, y + 2, x + 3, y + 3,
                                     x + 2, y + 4, x + 1, y + 6, x2 - 6, y + 1, x2 - 4, y + 2,
                                     x2 - 3, y + 3, x + 2, y2 - 4, x + 1, y2 - 6, x2 - 6, y2 - 1,
                                     x2 - 4, y2 - 2, x2 - 3, y2 - 3, x2 - 2, y2 - 4, x2 - 1, y2 - 6,
                                     x + 6, y2 - 1, x + 4, y2 - 2, x + 3, y2 - 3, x2 - 1, y + 6,
                                     x2 - 2, y + 4 );
                bc += 20;
            }
            bLineArray.putPoints( i * 8, 8, x + 6, y, x2 - 6, y, x, y + 6, x, y2 - 6,
                                  x + 6, y2, x2 - 6, y2, x2, y + 6, x2, y2 - 6 );
            ++x, ++y;
            --x2, --y2;
        }
        // highlights
        for ( i = 0, sc = 0; i < hWidth; ++i )
        {
            hPntArray.putPoints( sc, 12, x + 4, y + 1, x + 5, y + 1,   // top left
                                 x + 3, y + 2, x + 2, y + 3, x + 1, y + 4, x + 1, y + 5,
                                 x + 1, y2 - 5, x + 1, y2 - 4, x + 2, y2 - 3,   // half corners
                                 x2 - 5, y + 1, x2 - 4, y + 1, x2 - 3, y + 2 );
            lPntArray.putPoints( sc, 12, x2 - 5, y2 - 1, x2 - 4, y2 - 1,   // btm right
                                 x2 - 3, y2 - 2, x2 - 2, y2 - 3, x2 - 1, y2 - 5, x2 - 1, y2 - 4,
                                 x + 3, y2 - 2, x + 4, y2 - 1, x + 5, y2 - 1,   //half corners
                                 x2 - 2, y + 3, x2 - 1, y + 4, x2 - 1, y + 5 );
            sc += 12;
            if ( i < hWidth - 1 )
            {
                hPntArray.putPoints( sc, 10, x + 6, y + 1, x + 4, y + 2,   // top left
                                     x + 3, y + 3, x + 2, y + 4, x + 1, y + 6,
                                     x2 - 6, y + 1, x2 - 4, y + 2,   // half corners
                                     x2 - 3, y + 3, x + 2, y2 - 4, x + 1, y2 - 6 );
                lPntArray.putPoints( sc, 10, x2 - 6, y2 - 1, x2 - 4, y2 - 2,   // btm right
                                     x2 - 3, y2 - 3, x2 - 2, y2 - 4, x2 - 1, y2 - 6,
                                     x + 6, y2 - 1, x + 4, y2 - 2,   // half corners
                                     x + 3, y2 - 3, x2 - 1, y + 6, x2 - 2, y + 4 );
                sc += 10;
            }
            hLineArray.putPoints( i * 4, 4, x + 6, y, x2 - 6, y, x, y + 6, x, y2 - 6 );
            lLineArray.putPoints( i * 4, 4, x + 6, y2, x2 - 6, y2, x2, y + 6, x2, y2 - 6 );
            ++x, ++y;
            --x2, --y2;
        }
        p->setPen( Qt::black );
        p->drawPoints( bPntArray );
        p->drawLineSegments( bLineArray );
        p->setPen( highPen );
        p->drawPoints( hPntArray );
        p->drawLineSegments( hLineArray );
        p->setPen( lowPen );
        p->drawPoints( lPntArray );
        p->drawLineSegments( lLineArray );
    }
    // Rectangular buttons
    else
    {
        QPointArray highShade( hWidth * 4 );
        QPointArray lowShade( hWidth * 4 );

        p->setPen( g.shadow() );
        for ( i = 0; i < bWidth && w > 2 && h > 2; ++i, ++x, ++y, w -= 2, h -= 2 )
            p->drawRect( x, y , w, h );

        if ( !hWidth )
            return ;

        x2 = x + w - 1, y2 = y + h - 1;
        for ( i = 0; i < hWidth; ++i, ++x, ++y, --x2, --y2 )
        {
            highShade.putPoints( i * 4, 4, x, y, x2, y, x, y, x, y2 );
            lowShade.putPoints( i * 4, 4, x, y2, x2, y2, x2, y, x2, y2 );
        }
        if ( style == Windows && hWidth > 1 )
        {
            p->setPen( highPen );
            p->drawLineSegments( highShade, 0, 2 );
            p->setPen( lowPen );
            p->drawLineSegments( lowShade, 0, 2 );

            p->setPen( ( sunken ) ? g.dark() : g.mid() );
            p->drawLineSegments( highShade, 4 );
            p->setPen( ( sunken ) ? g.mid() : g.dark() );
            p->drawLineSegments( lowShade, 4 );
        }
        else
        {
            p->setPen( ( sunken ) ? g.dark() : g.light() );
            p->drawLineSegments( highShade );
            p->setPen( ( sunken ) ? g.light() : g.dark() );
            p->drawLineSegments( lowShade );
        }
    }
}




int KThemeStyle::popupMenuItemHeight( bool /*checkable*/, QMenuItem *mi,
                                      const QFontMetrics &fm )
{
    int h2, h = 0;
    int offset = QMAX( decoWidth( MenuItemDown ), decoWidth( MenuItem ) ) + 4;

    if ( mi->isSeparator() )
        return ( 2 );
    if ( mi->isChecked() )
        h = isPixmap( CheckMark ) ? uncached( CheckMark ) ->height() + offset :
            offset + 16;
    if ( mi->pixmap() )
    {
        h2 = mi->pixmap() ->height() + offset;
        h = h2 > h ? h2 : h;
    }
    if ( mi->iconSet() )
    {
        h2 = mi->iconSet() ->
             pixmap( QIconSet::Small, QIconSet::Normal ).height() + offset;
        h = h2 > h ? h2 : h;
    }
    h2 = fm.height() + offset;
    h = h2 > h ? h2 : h;
    return ( h );
}




#include "kthemestyle.moc"



