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
#include <QPainter>

class QColor;
class QBrush;
class KColorSchemePrivate;

/**
 * A set of methods used to work with colors.
 */
class KColorScheme {
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

    KColorScheme(const KColorScheme&);

    /**
     * Construct a palette from the system colors in the given color set.
     */
    explicit KColorScheme(ColorSet = View);

    /**
     * Retrieve the requested background brush.
     */
    QBrush background(BackgroundRole = NormalBackground);

    /**
     * Retrieve the requested foreground brush.
     */
    QBrush foreground(ForegroundRole = NormalText);

    /**
     * Retrieve the requested decoration brush.
     */
    QBrush decoration(DecorationRole);

private:
    KColorSchemePrivate* d;
};

#endif // KCOLORSCHEME_H
// kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on;
