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

#ifndef KCOLORSCHEME_H
#define KCOLORSCHEME_H

#include <kdeui_export.h>
#include <ksharedconfig.h>

class KConfigBase;
class QColor;
class QBrush;

/**
 * A set of methods used to work with colors.
 */
class KDEUI_EXPORT KColorScheme {
public:

    /**
     * This enumeration describes the color set for which a color is being
     * selected.
     */
    enum ColorSet {
        /**
         * Views; e.g. frames, input fields, etc.
         */
        View,
        /**
         * Non-editable window elements; e.g. menus.
         */
        Window,
        /**
         * Buttons
         */
        Button,
        /**
         * Selected items in views.
         */
        Selection,
        /**
         * Tooltips.
         */
        Tooltip
    };

    /**
     * This enumeration describes the background color being selected from the
     * given set. Background colors are suitable for drawing under text, and
     * should never be used to draw text. All roles are valid for all sets.
     */
    enum BackgroundRole {
        /**
         * Normal background.
         */
        NormalBackground = 0,
        /**
         * Alternate background, e.g. for use in lists. May be the same as
         * BackgroundNormal.
         */
        AlternateBackground = 1
    };

    /**
     * This enumeration describes the foreground color being selected from the
     * given set. Foreground colors are suitable for drawing text, and should
     * never be used to draw backgrounds. All roles are valid for all sets.
     *
     * As an exception to the above, you may use TODO to produce new background
     * colors from foreground colors.
     */
    enum ForegroundRole {
        /**
         * Normal foreground.
         */
        NormalText = 0,
        /**
         * Second color, e.g. comments, items which are old, inactive or
         * disabled.
         */
        InactiveText = 1,
        /**
         * Third color, e.g. items which are new, active, requesting
         * attention, etc. May be used as a hover color for clickable items.
         */
        ActiveText = 2,
        /**
         * Fourth color, e.g. (unvisited) links or other clickable items.
         */
        LinkText = 3,
        /**
         * Fifth color, e.g. (visited) links.
         */
        VisitedText = 4,
        /**
         * Sixth color, e.g. errors, untrusted content, etc.
         */
        NegativeText = 5,
        /**
         * Seventh color, e.g. warnings, secure/encrypted content.
         */
        NeutralText = 6,
        /**
         * Eigth color, e.g. success messages, trusted content.
         */
        PositiveText = 7
    };

    /**
     * This enumeration describes the decoration color being selected from the
     * given set. Decoration colors are used to draw decorations (such as
     * frames) for special purposes. They should be treated as foreground
     * colors, but they are not intended for drawing text. All roles are valid
     * for all sets.
     */
    enum DecorationRole {
        /**
         * Color used to draw decorations for items which have input focus.
         */
        FocusColor,
        /**
         * Color used to draw decorations for items which will be activated by
         * clicking.
         */
        HoverColor
    };

    /**
     * This enumeration describes the color shade being selected from the given
     * set. Color shades are used to draw "3d" elements, such as frames and
     * bevels. All roles are valid for all sets.
     */
    enum ShadeRole {
        /**
         * The light color is lighter than dark() or shadow() and contrasts
         * with the base color.
         */
        LightShade,
        /**
         * The midlight color is in between base() and light().
         */
        MidlightShade,
        /**
         * The mid color is in between base() and dark().
         */
        MidShade,
        /**
         * The dark color is in between mid() and shadow().
         */
        DarkShade,
        /**
         * The shadow color is darker than light() or midlight() and contrasts
         * the base color.
         */
        ShadowShade
    };

    KColorScheme(const KColorScheme&);
    virtual ~KColorScheme();

    KColorScheme& operator=(const KColorScheme&);

    /**
     * Construct a palette from given color set, using the colors from the
     * given KConfig (if null, the system colors are used).
     */
    explicit KColorScheme(ColorSet = View, KSharedConfigPtr = KSharedConfigPtr());

    /**
     * Retrieve the requested background brush.
     */
    QBrush background(BackgroundRole = NormalBackground) const;

    /**
     * Retrieve the requested foreground brush.
     */
    QBrush foreground(ForegroundRole = NormalText) const;

    /**
     * Retrieve the requested decoration brush.
     */
    QBrush decoration(DecorationRole) const;

    /**
     * Retrieve the requested shade color, using
     * KColorScheme::background(KColorScheme::NormalBackground)
     * as the base color and the contrast setting from the KConfig used to
     * create this KColorScheme instance (the system contrast setting, if no
     * KConfig was specified).
     *
     * @note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with midlight() == shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with light() == mid().
     */
    QColor shade(ShadeRole) const;

    /**
     * Retrieve the requested shade color, using the specified color as the
     * base color and the system contrast setting.
     *
     * @note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with midlight() == shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with light() == mid().
     */
    static QColor shade(const QColor&, ShadeRole);

    /**
     * Retrieve the requested shade color, using the specified color as the
     * base color and the specified contrast.
     *
     * @param contrast Amount roughly specifying the contrast by which to
     * adjust the base color, between -1.0 and 1.0 (values between 0.0 and 1.0
     * correspond to the value from KGlobalSettings::contrastF)
     * @param chromaAdjust (optional) Amount by which to adjust the chroma of
     * the shade (1.0 means no adjustment)
     *
     * @note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with midlight() == shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with light() == mid().
     *
     * @see KColorUtils::shade
     */
    static QColor shade(const QColor&, ShadeRole,
                        qreal contrast, qreal chromaAdjust = 0.0);

private:
    class KColorSchemePrivate* d;
};

#endif // KCOLORSCHEME_H
// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
