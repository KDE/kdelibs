/*  This file is part of the Kate project.
 *
 *  Copyright (C) 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KATE_TEXTBLOCK_H
#define KATE_TEXTBLOCK_H

#include <QtCore/QVector>
#include <QtCore/QSet>

#include "katepartprivate_export.h"
#include <ktexteditor/cursor.h>
#include "katetextline.h"

namespace Kate {

class TextBuffer;
class TextCursor;
class TextRange;

/**
 * Class representing a text block.
 * This is used to build up a Kate::TextBuffer.
 */
class KATEPART_TESTS_EXPORT TextBlock {
  friend class TextCursor;
  friend class TextRange;
  friend class TextBuffer;

  public:
    /**
     * Construct an empty text block.
     * @param buffer parent text buffer
     * @param startLine start line of this block
     */
    TextBlock (TextBuffer *buffer, int startLine);

    /**
     * Destruct the text block
     */
    ~TextBlock ();

    /**
     * Start line of this block.
     * @return start line of this block
     */
    int startLine () const { return m_startLine; }

    /**
     * Set start line of this block.
     * @param startLine new start line of this block
     */
    void setStartLine (int startLine);

    /**
     * Retrieve a text line.
     * @param line wanted line number
     * @return text line
     */
    TextLine line (int line) const;

    /**
     * Append a new line.
     * @param line line to append
     */
    void appendLine (TextLine line) { m_lines.append (line); }

    /**
     * Number of lines in this block.
     * @return number of lines
     */
    int lines () const { return m_lines.size(); }

    /**
     * Retrieve text of block.
     * @param text for this block, lines separated by '\n'
     */
    void text (QString &text) const;

    /**
     * Wrap line at given cursor position.
     * @param position line/column as cursor where to wrap
     * @param nextBlock next block, if any
     */
    void wrapLine (const KTextEditor::Cursor &position);

    /**
     * Unwrap given line.
     * @param line line to unwrap
     * @param previousBlock previous block, if any, if we unwrap first line in block, we need to have this
     */
    void unwrapLine (int line, TextBlock *previousBlock);

    /**
     * Insert text at given cursor position.
     * @param position position where to insert text
     * @param text text to insert
     */
    void insertText (const KTextEditor::Cursor &position, const QString &text);

    /**
     * Remove text at given range.
     * @param range range of text to remove, must be on one line only.
     * @param removedText will be filled with removed text
     */
    void removeText (const KTextEditor::Range &range, QString &removedText);

    /**
     * Debug output, print whole block content with line numbers and line length
     * @param blockIndex index of this block in buffer
     */
    void debugPrint (int blockIndex) const;

    /**
     * Split given block. A new block will be created and all lines starting from the given index will
     * be moved to it, together with the cursors belonging to it.
     * @param fromLine line from which to split
     * @return new block containing the lines + cursors removed from this one
     */
    TextBlock *splitBlock (int fromLine);

    /**
     * Merge this block with given one, the given one must be a direct predecessor.
     * @param targetBlock block to merge with
     */
    void mergeBlock (TextBlock *targetBlock);

    /**
     * Delete the block content, delete all lines and delete all cursors not bound to ranges.
     * This is used in destructor of TextBuffer, for fast cleanup. Only stuff remaining afterwards are cursors which are
     * part of a range, TextBuffer will delete them itself...
     */
    void deleteBlockContent ();

    /**
     * Clear the block content, delete all lines, move all cursors not bound to range to given block at 0,0.
     * This is used by clear() of TextBuffer.
     * @param targetBlock empty target block for cursors
     */
    void clearBlockContent (TextBlock *targetBlock);

  private:
    /**
     * parent text buffer
     */
    TextBuffer *m_buffer;

    /**
     * Lines contained in this buffer. These are shared pointers.
     */
    QVector<Kate::TextLine> m_lines;

    /**
     * Startline of this block
     */
    int m_startLine;

    /**
     * Set of cursors for this block.
     */
    QSet<TextCursor *> m_cursors;

    /**
     * Set of ranges spanning this block or being contained in it.
     * This is used for fast lookup of ranges.
     * Only expensive for ranges spanning many blocks, which are rare, beside selection.
     */
    QSet<TextRange *> m_ranges;
};

}

#endif
