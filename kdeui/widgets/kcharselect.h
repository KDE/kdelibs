/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kcharselect_h
#define kcharselect_h

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <kglobal.h>
#include <kdeui_export.h>

class KActionCollection;

class QFont;
class QUrl;

/**
 * @short Character selection widget
 *
 * This widget allows the user to select a character of a
 * specified font and to browse Unicode information
 *
 * \image html kcharselect.png "Character Selection Widget"
 *
 * You can specify the font whose characters should be displayed via
 * setCurrentFont(). Using the Controls argument in the contructor
 * you can create a compact version of KCharSelect if there is not enough
 * space and if you don't need all features.
 *
 * KCharSelect displays one Unicode block at a time and provides
 * categorized access to them. Unicode character names and further details,
 * including cross references, are displayed. Additionally, there is a search
 * to find characters.
 *
 * To get the current selected character, use the currentChar()
 * method. You can set the character which should be displayed with
 * setCurrentChar().
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @author Daniel Laidig <d.laidig@gmx.de>
 */

class KDEUI_EXPORT KCharSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont)
    Q_PROPERTY(QChar currentChar READ currentChar WRITE setCurrentChar)
    Q_PROPERTY(QList<QChar> displayedChars READ displayedChars)

public:
    /**
     * Flags to set the shown widgets
     */
    enum Control {
        /**
         * Shows the search widgets
         */
        SearchLine = 0x01,
        /**
         * Shows the font combo box
         */
        FontCombo = 0x02,
        /**
         * Shows the font size spin box
         */
        FontSize = 0x04,
        /**
         * Shows the category/block selection combo boxes
         */
        BlockCombos = 0x08,
        /**
         * Shows the actual table
         */
        CharacterTable = 0x10,
        /**
         * Shows the detail browser
         */
        DetailBrowser = 0x20,
        /**
         * Shows the Back/Forward buttons
         */
        HistoryButtons = 0x40,
        /**
         * Shows everything
         */
        AllGuiElements      = 65535
    };
    Q_DECLARE_FLAGS(Controls,
                    Control)

    /** @deprecated */
    KDE_CONSTRUCTOR_DEPRECATED explicit KCharSelect(
            QWidget *parent,
            const Controls controls = AllGuiElements);

    /**
     * Constructor. @p controls can be used to show a custom set of widgets.
     *
     * The widget uses the following actions:
     *   - KStandardActions::find() (edit_find)
     *   - KStandardActions::back() (go_back)
     *   - KStandardActions::forward() (go_forward)
     *
     * If you provide a KActionCollection, this will be populated with the above actions,
     * which you can then manually trigger or place in menus and toolbars.
     *
     * @param parent     the parent widget for this KCharSelect (see QWidget documentation)
     * @param collection if this is not @c null, KCharSelect will place its actions into this
     *                   collection
     * @param controls   selects the visible controls on the KCharSelect widget
     *
     * @since 4.2
     */
    explicit KCharSelect(
            QWidget *parent,
            KActionCollection *collection,
            const Controls controls = AllGuiElements);

    ~KCharSelect();

    /**
     * Reimplemented.
     */
    virtual QSize sizeHint() const;

    /**
     * Returns the currently selected character.
     */
    QChar currentChar() const;

    /**
     * Returns the currently displayed font.
     */
    QFont currentFont() const;

    /**
     * Returns a list of currently displayed characters.
     */
    QList<QChar> displayedChars() const;

public Q_SLOTS:
    /**
     * Highlights the character @p c. If the character is not displayed, the block is changed.
     *
     * @param c the character to highlight
     */
    void setCurrentChar(const QChar &c);

    /**
     * Sets the font which is displayed to @p font
     *
     * @param font the display font for the widget
     */
    void setCurrentFont(const QFont &font);

Q_SIGNALS:
    /**
     * A new font is selected or the font size changed.
     *
     * @param font the new font
     */
    void currentFontChanged(const QFont &font);
    /**
     * The current character is changed.
     *
     * @param c the new character
     */
    void currentCharChanged(const QChar &c);
    /**
     * The currently displayed characters are changed (search results or block).
     */
    void displayedCharsChanged();
    /**
     * A character is selected to be inserted somewhere.
     *
     * @param c the selected character
     */
    void charSelected(const QChar &c);

private:
    Q_PRIVATE_SLOT(d, void _k_activateSearchLine())
    Q_PRIVATE_SLOT(d, void _k_back())
    Q_PRIVATE_SLOT(d, void _k_forward())
    Q_PRIVATE_SLOT(d, void _k_fontSelected())
    Q_PRIVATE_SLOT(d, void _k_updateCurrentChar(const QChar &c))
    Q_PRIVATE_SLOT(d, void _k_slotUpdateUnicode(const QChar &c))
    Q_PRIVATE_SLOT(d, void _k_sectionSelected(int index))
    Q_PRIVATE_SLOT(d, void _k_blockSelected(int index))
    Q_PRIVATE_SLOT(d, void _k_searchEditChanged())
    Q_PRIVATE_SLOT(d, void _k_search())
    Q_PRIVATE_SLOT(d, void _k_linkClicked(QUrl))

    class KCharSelectPrivate;
    KCharSelectPrivate* const d;

    void init(const Controls, KActionCollection *);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCharSelect::Controls)

#endif
