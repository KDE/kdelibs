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

#include "nestedlisthelper.h"

#include <QtGui/QKeyEvent>
#include <QtGui/QTextCursor>
#include <QtGui/QTextList>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocumentFragment>

#include <ktextedit.h>
#include <kdebug.h>

NestedListHelper::NestedListHelper(QTextEdit *te)
{
    textEdit = te;
    listBottomMargin = 12;
    listTopMargin = 12;
    listNoMargin = 0;
}

NestedListHelper::~NestedListHelper()
{
}

bool NestedListHelper::handleBeforeKeyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textEdit->textCursor();

    // Only attempt to handle Backspace while on a list
    if ((event->key() != Qt::Key_Backspace)
            || (!cursor.currentList()))
        return false;

    bool handled = false;

    if (!cursor.hasSelection()
            && cursor.currentList()
            && event->key() == Qt::Key_Backspace
            && cursor.atBlockStart()) {
        handleOnIndentLess();
        handled = true;
    }

    if (cursor.hasSelection()
            && cursor.currentList()
            && event->key() == Qt::Key_Backspace
            && cursor.atBlockStart()) {

        // Workaround for qt bug 211460:
        // If there is a list with selection like this:
        //
        //   * one
        //   * <cursor>t<anchor>wo
        //
        // and backspace is pressed, the bullet is removed, but not
        // the 't'.
        // Fixed scheduled for qt4.5
        // -- Stephen Kelly, 8th June 2008

        cursor.removeSelectedText();
        handled = true;
    }

    return handled;
}

bool NestedListHelper::canIndent() const
{
    if ((textEdit->textCursor().block().isValid())
//            && (  textEdit->textCursor().block().previous().isValid() )
       ) {
        QTextBlock block = textEdit->textCursor().block();
        QTextBlock prevBlock = textEdit->textCursor().block().previous();
        if (block.textList()) {
            if (prevBlock.textList()) {
                return block.textList()->format().indent() <= prevBlock.textList()->format().indent();
            }
        } else {
            return true;
        }
    }
    return false;
}

bool NestedListHelper::canDedent() const
{
    QTextBlock thisBlock = textEdit->textCursor().block();
    QTextBlock nextBlock = thisBlock.next();
    if (thisBlock.isValid()) {
        int nextBlockIndent = 0;
        int thisBlockIndent = 0;
        if (nextBlock.isValid() && nextBlock.textList())
            nextBlockIndent = nextBlock.textList()->format().indent();
        if (thisBlock.textList()) {
            thisBlockIndent = thisBlock.textList()->format().indent();
            if (thisBlockIndent >= nextBlockIndent)
                return thisBlock.textList()->format().indent() > 0;
        }
    }
    return false;

}

bool NestedListHelper::handleAfterKeyPressEvent(QKeyEvent *event)
{
    // Only attempt to handle Backspace and Return
    if ((event->key() != Qt::Key_Backspace)
            && (event->key() != Qt::Key_Return))
        return false;

    QTextCursor cursor = textEdit->textCursor();
    bool handled = false;

    if (!cursor.hasSelection() && cursor.currentList()) {

        // Check if we're on the last list item.
        // itemNumber is zero indexed
        QTextBlock currentBlock = cursor.block();
        if (cursor.currentList()->count() == cursor.currentList()->itemNumber(currentBlock) + 1) {
            // Last block in this list, but may have just gained another list below.
            if (currentBlock.next().textList()) {
                reformatList();
            }

            // No need to reformatList in this case. reformatList is slow.
            if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Backspace)) {
                reformatBoundingItemSpacing();
                handled = true;
            }
        } else {
            reformatList();
        }
    }
    return handled;
}


bool NestedListHelper::handleAfterDropEvent(QDropEvent *dropEvent)
{
    Q_UNUSED(dropEvent);
    QTextCursor cursor = topOfSelection();

    QTextBlock droppedBlock = cursor.block();
    int firstDroppedItemIndent = droppedBlock.textList()->format().indent();

    int minimumIndent = droppedBlock.previous().textList()->format().indent();

    if (firstDroppedItemIndent < minimumIndent) {
        cursor = QTextCursor(droppedBlock);
        QTextListFormat fmt = droppedBlock.textList()->format();
        fmt.setIndent(minimumIndent);
        QTextList* list = cursor.createList(fmt);

        int endOfDrop = bottomOfSelection().position();
        while (droppedBlock.next().position() < endOfDrop) {
            droppedBlock = droppedBlock.next();
            if (droppedBlock.textList()->format().indent() != firstDroppedItemIndent) {

                // new list?
            }
            list->add(droppedBlock);
        }
//         list.add( droppedBlock );
    }

    reformatBoundingItemSpacing();
    return true;
}

void NestedListHelper::processList(QTextList* list)
{
    QTextBlock block = list->item(0);
    int thisListIndent = list->format().indent();

    QTextCursor cursor = QTextCursor(block);
    list = cursor.createList(list->format());
    bool processingSubList  = false;
    while (block.next().textList() != 0) {
        block = block.next();

        QTextList* nextList = block.textList();
        int nextItemIndent = nextList->format().indent();
        if (nextItemIndent < thisListIndent) {
            return;
        } else if (nextItemIndent > thisListIndent) {
            if (processingSubList) {
                continue;
            }
            processingSubList = true;
            processList(nextList);
        } else {
            processingSubList = false;
            list->add(block);
        }
    }
//     delete nextList;
//     nextList = 0;
}

void NestedListHelper::reformatList(QTextBlock block)
{
    if (block.textList()) {
        int minimumIndent =  block.textList()->format().indent();

        // Start at the top of the list
        while (block.previous().textList() != 0) {
            if (block.previous().textList()->format().indent() < minimumIndent) {
                break;
            }
            block = block.previous();
        }

        processList(block.textList());

    }
}

void NestedListHelper::reformatList()
{
    QTextCursor cursor = textEdit->textCursor();
    reformatList(cursor.block());
}

QTextCursor NestedListHelper::topOfSelection()
{
    QTextCursor cursor = textEdit->textCursor();

    if (cursor.hasSelection())
        cursor.setPosition(qMin(cursor.position(), cursor.anchor()));
    return cursor;
}

QTextCursor NestedListHelper::bottomOfSelection()
{
    QTextCursor cursor = textEdit->textCursor();

    if (cursor.hasSelection())
        cursor.setPosition(qMax(cursor.position(), cursor.anchor()));
    return cursor;
}

void NestedListHelper::handleOnIndentMore()
{
    QTextCursor cursor = textEdit->textCursor();

    QTextListFormat listFmt;
    if (!cursor.currentList()) {

        QTextListFormat::Style style;
        cursor = topOfSelection();
        cursor.movePosition(QTextCursor::PreviousBlock);
        if (cursor.currentList()) {
            style = cursor.currentList()->format().style();
        } else {

            cursor = bottomOfSelection();
            cursor.movePosition(QTextCursor::NextBlock);

            if (cursor.currentList()) {
                style = cursor.currentList()->format().style();
            } else {
                style = QTextListFormat::ListDisc;
            }
        }
        handleOnBulletType(style);
    } else {
        listFmt = cursor.currentList()->format();
        listFmt.setIndent(listFmt.indent() + 1);

        cursor.createList(listFmt);
        reformatList();
    }

    reformatBoundingItemSpacing();
}

void NestedListHelper::handleOnIndentLess()
{
    QTextCursor cursor = textEdit->textCursor();
    QTextList* currentList = cursor.currentList();
    if (!currentList)
        return;
    QTextListFormat listFmt;
    listFmt = currentList->format();
    if (listFmt.indent() > 1) {
        listFmt.setIndent(listFmt.indent() - 1);
        cursor.createList(listFmt);
        reformatList(cursor.block());
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
        reformatList(cursor.block().next());
    }
    reformatBoundingItemSpacing();
}


void NestedListHelper::handleOnBulletType(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    if (styleIndex != 0) {
        QTextListFormat::Style style = (QTextListFormat::Style)styleIndex;
        QTextList *currentList = cursor.currentList();
        QTextListFormat listFmt;

        cursor.beginEditBlock();

        if (currentList) {
            listFmt = currentList->format();
            listFmt.setStyle(style);
            currentList->setFormat(listFmt);
        } else {
            listFmt.setStyle(style);
            cursor.createList(listFmt);
        }

        cursor.endEditBlock();
    } else {
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.setBlockFormat(bfmt);
        reformatBoundingItemSpacing();
    }

    reformatBoundingItemSpacing();
    reformatList();
}

void NestedListHelper::reformatBoundingItemSpacing(QTextBlock block)
{
    // This is a workaround for qt bug 201228. Spacing between items is not kept
    // consistent.
    // Fixed scheduled for qt4.5
    // -- Stephen Kelly, 8th June 2008

    int nextBlockTopMargin = listNoMargin;
    int previousBlockBottomMargin = listNoMargin;
    int thisBlockBottomMargin = listBottomMargin;
    int thisBlockTopMargin = listTopMargin;
    bool prevBlockValid = block.previous().isValid();
    bool nextBlockValid = block.next().isValid();

    if (block.textList()) {
        if (prevBlockValid && block.previous().textList()) {
            thisBlockTopMargin = listNoMargin;
        }

        if (nextBlockValid && block.next().textList()) {
            thisBlockBottomMargin = listNoMargin;
        }
    } else {
        if (prevBlockValid && !block.previous().textList()) {
            thisBlockTopMargin = listNoMargin;
        }
        if (nextBlockValid && !block.next().textList()) {
            thisBlockBottomMargin = listNoMargin;
        }

    }
    QTextBlockFormat fmt;
    QTextCursor cursor;

    fmt = block.blockFormat();
    fmt.setBottomMargin(thisBlockBottomMargin);
    fmt.setTopMargin(thisBlockTopMargin);
    cursor = QTextCursor(block);
    cursor.setBlockFormat(fmt);

    if (nextBlockValid) {
        block = block.next();
        fmt = block.blockFormat();
        fmt.setTopMargin(nextBlockTopMargin);
        cursor = QTextCursor(block);
        cursor.setBlockFormat(fmt);

        block = block.previous();
    }
    if (prevBlockValid) {
        block = block.previous();
        fmt = block.blockFormat();
        fmt.setBottomMargin(previousBlockBottomMargin);
        cursor = QTextCursor(block);
        cursor.setBlockFormat(fmt);
    }
}

void NestedListHelper::reformatBoundingItemSpacing()
{
    reformatBoundingItemSpacing(topOfSelection().block());
    reformatBoundingItemSpacing(bottomOfSelection().block());
}
