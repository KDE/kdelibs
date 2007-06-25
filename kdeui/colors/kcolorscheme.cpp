/* This file is part of the KDE project
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kcolorscheme.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kglobalsettings.h>
#include <kcolorutils.h>

#include <QtGui/QColor>
#include <QtGui/QBrush>

typedef struct {
    int NormalBackground[3];
    int AlternateBackground[3];
    int NormalText[3];
    int InactiveText[3];
} DefaultColors;

DefaultColors defaultViewColors = {
    { 255, 255, 255 },
    { 241, 241, 239 },
    {   0,   0,   0 },
    { 152, 152, 152 }
};

DefaultColors defaultWindowColors = {
    { 241, 241, 241 },
    { 227, 227, 227 },
    {   0,   0,   0 },
    { 152, 152, 152 }
};

DefaultColors defaultButtonColors = {
    { 218, 221, 215 },
    { 218, 221, 215 },
    {   0,   0,   0 },
    { 152, 154, 149 }
};

DefaultColors defaultSelectionColors = {
    {  60, 131, 208 },
    {  60, 131, 208 },
    { 255, 255, 255 },
    { 177, 202, 232 }
};

DefaultColors defaultTooltipColors = {
    { 255, 248, 209 },
    { 255, 248, 209 },
    {   0,   0,   0 },
    { 232, 185, 149 }
};

//BEGIN KColorSchemePrivate
class KColorSchemePrivate
{
public:
    explicit KColorSchemePrivate(const KSharedConfigPtr&, const char*, DefaultColors);
    KColorSchemePrivate(const KColorSchemePrivate&);

    QColor background(KColorScheme::BackgroundRole);
    QColor foreground(KColorScheme::ForegroundRole);
    QColor decoration(KColorScheme::DecorationRole);
    qreal contrast();
private:
    KConfigGroup _config;
    DefaultColors _defaults;
    int _contrast;
};

KColorSchemePrivate::KColorSchemePrivate(const KSharedConfigPtr &config, const char *group, DefaultColors defaults)
    : _config( config, group ), _defaults( defaults )
{
    KConfigGroup g( config, "KDE" );
    _contrast = g.readEntry( "contrast", 7 );
}

KColorSchemePrivate::KColorSchemePrivate(const KColorSchemePrivate& other)
    : _config( other._config ), _defaults( other._defaults), _contrast( other._contrast )
{
}

#define DEFAULT(a) QColor( _defaults.a[0], _defaults.a[1], _defaults.a[2] )

QColor KColorSchemePrivate::background(KColorScheme::BackgroundRole role)
{
    switch (role) {
        case KColorScheme::AlternateBackground:
            return _config.readEntry( "BackgroundAlternate", DEFAULT(AlternateBackground) );
        default:
            return _config.readEntry( "BackgroundNormal", DEFAULT(NormalBackground) );
    }
}

QColor KColorSchemePrivate::foreground(KColorScheme::ForegroundRole role)
{
    switch (role) {
        case KColorScheme::InactiveText:
            return _config.readEntry( "ForegroundInactive", DEFAULT(InactiveText) );
        case KColorScheme::ActiveText:
            return _config.readEntry( "ForegroundActive", QColor(255,0,0) );
        case KColorScheme::LinkText:
            return _config.readEntry( "ForegroundLink", QColor(0,0,255) );
        case KColorScheme::VisitedText:
            return _config.readEntry( "ForegroundVisited", QColor(88,55,150) );
        case KColorScheme::NegativeText:
            return _config.readEntry( "ForegroundNegative", QColor(107,0,0) );
        case KColorScheme::NeutralText:
            return _config.readEntry( "ForegroundNeutral", QColor(0,90,95) );
        case KColorScheme::PositiveText:
            return _config.readEntry( "ForegroundPositive", QColor(0,95,0) );
        default:
            return _config.readEntry( "ForegroundNormal", DEFAULT(NormalText) );
    }
}

QColor KColorSchemePrivate::decoration(KColorScheme::DecorationRole role)
{
    switch (role) {
        case KColorScheme::FocusColor:
            return _config.readEntry( "DecorationFocus", QColor(239,132,65) );
        default:
            return _config.readEntry( "DecorationHover", QColor(72,177,60) );
    }
}

qreal KColorSchemePrivate::contrast()
{
    return 0.1 * (qreal)_contrast;
}
//END KColorSchemePrivate

KColorScheme::KColorScheme(const KColorScheme &other)
{
    d = new KColorSchemePrivate(*other.d);
}

KColorScheme& KColorScheme::operator=(const KColorScheme& other)
{
    if (this != &other) {
        delete d;
        d = new KColorSchemePrivate(*other.d);
    }
    return *this;
}

KColorScheme::~KColorScheme()
{
    delete d;
}

KColorScheme::KColorScheme(ColorSet set, KSharedConfigPtr config)
{
    if (!config) {
        config = KGlobal::config();
    }
    switch (set) {
        case Window:
            d = new KColorSchemePrivate(config, "Colors:Window", defaultWindowColors);
            break;
        case Button:
            d = new KColorSchemePrivate(config, "Colors:Button", defaultButtonColors);
            break;
        case Selection:
            d = new KColorSchemePrivate(config, "Colors:Selection", defaultSelectionColors);
            break;
        case Tooltip:
            d = new KColorSchemePrivate(config, "Colors:Tooltip", defaultTooltipColors);
            break;
        default:
            d = new KColorSchemePrivate(config, "Colors:View", defaultViewColors);
    }
}

QBrush KColorScheme::background(BackgroundRole role) const
{
    return QBrush( d->background( role ) );
}

QBrush KColorScheme::foreground(ForegroundRole role) const
{
    return d->foreground(role);
}

QBrush KColorScheme::decoration(DecorationRole role) const
{
    return d->decoration(role);
}

QColor KColorScheme::shade(ShadeRole role) const
{
    return shade(background().color(), role, d->contrast());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role)
{
    return shade(color, role, KGlobalSettings::contrastF());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role, qreal contrast, qreal chromaAdjust)
{
    // nan -> 1.0
    contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
    qreal y = KColorUtils::luma(color), yi = 1.0 - y;

    // handle very dark colors (base, mid, dark, shadow == midlight, light)
    if (y < 0.006) {
        switch (role) {
            case KColorScheme::LightShade:
                return KColorUtils::shade(color, 0.05 + 0.95 * contrast, chromaAdjust);
            case KColorScheme::MidShade:
                return KColorUtils::shade(color, 0.01 + 0.20 * contrast, chromaAdjust);
            case KColorScheme::DarkShade:
                return KColorUtils::shade(color, 0.02 + 0.40 * contrast, chromaAdjust);
            default:
                return KColorUtils::shade(color, 0.03 + 0.60 * contrast, chromaAdjust);
        }
    }

    // handle very light colors (base, midlight, light == mid, dark, shadow)
    if (y > 0.93) {
        switch (role) {
            case KColorScheme::MidlightShade:
                return KColorUtils::shade(color, -0.02 - 0.20 * contrast, chromaAdjust);
            case KColorScheme::DarkShade:
                return KColorUtils::shade(color, -0.06 - 0.60 * contrast, chromaAdjust);
            case KColorScheme::ShadowShade:
                return KColorUtils::shade(color, -0.10 - 0.90 * contrast, chromaAdjust);
            default:
                return KColorUtils::shade(color, -0.04 - 0.40 * contrast, chromaAdjust);
        }
    }

    // handle everything else
    qreal lightAmount = (0.05 + y * 0.55) * (0.25 + contrast * 0.75);
    qreal darkAmount =  (     - y       ) * (0.55 + contrast * 0.35);
    switch (role) {
        case KColorScheme::LightShade:
            return KColorUtils::shade(color, lightAmount, chromaAdjust);
        case KColorScheme::MidlightShade:
            return KColorUtils::shade(color, (0.15 + 0.35 * yi) * lightAmount, chromaAdjust);
        case KColorScheme::MidShade:
            return KColorUtils::shade(color, (0.35 + 0.15 * y) * darkAmount, chromaAdjust);
        case KColorScheme::DarkShade:
            return KColorUtils::shade(color, darkAmount, chromaAdjust);
        default:
            return KColorUtils::darken(KColorUtils::shade(color, darkAmount, chromaAdjust), 0.5 + 0.3 * y);
    }
}
// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
