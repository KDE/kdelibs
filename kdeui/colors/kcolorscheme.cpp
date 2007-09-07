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
class KColorSchemePrivate : public QSharedData
{
public:
    explicit KColorSchemePrivate(const KSharedConfigPtr&, QPalette::ColorGroup, const char*, DefaultColors);

    QColor background(KColorScheme::BackgroundRole) const;
    QColor foreground(KColorScheme::ForegroundRole) const;
    QColor decoration(KColorScheme::DecorationRole) const;
    qreal contrast() const;
private:
    struct {
        QColor fg[8], bg[8], deco[2];
    } _colors;
    qreal _contrast;
};

#define DEFAULT(a) QColor( defaults.a[0], defaults.a[1], defaults.a[2] )

KColorSchemePrivate::KColorSchemePrivate(const KSharedConfigPtr &config,
                                         QPalette::ColorGroup state,
                                         const char *group,
                                         DefaultColors defaults)
{
    KConfigGroup cfg( config, group );
    _contrast = KGlobalSettings::contrastF( config );

    // loaded-from-config colors
    _colors.fg[0] = cfg.readEntry( "ForegroundNormal", DEFAULT(NormalText) );
    _colors.fg[1] = cfg.readEntry( "ForegroundInactive", DEFAULT(InactiveText) );
    _colors.fg[2] = cfg.readEntry( "ForegroundActive", QColor(255,0,0) );
    _colors.fg[3] = cfg.readEntry( "ForegroundLink", QColor(0,0,255) );
    _colors.fg[4] = cfg.readEntry( "ForegroundVisited", QColor(88,55,150) );
    _colors.fg[5] = cfg.readEntry( "ForegroundNegative", QColor(107,0,0) );
    _colors.fg[6] = cfg.readEntry( "ForegroundNeutral", QColor(0,90,95) );
    _colors.fg[7] = cfg.readEntry( "ForegroundPositive", QColor(0,95,0) );

    _colors.bg[0] = cfg.readEntry( "BackgroundNormal", DEFAULT(NormalBackground) );
    _colors.bg[1] = cfg.readEntry( "BackgroundAlternate", DEFAULT(AlternateBackground) );

    _colors.deco[0] = cfg.readEntry( "DecorationHover", QColor(72,177,60) );
    _colors.deco[1] = cfg.readEntry( "DecorationFocus", QColor(239,132,65) );

    // apply state adjustments
    if (state != QPalette::Active) {
        // TODO - now tweak all the colors based on the state!
        // FIXME - this code is temporary, proof-of-concept
        if (state == QPalette::Inactive) {
            _colors.bg[0] = KColorUtils::darken(_colors.bg[0], 0.3);
            _colors.bg[1] = KColorUtils::darken(_colors.bg[1], 0.3);
            _colors.deco[0] = KColorUtils::darken(_colors.deco[0], 0.3);
            _colors.deco[1] = KColorUtils::darken(_colors.deco[1], 0.3);
            for (int i=0; i<7; i++) {
                _colors.fg[i] = KColorUtils::darken(_colors.fg[i], 0.3);
            }
        }
        else {
            for (int i=0; i<7; i++) {
                _colors.fg[i] = KColorUtils::mix(_colors.fg[i], _colors.bg[0], 0.7);
            }
        }
    }

    // calculated backgrounds
    _colors.bg[2] = KColorUtils::tint( _colors.bg[0], _colors.fg[2] );
    _colors.bg[3] = KColorUtils::tint( _colors.bg[0], _colors.fg[3] );
    _colors.bg[4] = KColorUtils::tint( _colors.bg[0], _colors.fg[4] );
    _colors.bg[5] = KColorUtils::tint( _colors.bg[0], _colors.fg[5] );
    _colors.bg[6] = KColorUtils::tint( _colors.bg[0], _colors.fg[6] );
    _colors.bg[7] = KColorUtils::tint( _colors.bg[0], _colors.fg[7] );
}

QColor KColorSchemePrivate::background(KColorScheme::BackgroundRole role) const
{
    switch (role) {
        case KColorScheme::AlternateBackground:
            return _colors.bg[1];
        case KColorScheme::ActiveBackground:
            return _colors.bg[2];
        case KColorScheme::LinkBackground:
            return _colors.bg[3];
        case KColorScheme::VisitedBackground:
            return _colors.bg[4];
        case KColorScheme::NegativeBackground:
            return _colors.bg[5];
        case KColorScheme::NeutralBackground:
            return _colors.bg[6];
        case KColorScheme::PositiveBackground:
            return _colors.bg[7];
        default:
            return _colors.bg[0];
    }
}

QColor KColorSchemePrivate::foreground(KColorScheme::ForegroundRole role) const
{
    switch (role) {
        case KColorScheme::InactiveText:
            return _colors.fg[1];
        case KColorScheme::ActiveText:
            return _colors.fg[2];
        case KColorScheme::LinkText:
            return _colors.fg[3];
        case KColorScheme::VisitedText:
            return _colors.fg[4];
        case KColorScheme::NegativeText:
            return _colors.fg[5];
        case KColorScheme::NeutralText:
            return _colors.fg[6];
        case KColorScheme::PositiveText:
            return _colors.fg[7];
        default:
            return _colors.fg[0];
    }
}

QColor KColorSchemePrivate::decoration(KColorScheme::DecorationRole role) const
{
    switch (role) {
        case KColorScheme::FocusColor:
            return _colors.deco[1];
        default:
            return _colors.deco[0];
    }
}

qreal KColorSchemePrivate::contrast() const
{
    return _contrast;
}
//END KColorSchemePrivate

KColorScheme::KColorScheme(const KColorScheme &other) : d(other.d)
{
}

KColorScheme& KColorScheme::operator=(const KColorScheme& other)
{
    d = other.d;
    return *this;
}

KColorScheme::~KColorScheme()
{
}

KColorScheme::KColorScheme(ColorSet set, KSharedConfigPtr config)
{
    // bleh, copied code, a good reason for this ctor to go away
    if (!config) {
        config = KGlobal::config();
    }
    switch (set) {
        case Window:
            d = new KColorSchemePrivate(config, QPalette::Active, "Colors:Window", defaultWindowColors);
            break;
        case Button:
            d = new KColorSchemePrivate(config, QPalette::Active, "Colors:Button", defaultButtonColors);
            break;
        case Selection:
            d = new KColorSchemePrivate(config, QPalette::Active, "Colors:Selection", defaultSelectionColors);
            break;
        case Tooltip:
            d = new KColorSchemePrivate(config, QPalette::Active, "Colors:Tooltip", defaultTooltipColors);
            break;
        default:
            d = new KColorSchemePrivate(config, QPalette::Active, "Colors:View", defaultViewColors);
    }
}

KColorScheme::KColorScheme(QPalette::ColorGroup state, ColorSet set, KSharedConfigPtr config)
{
    if (!config) {
        config = KGlobal::config();
    }
    switch (set) {
        case Window:
            d = new KColorSchemePrivate(config, state, "Colors:Window", defaultWindowColors);
            break;
        case Button:
            d = new KColorSchemePrivate(config, state, "Colors:Button", defaultButtonColors);
            break;
        case Selection:
            // inactiver/disabled uses Window colors instead, ala gtk
            if (state == QPalette::Active)
                d = new KColorSchemePrivate(config, state, "Colors:Selection", defaultSelectionColors);
            else
                d = new KColorSchemePrivate(config, state, "Colors:Window", defaultWindowColors);
            break;
        case Tooltip:
            d = new KColorSchemePrivate(config, state, "Colors:Tooltip", defaultTooltipColors);
            break;
        default:
            d = new KColorSchemePrivate(config, state, "Colors:View", defaultViewColors);
    }
}

QBrush KColorScheme::background(BackgroundRole role) const
{
    return QBrush( d->background( role ) );
}

QBrush KColorScheme::foreground(ForegroundRole role) const
{
    return QBrush( d->foreground(role) );
}

QBrush KColorScheme::decoration(DecorationRole role) const
{
    return QBrush( d->decoration(role) );
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
