#include "bidi.h"
#include "render_flow.h"
#include "render_text.h"
using namespace khtml;

#include "kdebug.h"

//#define BIDI_DEBUG 1

// ---------------------------------------------------------------------

/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
BidiContext::BidiContext(unsigned char l, QChar::Direction e, BidiContext *p, bool o)
    : level(l) , override(o), dir(e)
{
    if(p) {
	p->ref();
	parent = p;
    }
    count = 0;
}

BidiContext::~BidiContext()
{
    if(parent) parent->deref();
}

void BidiContext::ref() const
{
    count++;
}

void BidiContext::deref() const
{
    count--;
    if(count <= 0) delete this;
}

// ---------------------------------------------------------------------

BidiIterator::BidiIterator()
{
    par = 0;
    obj = 0;
    pos = 0;
}

BidiIterator::BidiIterator(RenderFlow *_par)
{
    par = _par;
    obj = par->first();
    pos = 0;
}

BidiIterator::BidiIterator(const BidiIterator &it)
{
    par = it.par;
    obj = it.obj;
    pos = it.pos;
}

BidiIterator::BidiIterator(RenderFlow *_par, RenderObject *_obj, int _pos)
{
    par = _par;
    obj = _obj;
    pos = _pos;
}

BidiIterator &BidiIterator::operator = (const BidiIterator &it)
{
    obj = it.obj;
    pos = it.pos;
    par = it.par;
    return *this;
}

void BidiIterator::operator ++ ()
{
    if(!obj) return;
    if(obj->isText()) {
	pos++;
	if(pos >= obj->length()) {
	    obj = par->next(obj);
	    pos = 0;
	}
    } else {
	obj = par->next(obj);
	pos = 0;
    }
}

bool BidiIterator::atEnd()
{
    if(!obj) return true;
    return false;
}

const QChar &BidiIterator::current()
{
    static const QChar nbsp = QChar(0xA0);
    if( !obj || !obj->isText()) return nbsp; // non breaking space
    return static_cast<RenderText *>(obj)->text()[pos];
}

QChar::Direction BidiIterator::direction()
{
    if(!obj || !obj->isText() || obj->length() <= 0) return QChar::DirON;
    return static_cast<RenderText *>(obj)->text()[pos].direction();
}

inline bool operator==( const BidiIterator &it1, const BidiIterator &it2 )
{
    if(it1.pos != it2.pos) return false;
    if(it1.obj != it2.obj) return false;
    return true;
}

inline bool operator!=( const BidiIterator &it1, const BidiIterator &it2 )
{
    if(it1.pos != it2.pos) return true;
    if(it1.obj != it2.obj) return true;
    return false;
}

// the implementation, when objects are different is not efficient.
// on the other hand, this case should be rare
inline bool operator > ( const BidiIterator &it1, const BidiIterator &it2 )
{
    if(it1.obj != it2.obj)
    {
#if BIDI_DEBUG > 1
	//kdDebug( 6041 ) << "BidiIterator operator >: objects differ" << endl;
#endif
	if(!it1.obj) return true;
	
	RenderObject *o = it2.obj;
	while(o)
	{
	    if(o == it1.obj) return true;
	    o = it1.par->next(o);
	}
#if BIDI_DEBUG > 1
	//kdDebug( 6041 ) << "BidiIterator operator >: false" << endl;
#endif
	return false;
    }
    return (it1.pos > it2.pos);
}

inline bool operator < ( const BidiIterator &it1, const BidiIterator &it2 )
{
    return (it2 > it1);
}

// -------------------------------------------------------------------------------------------------

void RenderFlow::appendRun(QList<BidiRun> &runs, const BidiIterator &sor, const BidiIterator &eor,
			   BidiContext *context, QChar::Direction dir)
{
    kdDebug(6041) << "appendRun: dir="<<(int)dir<<endl;
    
    int start = sor.pos;
    RenderObject *obj = sor.obj;
    while( obj != eor.obj ) {
	if(!obj->isHidden()) {
	    kdDebug(6041) << "appendRun: "<< start << "/" << obj->length() <<endl;
	    runs.append( new BidiRun(start, obj->length(), obj, context, dir) );
	}
	start = 0;
	obj = next(obj);
    }
    if( obj ) {
	kdDebug(6041) << "appendRun: "<< start << "/" << eor.pos <<endl;
	runs.append( new BidiRun(start, eor.pos, obj, context, dir) );
    }
}


// collects one line of the paragraph and transforms it to visual order
BidiContext *RenderFlow::bidiReorderLine(BidiStatus &status, const BidiIterator &start, const BidiIterator &end, BidiContext *startEmbed)
{
 
    kdDebug(6041) << "reordering Line from " << start.obj << "/" << start.pos << " to " << end.obj << "/" << end.pos << endl;

    QList<BidiRun> runs;
    runs.setAutoDelete(true);

    BidiContext *context = startEmbed;
    context->ref();

    QChar::Direction dir = QChar::DirON;

    BidiIterator sor = start;
    BidiIterator eor = start;

    BidiIterator current = start;
    BidiIterator last = current;
    while(current < end) {
	QChar::Direction dirCurrent;
	if(current.atEnd()) {
	    BidiContext *c = context;
	    while ( c->parent )
		c = c->parent;
	    dirCurrent = c->dir;
	} else
	    dirCurrent = current.direction();

	
#if BIDI_DEBUG > 1
	kdDebug(6041) << "directions: dir=" << (int)dir << " current=" << (int)dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << (int)context->dir << " level =" << (int)context->level << endl;
#endif
	
	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the Bidi specs)
	case QChar::DirRLE:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new BidiContext(level, QChar::DirR, context);
		    context->ref();
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRE:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    appendRun(runs, sor, eor, context, dir);	
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new BidiContext(level, QChar::DirL, context);
		    context->ref();
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirRLO:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new BidiContext(level, QChar::DirR, context, true);
		    context->ref();
		    dir = QChar::DirR;
		    status.last = QChar::DirR;
		    status.lastStrong = QChar::DirR;
		}
		break;
	    }
	case QChar::DirLRO:
	    {
		unsigned char level = context->level;
		if(level%2) // we have an odd level
		    level++;
		else
		    level += 2;
		if(level < 61) {
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    context = new BidiContext(level, QChar::DirL, context, true);
		    context->ref();
		    dir = QChar::DirL;
		    status.last = QChar::DirL;
		    status.lastStrong = QChar::DirL;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		BidiContext *c = context->parent;
		if(c) {
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    status.last = context->dir;
		    context->deref();
		    context = c;
		    if(context->override)
			dir = context->dir;
		    else
			dir = QChar::DirON;
		    status.lastStrong = context->dir;
		}		
		break;
	    }
	
	    // strong types
	case QChar::DirL:
	    if(dir == QChar::DirON)
		dir = QChar::DirL;
	    switch(status.last)
		{
		case QChar::DirL:
		    eor = current; status.eor = QChar::DirL; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		case QChar::DirAN:
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(dir != QChar::DirL) {
			//last stuff takes embedding dir
			if( context->dir == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				appendRun(runs, sor, eor, context, dir);
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    else
				eor = last;
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			} else {
			    if(status.eor == QChar::DirR) {
				appendRun(runs, sor, eor, context, dir);
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirL;
			    } else {
				eor = current; status.eor = QChar::DirL; break;
			    }
			}
		    } else {
			eor = current; status.eor = QChar::DirL;
		    }
		default:
		    break;
		}
	    status.lastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(dir == QChar::DirON) dir = QChar::DirR;
	    switch(status.last)
		{
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if( status.eor != QChar::DirR && status.eor != QChar::DirAL ) {
			//last stuff takes embedding dir
			if(context->dir == QChar::DirR || status.lastStrong == QChar::DirR) {
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = last;
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			}
		    } else {
			eor = current; status.eor = QChar::DirR;
		    }
		default:
		    break;
		}
	    status.lastStrong = dirCurrent;
	    break;

	    // weak types:

	case QChar::DirNSM:
	    // ### if @sor, set dir to dirSor
	    break;
	case QChar::DirEN:
	    if(status.lastStrong != QChar::DirAL) {
		// if last strong was AL change EN to AN
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirAL)
			dir = QChar::DirAN;
		    else
			dir = QChar::DirL;
		}
		switch(status.last)
		    {
		    case QChar::DirEN:
		    case QChar::DirL:
		    case QChar::DirET:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			appendRun(runs, sor, eor, context, dir);
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN) {
			    eor = current; status.eor = QChar::DirEN; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:		
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = last;
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				appendRun(runs, sor, eor, context, dir);
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				eor = last;
				dir = QChar::DirR;
				appendRun(runs, sor, eor, context, dir);
				++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirAN;
			    } else {
				eor = current; status.eor = dirCurrent;
			    }
			}
		    default:
			break;
		    }
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(dir == QChar::DirON) dir = QChar::DirAN;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirAN:
		    eor = current; status.eor = QChar::DirAN; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		    appendRun(runs, sor, eor, context, dir);
		    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; status.eor = QChar::DirR; break;
		    }
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:		
		    if(status.eor == QChar::DirR) {
			// neutrals go to R
			eor = last;
			appendRun(runs, sor, eor, context, dir);
			++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    eor = last;
			    dir = QChar::DirR;
			    appendRun(runs, sor, eor, context, dir);
			    ++eor; sor = eor; dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			} else {
			    eor = current; status.eor = dirCurrent;
			}
		    }
		default:
		    break;
		}
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(status.last == QChar::DirEN) {
		dirCurrent = QChar::DirEN;
		eor = current; status.eor = dirCurrent;
		break;
	    }
	    break;

	// boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
	    // neutrals
	case QChar::DirB:
	    // ### what do we do with newline and paragraph seperators that come to here?
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << context->dir << endl;

	if(current.atEnd()) break;
	
	// set status.last as needed.
	switch(dirCurrent)
	    {
	    case QChar::DirET:
	    case QChar::DirES:
	    case QChar::DirCS:
	    case QChar::DirS:
	    case QChar::DirWS:
	    case QChar::DirON:
		switch(status.last)
		    {
		    case QChar::DirL:
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirEN:
		    case QChar::DirAN:
			status.last = dirCurrent;
			break;
		    default:
			status.last = QChar::DirON;
		    }
		break;
	    case QChar::DirNSM:
	    case QChar::DirBN:
		// ignore these
		break;
	    default:
		status.last = dirCurrent;
	    }

	last = current;
	++current;
    }

#ifdef BIDI_DEBUG
    kdDebug(6041) << "reached end of paragraph current=" << current.pos << ", eor=" << eor.pos << endl;
#endif
    eor = current;

    appendRun(runs, sor, eor, context, dir);

    BidiContext *endEmbed = context;
    // both commands below together give a noop...
    //endEmbed->ref();
    //context->deref();

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    BidiRun *r = runs.first();
    while ( r ) {
	//printf("level = %d\n", r->level);
	if ( r->level > levelHigh )
	    levelHigh = r->level;
	if ( r->level < levelLow )
	    levelLow = r->level;
	r = runs.next();
    }

    // implements reordering of the line (L2 according to Bidi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!levelLow%2) levelLow++;

#ifdef BIDI_DEBUG
    kdDebug(6041) << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    kdDebug(6041) << "logical order is:" << endl;
    QListIterator<BidiRun> it2(runs);
    BidiRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
	kdDebug(6041) << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = runs.count() - 1;

    while(levelHigh >= levelLow)
	{
	    int i = 0;
	    while ( i < count )
		{
	    while(i < count && runs.at(i)->level < levelHigh) i++;
	    int start = i;
	    while(i <= count && runs.at(i)->level >= levelHigh) i++;
	    int end = i-1;

	    if(start != end)
	    {
		//kdDebug(6041) << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++)
		{
		    BidiRun *first = runs.take(start+j);
		    BidiRun *last = runs.take(end-j-1);
		    runs.insert(start+j, last);
		    runs.insert(end-j, first);
		}
	    }
	    i++;
	    if(i >= count) break;
	}
	levelHigh--;
    }

#ifdef BIDI_DEBUG
    kdDebug(6041) << "visual order is:" << endl;
    QListIterator<BidiRun> it3(runs);
    BidiRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
	kdDebug(6041) << "    " << r3 << endl;
    }
#endif

    int maxHeight = 0;
    int maxAscent = 0;
    int maxDescent = 0;
    r = runs.first();
    while ( r ) {
	int height = r->obj->bidiHeight();
	int offset = r->obj->verticalPositionHint();
	kdDebug(6041) << "object="<< r->obj << " height="<<height<<" offset="<<offset<<endl;
	r->yOffset = -offset;
	if(offset < 0) {
	    if(maxHeight < height) maxHeight = height;
	} else {
	    int descent = height - offset;
	    if(maxAscent < offset) maxAscent = offset;
	    if(maxDescent < descent) maxDescent = descent;
	}	
	r = runs.next();
    }
    if(maxHeight < maxAscent + maxDescent) maxHeight = maxAscent + maxDescent;

    r = runs.first();
    while (r ) {
	if(r->yOffset == -PositionTop)
	    r->yOffset = m_height;
	else if(r->yOffset == -PositionBottom)
	    r->yOffset = maxHeight + m_height - r->obj->bidiHeight();
	else
	    r->yOffset += maxAscent + m_height;
	r = runs.next();
    }
    kdDebug(6041) << "yPos of line=" << m_height << "  lineHeight=" << maxHeight << endl;
    
    // now construct the reordered string out of the runs...

    r = runs.first();
    int x = leftMargin(m_height);
    while ( r ) {
	if(r->obj->isText()) 
	    r->width = static_cast<RenderText *>(r->obj)->width(r->start, r->stop-r->start);
	else
	    r->width = r->obj->width();
	kdDebug(6041) << "positioning " << r->obj << " start=" << r->start << " stop" << r->stop << " yPos=" << r->yOffset << endl;
	r->obj->position(x, r->yOffset, r->start, r->stop - r->start, r->width, r->level%2);
	x += r->width;
	r = runs.next();
    }

    m_height += maxHeight;

    return endEmbed;
}


void RenderFlow::layoutInlineChildren()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "layoutInlineChildren" << endl;
#endif
    int toAdd = 0;
    m_height = 0;

    if(m_style->hasBorder())
    {
	m_height = borderTop();
	toAdd = borderBottom();
    }
    if(m_style->hasPadding())
    {
	m_height += paddingTop();
	toAdd += paddingBottom();
    }
    if(m_first) {
	// layout replaced elements
	RenderObject *o = first();
	while ( o ) {
	    if(o->isReplaced() || o->isFloating() || o->isPositioned()) {
		kdDebug(6041) << "layouting replaced or floating child" << endl;
		o->layout();
	    }
	    else if(o->isText())
		static_cast<RenderText *>(o)->deleteSlaves();
	    o = next(o);
	}
	
	BidiContext *startEmbed;
	if( style()->direction() == LTR )
	    startEmbed = new BidiContext( 0, QChar::DirL );
	else
	    startEmbed = new BidiContext( 1, QChar::DirR );
	startEmbed->ref();
	BidiStatus status;

	BidiIterator start(this);
	BidiIterator end(this);
	
	while( !end.atEnd() ) {
	    start = end;
	    if( start.current() == QChar('\n') ) {
		++start;
		if( start.atEnd() )
		    break;
	    }
	    if(!m_pre) {
		// remove leading spaces
		while(!start.atEnd() && start.direction() == QChar::DirWS )
		    ++start;
	    }
	    newLine();
	    end = findNextLineBreak(start);
	    startEmbed = bidiReorderLine(status, start, end, startEmbed);
	}
    }
    m_height += toAdd;
}


BidiIterator RenderFlow::findNextLineBreak(const BidiIterator &start)
{
    kdDebug(6041) << "findnextLineBreak" << endl;
    BidiIterator lBreak = start;
    BidiIterator current = start;

    int width = lineWidth(m_height);
    int w = 0;
    int tmpW = 0;
    while( w + tmpW < width ) {
	RenderObject *o = current.obj;
	if(!o) { 
	    lBreak = current;
	    kdDebug(6041) << "sol: " << start.obj << " " << start.pos << "   end: " << lBreak.obj << " " << lBreak.pos << "   width=" << w << endl;
	    return current;
	}
	if( o->isSpecial() ) {
	    // add to special objects...
	    // ### check if it fits in the current line. If yes, add it directly. If no, add it delayed
	    specialHandler(o);
	} else if( current.direction() == QChar::DirWS ) {
	    lBreak = current;
	    w += tmpW;
	    tmpW = 0;
	} else if( current.current() == QChar('\n') ) {
	    kdDebug(6041) << "sol: " << start.obj << " " << start.pos << "   end: " << lBreak.obj << " " << lBreak.pos << "   width=" << w << endl;
	    return current;
	}
	if( o->isText() )
	    tmpW += static_cast<RenderText *>(o)->width(current.pos, 1);
	else
	    tmpW += o->width();
	++current;
    }
    if(w == 0)
	lBreak = current;
    kdDebug(6041) << "sol: " << start.obj << " " << start.pos << "   end: " << lBreak.obj << " " << lBreak.pos << "   width=" << w << endl;

    
    return lBreak;
}

RenderObject *RenderFlow::first()
{
    if(!m_first) return 0;
    RenderObject *o = m_first;

    if(!o->isText() && !o->isBR() && !o->isReplaced() && !o->isFloating() && !o->isPositioned())
	o = next(o) ;

    return o;
}

RenderObject *RenderFlow::next(RenderObject *current)
{
    if(!current) return 0;

    while(current != 0)
    {
	//kdDebug( 6040 ) << "current = " << current << endl;
	RenderObject *next = nextObject(current);

	if(!next) return 0;

	if(next->isText() || next->isBR() || next->isFloating() || next->isReplaced() || next->isPositioned())
	    return next;
	current = next;
    }
    return 0;
}

RenderObject *RenderFlow::nextObject(RenderObject *current)
{
    RenderObject *next = 0;
    if(!current->isFloating() && !current->isReplaced() && !current->isPositioned())
	next = current->firstChild();
    if(next) return next;

    while(current && current != this)
    {
	next = current->nextSibling();
	if(next) return next;
	current = current->parent();
    }
    return 0;
}

