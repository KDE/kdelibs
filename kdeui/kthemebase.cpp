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
#include <kthemebase.h>
#include <kpixmapeffect.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qimage.h>
#include <stdlib.h>
#include <krootprop.h>

template class QIntCache<KThemePixmap>;

static const char * const widgetEntries[] = { // unsunken widgets (see header)
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
    unsigned int width       :12;
    unsigned int height      :12;
    unsigned int border      :1;
    unsigned int mask        :1;
};

union kthemeKey{
    kthemeKeyData data;
    unsigned int cacheKey;
};

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
        kdWarning() << "KThemeBase: Tried making border from empty pixmap" << endl;
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
    }

    if(sourceID == ActiveTab && destID == InactiveTab)
        aTabLine = iTabLine;
    else if(sourceID == InactiveTab && destID == ActiveTab)
        iTabLine = aTabLine;
}

void KThemeBase::readConfig(Qt::GUIStyle /*style*/)
{
#define PREBLEND_ITEMS 12
    static const WidgetType preBlend[]={Slider, IndicatorOn, IndicatorOff,
    ExIndicatorOn, ExIndicatorOff, HScrollDeco, VScrollDeco, HScrollDecoDown,
    VScrollDecoDown, ComboDeco, ComboDecoDown, CheckMark};

    int i;
    QString tmpStr;
    QString pixnames[WIDGETS]; // used for duplicate check
    QString brdnames[WIDGETS];
    bool loaded[WIDGETS]; // used for preloading for CopyWidget
    if (configFileName.isEmpty())
       configFileName="kstylerc";
    KConfig config(configFileName, true, false);

    // Are we initalized?
      for(i=0; i < INHERIT_ITEMS; ++i)
            applyResourceGroup(&config, i);
        for(; i < INHERIT_ITEMS*2; ++i){
            if(config.hasGroup(widgetEntries[i]))
                applyResourceGroup(&config, i);
#ifndef Q_WS_QWS //FIXME
            else{
                KRootProp copyProp(widgetEntries[i]);
                copyProp.writeEntry("CopyWidget",
                                    QString(widgetEntries[i-INHERIT_ITEMS]));
                copyProp.sync();
            }
#endif
        }
       for(; i < WIDGETS; ++i)
           applyResourceGroup(&config, i);
      applyMiscResourceGroup(&config);

    // initalize defaults that may not be read
    for(i=0; i < WIDGETS; ++i)
        loaded[i] = false;
    btnXShift = btnYShift = focus3DOffset = 0;
    aTabLine = iTabLine = true;
    roundedButton = roundedCombo = roundedSlider = focus3D = false;
    splitterWidth = 10;

    for(i=0; i < WIDGETS; ++i)
        readResourceGroup(i, pixnames, brdnames, loaded);

    // misc items
    readMiscResourceGroup();

    // Handle preblend items
    for(i=0; i < PREBLEND_ITEMS; ++i){
        if(pixmaps[preBlend[i]] != NULL && blends[preBlend[i]] != 0.0)
            blend(preBlend[i]);
    }
}

KThemeBase::KThemeBase(const QString & configFile)
    :KStyle(), configFileName(configFile)
{
    KGlobal::dirs()->addResourceType("kstyle_pixmap", KStandardDirs::kde_default("data") + "kstyle/pixmaps/");
    readConfig(Qt::WindowsStyle);
    cache = new KThemeCache(cacheSize);
}

void KThemeBase::applyConfigFile(const QString &file)
{
    // Do copy ourselves
    QFile src(file);
    QFile dest(locateLocal("config", "kstylerc"));

    if(!src.open(IO_ReadOnly)){
        kdWarning() << "Cannot open theme file for reading!" << endl;
        return;
    }
    if(!dest.open(IO_WriteOnly)){
        kdWarning() << "Cannot write to theme settings!" << endl;
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
    KConfigGroupSaver saver(globalConfig, "General");
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

    // Keep track of the current theme so that we can select the right one
    // in the KControl module.
    globalConfig->writeEntry("currentTheme", file, true, true);

    inConfig.setGroup("Misc");
    globalConfig->writeEntry("widgetStyleName",
                             inConfig.readEntry("Name", " "), true,
                             true);
    globalConfig->sync();

    // delete all properties;
#ifndef Q_WS_QWS //FIXME
    for(input=0; input < WIDGETS; ++input){
        KRootProp prop(widgetEntries[input]);
        prop.destroy();
    }
    KRootProp misc("Misc");
    misc.destroy();
#endif
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
    kdWarning() << "KThemeBase: Unable to load image " << name << endl;
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
    kdWarning() << "KThemeBase: Unable to load pixmap " << name << endl;
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
                    kdWarning() << "We would have inserted a null pixmap!" << endl;
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
                    kdWarning() << "We would have inserted a null pixmap!" << endl;
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
                    kdWarning() << "We would have inserted a null pixmap!" << endl;
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

KThemePixmap* KThemeBase::scaleBorder(int w, int h, WidgetType widget)
{
    KThemePixmap *pixmap = NULL;
    if(!pbPixmaps[widget] && !pbWidth[widget])
        return(NULL);
    pixmap = cache->pixmap(w, h, widget, true);
    if(pixmap){
        pixmap = new KThemePixmap(*pixmap);
    }
    else{
        pixmap = new KThemePixmap();
        pixmap->resize(w, h);
        QBitmap mask;
        mask.resize(w, h);
        mask.fill(color0);
        QPainter mPainter;
        mPainter.begin(&mask);

        QPixmap *tmp = borderPixmap(widget)->border(KThemePixmap::TopLeft);
        const QBitmap *srcMask = tmp->mask();
        int bdWidth = tmp->width();

        bitBlt(pixmap, 0, 0, tmp, 0, 0, bdWidth, bdWidth,
               Qt::CopyROP, false);
        if(srcMask)
            bitBlt(&mask, 0, 0, srcMask, 0, 0, bdWidth, bdWidth,
                   Qt::CopyROP, false);
        else
            mPainter.fillRect(0, 0, bdWidth, bdWidth, color1);


        tmp = borderPixmap(widget)->border(KThemePixmap::TopRight);
        srcMask = tmp->mask();
        bitBlt(pixmap, w-bdWidth, 0, tmp, 0, 0, bdWidth,
               bdWidth, Qt::CopyROP, false);
        if(srcMask)
            bitBlt(&mask, w-bdWidth, 0, srcMask, 0, 0, bdWidth,
                   bdWidth, Qt::CopyROP, false);
        else
            mPainter.fillRect(w-bdWidth, 0, bdWidth, bdWidth, color1);

        tmp = borderPixmap(widget)->border(KThemePixmap::BottomLeft);
        srcMask = tmp->mask();
        bitBlt(pixmap, 0, h-bdWidth, tmp, 0, 0, bdWidth,
               bdWidth, Qt::CopyROP, false);
        if(srcMask)
            bitBlt(&mask, 0, h-bdWidth, srcMask, 0, 0, bdWidth,
                   bdWidth, Qt::CopyROP, false);
        else
            mPainter.fillRect(0, h-bdWidth, bdWidth, bdWidth, color1);

        tmp = borderPixmap(widget)->border(KThemePixmap::BottomRight);
        srcMask = tmp->mask();
        bitBlt(pixmap, w-bdWidth, h-bdWidth, tmp, 0, 0,
               bdWidth, bdWidth, Qt::CopyROP, false);
        if(srcMask)
            bitBlt(&mask, w-bdWidth, h-bdWidth, srcMask, 0, 0,
                   bdWidth, bdWidth, Qt::CopyROP, false);
        else
            mPainter.fillRect(w-bdWidth, h-bdWidth, bdWidth, bdWidth, color1);

        QPainter p;
        p.begin(pixmap);
        if(w-bdWidth*2 > 0){
            tmp = borderPixmap(widget)->border(KThemePixmap::Top);
            srcMask = tmp->mask();
            p.drawTiledPixmap(bdWidth, 0, w-bdWidth*2, bdWidth, *tmp);
            if(srcMask)
                bitBlt(&mask, bdWidth, 0, srcMask, 0, 0,
                       w-bdWidth*2, bdWidth, Qt::CopyROP, false);
            else
                mPainter.fillRect(bdWidth, 0, w-bdWidth*2, bdWidth, color1);

            tmp = borderPixmap(widget)->border(KThemePixmap::Bottom);
            srcMask = tmp->mask();
            p.drawTiledPixmap(bdWidth, h-bdWidth, w-bdWidth*2, bdWidth,
                               *tmp);
            if(srcMask)
                bitBlt(&mask, bdWidth, h-bdWidth, srcMask, 0, 0,
                       w-bdWidth*2, bdWidth, Qt::CopyROP, false);
            else
                mPainter.fillRect(bdWidth, h-bdWidth, w-bdWidth*2, bdWidth,
                                  color1);
        }
        if(h-bdWidth*2 > 0){
            tmp = borderPixmap(widget)->border(KThemePixmap::Left);
            srcMask = tmp->mask();
            p.drawTiledPixmap(0, bdWidth, bdWidth, h-bdWidth*2, *tmp);
            if(srcMask)
                bitBlt(&mask, 0, bdWidth, srcMask, 0, 0,
                       bdWidth, h-bdWidth*2, Qt::CopyROP, false);
            else
                mPainter.fillRect(0, bdWidth, bdWidth, h-bdWidth*2, color1);

            tmp = borderPixmap(widget)->border(KThemePixmap::Right);
            srcMask = tmp->mask();
            p.drawTiledPixmap(w-bdWidth, bdWidth, bdWidth, h-bdWidth*2,
                               *tmp);
            if(srcMask)
                bitBlt(&mask, w-bdWidth, bdWidth, srcMask, 0, 0,
                       bdWidth, h-bdWidth*2, Qt::CopyROP, false);
            else
                mPainter.fillRect(w-bdWidth, bdWidth, bdWidth, h-bdWidth*2, color1);
        }
        p.end();
        mPainter.end();
        pixmap->setMask(mask);
        cache->insert(pixmap, KThemeCache::FullScale, widget, true);
        if(!pixmap->mask())
            kdWarning() << "No mask for border pixmap!" << endl;
    }
    return(pixmap);
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
                int offset = decoWidth(widget);
                s.resize(w-offset*2, h-offset*2);
                QColor lc(*grLowColors[widget]);
                QColor hc(*grHighColors[widget]);
                if(bevelContrast(widget)){
                    int bc = bevelContrast(widget);
                    // want single increments, not factors like light()/dark()
                    lc.setRgb(lc.red()-bc, lc.green()-bc, lc.blue()-bc);
                    hc.setRgb(hc.red()+bc, hc.green()+bc, hc.blue()+bc);
                }
                KPixmapEffect::gradient(*pixmaps[widget],
                                        lc, hc,
                                        KPixmapEffect::DiagonalGradient);
                KPixmapEffect::gradient(s, *grHighColors[widget],
                                        *grLowColors[widget],
                                        KPixmapEffect::DiagonalGradient);
                bitBlt(pixmaps[widget], offset, offset, &s, 0, 0, w-offset*2,
                       h-offset*2, Qt::CopyROP);
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
        highlightVal=100+(2*KGlobalSettings::contrast()+4)*16/10;
        lowlightVal=100+((2*KGlobalSettings::contrast()+4)*10);
        return(new QColorGroup(fg, bg, bg.light(highlightVal),
                               bg.dark(lowlightVal), bg.dark(120),
                               fg, kapp->palette().active().base()));
    }
    else
        return(new QColorGroup( fg, bg, bg.light(150), bg.dark(),
                                bg.dark(120), fg,
                                kapp->palette().active().base()));
}

void KThemeBase::applyMiscResourceGroup(KConfig *config)
{
#ifndef Q_WS_QWS //FIXME
    KRootProp *tmpProp = new KRootProp("Misc"); // clear the old property
    tmpProp->destroy();
    delete tmpProp;

    KRootProp prop("Misc");
    KConfigGroupSaver saver(config, "Misc");
    QString tmpStr;

    tmpStr = config->readEntry("SButtonPosition");
    if(tmpStr == "BottomLeft")
        prop.writeEntry("SButtonPosition", (int)SBBottomLeft);
    else if(tmpStr == "BottomRight")
        prop.writeEntry("SButtonPosition", (int)SBBottomRight);
    else{
        if(tmpStr != "Opposite" && !tmpStr.isEmpty())
            kdWarning() << "KThemeBase: Unrecognized sb button option " << tmpStr << ", using Opposite." << endl;
        prop.writeEntry("SButtonPosition", (int)SBOpposite);
    }
    tmpStr = config->readEntry("ArrowType");
    if(tmpStr == "Small")
        prop.writeEntry("ArrowType", (int)SmallArrow);
    else if(tmpStr == "3D")
        prop.writeEntry("ArrowType", (int)MotifArrow);
    else{
        if(tmpStr != "Normal" && !tmpStr.isEmpty())
            kdWarning() << "KThemeBase: Unrecognized arrow option " << tmpStr << ", using Normal." << endl;
        prop.writeEntry("ArrowType", (int)LargeArrow);
    }
    tmpStr = config->readEntry("ShadeStyle");
    if(tmpStr == "Motif")
        prop.writeEntry("ShadeStyle", (int)Motif);
    else if(tmpStr == "Next")
        prop.writeEntry("ShadeStyle", (int)Next);
    else if(tmpStr == "KDE")
        prop.writeEntry("ShadeStyle", (int)KDE);
    else
        prop.writeEntry("ShadeStyle", (int)Windows);

    prop.writeEntry("FrameWidth", config->readNumEntry("FrameWidth", 2));
    prop.writeEntry("Cache", config->readNumEntry("Cache", 1024));
    prop.writeEntry("ScrollBarExtent",
                    config->readNumEntry("ScrollBarExtent", 16));
#endif
}

void KThemeBase::readMiscResourceGroup()
{
#ifndef Q_WS_QWS //FIXME
    KRootProp prop("Misc");

    sbPlacement = (SButton)prop.readNumEntry("SButtonPosition",
                                             (int)SBOpposite);
    arrowStyle = (ArrowStyle)prop.readNumEntry("ArrowType",
                                              (int)LargeArrow);
    shading = (ShadeStyle)prop.readNumEntry("ShadeStyle", (int)Windows);
    defaultFrame = prop.readNumEntry("FrameWidth", 2);
    cacheSize = prop.readNumEntry("Cache", 1024);
    sbExtent = prop.readNumEntry("ScrollBarExtent", 16);
#endif
}

void KThemeBase::applyResourceGroup(KConfig *config, int i)
{
#ifndef Q_WS_QWS //FIXME
    QString tmpStr;
    int tmpVal;

    // clear the old property
    KRootProp *tmpProp = new KRootProp(widgetEntries[i]);
    tmpProp->destroy();
    delete tmpProp;

    KRootProp prop(widgetEntries[i]);
    KConfigGroupSaver saver(config, widgetEntries[i]);

    tmpStr = config->readEntry("CopyWidget", "");
    prop.writeEntry("CopyWidget", tmpStr);
    if(!tmpStr.isEmpty())
        return;

    tmpStr = config->readEntry("Scale");
    if(tmpStr == "Full")
        tmpVal = (int)FullScale;
    else if(tmpStr == "Horizontal")
        tmpVal = (int)HorizontalScale;
    else if(tmpStr == "Vertical")
        tmpVal = (int)VerticalScale;
    else{
        if(tmpStr != "Tile" && !tmpStr.isEmpty())
            kdWarning() << "KThemeBase: Unrecognized scale option " << tmpStr << ", using Tile." << endl;
        tmpVal = (int)TileScale;
    }
    prop.writeEntry("ScaleHint", tmpVal);

    // Gradient type
    tmpStr = config->readEntry("Gradient");
    if(tmpStr == "Diagonal")
        tmpVal = (int)GrDiagonal;
    else if(tmpStr == "Horizontal")
        tmpVal = (int)GrHorizontal;
    else if(tmpStr == "Vertical")
        tmpVal = (int)GrVertical;
    else if(tmpStr == "Pyramid")
        tmpVal = (int)GrPyramid;
    else if(tmpStr == "Rectangle")
        tmpVal = (int)GrRectangle;
    else if(tmpStr == "Elliptic")
        tmpVal = (int)GrElliptic;
    else if(tmpStr == "ReverseBevel")
        tmpVal = (int)GrReverseBevel;
    else{
        if(tmpStr != "None" && !tmpStr.isEmpty())
            kdWarning() << "KThemeBase: Unrecognized gradient option " << tmpStr << ", using None." << endl;
        tmpVal = (int)GrNone;
    }
    prop.writeEntry("Gradient", tmpVal);

    // Blend intensity
    tmpStr.setNum(config->readDoubleNumEntry("BlendIntensity", 0.0));
    prop.writeEntry("Blend", tmpStr);

    // Bevel contrast
    prop.writeEntry("BContrast", config->readNumEntry("BevelContrast", 0));

    // Border width
    prop.writeEntry("Border", config->readNumEntry("Border", 1));

    // Highlight width
    prop.writeEntry("Highlight", config->readNumEntry("Highlight", 1));

    // Gradient low color or blend background
    if(config->hasKey("GradientLow"))
        prop.writeEntry("GrLow", config->readColorEntry("GradientLow",
            &kapp->palette().active().background()));

    // Gradient high color
    if(config->hasKey("GradientHigh"))
        prop.writeEntry("GrHigh", config->readColorEntry("GradientHigh",
            &kapp->palette().active().foreground()));

    // Extended color attributes
    if(config->hasKey("Foreground") || config->hasKey("Background")){
        QColor fg, bg;
        if(config->hasKey("Background"))
            bg = config->readColorEntry("Background", &bg);
        if(config->hasKey("Foreground"))
            fg = config->readColorEntry("Foreground", &fg);
        prop.writeEntry("Foreground", fg);
        prop.writeEntry("Background", bg);

    }
    else
        colors[i] = NULL;

    // Pixmap
    tmpStr = config->readEntry("Pixmap", "");
    if(!tmpStr.isEmpty())
        prop.writeEntry("Pixmap", tmpStr);
    // Pixmap border
    tmpStr = config->readEntry("PixmapBorder", "");
    if(!tmpStr.isEmpty()){
        prop.writeEntry("PixmapBorder", tmpStr);
        prop.writeEntry("PixmapBWidth", config->
                        readNumEntry("PixmapBWidth", 0));
    }

    // Various widget specific settings. This was more efficent when bunched
        // together in the misc group, but this makes an easier to read config.
    if(i == SliderGroove)
        prop.writeEntry("SmallGroove",
                        config->readBoolEntry("SmallGroove", false));
    else if(i == ActiveTab || i == InactiveTab)
        prop.writeEntry("BottomLine",
                        config->readBoolEntry("BottomLine", true));
    else if(i == Splitter)
        prop.writeEntry("Width", config->readNumEntry("Width", 10));
    else if(i == ComboBox || i == ComboBoxDown){
        if(config->hasKey("Round"))
            prop.writeEntry("Round", config->readBoolEntry("Round", false));
        else
            prop.writeEntry("Round", 5000); // invalid, used w/multiple groups

    }
    else if (i == PushButton || i == PushButtonDown){
        if(config->hasKey("XShift"))
            prop.writeEntry("XShift", config->readNumEntry("XShift", 0));
        else
            prop.writeEntry("XShift", 5000);
        if(config->hasKey("YShift"))
            prop.writeEntry("YShift", config->readNumEntry("YShift", 0));
        else
            prop.writeEntry("YShift", 5000);
        if(config->hasKey("3DFocusRect"))
            prop.writeEntry("3DFRect", config->
                            readBoolEntry("3DFocusRect", false));
        else
            prop.writeEntry("3DFRect", 5000);
      if(config->hasKey("3DFocusOffset"))
          prop.writeEntry("3DFOffset", config->
                          readBoolEntry("3DFocusOffset", 0));
      else
          prop.writeEntry("3DFOffset", 5000);
      if(config->hasKey("Round"))
          prop.writeEntry("Round", config->readBoolEntry("Round", false));
      else
          prop.writeEntry("Round", 5000);
    }
#endif
}


void KThemeBase::readResourceGroup(int i, QString *pixnames, QString *brdnames,
                                   bool *loadArray)
{
#ifndef Q_WS_QWS //FIXME
    if(loadArray[i] == true){
        return; // already been preloaded.
    }

    int tmpVal;
    KRootProp prop(widgetEntries[i]);
    QString tmpStr;

    tmpStr = prop.readEntry(QString::fromLatin1("CopyWidget"));
    if(!tmpStr.isEmpty()){ // Duplicate another widget's config
        int sIndex;
        loadArray[i] = true;
        for(sIndex=0; sIndex < WIDGETS; ++sIndex){
            if(tmpStr == widgetEntries[sIndex]){
                if(!loadArray[sIndex]) // hasn't been loaded yet
                    readResourceGroup(sIndex, pixnames, brdnames,
                                      loadArray);
                break;
            }
        }
        if(loadArray[sIndex]){
            copyWidgetConfig(sIndex, i, pixnames, brdnames);
        }
        else
            kdWarning() << "KThemeBase: Unable to identify source widget for " << widgetEntries[i] << endl;
        return;
    }
    // special inheritance for disabled arrows (these are tri-state unlike
    // the rest of what we handle).
    for(tmpVal = DisArrowUp; tmpVal <= DisArrowRight; ++tmpVal){
        if(tmpVal == i){
            tmpStr = prop.readEntry(QString::fromLatin1("Pixmap"));
            if(tmpStr.isEmpty()){
                copyWidgetConfig(ArrowUp+(tmpVal-DisArrowUp), i, pixnames,
                                 brdnames);
                return;
            }
        }
    }

    // Scale hint
    scaleHints[i] = (ScaleHint)prop.readNumEntry(QString::fromLatin1("ScaleHint"), (int)TileScale);
    gradients[i] = (Gradient)prop.readNumEntry(QString::fromLatin1("Gradient"), (int)GrNone);

    // Blend intensity
    tmpStr = prop.readEntry(QString::fromLatin1("Blend"), QString::fromLatin1("0.0"));
    blends[i] = tmpStr.toFloat();

    // Bevel contrast
    bContrasts[i] = prop.readNumEntry(QString::fromLatin1("BContrast"), 0);

    // Border width
    borders[i] = prop.readNumEntry(QString::fromLatin1("Border"), 1);

    // Highlight width
    highlights[i] = prop.readNumEntry(QString::fromLatin1("Highlight"), 1);

    // Gradient low color or blend background
    if(gradients[i] != GrNone || blends[i] != 0.0)
        grLowColors[i] =
            new QColor(prop.readColorEntry(QString::fromLatin1("GrLow"),
                                           &kapp->palette().active().
                                           background()));
    else
        grLowColors[i] = NULL;

    // Gradient high color
    if(gradients[i] != GrNone)
        grHighColors[i] =
            new QColor(prop.readColorEntry(QString::fromLatin1("GrHigh"),
                                           &kapp->palette().active().
                                           background()));
    else
        grHighColors[i] = NULL;

    // Extended color attributes
    QColor fg, bg;
    fg = prop.readColorEntry(QString::fromLatin1("Foreground"), &fg);
    bg = prop.readColorEntry(QString::fromLatin1("Background"), &bg);
    if(fg.isValid() || bg.isValid()){
        if(!fg.isValid())
            fg = kapp->palette().active().foreground();
        if(!bg.isValid())
            bg = kapp->palette().active().background();
        colors[i] = makeColorGroup(fg, bg, Qt::WindowsStyle);
    }
    else
        colors[i] = NULL;

    // Pixmap
    int existing;
    tmpStr = prop.readEntry(QString::fromLatin1("Pixmap"));
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
    tmpStr = prop.readEntry(QString::fromLatin1("PixmapBorder"));
    brdnames[i] = tmpStr;
    pbDuplicate[i] = false;
    pbPixmaps[i] = NULL;
    pbWidth[i] = 0;
    if(!tmpStr.isEmpty()){
        pbWidth[i] = prop.readNumEntry(QString::fromLatin1("PixmapBWidth"), 0);
        if(pbWidth[i] == 0){
            kdWarning() << "KThemeBase: No border width specified for pixmapped border widget "
                    << widgetEntries[i] << endl;
            kdWarning() << "KThemeBase: Using default of 2." << endl;
            pbWidth[i] = 2;
        }
        // duplicate check
        for(existing=0; existing < i; ++existing){
            if(tmpStr == brdnames[existing]){
                pbPixmaps[i] = pbPixmaps[existing];
                pbDuplicate[i] = true;
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
        roundedSlider = prop.readNumEntry(QString::fromLatin1("SmallGroove"), false);
    else if(i == ActiveTab)
        aTabLine = prop.readNumEntry(QString::fromLatin1("BottomLine"), true);
    else if(i == InactiveTab)
        iTabLine = prop.readNumEntry(QString::fromLatin1("BottomLine"), true);
    else if(i == Splitter)
        splitterWidth = prop.readNumEntry(QString::fromLatin1("Width"), 10);
    else if(i == ComboBox || i == ComboBoxDown){
        tmpVal = prop.readNumEntry(QString::fromLatin1("Round"), 5000);
        if(tmpVal != 5000)
            roundedCombo = tmpVal;
    }
    else if (i == PushButton || i == PushButtonDown){
        tmpVal = prop.readNumEntry(QString::fromLatin1("XShift"), 0);
        if(tmpVal != 5000)
            btnXShift = tmpVal;
        tmpVal = prop.readNumEntry(QString::fromLatin1("YShift"), 0);
        if(tmpVal != 5000)
            btnYShift = tmpVal;
        tmpVal = prop.readNumEntry(QString::fromLatin1("3DFRect"), false);
        if(tmpVal != 5000)
            focus3D = tmpVal;
        tmpVal = prop.readNumEntry(QString::fromLatin1("3DFOffset"), 0);
        if(tmpVal != 5000)
            focus3DOffset = tmpVal;
        tmpVal = prop.readNumEntry(QString::fromLatin1("Round"), false);
        if(tmpVal != 5000)
            roundedButton = tmpVal;
    }
    loadArray[i] = true;
#endif
}


KThemePixmap::KThemePixmap(bool timer)
    : KPixmap()
{
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

KThemePixmap::KThemePixmap(const KThemePixmap &p)
    :KPixmap(p)
{
    if(p.t){
        t = new QTime;
        t->start();
    }
    else
        t = NULL;
    int i;
    for(i=0; i < 8; ++i)
        if(p.b[i])
            b[i] = new QPixmap(*p.b[i]);
        else
            b[i] = NULL;
}



KThemePixmap::~KThemePixmap()
{
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
    while(it.current()){
        if(it.current()->isOld())
            cache.remove(it.currentKey());
        else
            ++it;
    }
}

KThemePixmap* KThemeCache::pixmap(int w, int h, int widgetID, bool border,
                                  bool mask)
{

    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = h;
    key.data.border = border;
    key.data.mask = mask;

    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
    return(pix);
}

KThemePixmap* KThemeCache::horizontalPixmap(int w, int widgetID)
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = 0;
    key.data.border = false;
    key.data.mask = false;
    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
    return(pix);
}

KThemePixmap* KThemeCache::verticalPixmap(int h, int widgetID)
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = 0;
    key.data.height = h;
    key.data.border = false;
    key.data.mask = false;
    KThemePixmap *pix = cache.find((unsigned long)key.cacheKey);
    if(pix)
        pix->updateAccessed();
    return(pix);
}

bool KThemeCache::insert(KThemePixmap *pixmap, ScaleHint scale, int widgetID,
                         bool border, bool mask)
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = (scale == FullScale || scale == HorizontalScale) ?
        pixmap->width() : 0;
    key.data.height = (scale == FullScale || scale == VerticalScale) ?
        pixmap->height() : 0;
    key.data.border = border;
    key.data.mask = mask;

    if(cache.find((unsigned long)key.cacheKey, true) != NULL){
        return(true); // a pixmap of this scale is already in there
    }
    return(cache.insert((unsigned long)key.cacheKey, pixmap,
                        pixmap->width()*pixmap->height()*pixmap->depth()/8));
}

#include "kthemebase.moc"
