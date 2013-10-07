/*
    Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2007, 2008 Rob Buis <buis@kde.org>

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

#include "wtf/Platform.h"

#if ENABLE(SVG)
#include "SVGTextContentElement.h"

#include "cssvalues.h"

/*#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"*/
#include "ExceptionCode.h"
#include "FloatPoint.h"
#include "FloatRect.h"
/*#include "Frame.h"
#include "Position.h"*/
#include "RenderSVGText.h"
/*#include "SelectionController.h"*/
#include "SVGCharacterLayoutInfo.h"
#include "SVGRootInlineBox.h"
#include "SVGLength.h"
#include "SVGInlineTextBox.h"
#include "SVGNames.h"

namespace WebCore {

SVGTextContentElement::SVGTextContentElement(const QualifiedName& tagName, Document* doc)
    : SVGStyledElement(tagName, doc)
    , SVGTests()
    , SVGLangSpace()
    , SVGExternalResourcesRequired()
    , m_textLength(this, LengthModeOther)
    , m_lengthAdjust(LENGTHADJUST_SPACING)
{
}

SVGTextContentElement::~SVGTextContentElement()
{
}

ANIMATED_PROPERTY_DEFINITIONS(SVGTextContentElement, SVGLength, Length, length, TextLength, textLength, SVGNames::textLengthAttr, m_textLength)
ANIMATED_PROPERTY_DEFINITIONS(SVGTextContentElement, int, Enumeration, enumeration, LengthAdjust, lengthAdjust, SVGNames::lengthAdjustAttr, m_lengthAdjust)

static inline float cumulativeCharacterRangeLength(const Vector<SVGChar>::iterator& start, const Vector<SVGChar>::iterator& end, SVGInlineTextBox* textBox,
                                                   int startOffset, long startPosition, long length, bool isVerticalText, long& atCharacter)
{
    if (!length)
        return 0.0f;

    float textLength = 0.0f;
    RenderStyle* style = textBox->renderText()->style();

    bool usesFullRange = (startPosition == -1 && length == -1);

    for (Vector<SVGChar>::iterator it = start; it != end; ++it) {
        if (usesFullRange || (atCharacter >= startPosition && atCharacter <= startPosition + length)) {
            unsigned int newOffset = textBox->start() + (it - start) + startOffset;

            // Take RTL text into account and pick right glyph width/height.
            /*FIXME khtml if (textBox->direction() == RTL)
                newOffset = textBox->start() + textBox->end() - newOffset;*/

            // FIXME: does this handle multichar glyphs ok? not sure
            int charsConsumed = 0;
            String glyphName;
            if (isVerticalText)
                textLength += textBox->calculateGlyphHeight(style, newOffset, 0);
            else
                textLength += textBox->calculateGlyphWidth(style, newOffset, 0, charsConsumed, glyphName);
        }

        if (!usesFullRange) {
            if (atCharacter == startPosition + length - 1)
                break;

            atCharacter++;
        }
    }

    return textLength;
}

// Helper class for querying certain glyph information
struct SVGInlineTextBoxQueryWalker {
    typedef enum {
        NumberOfCharacters,
        TextLength,
        SubStringLength,
        StartPosition,
        EndPosition,
        Extent,
        Rotation,
        CharacterNumberAtPosition
    } QueryMode;

    SVGInlineTextBoxQueryWalker(const SVGTextContentElement* reference, QueryMode mode)
        : m_reference(reference)
        , m_mode(mode)
        , m_queryStartPosition(0)
        , m_queryLength(0)
        , m_queryPointInput()
        , m_queryLongResult(0)
        , m_queryFloatResult(0.0f)
        , m_queryPointResult()
        , m_queryRectResult()
        , m_stopProcessing(true)
        , m_atCharacter(0)
    {
    }

    void chunkPortionCallback(SVGInlineTextBox* textBox, int startOffset, const AffineTransform& chunkCtm,
                              const Vector<SVGChar>::iterator& start, const Vector<SVGChar>::iterator& end)
    {
        Q_UNUSED(chunkCtm);
        RenderStyle* style = textBox->renderText()->style();
        bool isVerticalText = style->svgStyle()->writingMode() == WM_TBRL || style->svgStyle()->writingMode() == WM_TB;

        switch (m_mode) {
        case NumberOfCharacters:
        {
            m_queryLongResult += (end - start);
            m_stopProcessing = false;
            return;
        }
        case TextLength:
        {
            float textLength = cumulativeCharacterRangeLength(start, end, textBox, startOffset, -1, -1, isVerticalText, m_atCharacter);

            if (isVerticalText)
                m_queryFloatResult += textLength;
            else
                m_queryFloatResult += textLength;

            m_stopProcessing = false;
            return;
        }
        case SubStringLength:
        {
            long startPosition = m_queryStartPosition;
            long length = m_queryLength;

            float textLength = cumulativeCharacterRangeLength(start, end, textBox, startOffset, startPosition, length, isVerticalText, m_atCharacter);

            if (isVerticalText)
                m_queryFloatResult += textLength;
            else
                m_queryFloatResult += textLength;

            if (m_atCharacter == startPosition + length)
                m_stopProcessing = true;
            else
                m_stopProcessing = false;

            return;
        }
        case StartPosition:
        {
            for (Vector<SVGChar>::iterator it = start; it != end; ++it) {
                if (m_atCharacter == m_queryStartPosition) {
                    m_queryPointResult = FloatPoint(it->x, it->y);
                    m_stopProcessing = true;
                    return;
                }

                m_atCharacter++;
            }

            m_stopProcessing = false;
            return;
        }
        case EndPosition:
        {
            for (Vector<SVGChar>::iterator it = start; it != end; ++it) {
                if (m_atCharacter == m_queryStartPosition) {
                    unsigned int newOffset = textBox->start() + (it - start) + startOffset;

                    // Take RTL text into account and pick right glyph width/height.
                    /*FIXME khtml if (textBox->direction() == RTL)
                        newOffset = textBox->start() + textBox->end() - newOffset;*/

                    int charsConsumed;
                    String glyphName;
                    if (isVerticalText)
                        m_queryPointResult.move(it->x, it->y + textBox->calculateGlyphHeight(style, newOffset, end - it));
                    else
                        m_queryPointResult.move(it->x + textBox->calculateGlyphWidth(style, newOffset, end - it, charsConsumed, glyphName), it->y);

                    m_stopProcessing = true;
                    return;
                }

                m_atCharacter++;
            }

            m_stopProcessing = false;
            return;
        }
        case Extent:
        {
            for (Vector<SVGChar>::iterator it = start; it != end; ++it) {
                if (m_atCharacter == m_queryStartPosition) {
                    unsigned int newOffset = textBox->start() + (it - start) + startOffset;
                    m_queryRectResult = textBox->calculateGlyphBoundaries(style, newOffset, *it);
                    m_stopProcessing = true;
                    return;
                }

                m_atCharacter++;
            }

            m_stopProcessing = false;
            return;
        }
        case Rotation:
        {
            for (Vector<SVGChar>::iterator it = start; it != end; ++it) {
                if (m_atCharacter == m_queryStartPosition) {
                    m_queryFloatResult = it->angle;
                    m_stopProcessing = true;
                    return;
                }

                m_atCharacter++;
            }

            m_stopProcessing = false;
            return;
        }
        case CharacterNumberAtPosition:
        {
            int offset = 0;
            SVGChar* charAtPos = textBox->closestCharacterToPosition(m_queryPointInput.x(), m_queryPointInput.y(), offset);

            offset += m_atCharacter;
            if (charAtPos && offset > m_queryLongResult)
                m_queryLongResult = offset;

            m_atCharacter += end - start;
            m_stopProcessing = false;
            return;
        }
        default:
            ASSERT_NOT_REACHED();
            m_stopProcessing = true;
            return;
        }
    }

    void setQueryInputParameters(long startPosition, long length, FloatPoint referencePoint)
    {
        m_queryStartPosition = startPosition;
        m_queryLength = length;
        m_queryPointInput = referencePoint;
    }

    long longResult() const { return m_queryLongResult; }
    float floatResult() const { return m_queryFloatResult; }
    FloatPoint pointResult() const { return m_queryPointResult; }
    FloatRect rectResult() const { return m_queryRectResult; }
    bool stopProcessing() const { return m_stopProcessing; }

private:
    const SVGTextContentElement* m_reference;
    QueryMode m_mode;

    long m_queryStartPosition;
    long m_queryLength;
    FloatPoint m_queryPointInput;

    long m_queryLongResult;
    float m_queryFloatResult;
    FloatPoint m_queryPointResult;
    FloatRect m_queryRectResult;

    bool m_stopProcessing;
    long m_atCharacter;
};

static Vector<SVGInlineTextBox*> findInlineTextBoxInTextChunks(const SVGTextContentElement* element, const Vector<SVGTextChunk>& chunks)
{
    Vector<SVGTextChunk>::const_iterator it = chunks.begin();
    const Vector<SVGTextChunk>::const_iterator end = chunks.end();

    Vector<SVGInlineTextBox*> boxes;

    for (; it != end; ++it) {
        Vector<SVGInlineBoxCharacterRange>::const_iterator boxIt = it->boxes.begin();
        const Vector<SVGInlineBoxCharacterRange>::const_iterator boxEnd = it->boxes.end();

        for (; boxIt != boxEnd; ++boxIt) {
            SVGInlineTextBox* textBox = static_cast<SVGInlineTextBox*>(boxIt->box);

            Node* textElement = textBox->renderText()->parent()->element();
            ASSERT(textElement);

            if (textElement == element || textElement->parent() == element)
                boxes.append(textBox);
        }
    }

    return boxes;
}

static inline SVGRootInlineBox* rootInlineBoxForTextContentElement(const SVGTextContentElement* element)
{
    RenderObject* object = element->renderer();

    if (!object || !object->isSVGText() || object->isText())
        return 0;

    RenderSVGText* svgText = static_cast<RenderSVGText*>(object);

    // Find root inline box
    SVGRootInlineBox* rootBox = static_cast<SVGRootInlineBox*>(svgText->firstRootBox());
    if (!rootBox) {
        // Layout is not sync yet!
        /*FIXME khtml element->document()->updateLayoutIgnorePendingStylesheets();*/
        rootBox = static_cast<SVGRootInlineBox*>(svgText->firstRootBox());
    }

    ASSERT(rootBox);
    return rootBox;
}

static inline SVGInlineTextBoxQueryWalker executeTextQuery(const SVGTextContentElement* element, SVGInlineTextBoxQueryWalker::QueryMode mode,
                                                           long startPosition = 0, long length = 0, FloatPoint referencePoint = FloatPoint())
{
    SVGRootInlineBox* rootBox = rootInlineBoxForTextContentElement(element);
    if (!rootBox)
        return SVGInlineTextBoxQueryWalker(0, mode);

    // Find all inline text box associated with our renderer
    Vector<SVGInlineTextBox*> textBoxes = findInlineTextBoxInTextChunks(element, rootBox->svgTextChunks());

    // Walk text chunks to find chunks associated with our inline text box
    SVGInlineTextBoxQueryWalker walkerCallback(element, mode);
    walkerCallback.setQueryInputParameters(startPosition, length, referencePoint);

    SVGTextChunkWalker<SVGInlineTextBoxQueryWalker> walker(&walkerCallback, &SVGInlineTextBoxQueryWalker::chunkPortionCallback);

    Vector<SVGInlineTextBox*>::iterator it = textBoxes.begin();
    Vector<SVGInlineTextBox*>::iterator end = textBoxes.end();

    for (; it != end; ++it) {
        rootBox->walkTextChunks(&walker, *it);

        if (walkerCallback.stopProcessing())
            break;
    }

    return walkerCallback;
}

long SVGTextContentElement::getNumberOfChars() const
{
    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::NumberOfCharacters).longResult();
}

float SVGTextContentElement::getComputedTextLength() const
{
    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::TextLength).floatResult();
}

float SVGTextContentElement::getSubStringLength(long charnum, long nchars, ExceptionCode& ec) const
{
    // Differences to SVG 1.1 spec, as the spec is clearly wrong. TODO: Raise SVG WG issue!
    // #1: We accept a 'long nchars' parameter instead of 'unsigned long nchars' to be able
    //     to catch cases where someone called us with a negative 'nchars' value - in those
    //     cases we'll just throw a 'INDEX_SIZE_ERR' (acid3 implicitly agrees with us)
    //
    // #2: We only throw if 'charnum + nchars' is greater than the number of characters, not
    //     if it's equal, as this really doesn't make any sense (no way to measure the last character!)
    //
    // #3: If 'charnum' is greater than or equal to 'numberOfChars', we're throwing an exception here
    //     as the result is undefined for every other value of 'nchars' than '0'.

    long numberOfChars = getNumberOfChars();
    if (charnum < 0 || nchars < 0 || numberOfChars <= charnum || charnum + nchars > numberOfChars) {
        ec = DOMException::INDEX_SIZE_ERR;
        return 0.0f;
    }

    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::SubStringLength, charnum, nchars).floatResult();
}

FloatPoint SVGTextContentElement::getStartPositionOfChar(long charnum, ExceptionCode& ec) const
{
    if (charnum < 0 || charnum > getNumberOfChars()) {
        ec = DOMException::INDEX_SIZE_ERR;
        return FloatPoint();
    }

    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::StartPosition, charnum).pointResult();
}

FloatPoint SVGTextContentElement::getEndPositionOfChar(long charnum, ExceptionCode& ec) const
{
    if (charnum < 0 || charnum > getNumberOfChars()) {
        ec = DOMException::INDEX_SIZE_ERR;
        return FloatPoint();
    }

    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::EndPosition, charnum).pointResult();
}

FloatRect SVGTextContentElement::getExtentOfChar(long charnum, ExceptionCode& ec) const
{
    if (charnum < 0 || charnum > getNumberOfChars()) {
        ec = DOMException::INDEX_SIZE_ERR;
        return FloatRect();
    }

    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::Extent, charnum).rectResult();
}

float SVGTextContentElement::getRotationOfChar(long charnum, ExceptionCode& ec) const
{
    if (charnum < 0 || charnum > getNumberOfChars()) {
        ec = DOMException::INDEX_SIZE_ERR;
        return 0.0f;
    }

    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::Rotation, charnum).floatResult();
}

long SVGTextContentElement::getCharNumAtPosition(const FloatPoint& point) const
{
    return executeTextQuery(this, SVGInlineTextBoxQueryWalker::CharacterNumberAtPosition, 0.0f, 0.0f, point).longResult();
}

void SVGTextContentElement::selectSubString(long charnum, long nchars, ExceptionCode& ec) const
{
    long numberOfChars = getNumberOfChars();
    if (charnum < 0 || nchars < 0 || charnum > numberOfChars) {
        ec = DOMException::INDEX_SIZE_ERR;
        return;
    }

    if (nchars > numberOfChars - charnum)
        nchars = numberOfChars - charnum;

    ASSERT(document());
    //khtml ASSERT(document()->frame());

    /*FIXME SelectionController* controller = document()->frame()->selectionController();
    if (!controller)
        return;

    // Find selection start
    VisiblePosition start(const_cast<SVGTextContentElement*>(this), 0, SEL_DEFAULT_AFFINITY);
    for (long i = 0; i < charnum; ++i)
        start = start.next();

    // Find selection end
    VisiblePosition end(start);
    for (long i = 0; i < nchars; ++i)
        end = end.next();

    controller->setSelection(Selection(start, end));*/
}

void SVGTextContentElement::parseMappedAttribute(MappedAttribute* attr)
{
    if (attr->name() == SVGNames::lengthAdjustAttr) {
        if (attr->value() == "spacing")
            setLengthAdjustBaseValue(LENGTHADJUST_SPACING);
        else if (attr->value() == "spacingAndGlyphs")
            setLengthAdjustBaseValue(LENGTHADJUST_SPACINGANDGLYPHS);
    } else if (attr->name() == SVGNames::textLengthAttr) {
        setTextLengthBaseValue(SVGLength(this, LengthModeOther, attr->value()));
        if (textLength().value() < 0.0)
            document()->accessSVGExtensions()->reportError("A negative value for text attribute <textLength> is not allowed");
    } else {
        if (SVGTests::parseMappedAttribute(attr))
            return;
        if (SVGLangSpace::parseMappedAttribute(attr)) {
            if (attr->id() == ATTR_XML_SPACE) {
                static const DOMString preserveString("preserve");

                if (attr->value() == preserveString)
                    addCSSProperty(attr, CSS_PROP_WHITE_SPACE, CSS_VAL_PRE);
                else
                    addCSSProperty(attr, CSS_PROP_WHITE_SPACE, CSS_VAL_NOWRAP);
            }
            return;
        }
        if (SVGExternalResourcesRequired::parseMappedAttribute(attr))
            return;

        SVGStyledElement::parseMappedAttribute(attr);
    }
}

bool SVGTextContentElement::isKnownAttribute(const QualifiedName& attrName)
{
    return (attrName.matches(SVGNames::lengthAdjustAttr) ||
            attrName.matches(SVGNames::textLengthAttr) ||
            SVGTests::isKnownAttribute(attrName) ||
            SVGLangSpace::isKnownAttribute(attrName) ||
            SVGExternalResourcesRequired::isKnownAttribute(attrName) ||
            SVGStyledElement::isKnownAttribute(attrName));
}

}

#endif // ENABLE(SVG)
