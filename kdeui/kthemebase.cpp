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

static char *widgetEntries[] = {"HorizScrollGroove", "VertScrollGroove",
"Slider", "SliderGroove", "IndicatorOn", "IndicatorOff", "Background",
"PushButton", "ExIndicatorOn", "ExIndicatorOff", "ComboBox", "ScrollBarSlider",
"Bevel", "ToolButton", "ScrollBarButton", "BarHandle", "ToolBar",
"ScrollBarDeco", "ComboDeco"};

void KThemeBase::readConfig(Qt::GUIStyle style)
{
    int i;
    QString tmpStr;
    warning(i18n("KThemeStyle: Reading theme settings."));
    // Read in the scale hints
    config->setGroup("Scale");
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
                    warning(i18n("KThemeStyle: Unrecognized scale option %s, using Tile."),
                            (const char*)tmpStr);
                scaleHints[i] = TileScale;
            }
    }
    // Read in gradient types
    config->setGroup("Gradients");
    for(i=0; i < WIDGETS; ++i){
        tmpStr = config->readEntry(widgetEntries[i]);
        if(tmpStr == "Diagonal")
            gradients[i] = Diagonal;
        else if(tmpStr == "Horizontal")
            gradients[i] =Horizontal;
        else if(tmpStr == "Vertical")
            gradients[i] = Vertical;
        else{
            if(tmpStr != "None" && !tmpStr.isEmpty())
                warning(i18n("KThemeStyle: Unrecognized gradient option %s, using None."),
                        (const char*)tmpStr);
            gradients[i] = None;
        }
    }
    // Read in gradient low colors
    config->setGroup("Gradient Lowcolor");
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != None){
            grLowColors[i] =
                new QColor(config->readColorEntry(widgetEntries[i],
                                                  &kapp->palette()->normal().
                                                  background()));
        }
        else
            grLowColors[i] = NULL;
    }
    // Read in gradient high colors
    config->setGroup("Gradient Highcolor");
    for(i=0; i < WIDGETS; ++i){
        if(gradients[i] != None){
            grHighColors[i] =
                new QColor(config->readColorEntry(widgetEntries[i],
                                                  &kapp->palette()->normal().
                                                  background()));
        }
        else
            grHighColors[i] = NULL;
    }
    // Read in the extended color attributes
    QColor bg[WIDGETS]; // We need to store these for a sec
    config->setGroup("Extended Background");
    for(i=0; i < WIDGETS; ++i){
        if(config->hasKey(widgetEntries[i]))
            bg[i] = config->readColorEntry(widgetEntries[i], &bg[i]);
    }
    // Combine fg and stored bg colors into a color group
    config->setGroup("Extended Foreground");
    QColor fg;
    for(i=0; i < WIDGETS; ++i){
        if(config->hasKey(widgetEntries[i]) || bg[i].isValid()){
            fg = kapp->palette()->normal().foreground();
            fg = config->readColorEntry(widgetEntries[i], &fg);
            colors[i] = makeColorGroup(fg, bg[i], style);
        }
        else
            colors[i] = NULL;
    }
    // Read in border widths
    config->setGroup("Borders");
    for(i=0; i < WIDGETS; ++i)
        borders[i] = config->readNumEntry(widgetEntries[i], 1);
    // Read in highlight widths
    config->setGroup("Highlights");
    for(i=0; i < WIDGETS; ++i)
        highlights[i] = config->readNumEntry(widgetEntries[i], 1);
    // Read in pixmaps
    config->setGroup("Pixmaps");
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
                            (const char *)pixnames[i]);
                    break;
                }
            }
            // load pixmap
            if(!duplicate[i]){
                if(tmpStr.isEmpty()){
                    warning(i18n("KThemeStyle: No pixmap specified for %s."),
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
    tmpStr = config->readEntry("SButtonType");
    if(tmpStr == "BottomLeft")
        sbPlacement = SBBottomLeft;
    else if(tmpStr == "BottomRight")
        sbPlacement = SBBottomRight;
    else{
        if(tmpStr != "Opposite" && !tmpStr.isEmpty())
            warning(i18n("KThemeStyle: Unrecognized sb button option %s, using Opposite."),
                    (const char*)tmpStr);
        sbPlacement = SBOpposite;
    }
    tmpStr = config->readEntry("ArrowType");
    if(tmpStr == "Small")
        arrowStyle = SmallArrow;
    else if(tmpStr == "3D")
        arrowStyle = MotifArrow;
    else{
        if(tmpStr != "Normal" && !tmpStr.isEmpty())
            warning(i18n("KThemeStyle: Unrecognized arrow option %s, using Windows."),
                    (const char*)tmpStr);
        arrowStyle = LargeArrow;
    }
    tmpStr = config->readEntry("ShadeStyle");
    if(tmpStr == "Motif")
        shading = Motif;
    else if(tmpStr == "Next")
        shading = Next;
    else
        shading = Windows;
    smallGroove = config->readBoolEntry("SmallSliderGroove", false);
    roundedButton = config->readBoolEntry("RoundButton", false);
    roundedCombo = config->readBoolEntry("RoundCombo", false);
    roundedSlider = config->readBoolEntry("RoundSlider", false);
    defaultFrame = config->readNumEntry("FrameWidth", 2);
    btnXShift = config->readNumEntry("ButtonXShift", 0);
    btnYShift = config->readNumEntry("ButtonYShift", 0);
    sliderLen = config->readNumEntry("SliderLength", 30);
    cacheSize = config->readNumEntry("CacheSize", 1024);
    
#ifdef KSTYLE_DEBUG
    for(existing=0, i=0; i < WIDGETS; ++i)
        if(images[i] || pixmaps[i])
            ++existing;
    warning("KThemeStyle: %d out of %d pixmaps allocated", existing, WIDGETS);
#endif
    warning(i18n("KThemeStyle: Finished reading theme settings."));

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

QColorGroup* KThemeBase::makeColorGroup(QColor &fg, QColor &bg,
                                        Qt::GUIStyle style)
{
    if(style == Qt::MotifStyle){
        int highlightVal, lowlightVal;
        highlightVal=100+(2*kapp->contrast()+4)*16/10;
        lowlightVal=100+(2*kapp->contrast()+4)*10;
        return(new QColorGroup(fg, bg, bg.light(highlightVal),
                               bg.dark(lowlightVal), bg.dark(120),
                               fg, kapp->palette()->normal().base()));
    }else
        return(new QColorGroup( fg, bg, bg.light(150), bg.dark(),
                                bg.dark(120), fg,
                                kapp->palette()->normal().base()));
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
    warning(i18n("KThemeStyle: Unable to load image %s."), name.ascii());
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
    warning(i18n("KThemeStyle: Unable to load pixmap %s."), name.ascii());
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
    if(gradients[widget] == Vertical){
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
    else if(gradients[widget] == Horizontal){
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
    else if(gradients[widget] == Diagonal){
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
