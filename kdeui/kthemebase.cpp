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
#include <qbitmap.h>
#include <stdlib.h>

static const char *widgetEntries[] = { // unsunken widgets (see header)
"PushButton", "ComboBox", "HSBarSlider", "VSBarSlider", "Bevel", "ToolButton",
"ScrollButton", "HScrollDeco", "VScrollDeco", "ComboDeco", "MenuItem", "Tab",
"ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight",
// sunken widgets
"PushButtonDown", "ComboBoxDown", "HSBarSliderDown", "VSBarSliderDown",
"BevelDown", "ToolButtonDown", "ScrollButtonDown", "HScrollDecoDown",
"VScrollDecoDown", "ComboDecoDown", "MenuItemDown", "TabDown", "SunkenArrowUp",
"SunkenArrowDown", "SunkenArrowLeft", "SunkenArrowRight",
// everything else
"HScrollGroove", "VScrollGroove", "Slider", "SliderGroove", "CheckBoxDown",
"CheckBox", "RadioDown", "Radio", "HBarHandle", "VBarHandle",
"ToolBar", "Splitter", "CheckMark", "MenuBar", "DisableArrowUp",
"DisableArrowDown", "DisableArrowLeft", "DisableArrowRight", "ProgressBar",
"ProgressBackground", "MenuBarItem", "Background"
};

#define INHERIT_ITEMS 16


// This is used to encode the keys. I used to use masks but I think this
// bitfield is nicer :) I don't know why C++ coders don't use these more..
// (mosfet)
struct kthemeKeyData{
    unsigned int id          :6;
    unsigned int width       :13;
    unsigned int height      :13;
};

union kthemeKey{
    kthemeKeyData data;
    unsigned int cacheKey;
};

// reads a single widget's config
void KThemeBase::readWidgetConfig(int i, KConfig *config, QString *pixnames,
                                  QString *brdnames, bool *loadArray)
{
    if(loadArray[i] == true){
        return; // already been preloaded.
    }
    
    config->setGroup(widgetEntries[i]);
    QString tmpStr;

    if(config->hasKey("CopyWidget")){ // Duplicate another widget's config
        int sIndex;
        tmpStr = config->readEntry("CopyWidget", "");
        if(!tmpStr.isEmpty()){
            loadArray[i] = true;
            for(sIndex=0; sIndex < WIDGETS; ++sIndex){
                if(tmpStr == widgetEntries[sIndex]){
                    if(!loadArray[sIndex]) // hasn't been loaded yet
                        readWidgetConfig(sIndex, config, pixnames, brdnames,
                                         loadArray);
                    break;
                }
            }
            if(loadArray[sIndex]){
                copyWidgetConfig(sIndex, i, pixnames, brdnames);
            }
            else{
                warning("KThemeBase: Unable to identify source widget for %s!",
                        widgetEntries[i]);
            }
            return;
        }
    }

    // special inheritance for disabled arrows (these are tri-state unlike
    // the rest of what we handle).
    int tmp;
    for(tmp = DisArrowUp; tmp <= DisArrowRight; ++tmp){
        if(tmp == i){
            if(config->readEntry("Pixmap", "").isEmpty()){
                copyWidgetConfig(ArrowUp+(tmp-DisArrowUp), i, pixnames,
                                 brdnames);
                return;
            }
        }
    }

    // Scale hint
    tmpStr = config->readEntry("Scale");
    if(tmpStr == "Full")
        scaleHints[i] = FullScale;
    else if(tmpStr == "Horizontal")
        scaleHints[i] =HorizontalScale;
    else if(tmpStr == "Vertical")
        scaleHints[i] = VerticalScale;
    else{
        if(tmpStr != "Tile" && !tmpStr.isEmpty())
            warning("KThemeBase: Unrecognized scale option %s, using Tile.",
                    tmpStr.ascii());
        scaleHints[i] = TileScale;
    }
    
    // Gradient type
    tmpStr = config->readEntry("Gradient");
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
    else if(tmpStr == "ReverseBevel")
        gradients[i] = GrReverseBevel;
    else{
        if(tmpStr != "None" && !tmpStr.isEmpty())
            warning("KThemeBase: Unrecognized gradient option %s, using None.",
                    tmpStr.ascii());
        gradients[i] = GrNone;
    }

    // Blend intensity
    blends[i] = config->readDoubleNumEntry("BlendIntensity", 0.0);

    // Bevel contrast
    bContrasts[i] = config->readNumEntry("BevelContrast", 0);

    // Border width
    borders[i] = config->readNumEntry("Border", 1);

    // Highlight width
    highlights[i] = config->readNumEntry("Highlight", 1);

    // Gradient low color or blend background
    if(gradients[i] != GrNone || blends[i] != 0.0)
        grLowColors[i] =
            new QColor(config->readColorEntry("GradientLow",
                                              &kapp->palette().normal().
                                              background()));
    else
        grLowColors[i] = NULL;

    // Gradient high color
    if(gradients[i] != GrNone)
        grHighColors[i] =
            new QColor(config->readColorEntry("GradientHigh",
                                              &kapp->palette().normal().
                                              background()));
    else
        grHighColors[i] = NULL;

    // Extended color attributes
    if(config->hasKey("Forground") || config->hasKey("Background")){
        QColor fg, bg;
        if(config->hasKey("Background"))
            bg = config->readColorEntry("Background", &bg);
        else
            bg = kapp->palette().normal().background();
        if(config->hasKey("Foreground"))
            fg = config->readColorEntry("Foreground", &fg);
        else
            fg = kapp->palette().normal().foreground();
        colors[i] = makeColorGroup(fg, bg, Qt::WindowsStyle);
    }
    else
        colors[i] = NULL;

    // Pixmap
    int existing;
    tmpStr = config->readEntry("Pixmap", "");
    pixnames[i] = tmpStr;
    duplicate[i] = false;
    pixmaps[i] = NULL;
    images[i] = NULL;
    // Scan for duplicate pixmaps(two identical pixmaps, tile scale, no blend,
    // no pixmapped border)
    if(!tmpStr.isEmpty()){
        for(existing=0; existing < i; ++existing){
            if(tmpStr == pixnames[existing] && scaleHints[i] == TileScale &&
               scaleHints[existing] == TileScale && blends[existing] == 0.0 &&
               blends[i] == 0.0){
                pixmaps[i] = pixmaps[existing];
                duplicate[i] = true;
                warning("KThemeBase: Marking %s as duplicate.",
                        pixnames[i].latin1());
                break;
            }
        }
    }
    // load
    if(!duplicate[i] && !tmpStr.isEmpty()){
        pixmaps[i] = loadPixmap(tmpStr);
        // load and save images for scaled/blended widgets for speed.
        if(scaleHints[i] == TileScale && blends[i] == 0.0)
            images[i] = NULL;
        else
            images[i] = loadImage(tmpStr);
    }

    // Pixmap border
    tmpStr = config->readEntry("PixmapBorder", "");
    brdnames[i] = tmpStr;
    pbDuplicate[i] = false;
    pbPixmaps[i] = NULL;
    pbWidth[i] = 0;
    if(!tmpStr.isEmpty()){
        pbWidth[i] = config->readNumEntry("PixmapBWidth", 0);
        if(pbWidth[i] == 0){
            warning("KThemeBase: No border width specified for pixmapped border widget %s",
                    widgetEntries[i]);
            warning("KThemeBase: Using default of 2.");
            pbWidth[i] = 2;
        }
        // duplicate check
        for(existing=0; existing < i; ++existing){
            if(tmpStr == brdnames[existing]){
                pbPixmaps[i] = pbPixmaps[existing];
                pbDuplicate[i] = true;
                warning("KThemeBase: Marking border pixmap %s as duplicate.",
                        brdnames[i].latin1());
                break;
            }
        }
    }
    // load
    if(!pbDuplicate[i] && !tmpStr.isEmpty())
        pbPixmaps[i] = loadPixmap(tmpStr);

    if(pbPixmaps[i] && !pbDuplicate[i])
        generateBorderPix(i);

    
    // Various widget specific settings. This was more efficent when bunched
    // together in the misc group, but this makes an easier to read config.
    if(i == SliderGroove)
        roundedSlider = config->readBoolEntry("SmallGroove", false);
    else if(i == ActiveTab)
        aTabLine = config->readBoolEntry("BottomLine", true);
    else if(i == InactiveTab)
        iTabLine = config->readBoolEntry("BottomLine", true);
    else if(i == Splitter)
        splitterWidth = config->readNumEntry("Width", 10);
    else if(i == ComboBox || i == ComboBoxDown){
        if(config->hasKey("Round"))
            roundedCombo = config->readBoolEntry("Round", false);
    }
    else if (i == PushButton || i == PushButtonDown){
        if(config->hasKey("XShift"))
            btnXShift = config->readNumEntry("XShift", 0);
        if(config->hasKey("YShift"))
            btnYShift = config->readNumEntry("YShift", 0);
        if(config->hasKey("3DFocusRect"))
            focus3D = config->readBoolEntry("3DFocusRect", false);
        if(config->hasKey("3DFocusOffset"))
            focus3DOffset = config->readNumEntry("3DFocusOffset", 0);
        if(config->hasKey("Round"))
            roundedButton = config->readBoolEntry("Round", false);
    }

    loadArray[i] = true;
}

void KThemeBase::generateBorderPix(int i)
{
    // separate pixmap into separate components
    if(pbPixmaps[i]){
        // evidently I have to do masks manually...
        const QBitmap *srcMask = pbPixmaps[i]->mask();
        QBitmap destMask(pbWidth[i], pbWidth[i]);
        QPixmap tmp(pbWidth[i], pbWidth[i]);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], 0, 0, pbWidth[i], pbWidth[i],
               Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, 0, 0, pbWidth[i], pbWidth[i],
                   Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::TopLeft, tmp);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], pbPixmaps[i]->width()-pbWidth[i], 0,
               pbWidth[i], pbWidth[i], Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, pbPixmaps[i]->width()-pbWidth[i],
                   0, pbWidth[i], pbWidth[i], Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::TopRight, tmp);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], 0, pbPixmaps[i]->height()-pbWidth[i],
               pbWidth[i], pbWidth[i], Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, 0, pbPixmaps[i]->height()-pbWidth[i],
                   pbWidth[i], pbWidth[i], Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::BottomLeft, tmp);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], pbPixmaps[i]->width()-pbWidth[i],
               pbPixmaps[i]->height()-pbWidth[i], pbWidth[i], pbWidth[i],
               Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, pbPixmaps[i]->width()-pbWidth[i],
                   pbPixmaps[i]->height()-pbWidth[i], pbWidth[i], pbWidth[i],
                   Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::BottomRight, tmp);

        tmp.resize(pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i]);
        destMask.resize(pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i]);
        bitBlt(&tmp, 0, 0, pbPixmaps[i], pbWidth[i], 0,
               pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i], Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, pbWidth[i], 0,
                   pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i],
                   Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::Top, tmp);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], pbWidth[i],
               pbPixmaps[i]->height()-pbWidth[i],
               pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i], Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, pbWidth[i],
                   pbPixmaps[i]->height()-pbWidth[i],
                   pbPixmaps[i]->width()-pbWidth[i]*2, pbWidth[i], Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::Bottom, tmp);

        tmp.resize(pbWidth[i], pbPixmaps[i]->height()-pbWidth[i]*2);
        destMask.resize(pbWidth[i], pbPixmaps[i]->height()-pbWidth[i]*2);
        bitBlt(&tmp, 0, 0, pbPixmaps[i], 0, pbWidth[i], pbWidth[i],
               pbPixmaps[i]->height()-pbWidth[i]*2, Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, 0, pbWidth[i], pbWidth[i],
                   pbPixmaps[i]->height()-pbWidth[i]*2, Qt::CopyROP, false);
            tmp.setMask(destMask);
        }

        pbPixmaps[i]->setBorder(KThemePixmap::Left, tmp);

        bitBlt(&tmp, 0, 0, pbPixmaps[i], pbPixmaps[i]->width()-pbWidth[i],
               pbWidth[i], pbWidth[i], pbPixmaps[i]->height()-pbWidth[i]*2,
               Qt::CopyROP, false);
        if(srcMask){
            bitBlt(&destMask, 0, 0, srcMask, pbPixmaps[i]->width()-pbWidth[i],
                   pbWidth[i], pbWidth[i], pbPixmaps[i]->height()-pbWidth[i]*2,
                   Qt::CopyROP, false);
            tmp.setMask(destMask);
        }
        pbPixmaps[i]->setBorder(KThemePixmap::Right, tmp);
    }
    else
        warning("KThemeBase: Tried making border from empty pixmap");
}
    

void KThemeBase::copyWidgetConfig(int sourceID, int destID, QString *pixnames,
                                 QString *brdnames)
{
    scaleHints[destID] = scaleHints[sourceID];
    gradients[destID] = gradients[sourceID];
    blends[destID] = blends[sourceID];
    bContrasts[destID] = bContrasts[sourceID];
    borders[destID] = borders[sourceID];
    highlights[destID] = highlights[sourceID];

    if(grLowColors[sourceID])
        grLowColors[destID] = new QColor(*grLowColors[sourceID]);
    else
        grLowColors[destID] = NULL;

    if(grHighColors[sourceID])
        grHighColors[destID] = new QColor(*grHighColors[sourceID]);
    else
        grHighColors[destID] = NULL;
    
    if(colors[sourceID])
        colors[destID] = new QColorGroup(*colors[sourceID]);
    else
        colors[destID] = NULL;

    // pixmap
    pixnames[destID] = pixnames[sourceID];
    duplicate[destID] = false;
    pixmaps[destID] = NULL;
    images[destID] = NULL;
    if(!pixnames[destID].isEmpty()){
        if(scaleHints[sourceID] == TileScale && blends[sourceID] == 0.0){
            pixmaps[destID] = pixmaps[sourceID];
            duplicate[destID] = true;
            warning("KThemeBase: Marking %s as duplicate.",
                    pixnames[destID].latin1());
        }
        if(!duplicate[destID]){
            pixmaps[destID] = loadPixmap(pixnames[destID]);
            if(scaleHints[destID] == TileScale && blends[destID] == 0.0)
                images[destID] = NULL;
            else
                images[destID] = loadImage(pixnames[destID]);
        }
    }

    // border pixmap
    pbDuplicate[destID] = false;
    pbPixmaps[destID] = NULL;
    pbWidth[destID] = pbWidth[sourceID];
    brdnames[destID] = brdnames[sourceID];
    if(!brdnames[destID].isEmpty()){
        pbPixmaps[destID] = pbPixmaps[sourceID];
        pbDuplicate[destID] = true;
        warning("KThemeBase: Marking border pixmap %s as duplicate.",
                pixnames[destID].latin1());
    }
    
    if(sourceID == ActiveTab && destID == InactiveTab)
        aTabLine = iTabLine;
    else if(sourceID == InactiveTab && destID == ActiveTab)
        iTabLine = aTabLine;
}

void KThemeBase::readConfig(Qt::GUIStyle /*style*/)
{
#define PREBLEND_ITEMS 12
    static WidgetType preBlend[]={Slider, IndicatorOn, IndicatorOff,
    ExIndicatorOn, ExIndicatorOff, HScrollDeco, VScrollDeco, HScrollDecoDown,
    VScrollDecoDown, ComboDeco, ComboDecoDown, CheckMark};

    int i;
    QString tmpStr;
    QString pixnames[WIDGETS]; // used for duplicate check
    QString brdnames[WIDGETS];
    bool loaded[WIDGETS]; // used for preloading for CopyWidget

    // initalize defaults that may not be read
    for(i=0; i < WIDGETS; ++i)
        loaded[i] = false;
    btnXShift = btnYShift = focus3DOffset = 0;
    aTabLine = iTabLine = true;
    roundedButton = roundedCombo = roundedSlider = focus3D = false;
    splitterWidth = 10;
    
    KConfig config("kstylerc", true, false);
    for(i=0; i < INHERIT_ITEMS; ++i) 
        readWidgetConfig(i, &config, pixnames, brdnames, loaded);
    for(; i < INHERIT_ITEMS*2; ++i){
        if(config.hasGroup(widgetEntries[i]))
            readWidgetConfig(i, &config, pixnames, brdnames, loaded);
        else
            copyWidgetConfig(i-INHERIT_ITEMS, i, pixnames, brdnames);
    }
    for(; i < WIDGETS; ++i) 
        readWidgetConfig(i, &config, pixnames, brdnames, loaded);

    // misc items
    config.setGroup("Misc");
    tmpStr = config.readEntry("SButtonPosition");
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
    tmpStr = config.readEntry("ArrowType");
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
    tmpStr = config.readEntry("ShadeStyle");
    if(tmpStr == "Motif")
        shading = Motif;
    else if(tmpStr == "Next")
        shading = Next;
    else
        shading = Windows;
    defaultFrame = config.readNumEntry("FrameWidth", 2);
    cacheSize = config. readNumEntry("Cache", 1024);

    // Handle preblend items
    for(i=0; i < PREBLEND_ITEMS; ++i){
        if(pixmaps[preBlend[i]] != NULL && blends[preBlend[i]] != 0.0)
            blend(preBlend[i]);
    }
}

KThemeBase::KThemeBase(const QString &)
    :KStyle()
{
    KGlobal::dirs()->addResourceType("kstyle_pixmap", KStandardDirs::kde_default("data") + "kstyle/pixmaps/");
    readConfig(Qt::WindowsStyle);
    cache = new KThemeCache(cacheSize);
}

void KThemeBase::applyConfigFile(const QString &file)
{
    // Fix to use KStdDirs

    // Do copy ourselves
    QFile src(file);
    QFile dest(QDir::homeDirPath() + "/.kde/share/config/kstylerc");
    if(!src.open(IO_ReadOnly)){
        warning("Cannot open theme file for reading!");
        return;
    }
    if(!dest.open(IO_WriteOnly)){
        warning("Cannot write to theme settings!");
        return;
    }
    int input = src.getch();
    while(input != -1){
        dest.putch(input);
        input = src.getch();
    }
    src.close();
    dest.close();

    // handle std color scheme
    KConfig inConfig(file, true, false);
    KConfig *globalConfig = KGlobal::config();
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
        if(!pbDuplicate[i] && pbPixmaps[i])
            delete pbPixmaps[i];
        if(colors[i])
            delete(colors[i]);
        if(grLowColors[i])
            delete(grLowColors[i]);
        if(grHighColors[i])
            delete(grHighColors[i]);
    }
    delete cache;
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
    KThemePixmap *pixmap = new KThemePixmap(false);
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
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::FullScale,
                                  widget);
                else
                    warning("We would have inserted a null pixmap!");
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
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::HorizontalScale, widget);
                else
                    warning("We would have inserted a null pixmap!");
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
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::VerticalScale, widget);
                else
                    warning("We would have inserted a null pixmap!");
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
    else if(gradients[widget] == GrReverseBevel){
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
                KPixmap s;
                s.resize(w, h);
                KPixmapEffect::gradient(*pixmaps[widget],
                                        *grHighColors[widget],
                                        *grLowColors[widget],
                                        KPixmapEffect::DiagonalGradient);
                KPixmapEffect::gradient(s,
                                        grLowColors[widget]->
                                        dark(bevelContrast(widget)),
                                        grHighColors[widget]->
                                        light(bevelContrast(widget)),
                                        KPixmapEffect::DiagonalGradient);
                pixmaps[widget]->setSecondary(s);
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

KThemePixmap::KThemePixmap(bool timer)
    : KPixmap()
{
    s = NULL;
    if(timer){
        t = new QTime;
        t->start();
    }
    else
        t = NULL;
    int i;
    for(i=0; i < 8; ++i)
        b[i] = NULL;
}

KThemePixmap::~KThemePixmap()
{
    if(s)
        delete s;
    if(t)
        delete t;
    int i;
    for(i=0; i < 8; ++i)
        if(b[i])
            delete b[i];
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
        if(it.current()->isOld())
            cache.remove(it.currentKey());
    }
}

KThemePixmap* KThemeCache::pixmap(int w, int h, int widgetID)
{

    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = h;
    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
    return(pix);
}

KThemePixmap* KThemeCache::horizontalPixmap(int w, int widgetID)
{
    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = 0;
    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
    return(pix);
}

KThemePixmap* KThemeCache::verticalPixmap(int h, int widgetID)
{
    kthemeKey key;
    key.data.id = widgetID;
    key.data.width = 0;
    key.data.height = h;
    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
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

    if(cache.find((unsigned long)key.cacheKey, true) != NULL){
        return(true); // a pixmap of this scale is already in there
    }
    return(cache.insert((unsigned long)key.cacheKey, pixmap,
                        pixmap->width()*pixmap->height()*pixmap->depth()/8));
}

#include "kthemebase.moc"
