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

#include <QtCore/QExplicitlySharedDataPointer>

class KConfigBase;
class QColor;
class QBrush;

class KColorSchemePrivate;

/**
 * A set of methods used to work with colors.
 *
 * KColorScheme currently provides access to the system color palette that the
 * user has selected (in the future, it is expected to do more). As of KDE4,
 * this class is the correct way to look up colors from the system palette,
 * as opposed to KGlobalSettings (such usage is deprecated). It greatly expands
 * on KGlobalSettings and QPalette by providing five distinct "sets" with
 * several color choices each, covering background, foreground, and decoration
 * colors.
 *
 * A KColorScheme instance represents colors corresponding to a "set", where a
 * set consists of those colors used to draw a particular type of element, such
 * as a menu, button, view, selected text, or tooltip. Each set has a distinct
 * set of colors, so you should always use the correct set for drawing and
 * never assume that a particular foreground for one set is the same as the
 * foreground for any other set. Individual colors may be quickly referenced by
 * creating an anonymous instance and invoking a lookup member.
 *
 * @see KColorScheme::ColorSet, KColorScheme::ForegroundRole,
 * KColorScheme::BackgroundRole, KColorScheme::DecorationRole,
 * KColorScheme::ShadeRole
 */
class KDEUI_EXPORT KColorScheme {
public:

    /**
     * This enumeration describes the color set for which a color is being
     * selected.
     *
     * Color sets define a color "environment", suitable for drawing all parts
     * of a given region. Colors from different sets should not be combined.
     */
    enum ColorSet {
        /**
         * Views; e.g. frames, input fields, etc.
         *
         * If it contains things that can be selected, it is probably a View.
         */
        View,
        /**
         * Non-editable window elements; e.g. menus.
         *
         * If it isn't a Button, View, or Tooltip, it is probably a Window.
         */
        Window,
        /**
         * Buttons.
         *
         * In addition to buttons, "button-like" controls such as non-editable
         * dropdowns should also use this role.
         */
        Button,
        /**
         * Selected items in views.
         *
         * Note that unfocused or disabled selections should use the Window
         * role. This makes it more obvious to the user that the view
         * containing the selection does not have input focus.
         */
        Selection,
        /**
         * Tooltips.
         *
         * The tooltip set can often be substituted for the view
         * set when editing is not possible, but the Window set is deemed
         * inappropriate. "What's This" help is an excellent example, another
         * might be pop-up notifications (depending on taste).
         */
        Tooltip
    };

    /**
     * This enumeration describes the background color being selected from the
     * given set.
     *
     * Background colors are suitable for drawing under text, and should never
     * be used to draw text. In combination with one of the overloads of
     * KColorScheme::shade, they may be used to generate colors for drawing
     * frames, bevels, and similar decorations.
     */
    enum BackgroundRole {
        /**
         * Normal background.
         */
        NormalBackground = 0,
        /**
         * Alternate background, e.g. for use in lists. May be the same as
         * BackgroundNormal, especially in sets other than View and Window.
         */
        AlternateBackground = 1,
        /**
         * Third color, e.g. items which are new, active, requesting
         * attention, etc. Alerting the user that a certain field must be
         * filled out would be a good usage. (Unlike ActiveText, this should
         * not be used for mouseover effects.)
         */
        ActiveBackground = 2,
        /**
         * Fourth color, e.g. (unvisited) links or other clickable items.
         */
        LinkBackground = 3,
        /**
         * Fifth color, e.g. (visited) links.
         */
        VisitedBackground = 4,
        /**
         * Sixth color, e.g. errors, untrusted content, etc.
         */
        NegativeBackground = 5,
        /**
         * Seventh color, e.g. warnings, secure/encrypted content.
         */
        NeutralBackground = 6,
        /**
         * Eigth color, e.g. success messages, trusted content.
         */
        PositiveBackground = 7
    };

    /**
     * This enumeration describes the foreground color being selected from the
     * given set.
     *
     * Foreground colors are suitable for drawing text, and should never be
     * used to draw backgrounds.
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
     * given set.
     *
     * Decoration colors are used to draw decorations (such as frames) for
     * special purposes. Like color shades, they are neither foreground nor
     * background colors. Text should not be painted over a decoration color,
     * and decoration colors should not be used to draw text.
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
     * set.
     *
     * Color shades are used to draw "3d" elements, such as frames and bevels.
     * They are neither foreground nor background colors. Text should not be
     * painted over a shade, and shades should not be used to draw text.
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
    QExplicitlySharedDataPointer<KColorSchemePrivate> d;
};

#endif // KCOLORSCHEME_H
// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
