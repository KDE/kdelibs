/* This file is part of the KDE libraries
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2003-2005 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KATE_RENDERER_H__
#define __KATE_RENDERER_H__

#include "katecursor.h"
#include <ktexteditor/attribute.h>
#include "katetextline.h"
#include "katelinelayout.h"
#include "katesmartregion.h"

#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtCore/QList>
#include <QtGui/QTextLine>

class KateDocument;
class KateView;
class KateRendererConfig;
class KateRenderRange;
namespace  KTextEditor { class Range; }

class KateLineLayout;
typedef KSharedPtr<KateLineLayout> KateLineLayoutPtr;

/**
 * Handles all of the work of rendering the text
 * (used for the views and printing)
 *
 **/
class KateRenderer
{
public:
    /**
     * Style of Caret (Vertical line or block)
     */
    enum caretStyles {
      Line,
      Block
    };

    /**
     * Constructor
     * @param doc document to render
     * @param view view which is output (0 for example for rendering to print)
     */
    explicit KateRenderer(KateDocument* doc, KateView *view = 0);

    /**
     * Destructor
     */
    ~KateRenderer();

    /**
     * Returns the document to which this renderer is bound
     */
    KateDocument* doc() const { return m_doc; }

    /**
     * Returns the view to which this renderer is bound
     */
    KateView* view() const { return m_view; }

    /**
     * update the highlighting attributes
     * (for example after an hl change or after hl config changed)
     */
    void updateAttributes ();

    /**
     * Determine whether the caret (text cursor) will be drawn.
     * @return should it be drawn?
     */
    inline bool drawCaret() const { return m_drawCaret; }

    /**
     * Set whether the caret (text cursor) will be drawn.
     * @param drawCaret should caret be drawn?
     */
    void setDrawCaret(bool drawCaret);

    /**
     * The style of the caret (text cursor) to be painted.
     * @return caretStyle
     */
    inline KateRenderer::caretStyles caretStyle() const { return m_caretStyle; }

    /**
     * Set the style of caret to be painted.
     * @param style style to set
     */
    void setCaretStyle(KateRenderer::caretStyles style);

    /**
     * Set a \a brush with which to override drawing of the caret.  Set to QColor() to clear.
     */
    void setCaretOverrideColor(const QColor& color);

    /**
     * @returns whether tabs should be shown (ie. a small mark
     * drawn to identify a tab)
     * @return tabs should be shown
     */
    inline bool showTabs() const { return m_showTabs; }

    /**
     * Set whether a mark should be painted to help identifying tabs.
     * @param showTabs show the tabs?
     */
    void setShowTabs(bool showTabs);

    /**
     * @returns whether trailing spaces should be shown.
     */
    inline bool showTrailingSpaces() const { return m_showSpaces; }

    /**
     * Set whether a mark should be painted for trailing spaces.
     */
    void setShowTrailingSpaces(bool showSpaces);

    /**
     * Sets the width of the tab. Helps performance.
     * @param tabWidth new tab width
     */
    void setTabWidth(int tabWidth);

    /**
     * @returns whether indent lines should be shown
     * @return indent lines should be shown
     */
    bool showIndentLines() const;

    /**
     * Set whether a guide should be painted to help identifying indent lines.
     * @param showLines show the indent lines?
     */
    void setShowIndentLines(bool showLines);

    /**
     * Sets the width of the tab. Helps performance.
     * @param indentWidth new indent width
     */
    void setIndentWidth(int indentWidth);

    /**
     * Show the view's selection?
     * @return show sels?
     */
    inline bool showSelections() const { return m_showSelections; }

    /**
     * Set whether the view's selections should be shown.
     * The default is true.
     * @param showSelections show the selections?
     */
    void setShowSelections(bool showSelections);

    /**
     * Change to a different font (soon to be font set?)
     */
    void increaseFontSizes();
    void decreaseFontSizes();
    const QFont& currentFont() const;
    const QFontMetrics& currentFontMetrics() const;

    /**
     * @return whether the renderer is configured to paint in a
     * printer-friendly fashion.
     */
    bool isPrinterFriendly() const;

    /**
     * Configure this renderer to paint in a printer-friendly fashion.
     *
     * Sets the other options appropriately if true.
     */
    void setPrinterFriendly(bool printerFriendly);

    /**
     * Text width & height calculation functions...
     */
    void layoutLine(KateLineLayoutPtr line, int maxwidth = -1, bool cacheLayout = false) const;

    /**
     * This is a smaller QString::isRightToLeft(). It's also marked as internal to kate
     * instead of internal to Qt, so we can modify. This method searches for the first
     * strong character in the paragraph and then returns its direction. In case of a 
     * line without any strong characters, the direction is forced to be LTR.
     * 
     * Back in KDE 4.1 this method counted chars, which lead to unwanted side effects.
     * (see https://bugs.kde.org/show_bug.cgi?id=178594). As this function is internal
     * the way it work will probably change between releases. Be warned!
     */
    bool isLineRightToLeft( KateLineLayoutPtr lineLayout ) const;

    /**
     * The ultimate decoration creation function.
     *
     * \param range line to return decoration for
     * \param selectionsOnly return decorations for selections and/or dynamic highlighting.
     */
    QList<QTextLayout::FormatRange> decorationsForLine(const KateTextLine::Ptr& textLine, int line, bool selectionsOnly = false, KateRenderRange* completionHighlight = 0L, bool completionSelected = false) const;

    KateSmartRegion& dynamicRegion() { return m_dynamicRegion; }

    // Width calculators
    uint spaceWidth() const;
    KDE_DEPRECATED uint textWidth(const KateTextLine::Ptr &, int cursorCol);
    KDE_DEPRECATED uint textWidth(const KateTextLine::Ptr &textLine, uint startcol, uint maxwidth, bool *needWrap, int *endX = 0);
    KDE_DEPRECATED uint textWidth(const KTextEditor::Cursor& cursor);

    /**
     * Returns the x position of cursor \p col on the line \p range.
     */
    int cursorToX(const KateTextLayout& range, int col) const;
    /// \overload
    int cursorToX(const KateTextLayout& range, const KTextEditor::Cursor& pos) const;
    /// \overload
    int cursorToX(const KateTextLayout& range, const KTextEditor::Cursor& pos, bool returnPastLine) const;

    /**
     * Returns the real cursor which is occupied by the specified x value, or that closest to it.
     * If \p returnPastLine is true, the column will be extrapolated out with the assumption
     * that the extra characters are spaces.
     */
    KTextEditor::Cursor xToCursor(const KateTextLayout& range, int x, bool returnPastLine = false) const;

    // Font height
    uint fontHeight();

    // Document height
    uint documentHeight();

    // Selection boundaries
    bool getSelectionBounds(int line, int lineLength, int &start, int &end) const;

    /**
     * This is the ultimate function to perform painting of a text line.
     *
     * The text line is painted from the upper limit of (0,0).  To move that,
     * apply a transform to your painter.
     *
     * @param paint           painter to use
     * @param range           layout to use in painting this line
     * @param xStart          starting width in pixels.
     * @param xEnd            ending width in pixels.
     * @param cursor          position of the caret, if placed on the current line.
     */
    void paintTextLine(QPainter& paint, KateLineLayoutPtr range, int xStart, int xEnd, const KTextEditor::Cursor* cursor = 0L);

    /**
     * Paint the background of a line
     *
     * Split off from the main @ref paintTextLine method to make it smaller. As it's being
     * called only once per line it shouldn't noticably affect performance and it
     * helps readability a LOT.
     *
     * @param paint           painter to use
     * @param layout          layout to use in painting this line
     * @param currentViewLine if one of the view lines is the current line, set
     *                        this to the index; otherwise -1.
     * @param xStart          starting width in pixels.
     * @param xEnd            ending width in pixels.
     */
    void paintTextLineBackground(QPainter& paint, KateLineLayoutPtr layout, int currentViewLine, int xStart, int xEnd);

    /**
     * This takes an in index, and returns all the attributes for it.
     * For example, if you have a ktextline, and want the KTextEditor::Attribute
     * for a given position, do:
     *
     *   attribute(myktextline->attribute(position));
     */
    KTextEditor::Attribute::Ptr attribute(uint pos) const;
    KTextEditor::Attribute::Ptr specificAttribute(int context) const;

  private:
    /**
     * Paint a trailing space on position (x, y).
     */
    void paintTrailingSpace(QPainter &paint, qreal x, qreal y);
    /**
     * Paint a tab stop marker on position (x, y).
     */
    void paintTabstop(QPainter &paint, qreal x, qreal y);

    /** Paint a SciTE-like indent marker. */
    void paintIndentMarker(QPainter &paint, uint x, uint y);

    void assignSelectionBrushesFromAttribute(QTextLayout::FormatRange& target, const KTextEditor::Attribute& attribute) const;

    KateDocument* m_doc;
    KateView *m_view;

    // cache of config values
    int m_tabWidth;
    int m_indentWidth;

    // some internal flags
    KateRenderer::caretStyles m_caretStyle;
    bool m_drawCaret;
    bool m_showSelections;
    bool m_showTabs;
    bool m_showSpaces;
    bool m_printerFriendly;
    QColor m_caretOverrideColor;

    QList<KTextEditor::Attribute::Ptr> m_attributes;

  /**
   * Configuration
   */
  public:
    inline KateRendererConfig *config () const { return m_config; }

    void updateConfig ();

  private:
    KateRendererConfig *m_config;

    KateSmartRegion m_dynamicRegion;
};

#endif
