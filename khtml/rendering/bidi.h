/*
 * Handling of bidirectional text.
 *
 * (c) 2000 Lars Knoll (knoll@kde.org)
 *
 * This file is Licensed under QPL version 1.
 */
#ifndef BIDI_H
#define BIDI_H

#include <qstring.h>

/* structure describing one part of a stream of BiDi objects.
 */
class BiDiObject
{
public:
    BiDiObject() {}
    virtual ~BiDiObject() {}

    virtual bool isText() const { return false; }
    virtual bool isHidden() const { return false; }
    virtual bool isSpecial() const { return false; }

    virtual const QChar *text() const { return 0; }
    virtual unsigned int length() const { return 1; }

    /* should return the width of the object. the parameters can be ignored for
     * non text objects, for text objects it should return the width of the character
     * at position pos
     */
    virtual unsigned int width( int pos) const { return width(pos, 1); }

    /* should return the width of the object. the parameters can be ignored for
     * non text objects, for text objects it should return the width needed to print
     * the text starting at position from, with length length.
     */
    virtual unsigned int width( int /*from*/, int /*len*/) const = 0;

    enum VerticalPositionHint {
	PositionTop = -1,
	PositionBottom = -2
    };

    /*
     * Distance between the top of the object and it's baseline.
     * Negative values have a special meaning, see the enum above.
     */
    virtual short verticalPositionHint() const = 0;
    /*
     * the height of the box
     */
    virtual int bidiHeight() const = 0;

    virtual void position(int x, int y, int from, int len, int width, bool reverse) = 0;
};	

class BiDiParagraphPrivate;

/* a class describing one paragraph of BiDi text. You'll have to override lineWidth()
 * in case you need shaped paragraphs.
 */
class BiDiParagraph
{
public:
    BiDiParagraph();
    virtual ~BiDiParagraph();

    QChar::Direction basicDirection();
    void setBasicDirection( QChar::Direction d ) { m_basicDirPreset = d; }

    enum TextAlignment {
	AlignAuto,
	AlignLeft,
	AlignRight,
	AlignCenter,
	AlignJustify     // ### implement !!!
    };

    void setAlignment(TextAlignment align) { m_align = align; }
    TextAlignment alignment() const { return m_align; }

    // calculates the visual order of the paragraph
    // and does a layout of it
    // returns the height of the layouted region (+ the initial offset yOff)
    int reorder(int xOff = 0, int yOff = 0);

    virtual unsigned short lineWidth(int yPos) const = 0;

    virtual BiDiObject *first() = 0;
    virtual BiDiObject *next(BiDiObject *current) = 0;

    bool ignoreNewline() const { return m_ignoreNewline; }
    void setIgnoreNewline(bool b) { m_ignoreNewline = b; }

    bool ignoreLeadingSpaces() const { return m_ignoreLeadingSpaces; }
    void setIgnoreLeadingSpaces(bool b) { m_ignoreLeadingSpaces = b; }

    bool ignoreTrailingSpaces() const { return m_ignoreTrailingSpaces; }
    void setIgnoreTrailingSpaces(bool b) { m_ignoreTrailingSpaces = b; }

    /**
     * get's called, when objects with isSpecial() or isHidden() = true would
     * get inserted into the flow.
     * Objects with isHidden() = true will not get inserted.
     */
    virtual void specialHandler(BiDiObject */*special*/) {}

    void setVisualOrdering(bool b) { m_visualOrdering = b; }
    bool visualOrdering() { return m_visualOrdering; }

protected:

    /**
     * hook. will get calle everytime a new line is to be created
     */
    virtual void newLine() {}

    virtual short leftMargin(int y) const = 0;
    virtual void closeParagraph() {}

    unsigned int currentY();
    void setCurrentY(int y);

    void calcBasicDirection();
    void collectRuns();

    // closes the current run and creates a new one, based on current, lastStrong, dir and
    // m_currentEmbedding->level
    void newRun();

    // closes the current run
    void appendRun();

    void createNewLine();

    // does reordering and set's the x positions of the line relativ to the start of the line
    void layoutLine(unsigned char levelLow, unsigned char levelHigh, bool lastLine = false );

    QChar::Direction m_basicDirPreset;
    TextAlignment m_align;
    bool m_ignoreNewline;
    bool m_ignoreLeadingSpaces;
    bool m_ignoreTrailingSpaces;
    bool m_visualOrdering;

private:
    void breakLines(int xOff, int yOff);


    BiDiParagraphPrivate *d;
};


#endif
