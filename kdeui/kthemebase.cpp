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
#include <kapp.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kglobal.h>

static char *widgetEntries[] = {"HorizScrollGroove", "VertScrollGroove",
"Slider", "SliderGroove", "IndicatorOn", "IndicatorOff", "Background",
"PushButton", "ExIndicatorOn", "ExIndicatorOff", "ComboBox", "ScrollBarSlider",
"Bevel", "ToolButton", "ScrollBarButton", "BarHandle", "ToolBar",
"ScrollBarDeco", "ComboDeco", "Splitter", "CheckMark", "MenuItemOn",
"MenuItemOff"};

// Used only internally for handling non-widget option keys. These are needed
// because they get iterated through in a couple of places and it allows the
// use of small for loops.

#define OPTIONS 16

static char *optionEntries[]={"SButtonType", "ArrowType", "ComboDeco",
"ShadeStyle", "RoundButton", "RoundCombo", "RoundSlider", "FrameWidth",
"ButtonXShift", "ButtonYShift", "SliderLength", "SplitterHandle", "Name",
"Description", "CacheSize", "SmallSliderGroove"};

enum OptionLabel{OptSButtonType=0, OptArrowType, OptComboDeco, OptShadeStyle,
OptRoundButton, OptRoundCombo, OptRoundSlider, OptFrameWidth, OptButtonXShift,
OptButtonYShift, OptSliderLength, OptSplitterHandle, OptName, OptDescription,
OptCacheSize, OptSmallGroove};

#define WGROUPS 9

static char *wGroupEntries[]={"Scale", "Gradients", "Gradient Lowcolor",
"Gradient Highcolor", "Extended Background", "Extended Foreground", "Borders",
"Highlights", "Pixmaps"};

enum WGroupLabel{WScale=0, WGradients, WGradientLow, WGradientHigh,
WExtBackground, WExtForeground, WBorders, WHighlights, WPixmaps};

void KThemeBase::readConfig(Qt::GUIStyle style)
{
    int i;
    QString tmpStr;
    warning("KThemeStyle: Reading theme settings.");
    // Read in the scale hints
    config->setGroup(wGroupEntries[WScale]);
    for(i=0; i < WIDGETS; ++i){
            tmpStr = config->readEntry(widgetEntries[i]);
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
    config->setGroup(wGroupEntries[WGradients]);
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config->readEntry(widgetEntries[i]);
        if(tmpStr == "Diagonal")
            gradients[i] = GrDiagonal;
        else if(tmpStr == "Horizontal")
            gradients[i] = GrHorizontal;
        else if(tmpStr == "Vertical")
            gradients[i] = GrVertical;
        else{
            if(tmpStr != "None" && !tmpStr.isEmpty())
                warning("KThemeStyle: Unrecognized gradient option %s, using None.",
                        tmpStr.ascii());
            gradients[i] = GrNone;
        }
    }
    // Read in gradient low colors
    config->setGroup(wGroupEntries[WGradientLow]);
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != GrNone){
            grLowColors[i] =
                new QColor(config->readColorEntry(widgetEntries[i],
                                                  &kapp->palette().normal().
                                                  background()));
        }
        else
            grLowColors[i] = NULL;
    }
    // Read in gradient high colors
    config->setGroup(wGroupEntries[WGradientHigh]);
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != GrNone){
            grHighColors[i] =
                new QColor(config->readColorEntry(widgetEntries[i],
                                                  &kapp->palette().normal().
                                                  background()));
        }
        else
            grHighColors[i] = NULL;
    }
    // Read in the extended color attributes
    QColor bg[WIDGETS]; // We need to store these for a sec
    config->setGroup(wGroupEntries[WExtBackground]);
    for(i=0; i < WIDGETS; ++i){
        if(config->hasKey(widgetEntries[i]))
            bg[i] = config->readColorEntry(widgetEntries[i], &bg[i]);
    }
    // Combine fg and stored bg colors into a color group
    config->setGroup(wGroupEntries[WExtForeground]);
    QColor fg;
    for(i=0; i < WIDGETS; ++i){
        if(config->hasKey(widgetEntries[i]) || bg[i].isValid()){
            fg = kapp->palette().normal().foreground();
            fg = config->readColorEntry(widgetEntries[i], &fg);
            colors[i] = makeColorGroup(fg, bg[i], style);
        }
        else
            colors[i] = NULL;
    }
    // Read in border widths
    config->setGroup(wGroupEntries[WBorders]);
    for(i=0; i < WIDGETS; ++i)
        borders[i] = config->readNumEntry(widgetEntries[i], 1);
    // Read in highlight widths
    config->setGroup(wGroupEntries[WHighlights]);
    for(i=0; i < WIDGETS; ++i)
        highlights[i] = config->readNumEntry(widgetEntries[i], 1);
    // Read in pixmaps
    config->setGroup(wGroupEntries[WPixmaps]);
    int existing;
    QString pixnames[WIDGETS];
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config->readEntry(widgetEntries[i], "");
        pixnames[i] = tmpStr;
        duplicate[i] = false;
        pixmaps[i] = NULL;
        images[i] = NULL;
        if(!tmpStr.isEmpty()){
            //scan for duplicate pixmaps
            for(existing=0; existing < i; ++existing){
                if(tmpStr == pixnames[existing] && scaleHints[i] ==
                   TileScale && scaleHints[existing] == TileScale){
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
                    if(scaleHints[i] == TileScale){
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
    // Read in misc settings
    config->setGroup("Misc");
    tmpStr = config->readEntry(optionEntries[OptSButtonType]);
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
    tmpStr = config->readEntry(optionEntries[OptArrowType]);
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
    tmpStr = config->readEntry(optionEntries[OptShadeStyle]);
    if(tmpStr == "Motif")
        shading = Motif;
    else if(tmpStr == "Next")
        shading = Next;
    else
        shading = Windows;
    smallGroove = config->
        readBoolEntry(optionEntries[OptSmallGroove], false);
    roundedButton = config->
        readBoolEntry(optionEntries[OptRoundButton], false);
    roundedCombo = config->
        readBoolEntry(optionEntries[OptRoundCombo], false);
    roundedSlider = config->
        readBoolEntry(optionEntries[OptRoundSlider], false);
    defaultFrame = config->
        readNumEntry(optionEntries[OptFrameWidth], 2);
    btnXShift = config->
        readNumEntry(optionEntries[OptButtonXShift], 0);
    btnYShift = config->
        readNumEntry(optionEntries[OptButtonYShift], 0);
    sliderLen = config->
        readNumEntry(optionEntries[OptSliderLength], 30);
    splitterWidth = config->
        readNumEntry(optionEntries[OptSplitterHandle], 10);
    cacheSize = config->
        readNumEntry(optionEntries[OptCacheSize], 1024);
    
#ifdef KSTYLE_DEBUG
    for(existing=0, i=0; i < WIDGETS; ++i)
        if(images[i] || pixmaps[i])
            ++existing;
    warning("KThemeStyle: %d out of %d pixmaps allocated", existing, WIDGETS);
#endif
    warning("KThemeStyle: Finished reading theme settings.");

}

KThemeBase::KThemeBase(const QString &configFile)
    :KStyle()
{
    if(configFile == QString::null)
        config = kapp->getConfig();
    else
        config = new KConfig(configFile, configFile);
    localDir = kapp->localkdedir()+"/share/apps/kstyle/pixmaps/";
    globalDir = kapp->kde_datadir()+"/kstyle/pixmaps/";
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

void KThemeBase::writeConfig(KConfigBase &inConfig, KConfigBase &outConfig)
{
    int group, widget;

    for(group=0; group < WGROUPS; ++group){
        outConfig.setGroup(wGroupEntries[group]);
        inConfig.setGroup(wGroupEntries[group]);
        for(widget=0; widget < WIDGETS; ++widget)
            outConfig.writeEntry(widgetEntries[widget],
                                 inConfig.readEntry(widgetEntries[widget],
                                                    " "), true, true);
    }
    inConfig.setGroup("Misc");
    outConfig.setGroup("Misc");
    for(widget=0; widget < OPTIONS; ++widget){
        outConfig.writeEntry(optionEntries[widget],
                             inConfig.readEntry(optionEntries[widget], " "),
                             true, true);
    }
    // Read in standard color scheme. This is kind of messed up because it
    // conflicts with colorscm... But themes do need to be able to specify
    // colors. (mosfet)
    outConfig.setGroup("General");
    inConfig.setGroup("General");
    if(inConfig.hasKey("foreground"))
        outConfig.writeEntry("foreground",
                             inConfig.readEntry("foreground", " "),
                             true, true);
    if(inConfig.hasKey("background"))
        outConfig.writeEntry("background",
                             inConfig.readEntry("background", " "),
                             true, true);
    if(inConfig.hasKey("selectForeground"))
        outConfig.writeEntry("selectForeground",
                             inConfig.readEntry("selectForeground", " "),
                             true, true);
    if(inConfig.hasKey("selectBackground"))
        outConfig.writeEntry("selectBackground",
                             inConfig.readEntry("selectBackground", " "),
                             true, true);
    if(inConfig.hasKey("windowForeground"))
        outConfig.writeEntry("windowForeground",
                             inConfig.readEntry("windowForeground", " "),
                             true, true);
    if(inConfig.hasKey("windowBackground"))
        outConfig.writeEntry("windowBackground",
                             inConfig.readEntry("windowBackground", " "),
                             true, true);
    outConfig.setGroup("KDE");
    inConfig.setGroup("KDE");

    if(inConfig.hasKey("Contrast"))
        outConfig.writeEntry("Contrast",
                             inConfig.readEntry("Contrast", " "), true, true);
    outConfig.writeEntry("widgetStyle",
                         inConfig.readEntry("widgetStyle", " "), true,
                         true);
    outConfig.sync();
}

void KThemeBase::applyConfigFile(const QString &file)
{
    KSimpleConfig inConfig(file, true);
    KConfig outConfig;
    writeConfig(inConfig, outConfig);
}

void KThemeBase::writeConfigFile(const QString &file)
{
    KConfig inConfig;
    KSimpleConfig outConfig(file);
    writeConfig(inConfig, outConfig);
}

// This should be merged with writeConfig if the deleteEntry stuff is moved
// to KConfigBase
void KThemeBase::compactConfigFile(const QString &file)
{
    int group, widget;
    KSimpleConfig config(file, false);

    warning("Using file %s", file.latin1());
    for(group=0; group < WGROUPS; ++group){
        config.setGroup(wGroupEntries[group]);
        for(widget=0; widget < WIDGETS; ++widget)
            if(!config.hasKey(widgetEntries[widget])){
                warning("Deleting key [%s] %s", wGroupEntries[group],
                        widgetEntries[widget]); 
		// Do something here ;-)
            }
    }
    config.setGroup("Misc");
    for(widget=0; widget < OPTIONS; ++widget){
        if(!config.hasKey(optionEntries[widget]))
           config.deleteEntry(optionEntries[widget], false);
    }
}

QColorGroup* KThemeBase::makeColorGroup(QColor &fg, QColor &bg,
                                        Qt::GUIStyle style)
{
    if(style == Qt::MotifStyle){
        int highlightVal, lowlightVal;
        highlightVal=100+(2*kapp->contrast()+4)*16/10;
        lowlightVal=100+(2*kapp->contrast()+4)*10;
        return(new QColorGroup(fg, bg, bg.light(highlightVal),
                               bg.dark(lowlightVal), bg.dark(120),
                               fg, kapp->palette().normal().base()));
    }else
        return(new QColorGroup( fg, bg, bg.light(150), bg.dark(),
                                bg.dark(120), fg,
                                kapp->palette().normal().base()));
}

QImage* KThemeBase::loadImage(QString &name)
{
    QImage *image = new QImage;
    image->load(localDir+name);
    if(!image->isNull())
        return(image);
    image->load(globalDir+name);
    if(!image->isNull())
        return(image);
    warning("KThemeStyle: Unable to load image %s.", name.ascii());
    delete image;
    return(NULL);
}
 
KPixmap* KThemeBase::loadPixmap(QString &name)
{
    KPixmap *pixmap = new KPixmap;
    pixmap->load(localDir+name);
    if(!pixmap->isNull())
        return(pixmap);
    pixmap->load(globalDir+name);
    if(!pixmap->isNull())
        return(pixmap);
    warning("KThemeStyle: Unable to load pixmap %s.", name.ascii());
    delete pixmap;
    return(NULL);
}

KPixmap* KThemeBase::scale(int w, int h, WidgetType widget)
{
    if(scaleHints[widget] == FullScale){
        if(!pixmaps[widget] || pixmaps[widget]->width() != w ||
           pixmaps[widget]->height() != h){
            KPixmap *cachePix = cache->pixmap(w, h, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::FullScale, widget);
                QImage tmpImg = images[widget]->smoothScale(w, h);
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
            }
        }
    }
    else if(scaleHints[widget] == HorizontalScale){
        if(pixmaps[widget]->width() != w){
            KPixmap *cachePix = cache->horizontalPixmap(w, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::HorizontalScale, widget);
                QImage tmpImg = images[widget]->
                    smoothScale(w, images[widget]->height());
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
            }
        }
    }
    else if(scaleHints[widget] == VerticalScale){
        if(pixmaps[widget]->height() != h){
            KPixmap *cachePix = cache->verticalPixmap(w, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                cache->insert(pixmaps[widget], KThemeCache::VerticalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                cache->insert(pixmaps[widget], KThemeCache::VerticalScale, widget);
                QImage tmpImg =
                    images[widget]->smoothScale(images[widget]->width(), h);
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->convertFromImage(tmpImg);
            }
        }
    }
    return(pixmaps[widget]);
}

KPixmap* KThemeBase::gradient(int w, int h, WidgetType widget)
{
    if(gradients[widget] == GrVertical){
        if(!pixmaps[widget] || pixmaps[widget]->height() != h){
            KPixmap *cachePix = cache->verticalPixmap(h, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::VerticalScale,
                                  widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::VerticalScale,
                                  widget);
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->resize(w, h);
                pixmaps[widget]->gradientFill(*grLowColors[widget],
                                              *grHighColors[widget], true);
            }
        }
    }
    else if(gradients[widget] == GrHorizontal){
        if(!pixmaps[widget] || pixmaps[widget]->width() != w){
            KPixmap *cachePix = cache->horizontalPixmap(w, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget],
                                  KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget],
                                  KThemeCache::HorizontalScale, widget);
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->resize(w, h);
                pixmaps[widget]->gradientFill(*grHighColors[widget],
                                              *grLowColors[widget],
                                              false);
            }
        }
    }
    else if(gradients[widget] == GrDiagonal){
        warning("Diagonal gradients not supported yet!");
        if(!pixmaps[widget] || pixmaps[widget]->width() != w ||
           pixmaps[widget]->height() != h){
            KPixmap *cachePix = cache->pixmap(w, h, widget);
            if(cachePix){
                cachePix = new KPixmap(*cachePix);
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::FullScale,
                                  widget);
                pixmaps[widget] = cachePix;
            }
            else{
                if(pixmaps[widget])
                    cache->insert(pixmaps[widget], KThemeCache::FullScale,
                                  widget);
                pixmaps[widget] = new KPixmap;
                pixmaps[widget]->resize(w, h);
                pixmaps[widget]->gradientFill(*grLowColors[widget],
                                              *grHighColors[widget],
                                              false);
            }
        }
    }
    return(pixmaps[widget]);
}

KPixmap* KThemeBase::scalePixmap(int w, int h, WidgetType widget)
{

    if(gradients[widget])
        return(gradient(w, h, widget));
    return(scale(w, h, widget));
}


KThemeCache::KThemeCache(int maxSize)
{
    cache.setMaxCost(maxSize*1024);
}

bool KThemeCache::insert(KPixmap *pixmap, ScaleHint scale, int widgetID)
{
    if(scale == FullScale || scale == HorizontalScale)
        widgetID |= pixmap->width() << 6;
    if(scale == FullScale || scale == VerticalScale)
        widgetID |= pixmap->height() << 19;

    if(cache.find(widgetID), false)
        return(true); // a pixmap of this scale is already in there
    return(cache.insert(widgetID, pixmap,
                        pixmap->width()*pixmap->height()*pixmap->depth()/8));
}

#include "kthemebase.moc"
