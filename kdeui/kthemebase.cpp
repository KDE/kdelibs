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

#include <kthemebase.h>
#include <kpixmapeffect.h>
#include <kapp.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qpainter.h>

static const char *widgetEntries[] = {"HorizScrollGroove", "VertScrollGroove",
"Slider", "SliderGroove", "IndicatorOn", "IndicatorOff", "Background",
"PushButton", "ExIndicatorOn", "ExIndicatorOff", "ComboBox", "ScrollBarSlider",
"Bevel", "ToolButton", "ScrollBarButton", "BarHandle", "ToolBar",
"ScrollBarDeco", "ComboDeco", "Splitter", "CheckMark", "MenuItemOn",
"MenuItemOff", "MenuBar", "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight",
"ProgressBar", "ProgressBackground"};

// Used only internally for handling non-widget option keys. These are needed
// because they get iterated through in a couple of places and it allows the
// use of small for loops.

#define OPTIONS 18

static const char *optionEntries[]={"SButtonType", "ArrowType", "ComboDeco",
"ShadeStyle", "RoundButton", "RoundCombo", "RoundSlider", "FrameWidth",
"ButtonXShift", "ButtonYShift", "SliderLength", "SplitterHandle", "Name",
"Description", "CacheSize", "SmallSliderGroove", "3DFocus", "FocusOffset"};

enum OptionLabel{OptSButtonType=0, OptArrowType, OptComboDeco, OptShadeStyle,
OptRoundButton, OptRoundCombo, OptRoundSlider, OptFrameWidth, OptButtonXShift,
OptButtonYShift, OptSliderLength, OptSplitterHandle, OptName, OptDescription,
OptCacheSize, OptSmallGroove, Opt3DFocus, OptFocusOffset};

#define WGROUPS 10

static const char *wGroupEntries[]={"Scale", "Gradients", "Gradient Lowcolor",
"Gradient Highcolor", "Extended Background", "Extended Foreground", "Borders",
"Highlights", "Pixmaps", "Blend"};

enum WGroupLabel{WScale=0, WGradients, WGradientLow, WGradientHigh,
WExtBackground, WExtForeground, WBorders, WHighlights, WPixmaps, WBlend};



// This is used to encode the keys. I used to use masks but I think this
// bitfield is nicer :) I don't know why C++ coders don't use these more..
// (mosfet)
struct kthemeKeyData{
    unsigned int id          :5;
    unsigned int width       :14;
    unsigned int height      :13;
};

union kthemeKey{
    kthemeKeyData data;
    unsigned int cacheKey;
};


void KThemeBase::readConfig(Qt::GUIStyle style)
{
    
    // Pixmap only widgets. These widgets aren't scaled and can be blended once
    // on loading
#define PREBLEND_ITEMS 8
    static WidgetType preBlend[]={Slider, IndicatorOn, IndicatorOff,
    ExIndicatorOn, ExIndicatorOff, ScrollDeco, ComboDeco, CheckMark};

    int i;
    QString tmpStr;
   // debug("KThemeStyle: Reading theme settings.");
    KConfig config("kstylerc", true, false);

    // Read in misc settings
    config.setGroup("Misc");
    tmpStr = config.readEntry(optionEntries[OptSButtonType]);
    if(tmpStr == "BottomLeft")
        sbPlacement = SBBottomLeft;
    else if(tmpStr == "BottomRight")
        sbPlacement = SBBottomRight;
    else{
        if(tmpStr != "Opposite" && !tmpStr.isEmpty())
            warning("KThemeStyle: Unrecognized sb button option %s, using Opposite.",
                    tmpStr.ascii());
        sbPlacement = SBOpposite;
    }
    tmpStr = config.readEntry(optionEntries[OptArrowType]);
    if(tmpStr == "Small")
        arrowStyle = SmallArrow;
    else if(tmpStr == "3D")
        arrowStyle = MotifArrow;
    else{
        if(tmpStr != "Normal" && !tmpStr.isEmpty())
            warning("KThemeStyle: Unrecognized arrow option %s, using Windows.",
                    tmpStr.ascii());
        arrowStyle = LargeArrow;
    }
    tmpStr = config.readEntry(optionEntries[OptShadeStyle]);
    if(tmpStr == "Motif")
        shading = Motif;
    else if(tmpStr == "Next")
        shading = Next;
    else
        shading = Windows;
    smallGroove = config.
        readBoolEntry(optionEntries[OptSmallGroove], false);
    roundedButton = config.
        readBoolEntry(optionEntries[OptRoundButton], false);
    roundedCombo = config.
        readBoolEntry(optionEntries[OptRoundCombo], false);
    roundedSlider = config.
        readBoolEntry(optionEntries[OptRoundSlider], false);
    focus3D = config.
        readBoolEntry(optionEntries[Opt3DFocus], false);
    focus3DOffset = config.
        readNumEntry(optionEntries[OptFocusOffset], 0);
    defaultFrame = config.
        readNumEntry(optionEntries[OptFrameWidth], 2);
    btnXShift = config.
        readNumEntry(optionEntries[OptButtonXShift], 0);
    btnYShift = config.
        readNumEntry(optionEntries[OptButtonYShift], 0);
    sliderLen = config.
        readNumEntry(optionEntries[OptSliderLength], 30);
    splitterWidth = config.
        readNumEntry(optionEntries[OptSplitterHandle], 10);
    cacheSize = config.
        readNumEntry(optionEntries[OptCacheSize], 1024);

    // Read in the scale hints
    config.setGroup(wGroupEntries[WScale]);
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config.readEntry(widgetEntries[i]);
        if(tmpStr == "Full")
            scaleHints[i] = FullScale;
        else if(tmpStr == "Horizontal")
            scaleHints[i] =HorizontalScale;
        else if(tmpStr == "Vertical")
            scaleHints[i] = VerticalScale;
        else{
            if(tmpStr != "Tile" && !tmpStr.isEmpty())
                warning("KThemeStyle: Unrecognized scale option %s, using Tile.",
                        tmpStr.ascii());
            scaleHints[i] = TileScale;
        }
    }
    // Read in gradient types
    config.setGroup(wGroupEntries[WGradients]);
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config.readEntry(widgetEntries[i]);
        if(tmpStr == "Diagonal")
            gradients[i] = GrDiagonal;
        else if(tmpStr == "Horizontal")
            gradients[i] = GrHorizontal;
        else if(tmpStr == "Vertical")
            gradients[i] = GrVertical;
        else if(tmpStr == "Pyramid")
            gradients[i] = GrPyramid;
        else if(tmpStr == "Rectangle")
            gradients[i] = GrRectangle;
        else if(tmpStr == "Elliptic")
            gradients[i] = GrElliptic;
        else{
            if(tmpStr != "None" && !tmpStr.isEmpty())
                warning("KThemeStyle: Unrecognized gradient option %s, using None.",
                        tmpStr.ascii());
            gradients[i] = GrNone;
        }
    }
    // Read in blend intensity values
    config.setGroup(wGroupEntries[WBlend]);
    for(i=0; i < WIDGETS; ++i)
        blends[i] = config.readDoubleNumEntry(widgetEntries[i], 0.0);

    // Read in gradient low colors (or blend background)
    config.setGroup(wGroupEntries[WGradientLow]);
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != GrNone || blends[i] != 0.0){
            grLowColors[i] =
                new QColor(config.readColorEntry(widgetEntries[i],
                                                 &kapp->palette().normal().
                                                 background()));
        }
        else
            grLowColors[i] = NULL;
    }
    // Read in gradient high colors
    config.setGroup(wGroupEntries[WGradientHigh]);
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != GrNone){
            grHighColors[i] =
                new QColor(config.readColorEntry(widgetEntries[i],
                                                 &kapp->palette().normal().
                                                 background()));
        }
        else
            grHighColors[i] = NULL;
    }
    // Read in the extended color attributes
    QColor bg[WIDGETS]; // We need to store these for a sec
    config.setGroup(wGroupEntries[WExtBackground]);
    for(i=0; i < WIDGETS; ++i){
        if(config.hasKey(widgetEntries[i]))
            bg[i] = config.readColorEntry(widgetEntries[i], &bg[i]);
    }
    // Combine fg and stored bg colors into a color group
    config.setGroup(wGroupEntries[WExtForeground]);
    QColor fg;
    for(i=0; i < WIDGETS; ++i){
        if(config.hasKey(widgetEntries[i]) || bg[i].isValid()){
            fg = kapp->palette().normal().foreground();
            fg = config.readColorEntry(widgetEntries[i], &fg);
            colors[i] = makeColorGroup(fg, bg[i], style);
        }
        else
            colors[i] = NULL;
    }
    // Read in border widths
    config.setGroup(wGroupEntries[WBorders]);
    for(i=0; i < WIDGETS; ++i)
        borders[i] = config.readNumEntry(widgetEntries[i], 1);
    // Read in highlight widths
    config.setGroup(wGroupEntries[WHighlights]);
    for(i=0; i < WIDGETS; ++i)
        highlights[i] = config.readNumEntry(widgetEntries[i], 1);
    // Read in pixmaps
    config.setGroup(wGroupEntries[WPixmaps]);
    int existing;
    QString pixnames[WIDGETS];
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config.readEntry(widgetEntries[i], "");
        pixnames[i] = tmpStr;
        duplicate[i] = false;
        pixmaps[i] = NULL;
        images[i] = NULL;
        if(!tmpStr.isEmpty()){
            // Scan for duplicate pixmaps (if two identical pixmaps are both
            // tile scale and not blended).
            for(existing=0; existing < i; ++existing){
                if(tmpStr == pixnames[existing] && scaleHints[i] ==
                   TileScale && scaleHints[existing] == TileScale
                   && blends[existing] == 0.0 && blends[i] == 0.0){
                    pixmaps[i] = pixmaps[existing];
                    images[i] = NULL;
                    duplicate[i] = true;
                    warning("KThemeStyle: Marking %s as duplicate.",
                            pixnames[i].ascii());
                    break;
                }
            }
            // load pixmap
            if(!duplicate[i]){
                if(tmpStr.isEmpty()){
                    warning("KThemeStyle: No pixmap specified for %s.",
                            widgetEntries[i]);
                }
                else{
                    if(scaleHints[i] == TileScale && blends[i] == 0.0){
                        pixmaps[i] = loadPixmap(tmpStr);
                        images[i] = NULL;
                    }
                    else{
                        images[i] = loadImage(tmpStr);
                        pixmaps[i] = loadPixmap(tmpStr);
                    }
                }
            }
        }
    }

    // Handle preblend items
    for(i=0; i < PREBLEND_ITEMS; ++i){
        if(pixmaps[preBlend[i]] != NULL && blends[preBlend[i]] != 0.0)
            blend(preBlend[i]);
    }
    
#ifdef KSTYLE_DEBUG
    for(existing=0, i=0; i < WIDGETS; ++i)
        if(images[i] || pixmaps[i])
            ++existing;
    warning("KThemeStyle: %d out of %d pixmaps allocated", existing, WIDGETS);
#endif
    warning("KThemeStyle: Finished reading theme settings.");

}

KThemeBase::KThemeBase(const QString &)
    :KStyle()
{
    KGlobal::dirs()->addResourceType("kstyle_pixmap", KStandardDirs::kde_default("data") + "kstyle/pixmaps/");
    readConfig(Qt::WindowsStyle);
    cache = new KThemeCache(cacheSize);
}                            

KThemeBase::~KThemeBase()
{
    int i;
    for(i=0; i < WIDGETS; ++i){
        if(!duplicate[i]){
            if(images[i])
                delete images[i];
            if(pixmaps[i])
                delete pixmaps[i];
        }
        if(colors[i])
            delete(colors[i]);
        if(grLowColors[i])
            delete(grLowColors[i]);
        if(grHighColors[i])
            delete(grHighColors[i]);
    }
    delete cache;
}

void KThemeBase::applyConfigFile(const QString &inFile)
{
    int group, widget;
    KConfig inConfig(inFile, true, false);
    KConfig outConfig("kstylerc", false, false);

    // Write the theme specific keys to kstylerc
    for(group=0; group < WGROUPS; ++group){
        outConfig.setGroup(wGroupEntries[group]);
        inConfig.setGroup(wGroupEntries[group]);
        for(widget=0; widget < WIDGETS; ++widget)
            outConfig.writeEntry(widgetEntries[widget],
                                 inConfig.readEntry(widgetEntries[widget],
                                                    " "));
    }
    inConfig.setGroup("Misc");
    outConfig.setGroup("Misc");
    for(widget=0; widget < OPTIONS; ++widget){
        outConfig.writeEntry(optionEntries[widget],
                             inConfig.readEntry(optionEntries[widget], " "));
    }

    // Write the std color scheme keys to kdeglobals
    KConfig *globalConfig = KGlobal::config();
    // I have no idea why I have to call rollback but if I don't *all* keys
    // get written to kdeglobals, not just the ones I write! Can someone look
    // at this? I am probably doing sumthin' stupid here ;-)
    globalConfig->rollback(true);
    globalConfig->setGroup("General");
    inConfig.setGroup("General");
    if(inConfig.hasKey("foreground"))
        globalConfig->writeEntry("foreground",
                                 inConfig.readEntry("foreground", " "), true,
                                 true);
    if(inConfig.hasKey("background"))
        globalConfig->writeEntry("background",
                                 inConfig.readEntry("background", " "), true,
                                true);
    if(inConfig.hasKey("selectForeground"))
        globalConfig->writeEntry("selectForeground",
                                 inConfig.readEntry("selectForeground", " "),
                                 true, true);
    if(inConfig.hasKey("selectBackground"))
        globalConfig->writeEntry("selectBackground",
                                 inConfig.readEntry("selectBackground", " "),
                                 true, true);
    if(inConfig.hasKey("windowForeground"))
        globalConfig->writeEntry("windowForeground",
                                 inConfig.readEntry("windowForeground", " "),
                                 true, true);
    if(inConfig.hasKey("windowBackground"))
        globalConfig->writeEntry("windowBackground",
                                 inConfig.readEntry("windowBackground", " "),
                                 true, true);
    globalConfig->setGroup("KDE");
    inConfig.setGroup("KDE");

    if(inConfig.hasKey("contrast"))
        globalConfig->writeEntry("contrast",
                                 inConfig.readEntry("contrast", " "), true,
                                 true);
    globalConfig->writeEntry("widgetStyle",
                             inConfig.readEntry("widgetStyle", " "), true,
                             true);
    globalConfig->sync();
}

QColorGroup* KThemeBase::makeColorGroup(QColor &fg, QColor &bg,
                                        Qt::GUIStyle)
{
    if(shading == Motif){
        int highlightVal, lowlightVal;
        highlightVal=100+(2*kapp->contrast()+4)*16/10;
        lowlightVal=100+((2*kapp->contrast()+4)*10);
        return(new QColorGroup(fg, bg, bg.light(highlightVal),
                               bg.dark(lowlightVal), bg.dark(120),
                               fg, kapp->palette().normal().base()));
    }
    else
        return(new QColorGroup( fg, bg, bg.light(150), bg.dark(),
                                bg.dark(120), fg,
                                kapp->palette().normal().base()));
}

QImage* KThemeBase::loadImage(QString &name)
{
    QImage *image = new QImage;
    QString path = locate("kstyle_pixmap", name);
    image->load(path);
    if(!image->isNull())
        return(image);
    warning("KThemeStyle: Unable to load image %s.", name.ascii());
    delete image;
    return(NULL);
}
 
KThemePixmap* KThemeBase::loadPixmap(QString &name)
{
    KThemePixmap *pixmap = new KThemePixmap;
    QString path = locate("kstyle_pixmap", name);
    pixmap->load(path);
    if (!pixmap->isNull())
       return pixmap;
    warning("KThemeStyle: Unable to load pixmap %s.", name.ascii());
    delete pixmap;
    return(NULL);
}

KThemePixmap* KThemeBase::scale(int w, int h, WidgetType widget)
{
    if(scaleHints[widget] == FullScale){
        if(!pixmaps[widget] || pixmaps[widget]->width() != w ||
           pixmaps[widget]->height() != h){
            KThemePixmap *cachePix = cache->pixmap(w, h, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                QImage tmpImg = images[widget]->smoothScale(w, h);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
                if(blends[widget] != 0.0)
                    blend(widget);
            }
        }
    }
    else if(scaleHints[widget] == HorizontalScale){
        if(pixmaps[widget]->width() != w){
            KThemePixmap *cachePix = cache->horizontalPixmap(w, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::HorizontalScale, widget);
                QImage tmpImg = images[widget]->
                    smoothScale(w, images[widget]->height());
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
                if(blends[widget] != 0.0)
                    blend(widget);
            }
        }
    }
    else if(scaleHints[widget] == VerticalScale){
        if(pixmaps[widget]->height() != h){
            KThemePixmap *cachePix = cache->verticalPixmap(w, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::VerticalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::VerticalScale, widget);
                QImage tmpImg =
                    images[widget]->smoothScale(images[widget]->width(), h);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
                if(blends[widget] != 0.0)
                    blend(widget);
            }
        }
    }
    // If blended tile here so the blend is scaled properly
    else if(scaleHints[widget] == TileScale && blends[widget] != 0.0){
        if(!pixmaps[widget] || pixmaps[widget]->width() != w ||
           pixmaps[widget]->height() != h){
            KThemePixmap *cachePix = cache->pixmap(w, h, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                QPixmap tile;
                tile.convertFromImage(*images[widget]);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->resize(w, h);
                QPainter p(pixmaps[widget]);
                p.drawTiledPixmap(0, 0, w, h, tile);
                if(blends[widget] != 0.0)
                    blend(widget);
            }
        }
    }
    return(pixmaps[widget]);
}

KThemePixmap* KThemeBase::blend(WidgetType widget)
{
    KPixmapEffect::GradientType g;
    switch(gradients[widget]){
    case GrHorizontal:
        g = KPixmapEffect::HorizontalGradient;
        break;
    case GrVertical:
        g = KPixmapEffect::VerticalGradient;
        break;
    case GrPyramid:
        g = KPixmapEffect::PyramidGradient;
        break;
    case GrRectangle:
        g = KPixmapEffect::RectangleGradient;
        break;
    case GrElliptic:
        g = KPixmapEffect::EllipticGradient;
        break;
    default:
        g = KPixmapEffect::DiagonalGradient;
        break;
    }
    KPixmapEffect::blend(*pixmaps[widget], blends[widget], *grLowColors[widget],
                         g, false);
    return(pixmaps[widget]);
}

KThemePixmap* KThemeBase::gradient(int w, int h, WidgetType widget)
{
    if(gradients[widget] == GrVertical){
        if(!pixmaps[widget] || pixmaps[widget]->height() != h){
            KThemePixmap *cachePix = cache->verticalPixmap(h, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::VerticalScale,
                                  widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::VerticalScale,
                                  widget);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->resize(w, h);
                KPixmapEffect::gradient(*pixmaps[widget], *grHighColors[widget],
                                        *grLowColors[widget],
                                        KPixmapEffect::VerticalGradient);
            }
        }
    }
    else if(gradients[widget] == GrHorizontal){
        if(!pixmaps[widget] || pixmaps[widget]->width() != w){
            KThemePixmap *cachePix = cache->horizontalPixmap(w, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget],
                                  KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget],
                                  KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->resize(w, h);
                KPixmapEffect::gradient(*pixmaps[widget], *grHighColors[widget],
                                        *grLowColors[widget],
                                        KPixmapEffect::HorizontalGradient);
            }
        }
    }
    else{
        KPixmapEffect::GradientType g;
        switch(gradients[widget]){
        case GrPyramid:
            g = KPixmapEffect::PyramidGradient;
            break;
        case GrRectangle:
            g = KPixmapEffect::RectangleGradient;
            break;
        case GrElliptic:
            g = KPixmapEffect::EllipticGradient;
            break;
        default:
            g = KPixmapEffect::DiagonalGradient;
            break;
        }
        if(!pixmaps[widget] || pixmaps[widget]->width() != w ||
           pixmaps[widget]->height() != h){
            KThemePixmap *cachePix = cache->pixmap(w, h, widget);
            if(cachePix){
                cachePix = new KThemePixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::FullScale,
                                  widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::FullScale,
                                  widget);
                pixmaps[widget] = new KThemePixmap;
                pixmaps[widget]->resize(w, h);
                KPixmapEffect::gradient(*pixmaps[widget], *grHighColors[widget],
                                        *grLowColors[widget], g);
            }
        }
    }
    return(pixmaps[widget]);
}

KThemePixmap* KThemeBase::scalePixmap(int w, int h, WidgetType widget)
{

    if(gradients[widget] && blends[widget] == 0.0)
        return(gradient(w, h, widget));

    return(scale(w, h, widget));
}

KThemeCache::KThemeCache(int maxSize, QObject *parent, const char *name)
    : QObject(parent, name)
{
    cache.setMaxCost(maxSize*1024);
    cache.setAutoDelete(true);
    flushTimer.start(300000); // 5 minutes
    connect(&flushTimer, SIGNAL(timeout()), SLOT(flushTimeout()));
}

void KThemeCache::flushTimeout()
{
    QIntCacheIterator<KThemePixmap> it(cache);
    for(;it.current(); ++it){
        if(it.current()->isOld()){
            kthemeKey key;
            key.cacheKey = it.currentKey();
            cache.remove(it.currentKey());
        }
    }
}

KThemePixmap* KThemeCache::pixmap(int w, int h, int widgetID)
{

    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = h;
    KThemePixmap *pix = cache.find(key.cacheKey);
    if(pix){
        pix->updateAccessed();
    }
    return(pix);
}

KThemePixmap* KThemeCache::horizontalPixmap(int w, int widgetID)
{
    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = 0;
    KThemePixmap *pix = cache.find(key.cacheKey);
    if(pix){
        pix->updateAccessed();
    }
    return(pix);
}

KThemePixmap* KThemeCache::verticalPixmap(int h, int widgetID)
{
    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = 0;
    key.data.height = h;
    KThemePixmap *pix = cache.find(key.cacheKey);
    if(pix){
        pix->updateAccessed();
    }
    return(pix);
}

bool KThemeCache::insert(KThemePixmap *pixmap, ScaleHint scale, int widgetID)
{
    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = (scale == FullScale || scale == HorizontalScale) ?
        pixmap->width() : 0;
    key.data.height = (scale == FullScale || scale == VerticalScale) ?
        pixmap->height() : 0;

    if(cache.find(key.cacheKey, true) != NULL){
        return(true); // a pixmap of this scale is already in there
    }
    return(cache.insert(key.cacheKey, pixmap,
                        pixmap->width()*pixmap->height()*pixmap->depth()/8));
}

#include "kthemebase.moc"
