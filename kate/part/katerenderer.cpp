/* This file is part of the KDE libraries
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "katerenderer.h"

#include "katelinerange.h"
#include "katedocument.h"
#include "katearbitraryhighlight.h"
#include "kateconfig.h"
#include "katehighlight.h"
#include "kateview.h"

#include <kdebug.h>

#include <qpainter.h>
#include <q3popupmenu.h>
#include <QTextLayout>
#include <QStack>

static const QChar tabChar('\t');
static const QChar spaceChar(' ');

KateRenderer::KateRenderer(KateDocument* doc, KateView *view)
  : m_doc(doc), m_view (view), m_caretStyle(KateRenderer::Insert)
    , m_drawCaret(true)
    , m_showSelections(true)
    , m_showTabs(true)
    , m_printerFriendly(false)
{
  m_config = new KateRendererConfig (this);

  m_tabWidth = m_doc->config()->tabWidth();
  m_indentWidth = m_tabWidth;
  if (m_doc->config()->configFlags() & KateDocumentConfig::cfSpaceIndent)
  {
    m_indentWidth = m_doc->config()->indentationWidth();
  }

  updateAttributes ();
}

KateRenderer::~KateRenderer()
{
  delete m_config;
}

void KateRenderer::updateAttributes ()
{
  m_schema = config()->schema ();
  m_attributes = m_doc->highlight()->attributes (m_schema);
}

KateAttribute* KateRenderer::attribute(uint pos) const
{
  if (pos < (uint)m_attributes->size())
    return &(*m_attributes)[pos];

  return &(*m_attributes)[0];
}

KateAttribute * KateRenderer::specificAttribute( int context ) const
{
  if (context >= 0 && context < m_attributes->count())
    return &(*m_attributes)[context];

  return &(*m_attributes)[0];
}

void KateRenderer::setDrawCaret(bool drawCaret)
{
  m_drawCaret = drawCaret;
}

void KateRenderer::setCaretStyle(KateRenderer::caretStyles style)
{
  m_caretStyle = style;
}

void KateRenderer::setShowTabs(bool showTabs)
{
  m_showTabs = showTabs;
}

void KateRenderer::setTabWidth(int tabWidth)
{
  m_tabWidth = tabWidth;
}

bool KateRenderer::showIndentLines() const
{
  return m_config->showIndentationLines();
}

void KateRenderer::setShowIndentLines(bool showIndentLines)
{
  m_config->setShowIndentationLines(showIndentLines);
}

void KateRenderer::setIndentWidth(int indentWidth)
{
  m_indentWidth = m_tabWidth;
  if (m_doc->config()->configFlags() & KateDocumentConfig::cfSpaceIndent)
  {
    m_indentWidth = indentWidth;
  }
}

void KateRenderer::setShowSelections(bool showSelections)
{
  m_showSelections = showSelections;
}

void KateRenderer::increaseFontSizes()
{
  QFont f ( *config()->font () );
  f.setPointSize (f.pointSize ()+1);

  config()->setFont (f);
}

void KateRenderer::decreaseFontSizes()
{
  QFont f ( *config()->font () );

  if ((f.pointSize ()-1) > 0)
    f.setPointSize (f.pointSize ()-1);

  config()->setFont (f);
}

bool KateRenderer::isPrinterFriendly() const
{
  return m_printerFriendly;
}

void KateRenderer::setPrinterFriendly(bool printerFriendly)
{
  m_printerFriendly = printerFriendly;
  setShowTabs(false);
  setShowSelections(false);
  setDrawCaret(false);
}

bool KateRenderer::paintTextLineBackground(QPainter& paint, int line, bool isCurrentLine, int xStart, int xEnd)
{
  if (isPrinterFriendly())
    return false;

  // font data
  KateFontStruct *fs = config()->fontStruct();

  // Normal background color
  QColor backgroundColor( config()->backgroundColor() );

  bool selectionPainted = false;
  /*if (showSelections() && m_view->lineSelected(line))
  {
    backgroundColor = config()->selectionColor();
    selectionPainted = true;
  }
  else*/
  {
    // paint the current line background if we're on the current line
    if (isCurrentLine)
      backgroundColor = config()->highlightedLineColor();

    // Check for mark background
    int markRed = 0, markGreen = 0, markBlue = 0, markCount = 0;

    // Retrieve marks for this line
    uint mrk = m_doc->mark( line );
    if (mrk)
    {
      for (uint bit = 0; bit < 32; bit++)
      {
        KTextEditor::MarkInterface::MarkTypes markType = (KTextEditor::MarkInterface::MarkTypes)(1<<bit);
        if (mrk & markType)
        {
          QColor markColor = config()->lineMarkerColor(markType);

          if (markColor.isValid()) {
            markCount++;
            markRed += markColor.red();
            markGreen += markColor.green();
            markBlue += markColor.blue();
          }
        }
      } // for
    } // Marks

    if (markCount) {
      markRed /= markCount;
      markGreen /= markCount;
      markBlue /= markCount;
      backgroundColor.setRgb(
        int((backgroundColor.red() * 0.9) + (markRed * 0.1)),
        int((backgroundColor.green() * 0.9) + (markGreen * 0.1)),
        int((backgroundColor.blue() * 0.9) + (markBlue * 0.1))
      );
    }
  } // background preprocessing

  // Draw line background
  paint.fillRect(0, 0, xEnd - xStart, fs->fontHeight, backgroundColor);

  return selectionPainted;
}

void KateRenderer::paintWhitespaceMarker(QPainter &paint, uint x, uint y)
{
  QPen penBackup( paint.pen() );
  paint.setPen( config()->tabMarkerColor() );
  paint.drawPoint(x,     y);
  paint.drawPoint(x + 1, y);
  paint.drawPoint(x,     y - 1);
  paint.setPen( penBackup );
}

/*KateSuperRange* KateRenderer::advanceSyntaxHL(const KTextEditor::Cursor& currentPos, KateSuperRange* currentRange) const
{
  bool outDebug = false;//currentPos.line() == 22;

  if (!currentRange)
    return 0L;

  KateSuperRange* tmpRange = 0L;
  // Check to see if we are exiting any ranges
  while (currentRange->end() <= currentPos) {
    if (outDebug)
      kdDebug() << k_funcinfo << currentRange << "Leaving range " << *currentRange << " (current col: " << currentPos.column() << ")" << endl;

    tmpRange = currentRange;

    Q_ASSERT(currentRange->parentRange());
    currentRange = currentRange->parentRange();
  }

  // We are now in a range which includes currentPos.
  // FIXME FIXME this needs to be asserted!
  Q_ASSERT(currentRange->includes(currentPos));

  // Check to see if we are entering any child ranges
  while (currentRange->childRanges().count()) {
    QValueList<KateSuperRange*>::ConstIterator it = currentRange->childRanges().begin();

    if (tmpRange) {
      it = currentRange->childRanges().find(tmpRange);
      tmpRange = 0L;
    }

    for (; it != currentRange->childRanges().end(); ++it)
      if ((*it)->includes(currentPos)) {
      currentRange = *it;

      if (outDebug)
        kdDebug() << k_funcinfo << currentRange << "Entering range " << *currentRange << " (current col: " << currentPos.column() << ")" << endl;

      goto doubleContinue;
      }

    // No matches; done...
      break;

    doubleContinue:
          continue;
  }

  return currentRange;
}

KateAttribute merge(QPtrList<KateRange> ranges)
{
  ranges.sort();

  KateAttribute ret;

  for (KateSuperRange* r = ranges.first(); r; r = ranges.next())
    if (r->attribute())
      ret += *(r->attribute());

  return ret;
}*/

class RenderRange {
  public:
    RenderRange(KateSuperRange* range = 0L)
      : m_currentRange(0L)
    {
      // Might happen if we ask for a non-existing range in RenderRangeList
      //Q_ASSERT(range);
      if (range) {
        addTo(range);
        m_currentPos = range->start();
      }
    }

    KTextEditor::Cursor nextBoundary() const
    {
      KateSuperRange* r = m_currentRange->deepestRangeIncluding(m_currentPos);
      foreach (KateSuperRange* child, m_currentRange->childRanges()) {
        if (child->start() > m_currentPos)
          return child->start();
      }
      return m_currentRange->end();
    }

    bool advanceTo(const KTextEditor::Cursor& pos) const
    {
      if (!m_currentRange)
        return false;

      bool ret = false;

      while (m_currentRange && !m_currentRange->includes(pos)) {
        m_attribs.pop();
        m_currentRange = m_currentRange->parentRange();
        ret = true;
      }

      Q_ASSERT(m_currentRange);

      KateSuperRange* r = m_currentRange->deepestRangeIncluding(pos);
      if (r != m_currentRange)
        ret = true;

      if (r)
        addTo(r);

      return ret;
    }

    KateAttribute* currentAttribute() const
    {
      if (m_attribs.count())
        return const_cast<KateAttribute*>(&m_attribs.top());
      return 0L;
    }

  private:
    void addTo(KateSuperRange* range) const
    {
      KateSuperRange* r = range;
      QStack<KateSuperRange*> reverseStack;
      while (r != m_currentRange) {
        reverseStack.append(r);
        r = r->parentRange();
      }

      KateAttribute a;
      while (reverseStack.count()) {
        if (KateAttribute* a2 = reverseStack.top()->attribute())
          a += *a2;
        m_attribs.append(a);
        reverseStack.pop();
      }

      m_currentRange = range;
    }

    mutable KateSuperRange* m_currentRange;
    mutable KTextEditor::Cursor m_currentPos;
    mutable QStack<KateAttribute> m_attribs;
};

class RenderRangeList : public QList<RenderRange>
{
  public:
    RenderRangeList(const QList<KateSuperRange*>& startingRanges)
    {
      foreach (KateSuperRange* range, startingRanges)
        append(RenderRange(range));
    }

    KTextEditor::Cursor nextBoundary() const
    {
      KTextEditor::Cursor ret = m_currentPos;
      bool first = true;
      foreach (const RenderRange& r, *this) {
        if (first) {
          ret = r.nextBoundary();
          first = false;

        } else {
          KTextEditor::Cursor nb = r.nextBoundary();
          if (ret > nb)
            ret = nb;
        }
      }
      return ret;
    }

    bool advanceTo(const KTextEditor::Cursor& pos) const
    {
      bool ret = false;

      foreach (const RenderRange& r, *this)
        ret |= r.advanceTo(pos);

      return ret;
    }

    KateAttribute generateAttribute() const
    {
      KateAttribute a;

      foreach (const RenderRange& r, *this)
        if (KateAttribute* a2 = r.currentAttribute())
          a += *a2;

      return a;
    }
    
  private:
    KTextEditor::Cursor m_currentPos;
};

void KateRenderer::paintIndentMarker(QPainter &paint, uint x, uint row)
{
  QPen penBackup( paint.pen() );
  paint.setPen( config()->tabMarkerColor() );

  const int height = config()->fontStruct()->fontHeight;
  const int top = 0;
  const int bottom = height-1;
  const int h = bottom - top + 1;

  // Dot padding.
  int pad = 0;
  if(row & 1 && h & 1) pad = 1;

  for(int i = top; i <= bottom; i++)
  {
    if((i + pad) & 1)
    {
      paint.drawPoint(x + 2, i);
    }
  }

  paint.setPen( penBackup );
}

void KateRenderer::paintTextLine(QPainter& paint, const KateLineRange* range, int xStart, int xEnd, const KTextEditor::Cursor* cursor)
{
  int line = range->line();

  // textline
  KateTextLine::Ptr textLine = range->textLine();

  Q_ASSERT(textLine);
  if (!textLine)
    return;

  bool showCursor = drawCaret() && cursor && range->includesCursor(*cursor);

  RenderRangeList renderRanges(m_doc->arbitraryHL()->startingRanges(range->rangeStart(), m_view));

  // length, chars + raw attribs
  uint len = textLine->length();
  uint oldLen = len;

  // should the cursor be painted (if it is in the current xstart - xend range)
  bool cursorVisible = false;
  int cursorMaxWidth = 0;

  // font data
  KateFontStruct * fs = config()->fontStruct();

  // Paint selection background as the whole line is selected
  // selection startcol/endcol calc
  bool hasSel = false;
  uint startSel = 0;
  uint endSel = 0;

  int minIndent = 0;

  // was the selection background already completely painted ?
  bool selectionPainted = false;
  bool isCurrentLine = (cursor && range->includesCursor(*cursor));
  selectionPainted = paintTextLineBackground(paint, line, isCurrentLine, xStart, xEnd);
  if (selectionPainted)
  {
    hasSel = true;
    startSel = 0;
    endSel = len + 1;
  }

  int startcol = range->startCol();
  if (startcol > (int)len)
    startcol = len;

  if (startcol < 0)
    startcol = 0;

  int endcol = range->wrap() ? range->endCol() : -1;
  if (endcol < 0)
    len = len - startcol;
  else
    len = endcol - startcol;

  // text attribs font/style data
  KateAttribute* attr = m_doc->highlight()->attributes(m_schema)->data();

  const QColor *cursorColor = &attr[0].textColor();

  // Start arbitrary highlighting
  KTextEditor::Cursor currentPos(line, startcol);
  KTextEditor::Cursor nextPos(line, startcol + 1);
  KateAttribute currentHL;

  if (showSelections() && !selectionPainted)
    hasSel = getSelectionBounds(line, oldLen, startSel, endSel);

  // Draws the dashed underline at the start of a folded block of text.
  if (range->startsInvisibleBlock()) {
    paint.setPen(QPen(config()->wordWrapMarkerColor(), 1, Qt::DashLine));
    paint.drawLine(0, fs->fontHeight - 1, xEnd - xStart, fs->fontHeight - 1);
  }

  // draw word-wrap-honor-indent filling
  if (range->xOffset() && range->xOffset() > xStart)
  {
    paint.fillRect(0, 0, range->xOffset() - xStart, fs->fontHeight,
      QBrush(config()->wordWrapMarkerColor(), Qt::DiagCrossPattern));
  }

  KateAttribute customHL = renderRanges.generateAttribute();

  if (range->layout()) {
    if (len > 0) {
      // Set up the selection backgrounds
      QVector<QTextLayout::FormatRange> selections;
      if (hasSel) {
        const QVector<int> &al = range->textLine()->attributesList();
        for (uint i = 0; i+2 < al.count(); i += 3) {
          if (al[i] + al[i+1] <= startSel)
            continue;

          QTextLayout::FormatRange fr;
          fr.start = QMAX(al[i], (int)startSel) - range->startCol();
          fr.length = QMIN(al[i+1], (int)endSel - range->startCol() - fr.start);
          fr.format.setBackground(config()->selectionColor());

          KateAttribute* a = specificAttribute(al[i+2]);
          if (a->itemSet(KateAttribute::SelectedTextColor))
            fr.format.setForeground(a->selectedTextColor());

          selections.append(fr);

          if (al[i+1] > endSel)
            break;
        }
      }

      // Draw the text :)
      range->layout()->draw(&paint, QPoint(-xStart,0), selections);
    }

    // Draw selection outside of areas where text is rendered
    if (hasSel && m_view->lineEndSelected(range->line(), range->endCol())) {
      QRect area((len ? range->endX() - range->startX() + range->xOffset() - xStart : 0), 0, xEnd - xStart, fs->fontHeight);
      paint.fillRect(area, config()->selectionColor());
    }

    // Draw cursor
    if (showCursor) {
      paint.save();

      // Make the cursor the desired width
      uint cursorWidth = (caretStyle() == Replace && (cursorMaxWidth > 2)) ? cursorMaxWidth : 2;
      paint.setPen(QPen(*cursorColor, cursorWidth));

      // Draw the cursor, start drawing in the middle as the above sets the width from the centre of the line
      range->layout()->drawCursor(&paint, QPoint(cursorWidth/2 - xStart,0), cursor->column() - range->startCol());

      paint.restore();
    }
  }

  // Optimisation to quickly draw an empty line of text
  /*if (len < 1)
  {
    if (showCursor && (cursor->column() >= int(startcol)))
    {
      cursorVisible = true;
      cursorXPos = xPos + cursor->column() * fs->myFontMetrics.width(spaceChar);
    }
  }
  else
  {
    bool isIMSel  = false;
    bool isIMEdit = false;

    bool isSel = false;

    KateAttribute customHL;

    const QColor *curColor = 0;
    const QColor *oldColor = 0;

    KateAttribute* oldAt = &attr[0];

    uint oldXPos = xPos;
    uint xPosAfter = xPos;

    KateAttribute currentHL;

    uint blockStartCol = startcol;
    uint curCol = startcol;
    uint nextCol = curCol + 1;

    // text + attrib data from line
    const uchar *textAttributes = textLine->attributes ();
    bool noAttribs = !textAttributes;

    // adjust to startcol ;)
    textAttributes = textAttributes + startcol;

    uint atLen = m_doc->highlight()->attributes(m_schema)->size();

    // Determine if we have trailing whitespace and store the column
    // if lastChar == -1, set to 0, if lastChar exists, increase by one
    uint trailingWhitespaceColumn = textLine->lastChar() + 1;
    const uint lastIndentColumn = textLine->firstChar();

    // Could be precomputed.
    const uint spaceWidth = fs->width (spaceChar, false, false, m_tabWidth);

    // Get current x position.
    int curPos = textLine->cursorX(curCol, m_tabWidth);

    while (curCol - startcol < len)
    {
      // make sure curPos is updated correctly.
      Q_ASSERT(curPos == textLine->cursorX(curCol, m_tabWidth));

      QChar curChar = textLine->string()[curCol];
      // Decide if this character is a tab - we treat the spacing differently
      // TODO: move tab width calculation elsewhere?
      bool isTab = curChar == tabChar;

      // Determine current syntax highlighting attribute
      // A bit legacy but doesn't need to change
      KateAttribute* curAt = (noAttribs || ((*textAttributes) >= atLen)) ? &attr[0] : &attr[*textAttributes];

      // X position calculation. Incorrect for fonts with non-zero leftBearing() and rightBearing() results.
      // TODO: make internal charWidth() function, use QFontMetrics::charWidth().
      xPosAfter += curAt->width(*fs, curChar, m_tabWidth);

      // Tab special treatment, move to charWidth().
      if (isTab)
        xPosAfter -= (xPosAfter % curAt->width(*fs, curChar, m_tabWidth));

      // Only draw after the starting X value
      if ((int)xPosAfter >= xStart)
      {
        // Determine if we're in a selection and should be drawing it
        isSel = (showSelections() && hasSel && (curCol >= startSel) && (curCol < endSel));

        // input method edit area
        isIMEdit = m_view && m_view->isIMEdit( line, curCol );

        // input method selection
        isIMSel = m_view && m_view->isIMSelection( line, curCol );

        // Determine current color, taking into account selection
        curColor = isSel ? &(curAt->selectedTextColor()) : &(curAt->textColor());

        bool hitArbitraryHLBoundary = renderRanges.advanceTo(nextPos);

        // Incorporate in arbitrary highlighting
        if (curAt != oldAt || curColor != oldColor || hitArbitraryHLBoundary) {
          KateAttribute hl = *curAt;

          hl += customHL;

          // use default highlighting color if we haven't defined one above.
          if (!hl.itemSet(KateAttribute::TextColor))
            hl.setTextColor(*curColor);

          if (!isSel)
            paint.setPen(hl.textColor());
          else
            paint.setPen(hl.selectedTextColor());

          paint.setFont(hl.font(*currentFont()));

          if (hitArbitraryHLBoundary)
            customHL = renderRanges.generateAttribute();

          currentHL = hl;
        }

        // Determine whether we can delay painting to draw a block of similarly formatted
        // characters or not
        // Reasons for NOT delaying the drawing until the next character
        // You have to detect the change one character in advance.
        // TODO: KateAttribute::canBatchRender()
        bool renderNow = false;
        if ((isTab)
          // formatting has changed OR
          || (hitArbitraryHLBoundary)

          // it is the end of the line OR
          || (curCol >= len - 1)

          // the rest of the line is trailing whitespace OR
          || (curCol + 1 >= trailingWhitespaceColumn)

          // indentation lines OR
          || (showIndentLines() && curCol < lastIndentColumn)

          // the x position is past the end OR
          || ((int)xPos > xEnd)

          // it is a different attribute OR
          || (!noAttribs && curAt != &attr[*(textAttributes+1)])

          // the selection boundary was crossed OR
          || (isSel != (hasSel && (nextCol >= startSel) && (nextCol < endSel)))

          // the next char is a tab (removed the "and this isn't" because that's dealt with above)
          // i.e. we have to draw the current text so the tab can be rendered as above.
          || (textLine->string()[nextCol] == tabChar)

          // input method edit area
          || ( m_view && (isIMEdit != m_view->isIMEdit( line, nextCol )) )

          // input method selection
          || ( m_view && (isIMSel !=  m_view->isIMSelection( line, nextCol )) )
        )
        {
          renderNow = true;
        }

        if (renderNow)
        {
          if (!isPrinterFriendly())
          {
            bool paintBackground = true;
            uint width = xPosAfter - oldXPos;
            QColor fillColor;

            if (isIMSel && !isTab)
            {
              // input method selection
              fillColor = m_view->colorGroup().color(QColorGroup::Foreground);
            }
            else if (isIMEdit && !isTab)
            {
              // XIM support
              // input method edit area
              const QColorGroup& cg = m_view->colorGroup();
              int h1, s1, v1, h2, s2, v2;
              cg.color( QColorGroup::Base ).hsv( &h1, &s1, &v1 );
              cg.color( QColorGroup::Background ).hsv( &h2, &s2, &v2 );
              fillColor.setHsv( h1, s1, ( v1 + v2 ) / 2 );
            }
            else if (!selectionPainted && (isSel || currentHL.itemSet(KateAttribute::BGColor)))
            {
              if (isSel)
              {
                fillColor = config()->selectionColor();

                // If this is the last block of text, fill up to the end of the line if the
                // selection stretches that far
                if ((curCol >= len - 1) && m_view->lineEndSelected (line, endcol))
                  width = xEnd - oldXPos;
              }
              else
              {
                fillColor = currentHL.bgColor();
              }
            }
            else
            {
              paintBackground = false;
            }

            if (paintBackground)
              paint.fillRect(oldXPos - xStart, 0, width, fs->fontHeight, fillColor);

            if (isIMSel && paintBackground && !isTab)
            {
              paint.save();
              paint.setPen( m_view->colorGroup().color( QColorGroup::BrightText ) );
            }

            // Draw indentation markers.
            if (showIndentLines() && curCol < lastIndentColumn)
            {
              // Draw multiple guides when tab width greater than indent width.
              const int charWidth = isTab ? m_tabWidth - curPos % m_tabWidth : 1;

              // Do not draw indent guides on the first line.
              int i = 0;
              if (curPos == 0 || curPos % m_indentWidth > 0)
                i = m_indentWidth - curPos % m_indentWidth;

              for (; i < charWidth; i += m_indentWidth)
              {
                // In most cases this is done one or zero times.
                paintIndentMarker(paint, xPos - xStart + i * spaceWidth, line);

                // Draw highlighted line.
                if (curPos+i == minIndent)
                {
                  paintIndentMarker(paint, xPos - xStart + 1 + i * spaceWidth, line+1);
                }
              }
            }
          }

          // or we will see no text ;)
          int y = fs->fontAscent;

          // make sure we redraw the right character groups on attrib/selection changes
          // Special case... de-special case some of it
          if (isTab || (curCol >= trailingWhitespaceColumn))
          {
            // Draw spaces too, because it might be eg. underlined
            static QString spaces;
            if (int(spaces.length()) != m_tabWidth)
              spaces.fill(' ', m_tabWidth);

            paint.drawText(oldXPos-xStart, y, isTab ? spaces : QString(" "));

            if (showTabs())
            {
            // trailing spaces and tabs may also have to be different options.
            //  if( curCol >= lastIndentColumn )
              paintWhitespaceMarker(paint, xPos - xStart, y);
            }

            // variable advancement
            blockStartCol = nextCol;
            oldXPos = xPosAfter;
          }
          else
          {
            // Here's where the money is...
            paint.drawText(oldXPos-xStart, y, textLine->string(), blockStartCol, nextCol-blockStartCol);

            // Draw preedit's underline
            if (isIMEdit) {
              QRect r( oldXPos - xStart, 0, xPosAfter - oldXPos, fs->fontHeight );
              paint.drawLine( r.bottomLeft(), r.bottomRight() );
            }

            // Put pen color back
            if (isIMSel) paint.restore();

            // We're done drawing?
            if ((int)xPos > xEnd)
              break;

            // variable advancement
            blockStartCol = nextCol;
            oldXPos = xPosAfter;
            //oldS = s+1;
          }
        } // renderNow

        // determine cursor X position
        if (showCursor && (cursor->column() == int(curCol)))
        {
          cursorVisible = true;
          cursorXPos = xPos;
          cursorMaxWidth = xPosAfter - xPos;
          cursorColor = &curAt->textColor();
        }
      } // xPosAfter >= xStart
      else
      {
        // variable advancement
        blockStartCol = nextCol;
        oldXPos = xPosAfter;
      }

      // increase xPos
      xPos = xPosAfter;

      // increase attribs pos
      textAttributes++;

      // to only switch font/color if needed
      oldAt = curAt;
      oldColor = curColor;

      // col move
      curCol++;
      nextCol++;
      currentPos.setColumn(currentPos.column() + 1);
      nextPos.setColumn(nextPos.column() + 1);

      //syntaxHL = nextSyntaxHL;
      //nextSyntaxHL = advanceSyntaxHL(nextPos, syntaxHL);

      // Update the current indentation pos.
      if (isTab)
      {
        curPos += m_tabWidth - (curPos % m_tabWidth);
      }
      else
      {
        curPos++;
      }
    }

    // Determine cursor position (if it is not within the range being drawn)
    if (showCursor && (cursor->column() >= int(curCol)))
    {
      cursorVisible = true;
      cursorXPos = xPos + (cursor->column() - int(curCol)) * fs->myFontMetrics.width(spaceChar);
      cursorMaxWidth = xPosAfter - xPos;
      cursorColor = &oldAt->textColor();
    }
  }*/

  // Paint cursor
  /*if (cursorVisible)
  {
    uint cursorWidth = (caretStyle() == Replace && (cursorMaxWidth > 2)) ? cursorMaxWidth : 2;
    paint.fillRect(cursorXPos-xStart, 0, cursorWidth, fs->fontHeight, *cursorColor);
  }*/

  // show word wrap marker if desirable
  if (!isPrinterFriendly() && config()->wordWrapMarker() && fs->fixedPitch())
  {
    paint.setPen( config()->wordWrapMarkerColor() );
    int _x = m_doc->config()->wordWrapAt() * fs->myFontMetrics.width('x') - xStart;
    paint.drawLine( _x,0,_x,fs->fontHeight );
  }
}

uint KateRenderer::textWidth(const KateTextLine::Ptr &textLine, int cursorCol)
{
  if (!textLine)
    return 0;

  int len = textLine->length();

  if (cursorCol < 0)
    cursorCol = len;

  KateFontStruct *fs = config()->fontStruct();

  int x = 0;
  int width;
  for (int z = 0; z < cursorCol; z++) {
    KateAttribute* a = attribute(textLine->attribute(z));

    if (z < len) {
      width = a->width(*fs, textLine->string(), z, m_tabWidth);
    } else {
      // DF: commented out. It happens all the time.
      /**
       * Rodda: If this is hit, there is a bug.
       * What this means is that the cursor's position in the line is past the
       * end of the line. When we haven't got the "wrap cursor" mode on, this
       * should never happen.  If you can trigger this, find out why, and fix
       * it.
       */
      //Q_ASSERT(!m_doc->wrapCursor());
      width = a->width(*fs, spaceChar, m_tabWidth);
    }

    x += width;

    if (textLine->getChar(z) == tabChar)
      x -= x % width;
  }

  return x;
}

uint KateRenderer::textWidth(const KateTextLine::Ptr &textLine, uint startcol, uint maxwidth, bool *needWrap, int *endX)
{
  KateFontStruct *fs = config()->fontStruct();
  uint x = 0;
  uint endcol = startcol;
  int endX2 = 0;
  int lastWhiteSpace = -1;
  int lastWhiteSpaceX = -1;

  // used to not wrap a solitary word off the first line, ie. the
  // first line should not wrap until some characters have been displayed if possible
  bool foundNonWhitespace = startcol != 0;
  bool foundWhitespaceAfterNonWhitespace = startcol != 0;

  *needWrap = false;

  uint z = startcol;
  for (; z < textLine->length(); z++)
  {
    KateAttribute* a = attribute(textLine->attribute(z));
    int width = a->width(*fs, textLine->string(), z, m_tabWidth);
    Q_ASSERT(width);
    x += width;

    if (textLine->getChar(z).isSpace())
    {
      lastWhiteSpace = z+1;
      lastWhiteSpaceX = x;

      if (foundNonWhitespace)
        foundWhitespaceAfterNonWhitespace = true;
    }
    else
    {
      if (!foundWhitespaceAfterNonWhitespace) {
        foundNonWhitespace = true;

        lastWhiteSpace = z+1;
        lastWhiteSpaceX = x;
      }
    }

    // How should tabs be treated when they word-wrap on a print-out?
    // if startcol != 0, this messes up (then again, word wrapping messes up anyway)
    if (textLine->getChar(z) == tabChar)
      x -= x % width;

    if (x <= maxwidth)
    {
      if (lastWhiteSpace > -1)
      {
        endcol = lastWhiteSpace;
        endX2 = lastWhiteSpaceX;
      }
      else
      {
        endcol = z+1;
        endX2 = x;
      }
    }
    else if (z == startcol)
    {
      // require a minimum of 1 character advancement per call, even if it means drawing gets cut off
      // (geez gideon causes troubles with starting the views very small)
      endcol = z+1;
      endX2 = x;
    }

    if (x >= maxwidth)
    {
      *needWrap = true;
      break;
    }
  }

  if (*needWrap)
  {
    if (endX)
      *endX = endX2;

    return endcol;
  }
  else
  {
    if (endX)
      *endX = x;

    return z+1;
  }
}

uint KateRenderer::textWidth(const KTextEditor::Cursor &cursor)
{
  int line = QMIN(QMAX(0, cursor.line()), (int)m_doc->lines() - 1);
  int col = QMAX(0, cursor.column());

  return textWidth(m_doc->kateTextLine(line), col);
}

uint KateRenderer::constrainCursor( KTextEditor::Cursor &cursor, int xPos, uint startCol)
{
  bool wrapCursor = m_view->wrapCursor();
  int len;
  int x, oldX;

  KateFontStruct *fs = config()->fontStruct();

  if (cursor.line() < 0) cursor.setLine(0);
  if (cursor.line() > (int)m_doc->lastLine()) cursor.setLine(m_doc->lastLine());
  KateTextLine::Ptr textLine = m_doc->kateTextLine(cursor.line());

  if (!textLine) return 0;

  len = textLine->length();

  x = oldX = 0;
  int z = startCol;
  while (x < xPos && (!wrapCursor || z < len)) {
    oldX = x;

    KateAttribute* a = attribute(textLine->attribute(z));

    int width = 0;

    if (z < len)
      width = a->width(*fs, textLine->string(), z, m_tabWidth);
    else
      width = a->width(*fs, spaceChar, m_tabWidth);

    x += width;

    if (textLine->getChar(z) == tabChar)
      x -= x % width;

    z++;
  }
  if (xPos - oldX < x - xPos && z > 0) {
    z--;
    x = oldX;
  }
  cursor.setColumn(z);
  return x;
}

const QFont *KateRenderer::currentFont() const
{
  return config()->font();
}

const QFontMetrics* KateRenderer::currentFontMetrics() const
{
  return config()->fontMetrics();
}

uint KateRenderer::textPos(uint line, int xPos, uint startCol, bool nearest)
{
  return textPos(m_doc->kateTextLine(line), xPos, startCol, nearest);
}

uint KateRenderer::textPos(const KateTextLine::Ptr &textLine, int xPos, uint startCol, bool nearest)
{
  Q_ASSERT(textLine);
  if (!textLine)
    return 0;

  KateFontStruct *fs = config()->fontStruct();

  int x, oldX;
  x = oldX = 0;

  uint z = startCol;
  uint len= textLine->length();
  while ( (x < xPos)  && (z < len)) {
    oldX = x;

    KateAttribute* a = attribute(textLine->attribute(z));
    x += a->width(*fs, textLine->string(), z, m_tabWidth);

    z++;
  }
  if ( ( (! nearest) || xPos - oldX < x - xPos ) && z > 0 ) {
    z--;
   // newXPos = oldX;
  }// else newXPos = x;
  return z;
}

uint KateRenderer::fontHeight()
{
  return config()->fontStruct ()->fontHeight;
}

uint KateRenderer::documentHeight()
{
  return m_doc->lines() * fontHeight();
}

bool KateRenderer::getSelectionBounds(uint line, uint lineLength, uint &start, uint &end)
{
  bool hasSel = false;

  if (m_view->hasSelection() && !m_view->blockSelectionMode())
  {
    if (m_view->lineIsSelection(line))
    {
      start = m_view->selectionStartColumn();
      end = m_view->selectionEndColumn();
      hasSel = true;
    }
    else if ((int)line == m_view->selectionStartLine())
    {
      start = m_view->selectionStartColumn();
      end = lineLength;
      hasSel = true;
    }
    else if ((int)line > m_view->selectionStartLine() && (int)line < m_view->selectionEndLine())
    {
      start = 0;
      end = lineLength;
      hasSel = true;
    }
    else if ((int)line == m_view->selectionEndLine())
    {
      start = 0;
      end = m_view->selectionEndColumn();
      hasSel = true;
    }
  }
  else if (m_view->lineHasSelected(line))
  {
    start = m_view->selectionStartColumn();
    end = m_view->selectionEndColumn();
    hasSel = true;
  }

  if (start > end) {
    int temp = end;
    end = start;
    start = temp;
  }

  return hasSel;
}

void KateRenderer::updateConfig ()
{
  // update the attibute list pointer
  updateAttributes ();

  if (m_view)
    m_view->updateRendererConfig();
}

uint KateRenderer::spaceWidth()
{
  return attribute(0)->width(*config()->fontStruct(), spaceChar, m_tabWidth);
}

void KateRenderer::layoutLine(KateLineRange& range, int maxwidth) const
{
  // if maxwidth == -1 we have no wrap

  if (!range.textLine() || range.textLine()->string().isEmpty()) {
    Q_ASSERT(range.textLine());
    range.setEndCol(0);
    range.setEndX(0);
    range.setWrap(false);
    return;
  }

  range.setLayout(new QTextLayout(range.textLine()->string().mid(range.startCol()), config()->fontStruct()->font(false, false)), true);

  Q_ASSERT(currentFont());

  QTextLayout* l = range.layout();
  l->setCacheEnabled(true);

  // Initial setup of the QTextLayout.

  // Tab width
  QTextOption opt;
  opt.setTabStop(m_doc->config()->tabWidth());
  l->setTextOption(opt);

  // FIXME update to new api... Retrieve decoration range list
  /*KateRangeList& ranges = m_doc->arbitraryHL()->rangesIncluding(range.line(), m_view);

  KTextEditor::Cursor rangeStart = range.rangeStart();

  for (KTextEditor::Cursor* r = ranges.firstBoundary(&rangeStart); r && r->line() == range.line(); r = ranges.nextBoundary())
    l->setBoundary(r->column());*/

  // Initialise syntax highlighting ranges
  QList<QTextLayout::FormatRange> formatRanges;
  const QVector<int> &al = range.textLine()->attributesList();
  for (int i = 0; i+2 < al.count(); i += 3) {
    QTextLayout::FormatRange fr;
    fr.start = al[i] - range.startCol();
    fr.length = al[i+1];
    fr.format = specificAttribute(al[i+2])->toFormat();

    //kdDebug() << k_funcinfo << "Adding attribute from " << fr.start << " length " << fr.length << " (colour "<< fr.format.foreground() <<" bold " << fr.format.fontWeight() << ")" << endl;
    formatRanges.append(fr);
  }
  l->setAdditionalFormats(formatRanges);

  // Begin layouting
  l->beginLayout();

  QTextLine line = l->createLine();
  if (line.isValid()) {
    if (maxwidth > 0)
      line.setLineWidth(maxwidth);

    //if (range.startX())
      //kdDebug() << "offset " << range.xOffset() << " pixels " << (range.xOffset() * spaceWidth()) << endl;

    line.setPosition(QPoint(range.xOffset(), 0));
  }

  range.setEndCol(range.startCol() + line.textLength());
  range.setEndX(range.startX() + line.naturalTextWidth());
  range.setWrap(range.endCol() < range.textLine()->length());

  l->endLayout();
}

int KateRenderer::cursorToX( KateLineRange & range, int col, bool doLayout, int maxwidth ) const
{
  return cursorToX(range, KTextEditor::Cursor(range.line(), col), doLayout, maxwidth);
}

int KateRenderer::cursorToX( KateLineRange & range, const KTextEditor::Cursor & pos, bool doLayout, int maxwidth ) const
{
  //kdDebug() << k_funcinfo << pos.column() << " doLayout " << doLayout << endl;
  //range.debugOutput();
  if (!range.layout() && !doLayout)
    return -1;
    
  if (!range.layout() || !range.includesCursor(pos)) {
    do {
      range.setStartCol(range.endCol());
      range.setStartX(range.endX());
      layoutLine(range, maxwidth);
      // FIXME most likely an infinite loop here      
    } while (!range.includesCursor(pos));
  }

  if (range.isEmpty())
    if (pos.column() == 0)
      return 0;
    else
      return -1;

  return range.layout()->lineAt(0).cursorToX(pos.column() - range.startCol());
}

int KateRenderer::cursorToX( const KTextEditor::Cursor & pos, int maxwidth ) const
{
  KateLineRange range(m_doc);
  range.setLine(pos.line());
  return cursorToX(range, pos, true, maxwidth);
}

int KateRenderer::xToCursor( KateLineRange & range, int x ) const
{
  // TODO: do we want to return endCol for out-of-bounds requests?

  if (!range.layout()) {
    if (range.isEmpty())
      return 0;

    kdDebug() << k_funcinfo << range.layout() << " " << x << endl;
    return -1;
  }

  if (x <  range.xOffset())
    x = range.xOffset();
  else if (x > range.width() + range.xOffset())
    x = range.width() + range.xOffset();

  return range.layout()->lineAt(0).xToCursor(x) + range.startCol();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
