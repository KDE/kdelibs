/**
 * Nested list helper
 *
 * Copyright 2008  Stephen Kelly <steveire@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef NESTEDLISTHELPER_H
#define NESTEDLISTHELPER_H

//@cond PRIVATE

class QTextEdit;

class QKeyEvent;
class QDropEvent;
class QTextCursor;
class QTextList;
class QTextBlock;

/**
 *
 * @short Helper class for automatic handling of nested lists in a text edit
 *
 *
 * @author Stephen Kelly
 * @since 4.1
 * @internal
 */
class NestedListHelper
{
public:

    /**
     * Create a helper
     *
     * @param te The text edit object to handle lists in.
     */
    NestedListHelper(QTextEdit *te);

    /**
     * Destructor
     */
    ~NestedListHelper();

    /**
     *
     * Handles a key press before it is processed by the text edit widget.
     *
     * Currently this causes a backspace at the beginning of a line or with a
     * multi-line selection to decrease the nesting level of the list.
     *
     * @param event The event to be handled
     * @return Whether the event was completely handled by this method.
     */
    bool handleBeforeKeyPressEvent(QKeyEvent *event);

    /**
     *
     * Handles a key press after it is processed by the text edit widget.
     *
     * Currently this causes a Return at the end of the last list item, or
     * a Backspace after the last list item to recalculate the spacing
     * between the list items.
     *
     * @param event The event to be handled
     * @return Whether the event was completely handled by this method.
     */
    bool handleAfterKeyPressEvent(QKeyEvent *event);

    bool handleAfterDropEvent(QDropEvent *event);

    /**
     * Increases the indent (nesting level) on the current list item or selection.
     */
    void handleOnIndentMore();

    /**
     * Decreases the indent (nesting level) on the current list item or selection.
     */
    void handleOnIndentLess();

    /**
     * Changes the style of the current list or creates a new list with
     * the specified style.
     *
     * @param styleIndex The QTextListStyle of the list.
     */
    void handleOnBulletType(int styleIndex);

    /**
     * @brief Check whether the current item in the list may be indented.
     *
     * An list item must have an item above it on the same or greater level
     * if it can be indented.
     *
     * Also, a block which is currently part of a list can be indented.
     *
     * @sa canDedent
     *
     * @return Whether the item can be indented.
     */
    bool canIndent() const;

    /**
     * \brief Check whether the current item in the list may be dedented.
     *
     * An item may be dedented if it is part of a list. Otherwise it can't be.
     *
     * @sa canIndent
     *
     * @return Whether the item can be dedented.
     */
    bool canDedent() const;

private:
    void reformatBoundingItemSpacing(QTextBlock block);
    void reformatBoundingItemSpacing();
    QTextCursor topOfSelection();
    QTextCursor bottomOfSelection();
    void processList(QTextList* list);
    void reformatList(QTextBlock block);
    void reformatList();

    QTextEdit *textEdit;

    int listBottomMargin;
    int listTopMargin;
    int listNoMargin;

};

//@endcond

#endif
