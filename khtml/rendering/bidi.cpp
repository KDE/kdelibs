/*
 * Handling of bidirectional text.
 *
 * (c) 2000 Lars Knoll (knoll@kde.org)
 *
 * This file is Licensed under QPL version 1.
 */
// setting BIDI_DEBUG to 1 gives some information, 2 gives a _lot_
//#define BIDI_DEBUG 1

#include "bidi.h"

#include <qstack.h>
#include <qlist.h>

#include "stdio.h"

// an iterator which goes through a BiDiParagraph
class BiDiIterator
{
public:
    BiDiIterator();
    BiDiIterator(BiDiParagraph *par);
    BiDiIterator(BiDiParagraph *par, BiDiObject *_obj, int _pos = 0);

    BiDiIterator(const BiDiIterator &it);
    BiDiIterator &operator = (const BiDiIterator &it);

    void operator ++ ();

    bool atEnd();

    const QChar &current();
    QChar::Direction direction();
    int width();

    BiDiParagraph *par;
    BiDiObject *obj;
    unsigned int pos;

    enum Type {
	Unknown,
	WhiteSpace,
	NewLine,
	Hidden
    };

    Type type;
};

BiDiIterator::BiDiIterator()
{
    par = 0;
    obj = 0;
    pos = 0;
    type = Unknown;
}

BiDiIterator::BiDiIterator(BiDiParagraph *_par)
{
    par = _par;
    obj = par->first();
    pos = 0;
    type = Unknown;
}

BiDiIterator::BiDiIterator(const BiDiIterator &it)
{
    par = it.par;
    obj = it.obj;
    pos = it.pos;
    type = it.type;
}

BiDiIterator::BiDiIterator(BiDiParagraph *_par, BiDiObject *_obj, int _pos)
{
    par = _par;
    obj = _obj;
    pos = _pos;
    type = Unknown;
}

BiDiIterator &BiDiIterator::operator = (const BiDiIterator &it)
{
    obj = it.obj;
    pos = it.pos;
    par = it.par;
    type = it.type;
    return *this;
}

void BiDiIterator::operator ++ ()
{
    if(!obj) return;
    pos++;
    if(pos >= obj->length())
    {
	obj = par->next(obj);
	pos = 0;
    }
}

bool BiDiIterator::atEnd()
{
    if(!obj) return true;
    if(pos < obj->length() || par->next(obj)) return false;
    return true;
}

const QChar &BiDiIterator::current()
{
    static const QChar nbsp = QChar(0xA0);
    if(!obj || !obj->isText()) return nbsp; // non breaking space
    return obj->text()[pos];
}

QChar::Direction BiDiIterator::direction()
{
    if(!obj || !obj->isText()) return QChar::DirON;
    return obj->text()[pos].direction();
}

int BiDiIterator::width()
{
    if(!obj) return 0;
    return obj->width(pos);
}

inline bool operator==( const BiDiIterator &it1, const BiDiIterator &it2 )
{
    if(it1.pos != it2.pos) return false;
    if(it1.obj != it2.obj) return false;
    return true;
}

inline bool operator!=( const BiDiIterator &it1, const BiDiIterator &it2 )
{
    if(it1.pos != it2.pos) return true;
    if(it1.obj != it2.obj) return true;
    return false;
}

// the implementation, when objects are different is not efficient.
// on the other hand, this case should be rare
inline bool operator > ( const BiDiIterator &it1, const BiDiIterator &it2 )
{
    if(it1.obj != it2.obj)
    {
#if BIDI_DEBUG > 1
//	printf("BiDiIterator operator >: objects differ\n");
#endif
	BiDiObject *o = it2.obj;
	while(o)
	{
	    if(o == it1.obj) return true;
	    o = it2.par->next(o);
	}
#if BIDI_DEBUG > 1
//	printf("BiDiIterator operator >: false\n");
#endif
	return false;
    }
    return (it1.pos > it2.pos);
}

inline bool operator < ( const BiDiIterator &it1, const BiDiIterator &it2 )
{
    return (it2 > it1);
}

static int special(BiDiParagraph *par, const BiDiIterator &it1, const BiDiIterator &it2, int y)
{
    int oldline=0;
    int change=0;

    BiDiObject *o = it1.obj;
    if(!o) return 0;
    BiDiObject *last = it2.obj;

    while(o != last)
    {
	if(o->isSpecial() || o->isHidden()) 
	{
	    if (!oldline)   
	    	oldline=par->lineWidth(y);
	    par->specialHandler(o);	    
	}
	o = it1.par->next(o);
	if(!o) break;
    }
    if(o && (o->isSpecial() || o->isHidden())) 	
    {
	if (!oldline)   
	    oldline=par->lineWidth(y);
	par->specialHandler(o);	    
    }
    
    if (oldline)
    	change = par->lineWidth(y) - oldline;

#if BIDI_DEBUG > 1
    printf("special(%p, %p) = %d\n", it1.obj, it2.obj,  change);
#endif
    return change;


}

static int width(BiDiParagraph *par, const BiDiIterator &it1, const BiDiIterator &it2,
		 bool ignoreLast = false)
{
    int w = 0;
    int to;

    BiDiObject *o = it1.obj;
    if(!o) return 0;
    BiDiObject *last = it2.obj;
    int from = it1.pos;
    if(o != last)
    {
	to = o->length();
	while(o != last)
	{
	    if(!o->isHidden())
		w += o->width(from, to - from);
	    o = it1.par->next(o);
	    from = 0;
	    if(!o) return w;
	    to = o->length();
	}
	from = 0;
    }
    to = it2.pos;
    if(!o->isHidden())
    {
	int len = to - from;
	if(!ignoreLast) len++;
	if(len > 0) w += o->width(from, len);
    }
#if BIDI_DEBUG > 1
    printf("width(%p/%d, %p/%d) = %d\n", it1.obj, it1.pos, it2.obj, it2.pos, w);
#endif
    return w;
}

// ----------------------------------------------------------------------------------

class BiDiRun
{
public:
    BiDiRun(const BiDiIterator &_from, const BiDiIterator &_to, int _level) {
	from = _from;
	to = _to;
	level = _level;
	breaks = 0;
    }
    ~BiDiRun()
    {
	delete breaks;
    }

    BiDiIterator from;
    BiDiIterator to;
    unsigned char level;

    QList<BiDiIterator> *breaks;
};

class BiDiWord
{
public:
    BiDiWord(BiDiObject *o, int _from, int _len, unsigned char embeddingLevel,
	     unsigned short _width)
    {
	object = o;
	from = _from;
	len = _len;
	level = embeddingLevel;
	width = _width;
	yOffset = 0;
    }

    BiDiObject *object;
    int from;
    unsigned short len;
    unsigned char level;
    unsigned short width;
    short yOffset;
};


class BiDiControl
{
public:
    BiDiControl(int _level, QChar::Direction _embedding,
		QChar::Direction _override = QChar::DirON)
    {	
	level = _level;
	embedding = _embedding;
	override = _override;
    }

    unsigned char level;
    // QChar::DirON means no override and no embedding set
    QChar::Direction override;
    QChar::Direction embedding;
};

class BiDiParagraphPrivate
{
public:
    BiDiParagraphPrivate(BiDiParagraph *p);
    ~BiDiParagraphPrivate();

    QChar::Direction basicDir;

    // these variables are used during layout calculation

    // the current position of the reordering algorithm
    BiDiIterator current;
    // the directionality of the current run. Can be R, L or AN (EN gets merged with L)
    QChar::Direction dir;
    // start of the current run
    BiDiIterator sor;
    // the current end of the run
    BiDiIterator eor;
    // direction of the ond of run. Can be only R, L, AL, EN, AN
    // (or ON in case we still don't know)
    QChar::Direction dirEor;
    // the last character
    BiDiIterator lastChar;
    QChar::Direction dirLastChar;
    // direction of the last strong char. Can be only L, R or AL
    QChar::Direction dirLastStrong;
    // the direction of the current embedding (L or R)
    QChar::Direction dirEmbedding;

    QStack<BiDiControl> stackEmbedding;
    BiDiControl *currentEmbedding;

    QList<BiDiRun> runs;
    QList<BiDiIterator> *breaks;

    QList<BiDiWord> line;
    int lineHeight;
    int y;
};

BiDiParagraphPrivate::BiDiParagraphPrivate(BiDiParagraph *p)
    : current(p),
      sor(p),
      eor(p),
      lastChar(p)
{
    currentEmbedding = 0;
    dir = QChar::DirON;

    runs.setAutoDelete(true);
    breaks = new QList<BiDiIterator>;
    breaks->setAutoDelete(true);
    line.setAutoDelete(true);
    y = 0;
    lineHeight = 0;
}

BiDiParagraphPrivate::~BiDiParagraphPrivate()
{
    delete currentEmbedding;
    delete breaks;
}

// ---------------------------------------------------------------------------------

BiDiParagraph::BiDiParagraph()
{
    d = 0;
    m_basicDirPreset = QChar::DirON;
    m_align = AlignAuto;
    m_ignoreNewline = true;
    m_ignoreLeadingSpaces = true;
    m_ignoreTrailingSpaces = true;
    m_visualOrdering = false;
}

BiDiParagraph::~BiDiParagraph()
{
    if(d) delete d;
}

// calculates the visual ordering and layout for the whole paragraph
int BiDiParagraph::reorder(int xOff, int yOff)
{
    if(d) delete d;
    d = new BiDiParagraphPrivate(this);

    d->basicDir = basicDirection();

#ifdef BIDI_DEBUG
    if(d->basicDir == QChar::DirL)
	printf("basic Dir = Left\n");
    else
	printf("basic Dir = Right\n");
#endif

    d->currentEmbedding = new BiDiControl(0, d->basicDir);
    if(d->basicDir == QChar::DirR) d->currentEmbedding->level = 1;

    // set to beginning of paragraph
    d->sor = BiDiIterator(this);

    if(d->sor.obj != 0) // non empty paragraph
    {
	d->current = d->sor;
	d->eor = d->sor;
	d->lastChar = d->sor;

	d->dir = QChar::DirON;
	d->dirLastChar = d->basicDir;
	d->dirLastStrong = d->basicDir;
	d->dirEmbedding = d->basicDir;
	d->dirEor = d->basicDir;

	collectRuns();
	breakLines(xOff, yOff);
    }
    closeParagraph();

    return d->y; // ### should return the height of the paragraph
}	

static void addWord(BiDiParagraph *par, QList<BiDiWord> &line, const BiDiIterator
		    &it1, const BiDiIterator &it2, int width, unsigned char level,
		    bool ignoreLast = false)
{
    BiDiWord *w;

    BiDiObject *o = it1.obj;
    int pos = it1.pos;

    if(!o) return;
    while(o != it2.obj)
    {
	if(!o->isHidden())
	{
	    int aWidth = o->width(pos, o->length() - pos);
	    w = new BiDiWord(o, pos, o->length() - pos, level, aWidth);
	    width -= aWidth;
	    line.append(w);
	}
	pos = 0;
	o = par->next(o);
    }
    if(o && !o->isHidden())
    {
	int pos2 = it2.pos;
	if(!ignoreLast) pos2++;
	w = new BiDiWord(o, pos, pos2 - pos, level, width);
	line.append(w);
    }
}

void BiDiParagraph::breakLines(int xOff, int yOff)
{
    d->y = yOff;

    int availableWidth = lineWidth(d->y);
    QListIterator<BiDiRun> it(d->runs);

    int levelHigh = 0;
    int levelLow = 100;

    for(; it.current(); ++it)
    {
	BiDiRun *r = it.current();
	BiDiIterator pos = r->from;
	if(r->level < levelLow) levelLow = r->level;
	if(r->level > levelHigh) levelHigh = r->level;
	
	if(r->breaks && ! r->breaks->isEmpty())
	{
	    QListIterator<BiDiIterator> breakIt(*r->breaks);

	    for(; breakIt.current(); ++breakIt)
	    {
		BiDiIterator b = *breakIt.current();
		// we have line breaks, and have to add them one by one until the
		// line is full
		int w = width(this, pos, b, true);
		availableWidth+=special(this,pos,b,d->y);
		if(w > availableWidth)
		{
		    layoutLine(levelLow, levelHigh, false);
		    d->line.clear();
		    levelLow = r->level;
		    levelHigh = r->level;
		    availableWidth = lineWidth(d->y);
		    ++pos;
		    if(b < pos) b = pos;
		    w = width(this, pos, b);
		}
		else if(b.type == BiDiIterator::NewLine)
		{	
		    addWord(this, d->line, pos, b, w, r->level, true);
		    layoutLine(levelLow, levelHigh, false);
		    d->line.clear();
		    levelLow = r->level;
		    levelHigh = r->level;
		    availableWidth = lineWidth(d->y);
		    pos = b;
		    continue;
		}
		addWord(this, d->line, pos, b, w, r->level, true);
		availableWidth -= w;
		pos = b;
		//++pos;
	    }
	    //++pos;
	}
	//++pos;
	int w = width(this, pos, r->to);
	availableWidth+=special(this,pos,r->to,d->y);
	if(w > availableWidth)
	{
	    layoutLine(levelLow, levelHigh);
	    d->line.clear();
	    levelLow = r->level;
	    levelHigh = r->level;
	    availableWidth = lineWidth(d->y);
	}
	addWord(this, d->line, pos, r->to, w, r->level);
	availableWidth -= w;
    }
    if(!d->line.isEmpty())
	layoutLine(levelLow, levelHigh, true);
}


QChar::Direction BiDiParagraph::basicDirection()
{
    if(m_basicDirPreset != QChar::DirON)
	return m_basicDirPreset;

    BiDiIterator it(this);
    while(!it.atEnd())
    {
	//printf("calcBasicDir: obj=%p, pos=%d\n", it.obj, it.pos);
	switch(it.direction())
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return QChar::DirL;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return QChar::DirR;
	default:
	    break;
	}
	++it;
    }
    return QChar::DirL;
}

unsigned int BiDiParagraph::currentY()
{
    if(d) return d->y;
    return 0;
}

void BiDiParagraph::setCurrentY(int y)
{
    if(!d) return;
    d->y = y;
}

void BiDiParagraph::layoutLine(unsigned char levelLow, unsigned char levelHigh, bool lastLine)
{
#ifdef BIDI_DEBUG
    printf("BiDiPargraph::layoutLine()\n");
#endif

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!levelLow%2) levelLow++;

#ifdef BIDI_DEBUG
    printf("reorderLine: lineLow = %d, lineHigh = %d\n", levelLow, levelHigh);
    printf("logical order is:\n");
    QListIterator<BiDiWord> it2(d->line);
    BiDiWord *r2;
    for ( ; (r2 = it2.current()); ++it2 )
    {
	printf("    %p\n", r2);
    }
#endif

    if(!m_visualOrdering)
    {
    int count = d->line.count() - 1;

    while(levelHigh >= levelLow)
    {
	int i = 0;
	while ( i < count )
	{
	    while(i < count && d->line.at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i < count && d->line.at(i)->level >= levelHigh) i++;
	    int end = i;

	    if(start != end)
	    {
		printf("reversing from %d to %d\n", start, end);
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    BiDiWord *first = d->line.take(start+j);
		    BiDiWord *last = d->line.take(end-j-1);
		    d->line.insert(start+j, last);
		    d->line.insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }
    }
#ifdef BIDI_DEBUG
    printf("visual order is:\n");
    QListIterator<BiDiWord> it3(d->line);
    BiDiWord *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	printf("    %p\n", r3);
    }
#endif


#ifdef BIDI_DEBUG
    printf("adjustYPositions:\n");
#endif

    QListIterator<BiDiWord> it(d->line);
    BiDiWord *r;
    int maxHeight = 0;
    int maxAscent = 0;
    int maxDescent = 0;
    for ( ; (r = it.current()); ++it )
    {
	int height = r->object->bidiHeight();
	int offset = r->object->verticalPositionHint();
	r->yOffset = -offset;
	if(offset < 0)
	{
	    if(maxHeight < height) maxHeight = height;
	}
	else
	{
	    int descent = height - offset;
	    if(maxAscent < offset) maxAscent = offset;
	    if(maxDescent < descent) maxDescent = descent;
	}	
    }
    if(maxHeight < maxAscent + maxDescent) maxHeight = maxAscent + maxDescent;

    it.toFirst();

    for ( ; (r = it.current()); ++it )
    {
	if(r->yOffset == -BiDiObject::PositionTop)
	    r->yOffset = d->y;
	else if(r->yOffset == -BiDiObject::PositionBottom)
	    r->yOffset = maxHeight + d->y - r->object->bidiHeight();
	else
	    r->yOffset += maxAscent + d->y;
    }

    d->lineHeight = maxHeight;


#ifdef BIDI_DEBUG
    printf("layoutingLine\n");
#endif
    it.toFirst();
    BiDiWord *w;
    int delta = 0;
    int spaces = 0;

    int xPos = leftMargin(d->y);
    if(m_align == AlignRight || (d->basicDir == QChar::DirR && m_align == AlignAuto))
    {
	xPos += lineWidth(d->y);
	for ( ; (w = it.current()); ++it )
	    xPos -= w->width;
	it.toFirst();
    }
    else if(m_align == AlignCenter)
    {
	int width = 0;
	for ( ; (w = it.current()); ++it )
	    width += w->width;
	it.toFirst();
	xPos += (lineWidth(d->y) - width)/2;
    }
    else if(m_align == AlignJustify && !lastLine)
    {
	int width = 0;
	spaces = -1;
	for ( ; (w = it.current()); ++it )
	{
	    width += w->width;
	    spaces++;
	}
	it.toFirst();
	if(spaces < 1)
	    spaces = 0;
	else
	    delta = lineWidth(d->y) - width;
#ifdef BIDI_DEBUG
	printf("justify delta = %d\n", delta);
#endif
    }

    for ( ; (w = it.current()); ++it )
    {
#ifdef BIDI_DEBUG
	printf("    %p: positioning slave %p (%d/%d) from=%d len=%d width=%d\n",
	       w, w->object, xPos, w->yOffset, w->from, w->len, w->width);
#endif
	int add = 0;
	if(spaces)
	{
	    add = delta/spaces;
	    delta -= add;
	    spaces--;
	}
	w->width += add;
	if(!m_visualOrdering)
	    w->object->position(xPos, w->yOffset, w->from, w->len, w->width, w->level%2);
	else
	    w->object->position(xPos, w->yOffset, w->from, w->len, w->width, false);
	xPos += w->width;
    }

    d->y += d->lineHeight;
    newLine();
}


void BiDiParagraph::appendRun()
{
    if(d->sor == d->eor) return;

    unsigned char level = d->currentEmbedding->level;
    // add level of run (cases I1 & I2)
    if(level % 2)
    {
	//printf("adding run in rtl embedding\n");
	if(d->dir == QChar::DirL || d->dir == QChar::DirAN)
	    level++;
    }
    else
    {
	//printf("adding run in ltr embedding\n");
	if(d->dir == QChar::DirR)
	    level++;
	else if(d->dir == QChar::DirAN)
	    level += 2;
    }

    BiDiRun *run = new BiDiRun(d->sor, d->eor, level);
    run->breaks = d->breaks;
    d->breaks = new QList<BiDiIterator>;
    d->breaks->setAutoDelete(true);

    d->runs.append(run);

#ifdef BIDI_DEBUG
    printf("closeRun %p: from=%p/%d, to=%p/%d\n", run, d->sor.obj,
	   d->sor.pos, d->eor.obj, d->eor.pos);
#endif

}

void BiDiParagraph::newRun()
{
    if(d->dir == QChar::DirON) d->dir = d->dirEmbedding;

    appendRun();

    ++d->eor;
    d->sor = d->eor;
    d->dir = QChar::DirON;

    d->dirEor = d->eor.direction();
    switch(d->dirEor)
    {
    case QChar::DirL:
    case QChar::DirR:
    case QChar::DirAL:
    case QChar::DirEN:
    case QChar::DirAN:
	break;
    case QChar::DirES:
    case QChar::DirET:
    case QChar::DirCS:
    case QChar::DirBN:
    case QChar::DirB:
    case QChar::DirS:
    case QChar::DirWS:
    case QChar::DirON:		
    default:
	d->dirEor = QChar::DirON;
    }
}

// collects one line of the paragraph and transforms it to visual order
void BiDiParagraph::collectRuns()
{
    while(1)
    {
	QChar::Direction dirCurrent;
	if(d->current.atEnd())
	    dirCurrent = d->basicDir;
	else
	    dirCurrent = d->current.direction();
	
#if BIDI_DEBUG > 1
	printf("directions: dir=%d current=%d last=%d d->eor=%d lastStrong=%d embedding=%d\n",
	       d->dir, dirCurrent, d->dirLastChar, d->dirEor, d->dirLastStrong, d->dirEmbedding);
#endif
	
	switch(dirCurrent)
	{

// embedding and overrides (X1-X9 in the BiDi specs)
// ### fixme: remove code duplication
	case QChar::DirRLE:
	{
	    //		printf("right to left embedding\n");
	    unsigned char level = d->currentEmbedding->level;
	    if(level%2) // we have an odd level
		level += 2;
	    else
		level++;
	    if(level < 61)
	    {
		newRun();
		d->stackEmbedding.push(d->currentEmbedding);
		d->currentEmbedding = new BiDiControl(level, QChar::DirR);
		d->dirLastChar = QChar::DirR;
		d->dirLastStrong = QChar::DirR;
		d->dirEmbedding = QChar::DirR;
	    }
	    break;
	}
	case QChar::DirLRE:
	{
	    //printf("left to right embedding\n");
	    unsigned char level = d->currentEmbedding->level;
	    if(level%2) // we have an odd level
		level++;
	    else
		level += 2;
	    if(level < 61)
	    {
		newRun();
		d->stackEmbedding.push(d->currentEmbedding);
		d->currentEmbedding = new BiDiControl(level, QChar::DirL);
		d->dirLastChar = QChar::DirL;
		d->dirLastStrong = QChar::DirL;
		d->dirEmbedding = QChar::DirL;
	    }
	    break;
	}
	case QChar::DirRLO:
	{
	    printf("right to left override\n");
	    unsigned char level = d->currentEmbedding->level;
	    if(level%2) // we have an odd level
		level += 2;
	    else
		level++;
	    if(level < 61)
	    {
		newRun();
		d->stackEmbedding.push(d->currentEmbedding);
		d->currentEmbedding = new BiDiControl(level, QChar::DirON, QChar::DirR);
		d->dir = QChar::DirR;
		d->dirLastChar = QChar::DirR;
		d->dirLastStrong = QChar::DirR;
		d->dirEmbedding = QChar::DirR;
	    }
	    break;
	}
	case QChar::DirLRO:
	{
	    printf("left to right override\n");
	    unsigned char level = d->currentEmbedding->level;
	    if(level%2) // we have an odd level
		level++;
	    else
		level += 2;
	    if(level < 61)
	    {
		newRun();
		d->stackEmbedding.push(d->currentEmbedding);
		d->currentEmbedding = new BiDiControl(level, QChar::DirON, QChar::DirL);
		d->dir = QChar::DirL;
		d->dirLastChar = QChar::DirL;
		d->dirLastStrong = QChar::DirL;
		d->dirEmbedding = QChar::DirL;
	    }
	    break;
	}
	case QChar::DirPDF:
	{
	    printf("pop directional format\n");
	    BiDiControl *control = d->stackEmbedding.pop();
	    if(control)
	    {
		newRun();
		delete d->currentEmbedding;
		d->currentEmbedding = control;
		d->dir = d->currentEmbedding->override;
		d->dirLastChar = d->dirEmbedding;
		d->dirLastStrong = d->dirEmbedding;
		if(d->currentEmbedding->level%2)
		    d->dirEmbedding = QChar::DirR;
		else
		    d->dirEmbedding = QChar::DirL;
	    }		
	    break;
	}
	
// strong types
	case QChar::DirL:
	    if(d->dir == QChar::DirON)
		d->dir = QChar::DirL;
	    switch(d->dirLastChar)
	    {
	    case QChar::DirL:
		d->eor = d->current; d->dirEor = QChar::DirL; break;
	    case QChar::DirR:
	    case QChar::DirAL:
	    case QChar::DirEN:
	    case QChar::DirAN:
		newRun(); break;
	    case QChar::DirES:
	    case QChar::DirET:
	    case QChar::DirCS:
	    case QChar::DirBN:
	    case QChar::DirB:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		if(d->dir != QChar::DirL)
		{
		    //last stuff takes embedding dir
		    if(d->dirEmbedding == QChar::DirR)
		    {
			if(d->dirEor != QChar::DirR) // AN or EN
			{
			    newRun();
			    d->dir = QChar::DirR;
			}
			else
			    d->eor = d->lastChar;
			newRun();
		    }
		    else
		    {
			if(d->dirEor == QChar::DirR)
			{
			    newRun();
			    d->dir = QChar::DirL;
			}
			else
			{
			    d->eor = d->current; d->dirEor = QChar::DirL; break;
			}
		    }
		}
		else
		{
		    d->eor = d->current; d->dirEor = QChar::DirL;
		}
	    default:
		break;
	    }
	    d->dirLastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(d->dir == QChar::DirON) d->dir = QChar::DirR;
	    switch(d->dirLastChar)
	    {
	    case QChar::DirR:
	    case QChar::DirAL:
		d->eor = d->current; d->dirEor = QChar::DirR; break;
	    case QChar::DirL:
	    case QChar::DirEN:
	    case QChar::DirAN:
		newRun(); break;
	    case QChar::DirES:
	    case QChar::DirET:
	    case QChar::DirCS:
	    case QChar::DirBN:
	    case QChar::DirB:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		if(d->dirEor != QChar::DirR
		   && d->dirEor != QChar::DirAL
		   )
		{
		    //last stuff takes embedding dir
		    if(d->dirEmbedding == QChar::DirR || d->dirLastStrong == QChar::DirR)
		    {
			newRun();
			d->dir = QChar::DirR;
			d->eor = d->current;
		    }
		    else
		    {
			d->eor = d->lastChar;
			newRun();
			d->dir = QChar::DirR;
		    }
		}
		else
		{
		    d->eor = d->current; d->dirEor = QChar::DirR;
		}
	    default:
		break;
	    }
	    d->dirLastStrong = dirCurrent;
	    break;

// weak types:

	case QChar::DirNSM:
	    // ### if @sor, set dir to dirSor
	    break;
	case QChar::DirEN:
	    if(d->dirLastStrong != QChar::DirAL) // if last strong was AL change EN to AL
	    {
		if(d->dir == QChar::DirON)
		{
		    if(d->dirLastStrong == QChar::DirL)
			d->dir = QChar::DirL;
		    else
			d->dir = QChar::DirAN;
		}
		switch(d->dirLastChar)
		{
		case QChar::DirEN:
		case QChar::DirL:
		case QChar::DirET:
		    d->eor = d->current;
		    d->dirEor = dirCurrent;
		    break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirAN:
		    newRun(); d->dir = QChar::DirAN; break;
		case QChar::DirES:
		case QChar::DirCS:
		    if(d->dirEor == QChar::DirEN)
		    {
			d->eor = d->current; d->dirEor = QChar::DirEN; break;
		    }
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:		
		    if(d->dirEor == QChar::DirR)
		    {
			// neutrals go to R
			d->eor = d->lastChar;
			newRun();
			d->dir = QChar::DirAN;
		    }
		    else if( d->dirEor == QChar::DirL ||
			     (d->dirEor == QChar::DirEN && d->dirLastStrong == QChar::DirL))
		    {
			d->eor = d->current; d->dirEor = dirCurrent;
		    }
		    else
		    {
			// numbers on both sides, neutrals get right to left direction
			if(d->dir != QChar::DirL)
			{
			    newRun();
			    d->eor = d->lastChar;
			    d->dir = QChar::DirR;
			    newRun();
			    d->dir = QChar::DirAN;
			}
			else
			    d->eor = d->current; d->dirEor = dirCurrent;
		    }			
		default:
		    break;
		}
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(d->dir == QChar::DirON) d->dir = QChar::DirAN;
	    switch(d->dirLastChar)
	    {
	    case QChar::DirL:
	    case QChar::DirAN:
		d->eor = d->current; d->dirEor = QChar::DirAN; break;
	    case QChar::DirR:
	    case QChar::DirAL:
	    case QChar::DirEN:
		newRun(); break;
	    case QChar::DirCS:
		if(d->dirEor == QChar::DirAN)
		{
		    d->eor = d->current; d->dirEor = QChar::DirR; break;
		}
	    case QChar::DirES:
	    case QChar::DirET:
	    case QChar::DirBN:
	    case QChar::DirB:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:		
		if(d->dirEor == QChar::DirR)
		{
		    // neutrals go to R
		    d->eor = d->lastChar;
		    newRun();
		    d->dir = QChar::DirAN;
		}
		else if( d->dirEor == QChar::DirL ||
			 (d->dirEor == QChar::DirEN && d->dirLastStrong == QChar::DirL))
		{
		    d->eor = d->current; d->dirEor = dirCurrent;
		}
		else
		{
		    // numbers on both sides, neutrals get right to left direction
		    if(d->dir != QChar::DirL)
		    {
			newRun();
			d->eor = d->lastChar;
			d->dir = QChar::DirR;
			newRun();
			d->dir = QChar::DirAN;
		    }
		    else
			d->eor = d->current; d->dirEor = dirCurrent;
		}			
	    default:
		break;
	    }
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(d->dirLastChar == QChar::DirEN)
	    {
		dirCurrent = QChar::DirEN;
		d->eor = d->current; d->dirEor = dirCurrent;
		break;
	    }
	    break;

	    // boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
// neutrals
	case QChar::DirB:
	    if(!m_ignoreNewline && d->current.current() == QChar('\n'))
	    {
		//printf("found a newline char\n");
		BiDiIterator *newline = new BiDiIterator(d->current);
		newline->type = BiDiIterator::NewLine;
		d->breaks->append(newline);
	    }
	    //printf("QBiDiParagraph: got a paragrpah separator!!!!!!\n");
	    // ###
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	{
	    BiDiIterator *space = new BiDiIterator(d->current);
	    space->type = BiDiIterator::WhiteSpace;
#ifdef BIDI_DEBUG
	    printf("found space at %p/%d\n", space->obj, space->pos);
#endif
	    d->breaks->append(space);
	}
	break;
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//printf("     after: dir=%d d->current=%d last=%d d->eor=%d lastStrong=%d embedding=%d\n",
	//        d->dir, dirCurrent, d->dirLastChar, d->dirEor, d->dirLastStrong, d->dirEmbedding);

	if(d->current.atEnd()) break;
	
	// set dirLastChar as needed.
	switch(dirCurrent)
	{
	case QChar::DirET:
	case QChar::DirES:
	case QChar::DirCS:
	case QChar::DirS:
	case QChar::DirWS:
	case QChar::DirON:
	    switch(d->dirLastChar)
	    {
	    case QChar::DirL:
	    case QChar::DirR:
	    case QChar::DirAL:
	    case QChar::DirEN:
	    case QChar::DirAN:
		d->dirLastChar = dirCurrent;
		break;
	    default:
		d->dirLastChar = QChar::DirON;
	    }
	    break;
	case QChar::DirNSM:
	case QChar::DirBN:
	    // ignore these
	    break;
	default:
	    d->dirLastChar = dirCurrent;
	}

	d->lastChar = d->current;
	++d->current;
    }

#ifdef BIDI_DEBUG
    printf("reached end of paragraph current=%d, eor=%d\n", d->current.pos, d->eor.pos);
#endif
    d->eor = d->current;
    appendRun();
}


