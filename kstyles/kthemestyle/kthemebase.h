/*
 $Id$

 This file is part of the KDE libraries
 Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

 KDE3 port (C) 2001 Maksim Orlovich <mo002j@mail.rochester.edu>

 Palette setup code is from KApplication,
Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
Copyright (C) 1998, 1999, 2000 KDE Team


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

#ifndef KTHEMEBASE_H
#define KTHEMEBASE_H

#include <qtimer.h>
#include <qdatetime.h>
#include <kpixmap.h>
#include <qintcache.h>
#include <qstring.h>
#include <kstyle.h>
#include <qsettings.h>
#include <qpalette.h> // for QColorGroup
#include "kstyledirs.h"
#include <qmap.h>

class QImage;



/**
 * This class adds simple time management to KPixmap for use in flushing
 * KThemeCache.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KThemePixmap : public KPixmap
{
public:
    enum BorderType{Top = 0, Bottom, Left, Right, TopLeft, TopRight, BottomLeft,
                    BottomRight};

    KThemePixmap( bool timer = true );
    KThemePixmap( const KThemePixmap &p );
    KThemePixmap( const KThemePixmap &p, const QPixmap& rp );
    ~KThemePixmap();
    QPixmap* border( BorderType type );
    void setBorder( BorderType type, const QPixmap &p );
    void updateAccessed();
    bool isOld();
protected:
    QTime *t;
    QPixmap *b[ 8 ];

private:
    class KThemePixmapPrivate;
    KThemePixmapPrivate *d;
};

inline QPixmap* KThemePixmap::border( BorderType type )
{
    return ( b[ type ] );
}

inline void KThemePixmap::setBorder( BorderType type, const QPixmap &p )
{
    if ( b[ type ] )
    {
        qWarning( "KThemePixmap: Overwriting existing border!" );
        delete( b[ type ] );
    }
    b[ type ] = new QPixmap( p );
}

inline void KThemePixmap::updateAccessed()
{
    if ( t )
        t->start();
}

inline bool KThemePixmap::isOld()
{
    return ( t ? t->elapsed() >= 300000 : false );
}

/**
 * A very simple pixmap cache for theme plugins. QPixmapCache is not used
 * since it uses QString keys which are not needed. All the information we
 * need can be encoded in a numeric key. Using QIntCache instead allows us to
 * skip the string operations.
 *
 * This class is mostly just inline methods that do bit operations on a key
 * composed of the widget ID, width and/or height, and then calls
 * QIntCache::find().
 *
 * One other thing to note is that full, horizontal, and vertically scaled
 * pixmaps are not used interchangeably. For example, if you insert a fully
 * scaled pixmap that is 32x32 then request a horizontally scaled pixmap with
 * a width of 32, they will not match. This is because a pixmap that has been
 * inserted into the cache has already been scaled at some point and it is
 * very likely the vertical height was not originally 32. Thus the pixmap
 * will be wrong when drawn, even though the horizontal width matches.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 *
 */
class KThemeCache : public QObject
{
    Q_OBJECT
public:
    /**
     * The scale hints supported by the cache. Note that Tiled is not here
     * since tiled pixmaps are kept only once in KThemeBase.
     */
    enum ScaleHint{FullScale, HorizontalScale, VerticalScale};
    /**
     * The constructor.
     *
     * @param maxSize The maximum size of the cache in kilobytes.
     */
    KThemeCache( int maxSize, QObject *parent = 0, const char *name = 0 );
    /**
     * Inserts a new pixmap into the cache.
     *
     * @param pixmap The pixmap to insert.
     * @param scale The scaling type of the pixmap.
     * @param widgetID The widget ID of the pixmap, usually from KThemeBase's
     * WidgetType enum.
     *
     * @return True if the insert was successful, false otherwise.
     */
    bool insert( KThemePixmap *pixmap, ScaleHint scale, int widgetID,
                 bool border = false, bool mask = false );
    /**
     * Returns a fully scaled pixmap.
     *
     * @param w The pixmap width to search for.
     * @param h The pixmap height to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the width, height, and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    KThemePixmap* pixmap( int w, int h, int widgetID, bool border = false,
                          bool mask = false );
    /**
     * Returns a horizontally scaled pixmap.
     *
     * @param w The pixmap width to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the width and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    KThemePixmap* horizontalPixmap( int w, int widgetID );
    /**
     * Returns a vertically scaled pixmap.
     *
     * @param h The pixmap height to search for.
     * @param widgetID The widget ID to search for.
     *
     * @return True if a pixmap matching the height and widget ID of
     * the pixmap exists, NULL otherwise.
     */
    KThemePixmap* verticalPixmap( int h, int widgetID );
protected slots:
    void flushTimeout();
protected:
    QIntCache<KThemePixmap> cache;
    QTimer flushTimer;

private:
    class KThemeCachePrivate;
    KThemeCachePrivate *d;
};



class KThemeBasePrivate;
/**
 * This is a base class for KDE themed styles. It implements a cache,
 * configuration file parsing, pixmap scaling, gradients, and a lot
 * of inline methods for accessing user specified parameters.
 *
 * Note that this class *does not* actually implement any themes. It just
 * provides the groundwork for doing so. The only reason to use this class
 * directly is if you plan to reimplement all of the widgets. Otherwise,
 * refer to KThemeStyle for a fully themed style you can derive from.
 *
 * @author Daniel M. Duley <mosfet@kde.org>
 */
class KThemeBase: public KStyle
{
    Q_OBJECT
public:
    /**
     * Constructs a new KThemeBase object.
     */
    KThemeBase( const QString &dirs, const QString &configFile );
    ~KThemeBase();
    /**
     * Describes if a pixmap should be scaled fully, horizontally, vertically,
     * or not at all and tiled.
     */
    enum ScaleHint{FullScale, HorizontalScale, VerticalScale, TileScale};
    /**
     * The default arrow types.
     */
    enum ArrowStyle{MotifArrow, LargeArrow, SmallArrow};
    /**
     * The default frame shading styles.
     */
    enum ShadeStyle{Motif, Windows, Next, KDE};
    /**
     * The default scrollbar button layout. BottomLeft is like what Next
     * uses, BottomRight is like Platinum, and Opposite it like Windows and
     * Motif.
     */
    enum SButton{SBBottomLeft, SBBottomRight, SBOpposite};
    /**
     * The gradient types. Horizontal is left to right, Vertical is top to
     * bottom, and diagonal is upper-left to bottom-right.
     */
    enum Gradient{GrNone, GrHorizontal, GrVertical, GrDiagonal, GrPyramid,
                  GrRectangle, GrElliptic, GrReverseBevel};
    /**
     * This provides a list of widget types that KThemeBase recognizes.
     */
    /* Internal note: The order here is important. Some widgets inherit
     * properties. This is usually for when you have two settings for the
     * same widget, ie: on(sunken), and off. The on settings will inherit
     * the properties of the off one when nothing is specified in the config.
     *
     * In order to be able to handle this while still having everything in
     * one group that is easy to loop from we have the following order:
     * unsunked(off) items, sunken(on)items, and then the ones that don't
     * matter. INHERIT_ITEMS define the number of widgets that have inheritence
     * so if 0 == PushButtonOff then INHERIT_ITEMS should == PushButtonOn
     * and so on. WIDGETS define the total number of widgets.
     */
    enum WidgetType{
        // Off (unsunken widgets)
        PushButton = 0, ComboBox, HScrollBarSlider, VScrollBarSlider, Bevel,
        ToolButton, ScrollButton, HScrollDeco, VScrollDeco,
        ComboDeco, MenuItem, InactiveTab, ArrowUp, ArrowDown, ArrowLeft,
        ArrowRight,
        // On (sunken widgets)
        PushButtonDown, ComboBoxDown, HScrollBarSliderDown,
        VScrollBarSliderDown, BevelDown, ToolButtonDown, ScrollButtonDown,
        HScrollDecoDown, VScrollDecoDown, ComboDecoDown, MenuItemDown,
        ActiveTab, SunkenArrowUp, SunkenArrowDown, SunkenArrowLeft,
        SunkenArrowRight,
        // Everything else (indicators must have separate settings)
        HScrollGroove, VScrollGroove, Slider, SliderGroove, IndicatorOn,
        IndicatorOff, IndicatorTri, ExIndicatorOn, ExIndicatorOff, HBarHandle, VBarHandle,
        ToolBar, Splitter, CheckMark, MenuBar, DisArrowUp, DisArrowDown,
        DisArrowLeft, DisArrowRight, ProgressBar, ProgressBg, MenuBarItem,
        Background, RotSliderGroove, RotInactiveTab, RotActiveTab, WIDGETS};

    /**
     * The scaling type specified by the KConfig file.
     *
     * @param widget A Widgets enum value.
     *
     * @return A ScaleHint enum value.
     */
    ScaleHint scaleHint( WidgetType widget ) const;
    /**
     * The gradient type specified by the KConfig file.
     *
     * @param widget A Widgets enum value.
     *
     * @return A Gradient enum value.
     */
    Gradient gradientHint( WidgetType widget ) const;
    /**
     * The color group specified for a given widget.
     * If a color group is set in the theme configuration
     * that is used, otherwise defaultColor is returned.
     *
     * @param defaultColor The colorGroup to set if one is available.
     *
     * @param widget The widget whose color group to retrieve.
     *
     */
    const QColorGroup* colorGroup( const QColorGroup &defaultGroup,
                                   WidgetType widget ) const;

    QBrush pixmapBrush( const QColorGroup &group, QColorGroup::ColorRole role,
                        int w, int h, WidgetType widget );
    /**
     * True if the widget has a pixmap or gradient specified.
     */
    bool isPixmap( WidgetType widget ) const;
    /**
     * True if the widget has a color group specified.
     */
    bool isColor( WidgetType widget ) const;
    /**
     * True if the user specified a 3D focus rectangle
     */
    bool is3DFocus() const;
    /**
     * If the user specified a 3D focus rectangle, they may also specify an
     * offset from the default rectangle to use when drawing it. This returns
     * the specified offset.
     */
    int focusOffset() const;
    /**
     * The border width of the specified widget.
     */
    int borderWidth( WidgetType widget ) const;
    /**
     * Pixmap border width of the specified widget.
     */
    int pixBorderWidth( WidgetType widget ) const;
    /**
     * Returns the border pixmap if enabled for the specified widget. This
     * will contain the originial pixmap, plus the edges separated in
     * KThemePixmap::border() if valid. If invalid it will return NULL.
     */
    KThemePixmap* borderPixmap( WidgetType widget ) const;
    /**
     * The highlight width of the specified widget.
     */
    int highlightWidth( WidgetType widget ) const;
    /**
     * The border plus highlight width of the widget.
     */
    int decoWidth( WidgetType widget ) const;
    /**
     * The extent (width for vertical, height for horizontal) requested
     * for the scrollbars.
     */
    int getSBExtent() const;
    /**
     * The scrollbar button layout.
     */
    SButton scrollBarLayout() const;
    /**
     * The arrow type.
     */
    ArrowStyle arrowType() const;
    /**
     * The shading type.
     */
    ShadeStyle shade() const;
    /**
     * The frame width.
     */
    int frameWidth() const;
    /**
     * The splitter width.
     */
    int splitWidth() const;
    /**
     * The contrast for some bevel effects such as reverse gradient.
     */
    int bevelContrast( WidgetType widget ) const;
    /**
     * The button text X shift.
     */
    int buttonXShift() const;
    /**
     * The button text Y shift.
     */
    int buttonYShift() const;
    /**
     * Returns either the slider length of the slider pixmap if available,
     * otherwise the length specified in the config file.
     */
    int sliderButtonLength() const;
    /**
     * True if rounded buttons are requested.
     */
    bool roundButton() const;
    /**
     * True if rounded comboboxes are requested.
     */
    bool roundComboBox() const;
    /**
     * True if rounded slider grooves are requested.
     */
    bool roundSlider() const;
    /**
     * True if a line should be drawn on the bottom of active tabs.
     */
    bool activeTabLine() const;
    /**
     * True if a line should be drawn on the bottom of inactive tabs.
     */
    bool inactiveTabLine() const;
    /**
     * Returns the current uncached pixmap for the given widget. This will
     * usually be either the last scaled or gradient pixmap if those have
     * been specified in the config file, the original pixmap if not, or NULL
     * if no pixmap has been specified.
     */
    KThemePixmap* uncached( WidgetType widget ) const;
    /**
     * Returns the pixmap for the given widget at the specified width and
     * height. This will return NULL if no pixmap or gradient is specified.
     * It may also return a different sized pixmap if the scaling
     * is set to Tiled. When using this method, you should call it using
     * the needed width and height then use QPainter::drawTiledPixmap to
     * paint it. Doing this, if the pixmap is scaled it will be the proper
     * size, otherwise it will be tiled.
     *
     * @param w Requested width.
     * @param h Requested height.
     * @param widget Widget type.
     * @return The pixmap or NULL if one is not specified.
     */
    virtual KThemePixmap *scalePixmap( int w, int h, WidgetType widget ) const;
protected:
    /**
     * This method reads a configuration file and sets things up so overrideColorGroup
    * works. Modiying user's config files within a style is evil, IMHO
    * (SadEagle). On the other hand, this will make it simply ignore settings
     *
     * @param file The configuration file to apply.
     */
    void applyConfigFile( QSettings & settings );

    /*
    * Generates a new palette based on the values for which have been specified explicitly
    * in the .themerc file.
    */
    QPalette overridePalette( const QPalette& pal );

    /**
     * Returns a QImage for the given widget if the widget is scaled, NULL
     * otherwise. QImages of the original pixmap are stored for scaled
     * widgets in order to facilitate fast and accurate smooth-scaling. This
     * also saves us a conversion from a pixmap to an image then back again.
     */
    QImage* image( WidgetType widget ) const;
    /**
     * Returns the gradient high color if one is specified, NULL otherwise.
     */
    QColor* gradientHigh( WidgetType widget ) const;
    /**
     * Returns the gradient low color if one is specified, NULL otherwise.
     */
    QColor* gradientLow( WidgetType widget ) const;
    /**
     * Reads in all the configuration file entries supported.
     *
     * @param colorStyle The style for the color groups. In KDE, colors were
     * calculated a little differently for Motif vs Windows styles. This
     * is obsolete.
     */
    void readConfig( Qt::GUIStyle colorStyle = Qt::WindowsStyle );
    void readWidgetConfig( int i, QSettings *config, QString *pixnames,
                           QString *brdnames, bool *loadArray );
    void copyWidgetConfig( int sourceID, int destID, QString *pixnames,
                           QString *brdnames );
    /**
     * Makes a full color group based on the given foreground and background
     * colors. This is the same code used by KDE (kapp.cpp) in previous
     * versions.
     */
    QColorGroup* makeColorGroup( const QColor &fg, const QColor &bg,
                                 Qt::GUIStyle style = Qt::WindowsStyle );
    KThemePixmap* scale( int w, int h, WidgetType widget ) const;
    KThemePixmap* scaleBorder( int w, int h, WidgetType type ) const;
    KThemePixmap* gradient( int w, int h, WidgetType widget ) const ;
    KThemePixmap* blend( WidgetType widget ) const;
    void generateBorderPix( int i );
    void applyResourceGroup( QSettings *config, int i );
    void applyMiscResourceGroup( QSettings *config );
    void readResourceGroup( int i, QString *pixnames, QString *brdnames,
                            bool *loadArray );
    void readMiscResourceGroup();
    /**
     * Attempts to load a pixmap from the default KThemeBase locations.
     */
    KThemePixmap* loadPixmap( QString &name );
    /**
     * Attempts to load a image from the default KThemeBase locations.
     */
    QImage* loadImage( QString &name );


    /**
    These are included for fuuture extension purposes..
    */
    virtual int pixelMetric ( PixelMetric metric, const QWidget * widget = 0 ) const
    {
        return KStyle::pixelMetric( metric, widget );
    }

    virtual void drawPrimitive ( PrimitiveElement pe, QPainter * p, const QRect & r, const QColorGroup & cg,
                                 SFlags flags = Style_Default,
                                 const QStyleOption& option = QStyleOption::Default ) const
    {
        KStyle::drawPrimitive ( pe, p, r, cg,
                                flags, option );
    }


    virtual void drawControl( ControlElement element,
                              QPainter *p,
                              const QWidget *widget,
                              const QRect &r,
                              const QColorGroup &cg,
                              SFlags how = Style_Default,
                              const QStyleOption& opt = QStyleOption::Default ) const
    {
        KStyle::drawControl( element, p, widget,
                             r, cg, how, opt );
    }

    virtual void drawControlMask( ControlElement element,
                                  QPainter *p,
                                  const QWidget *widget,
                                  const QRect &r,
                                  const QStyleOption& opt = QStyleOption::Default ) const
    {
        KStyle::drawControlMask( element, p, widget, r, opt );
    }


    virtual void drawComplexControl( ComplexControl control,
                                     QPainter *p,
                                     const QWidget* widget,
                                     const QRect &r,
                                     const QColorGroup &cg,
                                     SFlags flags = Style_Default,
                                     SCFlags controls = SC_All,
                                     SCFlags active = SC_None,
                                     const QStyleOption& opt = QStyleOption::Default ) const
    {
        KStyle::drawComplexControl( control, p, widget, r, cg, flags, controls, active, opt );
    }


    virtual void drawKStylePrimitive( KStylePrimitive kpe,
                                      QPainter* p,
                                      const QWidget* widget,
                                      const QRect &r,
                                      const QColorGroup &cg,
                                      SFlags flags = Style_Default,
                                      const QStyleOption& opt = QStyleOption::Default ) const
    {
        KStyle::drawKStylePrimitive( kpe,
                                     p, widget, r,
                                     cg, flags, opt );
    }


    virtual int styleHint( StyleHint sh,
                           const QWidget *widget = 0,
                           const QStyleOption& opt = QStyleOption::Default,
                           QStyleHintReturn* returnData = 0 ) const
    {
        return KStyle::styleHint( sh,
                                  widget,
                                  opt,
                                  returnData );
    }

    virtual QSize sizeFromContents( ContentsType contents,
                                    const QWidget *widget,
                                    const QSize &contentsSize,
                                    const QStyleOption& opt = QStyleOption::Default ) const
    {
        return KStyle::sizeFromContents( contents,
                                         widget, contentsSize, opt );
    }

private:
    KThemeBasePrivate *d;

    SButton sbPlacement;
    ArrowStyle arrowStyle;
    ShadeStyle shading;
    int defaultFrame;
    int btnXShift, btnYShift;
    int sliderLen;
    int splitterWidth;
    int focus3DOffset;
    int sbExtent;
    bool smallGroove;
    bool roundedButton, roundedCombo, roundedSlider;
    bool aTabLine, iTabLine;
    bool focus3D;
    KThemeCache *cache;
    int cacheSize;
    QString configFileName;
    QString configDirName;

    /**
     * The theme pixmaps. Many of these may be NULL if no pixmap is specified.
     * There may also be duplicate pixmap pointers if more than one widget
     * uses the same tiled pixmap. If a pixmap is tiled, it is kept here and
     * this acts as a cache. Otherwise this will hold whatever the last scaled
     * pixmap was.
     */
    mutable KThemePixmap *pixmaps[ WIDGETS ];
    /**
     * The theme images. These are for scaled images and are kept in order
     * to maintain fast smoothscaling.
     */
    mutable QImage *images[ WIDGETS ];
    /**
     * The border widths
     */
    mutable unsigned char borders[ WIDGETS ];
    /**
     * The highlight widths
     */
    mutable unsigned char highlights[ WIDGETS ];
    /**
     * The scale hints for pixmaps and gradients.
     */
    mutable ScaleHint scaleHints[ WIDGETS ];
    /**
     * All the color groups.
     */
    mutable QColorGroup *colors[ WIDGETS ];
    /**
     * Gradient low colors (or blend background).
     */
    mutable QColor *grLowColors[ WIDGETS ];
    /**
     * Gradient high colors.
     */
    mutable QColor *grHighColors[ WIDGETS ];
    /**
     * Gradient types.
     */
    mutable Gradient gradients[ WIDGETS ];
    /**
     * Blend intensity factors
     */
    mutable float blends[ WIDGETS ];
    /**
     * Bevel contrasts
     */
    mutable unsigned char bContrasts[ WIDGETS ];
    /**
     * Duplicate pixmap entries (used during destruction).
     */
    mutable bool duplicate[ WIDGETS ];
    /**
     * Pixmapped border widths
     */
    mutable int pbWidth[ WIDGETS ];
    /**
     * Pixmapped borders
     */
    mutable KThemePixmap *pbPixmaps[ WIDGETS ];
    /**
     * Duplicate border pixmapped border entries
     */
    mutable bool pbDuplicate[ WIDGETS ];

};

inline bool KThemeBase::isPixmap( WidgetType widget ) const
{
    return ( pixmaps[ widget ] != NULL || gradients[ widget ] != GrNone );
}

inline bool KThemeBase::isColor( WidgetType widget ) const
{
    return ( colors[ widget ] != NULL );
}

inline bool KThemeBase::is3DFocus() const
{
    return ( focus3D );
}

inline int KThemeBase::focusOffset() const
{
    return ( focus3DOffset );
}

inline int KThemeBase::bevelContrast( WidgetType widget ) const
{
    return ( bContrasts[ widget ] );
}

inline KThemeBase::ScaleHint KThemeBase::scaleHint( WidgetType widget ) const
{
    return ( ( widget < WIDGETS ) ? scaleHints[ widget ] : TileScale );
}

inline KThemeBase::Gradient KThemeBase::gradientHint( WidgetType widget ) const
{
    return ( ( widget < WIDGETS ) ? gradients[ widget ] : GrNone );
}

inline KThemePixmap* KThemeBase::uncached( WidgetType widget ) const
{
    return ( pixmaps[ widget ] );
}

inline QBrush KThemeBase::pixmapBrush( const QColorGroup &group,
                                       QColorGroup::ColorRole role,
                                       int w, int h, WidgetType widget )
{
    if ( pixmaps[ widget ] || images[ widget ] )
        return ( QBrush( group.color( role ), *scalePixmap( w, h, widget ) ) );
    else
        return ( group.color( role ) );
}

inline const QColorGroup* KThemeBase::colorGroup( const QColorGroup &defaultGroup,
        WidgetType widget ) const
{
    return ( ( colors[ widget ] ) ? colors[ widget ] : &defaultGroup );
}

inline int KThemeBase::borderWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? pbWidth[ widget ] : borders[ widget ] );
}

inline int KThemeBase::pixBorderWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] );
}

inline int KThemeBase::highlightWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? 0 : highlights[ widget ] );
}

inline int KThemeBase::decoWidth( WidgetType widget ) const
{
    return ( pbWidth[ widget ] ? pbWidth[ widget ] : borders[ widget ] + highlights[ widget ] );
}

inline QColor* KThemeBase::gradientHigh( WidgetType widget ) const
{
    return ( grHighColors[ widget ] );
}

inline QColor* KThemeBase::gradientLow( WidgetType widget ) const
{
    return ( grLowColors[ widget ] );
}

inline QImage* KThemeBase::image( WidgetType widget ) const
{
    return ( images[ widget ] );
}

inline KThemeBase::SButton KThemeBase::scrollBarLayout() const
{
    return ( sbPlacement );
}

inline KThemeBase::ArrowStyle KThemeBase::arrowType() const
{
    return ( arrowStyle );
}

inline KThemeBase::ShadeStyle KThemeBase::shade() const
{
    return ( shading );
}

inline int KThemeBase::frameWidth() const
{
    return ( defaultFrame );
}

inline int KThemeBase::buttonXShift() const
{
    return ( btnXShift );
}

inline int KThemeBase::splitWidth() const
{
    return ( splitterWidth );
}

inline int KThemeBase::buttonYShift() const
{
    return ( btnYShift );
}

inline int KThemeBase::sliderButtonLength() const
{
    if ( isPixmap( Slider ) )
        return ( uncached( Slider ) ->width() );
    else
        return ( sliderLen );
}

inline bool KThemeBase::roundButton() const
{
    return ( roundedButton );
}

inline bool KThemeBase::roundComboBox() const
{
    return ( roundedCombo );
}

inline bool KThemeBase::roundSlider() const
{
    return ( roundedSlider );
}

inline bool KThemeBase::activeTabLine() const
{
    return ( aTabLine );
}

inline bool KThemeBase::inactiveTabLine() const
{
    return ( iTabLine );
}

inline int KThemeBase::getSBExtent() const
{
    return ( sbExtent );
}

inline KThemePixmap* KThemeBase::borderPixmap( WidgetType widget ) const
{
    return ( pbPixmaps[ widget ] );
}

#endif
