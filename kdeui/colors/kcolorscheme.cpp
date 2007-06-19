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

// BEGIN KColorSchemePrivate
class KColorSchemePrivate
{
public:
    explicit KColorSchemePrivate(const char* group, DefaultColors);
    KColorSchemePrivate(const KColorSchemePrivate&);

    QColor background(KColorScheme::BackgroundRole);
    QColor foreground(KColorScheme::ForegroundRole);
    QColor decoration(KColorScheme::DecorationRole);
private:
    KConfigGroup _config;
    DefaultColors _defaults;
};

KColorSchemePrivate::KColorSchemePrivate(const char *group, DefaultColors defaults)
    : _config( KGlobal::config(), group ), _defaults( defaults )
{
}

KColorSchemePrivate::KColorSchemePrivate(const KColorSchemePrivate& other)
    : _config( other._config )
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
// END KColorSchemePrivate

KColorScheme::KColorScheme(const KColorScheme &other)
{
    d = new KColorSchemePrivate(*other.d);
}

KColorScheme::~KColorScheme()
{
    delete d;
}

KColorScheme::KColorScheme(ColorSet set)
{
    KConfigGroup cg( KGlobal::config(), "KDE" );
    switch (set) {
        case Window:
            d = new KColorSchemePrivate( "Colors:Window", defaultWindowColors );
            break;
        case Button:
            d = new KColorSchemePrivate( "Colors:Button", defaultButtonColors );
            break;
        case Selection:
            d = new KColorSchemePrivate( "Colors:Selection", defaultSelectionColors );
            break;
        case Tooltip:
            d = new KColorSchemePrivate( "Colors:Tooltip", defaultTooltipColors );
            break;
        default:
            d = new KColorSchemePrivate( "Colors:View", defaultViewColors );
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
    return shade(background().color(), role);
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role)
{
    return shade(color, role, KGlobalSettings::contrastF());
}

QColor KColorScheme::shade(const QColor &color, ShadeRole role, qreal contrast, qreal chromaAdjust)
{
    // TODO
    // Extremas still need to be handled specially; dark colors result in all
    // shades lighter than the base, and light colors result in all shades
    // darker than the base.

    // nan -> 1.0
    contrast = (1.0 > contrast ? (-1.0 < contrast ? contrast : -1.0) : 1.0);
    qreal y = KColorUtils::luma(color), yi = 1.0 - y;
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
