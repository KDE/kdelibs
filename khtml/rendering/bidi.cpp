/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include "rendering/bidi.h"
#include "rendering/break_lines.h"
#include "rendering/render_block.h"
#include "rendering/render_text.h"
#include "rendering/render_arena.h"
using namespace khtml;

#include <kdebug.h>
#include <kglobal.h>
#include <qdatetime.h>
#include <qfontmetrics.h>

#define BIDI_DEBUG 0
//#define DEBUG_LINEBREAKS

#if BIDI_DEBUG > 1

// the ones from the QChar class
static const char *directions[] = {
    "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
    "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN"
};

inline kdbgstream &operator<<(kdbgstream &stream, QChar::Direction d) {
    return (stream << directions[d]);
}


#endif

inline BidiIterator::BidiIterator()
{
    par = 0;
    obj = 0;
    pos = 0;
}

static BidiIterator sor;
static BidiIterator eor;
static BidiIterator last;
static BidiIterator current;
static BidiContext *context;
static BidiStatus status;
static QPtrList<BidiRun> *sruns = 0;
static QChar::Direction dir;
static bool adjustEmbeddding = false;
static bool emptyRun = true;
static int numSpaces;

static void embed( QChar::Direction d );
static void appendRun();

#ifndef NDEBUG
static bool inBidiIteratorDetach;
#endif

static int getBPMWidth(int childValue, Length cssUnit)
{
    if (!cssUnit.isVariable())
        return (cssUnit.isFixed() ? cssUnit.value() : childValue);
    return 0;
}

static int getBorderPaddingMargin(RenderObject* child, bool endOfInline)
{
    RenderStyle* cstyle = child->style();
    int result = 0;
    bool leftSide = (cstyle->direction() == LTR) ? !endOfInline : endOfInline;
    result += getBPMWidth((leftSide ? child->marginLeft() : child->marginRight()),
                          (leftSide ? cstyle->marginLeft() :
                           cstyle->marginRight()));
    result += getBPMWidth((leftSide ? child->paddingLeft() : child->paddingRight()),
                          (leftSide ? cstyle->paddingLeft() :
                           cstyle->paddingRight()));
    result += leftSide ? child->borderLeft() : child->borderRight();
    return result;
}

static int inlineWidth(RenderObject* child, bool start = true, bool end = true)
{
    int extraWidth = 0;
    RenderObject* parent = child->parent();
    while (parent->isInline() && !parent->isReplacedBlock()) {
        if (start && parent->firstChild() == child)
            extraWidth += getBorderPaddingMargin(parent, false);
        if (end && parent->lastChild() == child)
            extraWidth += getBorderPaddingMargin(parent, true);
        child = parent;
        parent = child->parent();
    }
    return extraWidth;
}

void BidiIterator::detach(RenderArena* renderArena)
{
#ifndef NDEBUG
    inBidiIteratorDetach = true;
#endif
    delete this;
#ifndef NDEBUG
    inBidiIteratorDetach = false;
#endif

    // Recover the size left there for us by operator delete and free the memory.
    renderArena->free(*(size_t *)this, this);
}

void* BidiIterator::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void BidiIterator::operator delete(void* ptr, size_t sz)
{
    assert(inBidiIteratorDetach);

    // Stash size where detach can find it.
    *(size_t*)ptr = sz;
}

// ---------------------------------------------------------------------

/* a small helper class used internally to resolve Bidi embedding levels.
   Each line of text caches the embedding level at the start of the line for faster
   relayouting
*/
BidiContext::BidiContext(unsigned char l, QChar::Direction e, BidiContext *p, bool o)
    : level(l) , override(o), dir(e)
{
    parent = p;
    if(p) {
        p->ref();
        basicDir = p->basicDir;
    } else
        basicDir = e;
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

static inline RenderObject *Bidinext(RenderObject *par, RenderObject *current)
{
    RenderObject *next = 0;
    while(current != 0)
    {
        //kdDebug( 6040 ) << "current = " << current << endl;
	if(!current->isFloating() && !current->isReplaced() && !current->isPositioned()) {
	    next = current->firstChild();
	    if ( next && adjustEmbeddding ) {
		EUnicodeBidi ub = next->style()->unicodeBidi();
		if ( ub != UBNormal ) {
		    EDirection dir = next->style()->direction();
// 		    qDebug("element: unicode-bidi=%d, dir=%d", ub, dir);
		    QChar::Direction d = ( ub == Embed ? ( dir == RTL ? QChar::DirRLE : QChar::DirLRE )
					   : ( dir == RTL ? QChar::DirRLO : QChar::DirLRO ) );
		    embed( d );
		}
	    }
	}
	if(!next) {
	    while(current && current != par) {
		next = current->nextSibling();
		if(next) break;
		if ( adjustEmbeddding && current->style()->unicodeBidi() != UBNormal && !emptyRun ) {
		    embed( QChar::DirPDF );
		}
		current = current->parent();
	    }
	}

        if(!next) break;

        if(next->isText() || next->isBR() || next->isFloating() || next->isReplaced() || next->isPositioned())
            break;
        current = next;
    }
    return next;
}

static RenderObject *first( RenderObject *par )
{
    if(!par->firstChild()) return 0;
    RenderObject *o = par->firstChild();

    if(!o->isText() && !o->isBR() && !o->isReplaced() && !o->isFloating() && !o->isPositioned())
        o = Bidinext( par, o );

    return o;
}

BidiIterator::BidiIterator(RenderFlow *_par)
{
    par = _par;
    if ( par && adjustEmbeddding ) {
	EUnicodeBidi ub = par->style()->unicodeBidi();
	if ( ub != UBNormal ) {
	    EDirection dir = par->style()->direction();
// 	    qDebug("element: unicode-bidi=%d, dir=%d", ub, dir);
	    QChar::Direction d = ( ub == Embed ? ( dir == RTL ? QChar::DirRLE : QChar::DirLRE )
				   : ( dir == RTL ? QChar::DirRLO : QChar::DirLRO ) );
	    embed( d );
	}
    }
    obj = first( par );
    isText = obj ? obj->isText() : false;
    pos = isText ? static_cast<RenderText *>(obj)->forcedMinOffset() : 0;
    //kdDebug(6041) << "bidiiterator init: pos " << pos << endl;
}

inline BidiIterator::BidiIterator(const BidiIterator &it)
{
    par = it.par;
    obj = it.obj;
    pos = it.pos;
    isText = obj ? obj->isText() : false;
}

inline BidiIterator::BidiIterator(RenderFlow *_par, RenderObject *_obj, int _pos)
{
    par = _par;
    obj = _obj;
    isText = obj ? obj->isText() : false;
    pos = _pos > 0 ? _pos : (
    		isText ? static_cast<RenderText *>(obj)->forcedMinOffset() : 0);
    //kdDebug(6041) << "bidiiterator init(2): pos " << pos << endl;
}

inline BidiIterator &BidiIterator::operator = (const BidiIterator &it)
{
    obj = it.obj;
    pos = it.pos;
    par = it.par;
    isText = obj ? obj->isText() : false;
    return *this;
}

inline void BidiIterator::operator ++ ()
{
    if(!obj) return;
    if(isText) {
        pos++;
        if(pos >= static_cast<RenderText *>(obj)->stringLength()) {
            obj = Bidinext( par, obj );
	    isText = obj ? obj->isText() : false;
            pos = isText ? static_cast<RenderText *>(obj)->forcedMinOffset() : 0;
//kdDebug(6041) << "bidiiterator ++(1): pos " << pos << endl;
        }
    } else {
        obj = Bidinext( par, obj );
	isText = obj ? obj->isText() : false;
        pos = isText ? static_cast<RenderText *>(obj)->forcedMinOffset() : 0;
//kdDebug(6041) << "bidiiterator ++(2): pos " << pos << endl;
    }
}

inline bool BidiIterator::atEnd() const
{
    if(!obj) return true;
    return false;
}

static const QChar nbsp = QChar(0xA0);

inline const QChar &BidiIterator::current() const
{
    if( !isText ) return nbsp; // non breaking space
    return static_cast<RenderText *>(obj)->text()[pos];
}

inline QChar::Direction BidiIterator::direction() const
{
    if( !isText ) return QChar::DirON;

    RenderText *renderTxt = static_cast<RenderText *>( obj );
    if ( pos >= renderTxt->stringLength() )
        return QChar::DirON;
    return renderTxt->text()[pos].direction();
}

// -------------------------------------------------------------------------------------------------

static void appendRun()
{
    if ( emptyRun ) return;
#if BIDI_DEBUG > 1
    kdDebug(6041) << "appendRun: dir="<<(int)dir<<endl;
#endif

    bool b = adjustEmbeddding;
    adjustEmbeddding = false;

    int start = sor.pos;
    RenderObject *obj = sor.obj;
    while( obj && obj != eor.obj ) {
        if(!obj->isHidden()) {
#if BIDI_DEBUG > 1
            kdDebug(6041) << "appendRun: "<< start << "/" << obj->length() <<endl;
#endif
            sruns->append( new BidiRun(start, obj->length(), obj, context, dir) );
        }
        obj = Bidinext( sor.par, obj );
        start = obj && obj->isText() ? static_cast<RenderText *>(obj)->forcedMinOffset() : 0;
    }
    if( obj && !obj->isHidden()) {
#if BIDI_DEBUG > 1
        kdDebug(6041) << "appendRun: "<< start << "/" << eor.pos <<endl;
#endif
        sruns->append( new BidiRun(start, eor.pos + 1, obj, context, dir) );
    }

    ++eor;
    sor = eor;
    dir = QChar::DirON;
    status.eor = QChar::DirON;
    adjustEmbeddding = b;
}

static void embed( QChar::Direction d )
{
#if BIDI_DEBUG > 1
    qDebug("*** embed dir=%d emptyrun=%d", d, emptyRun );
#endif
    bool b = adjustEmbeddding ;
    adjustEmbeddding = false;
    if ( d == QChar::DirPDF ) {
	BidiContext *c = context->parent;
	if(c && sruns) {
	    if ( eor != last ) {
		appendRun();
		eor = last;
	    }
	    appendRun();
	    emptyRun = true;
	    status.last = context->dir;
	    context->deref();
	    context = c;
	    if(context->override)
		dir = context->dir;
	    else
		dir = QChar::DirON;
	    status.lastStrong = context->dir;
	}
    } else {
	QChar::Direction runDir;
	if( d == QChar::DirRLE || d == QChar::DirRLO )
	    runDir = QChar::DirR;
	else
	    runDir = QChar::DirL;
	bool override;
	if( d == QChar::DirLRO || d == QChar::DirRLO )
	    override = true;
	else
	    override = false;

	unsigned char level = context->level;
	if ( runDir == QChar::DirR ) {
	    if(level%2) // we have an odd level
		level += 2;
	    else
		level++;
	} else {
	    if(level%2) // we have an odd level
		level++;
	    else
		level += 2;
	}

	if(level < 61) {
	    if ( sruns ) {
		if ( eor != last ) {
		    appendRun();
		    eor = last;
		}
		appendRun();
		emptyRun = true;

	    }
	    context = new BidiContext(level, runDir, context, override);
	    context->ref();
	    if ( override )
		dir = runDir;
	    status.last = runDir;
	    status.lastStrong = runDir;
	}
    }
    adjustEmbeddding = b;
}

InlineFlowBox* RenderBlock::createLineBoxes(RenderObject* obj)
{
    // See if we have an unconstructed line box for this object that is also
    // the last item on the line.
    KHTMLAssert(obj->isInlineFlow() || obj == this);
    RenderFlow* flow = static_cast<RenderFlow*>(obj);

    // Get the last box we made for this render object.
    InlineFlowBox* box = flow->lastLineBox();

    // If this box is constructed then it is from a previous line, and we need
    // to make a new box for our line.  If this box is unconstructed but it has
    // something following it on the line, then we know we have to make a new box
    // as well.  In this situation our inline has actually been split in two on
    // the same line (this can happen with very fancy language mixtures).
    if (!box || box->isConstructed() || box->nextOnLine()) {
        InlineBox* newBox = obj->createInlineBox(false, obj == this);
        KHTMLAssert(newBox->isInlineFlowBox());
        box = static_cast<InlineFlowBox*>(newBox);
        box->setFirstLineStyleBit(m_firstLine);

        // We have a new box. Append it to the inline box we get by constructing our
        // parent.  If we have hit the block itself, then |box| represents the root
        // inline box for the line, and it doesn't have to be appended to any parent
        // inline.
        if (obj != this) {
            InlineFlowBox* parentBox = createLineBoxes(obj->parent());
            parentBox->addToLine(box);
        }
    }

    return box;
}

InlineFlowBox* RenderBlock::constructLine(const BidiIterator &start, const BidiIterator &end)
{
    if (!sruns)
        return 0; // We had no runs. Don't make a root inline box at all. The line is empty.

    InlineFlowBox* parentBox = 0;
    QPtrListIterator<BidiRun> it(*sruns);
    for (BidiRun *r; (r = it.current()); ++it) {
        // Create a box for our object.
        r->box = r->obj->createInlineBox(r->obj->isPositioned(), false);

        // If we have no parent box yet, or if the run is not simply a sibling,
        // then we need to construct inline boxes as necessary to properly enclose the
        // run's inline box.
        if (!parentBox || (parentBox->object() != r->obj->parent()))
            // Create new inline boxes all the way back to the appropriate insertion point.
            parentBox = createLineBoxes(r->obj->parent());

        // Append the inline box to this line.
        parentBox->addToLine(r->box);
    }

    // We should have a root inline box.  It should be unconstructed and
    // be the last continuation of our line list.
    KHTMLAssert(lastLineBox() && !lastLineBox()->isConstructed());

    // Set bits on our inline flow boxes that indicate which sides should
    // paint borders/margins/padding.  This knowledge will ultimately be used when
    // we determine the horizontal positions and widths of all the inline boxes on
    // the line.
    RenderObject* endObject = 0;
    bool lastLine = !end.obj;
    if (end.obj && end.pos == 0)
        endObject = end.obj;
    // FIXME: this is a dummy call, the function has no implementation
    lastLineBox()->determineSpacingForFlowBoxes(lastLine, endObject);

    // Now mark the line boxes as being constructed.
    lastLineBox()->setConstructed();

    // Return the last line.
    return lastLineBox();
}

// collects one line of the paragraph and transforms it to visual order
void RenderBlock::bidiReorderLine(const BidiIterator &start, const BidiIterator &end)
{
    if ( start == end ) {
	if ( start.current() == '\n' ) {
	    m_height += lineHeight( m_firstLine );
	}
	return;
    }
#if BIDI_DEBUG > 1
    kdDebug(6041) << "reordering Line from " << start.obj << "/" << start.pos << " to " << end.obj << "/" << end.pos << endl;
#endif

    //    context->ref();

    dir = QChar::DirON;
    emptyRun = true;

    numSpaces = 0;

    current = start;
    last = current;
    bool atEnd = false;
    while( 1 ) {

        QChar::Direction dirCurrent;
        if(atEnd ) {
            //kdDebug(6041) << "atEnd" << endl;
            BidiContext *c = context;
	    if ( current.atEnd())
		while ( c->parent )
		    c = c->parent;
            dirCurrent = c->dir;
        } else {
            dirCurrent = current.direction();
	}

#ifndef QT_NO_UNICODETABLES

	if ( context->override &&
	     dirCurrent != QChar::DirRLE &&
	     dirCurrent != QChar::DirLRE &&
	     dirCurrent != QChar::DirRLO &&
	     dirCurrent != QChar::DirLRO &&
	     dirCurrent != QChar::DirPDF ) {
	    eor = current;
	    goto skipbidi;
	}

#if BIDI_DEBUG > 1
        kdDebug(6041) << "directions: dir=" << dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << (int)context->dir << " level =" << (int)context->level << endl;
#endif
        switch(dirCurrent) {

            // embedding and overrides (X1-X9 in the Bidi specs)
        case QChar::DirRLE:
        case QChar::DirLRE:
        case QChar::DirRLO:
        case QChar::DirLRO:
        case QChar::DirPDF:
	    eor = last;
	    embed( dirCurrent );
	    break;

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
                    appendRun();
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
                            if(!(status.eor == QChar::DirR)) {
                                // AN or EN
                                appendRun();
                                dir = QChar::DirR;
                            }
                            else
                                eor = last;
                            appendRun();
			    dir = QChar::DirL;
			    status.eor = QChar::DirL;
                        } else {
                            if(status.eor == QChar::DirR) {
                                appendRun();
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
                    appendRun();
		    dir = QChar::DirR;
		    eor = current;
		    status.eor = QChar::DirR;
                    break;
                case QChar::DirES:
                case QChar::DirET:
                case QChar::DirCS:
                case QChar::DirBN:
                case QChar::DirB:
                case QChar::DirS:
                case QChar::DirWS:
                case QChar::DirON:
                    if( !(status.eor == QChar::DirR) && !(status.eor == QChar::DirAL) ) {
                        //last stuff takes embedding dir
                        if(context->dir == QChar::DirR || status.lastStrong == QChar::DirR) {
                            appendRun();
                            dir = QChar::DirR;
                            eor = current;
			    status.eor = QChar::DirR;
                        } else {
                            eor = last;
                            appendRun();
                            dir = QChar::DirR;
			    status.eor = QChar::DirR;
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
            if(!(status.lastStrong == QChar::DirAL)) {
                // if last strong was AL change EN to AN
                if(dir == QChar::DirON) {
                    if(status.lastStrong == QChar::DirAL)
                        dir = QChar::DirAN;
                    else
                        dir = QChar::DirL;
                }
                switch(status.last)
                    {
                    case QChar::DirET:
			if ( status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL ) {
			    appendRun();
			    dir = QChar::DirEN;
			    status.eor = QChar::DirEN;
			}
			// fall through
                    case QChar::DirEN:
                    case QChar::DirL:
                        eor = current;
                        status.eor = dirCurrent;
                        break;
                    case QChar::DirR:
                    case QChar::DirAL:
                    case QChar::DirAN:
                        appendRun();
			status.eor = QChar::DirEN;
                        dir = QChar::DirEN;
			break;
                    case QChar::DirES:
                    case QChar::DirCS:
                        if(status.eor == QChar::DirEN) {
                            eor = current; break;
                        }
                    case QChar::DirBN:
                    case QChar::DirB:
                    case QChar::DirS:
                    case QChar::DirWS:
                    case QChar::DirON:
                        if(status.eor == QChar::DirR) {
                            // neutrals go to R
                            eor = last;
                            appendRun();
                            dir = QChar::DirEN;
			    status.eor = QChar::DirEN;
                        }
                        else if( status.eor == QChar::DirL ||
                                 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                            eor = current; status.eor = dirCurrent;
                        } else {
                            // numbers on both sides, neutrals get right to left direction
                            if(dir != QChar::DirL) {
                                appendRun();
                                eor = last;
                                dir = QChar::DirR;
                                appendRun();
                                dir = QChar::DirEN;
				status.eor = QChar::DirEN;
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
                    appendRun();
		    dir = QChar::DirAN; status.eor = QChar::DirAN;
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
                        appendRun();
                        dir = QChar::DirAN;
			status.eor = QChar::DirAN;
                    } else if( status.eor == QChar::DirL ||
                               (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                        eor = current; status.eor = dirCurrent;
                    } else {
                        // numbers on both sides, neutrals get right to left direction
                        if(dir != QChar::DirL) {
                            appendRun();
                            eor = last;
                            dir = QChar::DirR;
                            appendRun();
                            dir = QChar::DirAN;
			    status.eor = QChar::DirAN;
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
	    numSpaces++;
        case QChar::DirON:
            break;
        default:
            break;
        }

    skipbidi:
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
	    case QChar::DirEN:
		if ( status.last == QChar::DirL ) {
		    status.last = QChar::DirL;
		    break;
		}
		// fall through
            default:
                status.last = dirCurrent;
            }
#endif

	if ( atEnd ) break;
        last = current;

	if ( emptyRun ) {
	    sor = current;
	    eor = current;
	    emptyRun = false;
	}

	// this causes the operator ++ to open and close embedding levels as needed
	// for the CSS unicode-bidi property
	adjustEmbeddding = true;
        ++current;
	adjustEmbeddding = false;

	if ( current == end ) {
	    if ( emptyRun )
		break;
	    atEnd = true;
	}
    }

#if BIDI_DEBUG > 0
    kdDebug(6041) << "reached end of line current=" << current.obj << "/" << current.pos
		  << ", eor=" << eor.obj << "/" << eor.pos << endl;
#endif
    if ( !emptyRun && sor != current ) {
	    eor = last;
	    appendRun();
    }

    BidiContext *endEmbed = context;
    // both commands below together give a noop...
    //endEmbed->ref();
    //context->deref();

    // reorder line according to run structure...

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    BidiRun *r = sruns->first();

    while ( r ) {
        //paintf("level = %d\n", r->level);
        if ( r->level > levelHigh )
            levelHigh = r->level;
        if ( r->level < levelLow )
            levelLow = r->level;
        r = sruns->next();
    }

    // implements reordering of the line (L2 according to Bidi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if( !(levelLow%2) ) levelLow++;

#if BIDI_DEBUG > 0
    kdDebug(6041) << "lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
    kdDebug(6041) << "logical order is:" << endl;
    QPtrListIterator<BidiRun> it2(*sruns);
    BidiRun *r2;
    for ( ; (r2 = it2.current()); ++it2 )
        kdDebug(6041) << "    " << r2 << "  start=" << r2->start << "  stop=" << r2->stop << "  level=" << (uint)r2->level << endl;
#endif

    int count = sruns->count() - 1;

    // do not reverse for visually ordered web sites
    if(!style()->visuallyOrdered()) {
        while(levelHigh >= levelLow) {
            int i = 0;
            while ( i < count ) {
                while(i < count && sruns->at(i)->level < levelHigh)
                    i++;
                int start = i;
                while(i <= count && sruns->at(i)->level >= levelHigh)
                    i++;
                int end = i-1;

                if(start != end) {
                    //kdDebug(6041) << "reversing from " << start << " to " << end << endl;
                    for(int j = 0; j < (end-start+1)/2; j++)
                        {
                            BidiRun *first = sruns->take(start+j);
                            BidiRun *last = sruns->take(end-j-1);
                            sruns->insert(start+j, last);
                            sruns->insert(end-j, first);
                        }
                }
                i++;
                if(i >= count) break;
            }
            levelHigh--;
        }
    }

#if BIDI_DEBUG > 0
    kdDebug(6041) << "visual order is:" << endl;
    QPtrListIterator<BidiRun> it3(*sruns);
    BidiRun *r3;
    for ( ; (r3 = it3.current()); ++it3 )
    {
        kdDebug(6041) << "    " << r3 << endl;
    }
#endif
}

void RenderBlock::computeVerticalPositionsForLine(InlineFlowBox* lineBox, BidiContext* endEmbed)
{
    int maxPositionTop = 0;
    int maxPositionBottom = 0;
    int maxAscent = 0;
    int maxDescent = 0;
    BidiRun *r = sruns->first();
    while ( r ) {
        int height = r->obj->lineHeight( m_firstLine);
	int baseline = r->obj->baselinePosition( m_firstLine);
        r->box->setHeight(height);
	r->box->setBaseline(baseline);
// 	if ( r->baseline > r->height )
// 	    r->baseline = r->height;
	// FIXME: where did this go in WebCore?
        r->vertical = r->obj->verticalPositionHint( m_firstLine );
        //kdDebug(6041) << "object="<< r->obj << " height="<<r->height<<" baseline="<< r->baseline << " vertical=" << r->vertical <<endl;
        //int ascent;
        if ( r->vertical == PositionTop ) {
            if ( maxPositionTop < height ) maxPositionTop = height;
        }
        else if ( r->vertical == PositionBottom ) {
            if ( maxPositionBottom < height ) maxPositionBottom = height;
        }
        else {
            int ascent = baseline - r->vertical;
            int descent = height - ascent;
            if(maxAscent < ascent) maxAscent = ascent;
            if(maxDescent < descent) maxDescent = descent;
        }
        r = sruns->next();
    }
    if ( maxAscent+maxDescent < kMax( maxPositionTop, maxPositionBottom ) ) {
        // now the computed lineheight needs to be extended for the
        // positioned elements
        // see khtmltests/rendering/html_align.html
        // ### only iterate over the positioned ones!
        for ( r = sruns->first(); r; r = sruns->next() ) {
	    int height = r->box->height();
            if ( r->vertical == PositionTop ) {
                if ( maxAscent + maxDescent < height )
                    maxDescent = height - maxAscent;
            }
            else if ( r->vertical == PositionBottom ) {
                if ( maxAscent + maxDescent < height )
                    maxAscent = height - maxDescent;
            }
            else
                continue;

            if ( maxAscent + maxDescent >= kMax( maxPositionTop, maxPositionBottom ) )
                break;

        }
    }
    int maxHeight = maxAscent + maxDescent;
    // CSS2: 10.8.1: line-height on the block level element specifies the *minimum*
    // height of the generated line box
    r = sruns->first();
    // ### we have no reliable way of detecting empty lineboxes - which
    // are not allowed to have any height. sigh.(Dirk)
//     if ( r ) {
//         int blockHeight = lineHeight( m_firstLine );
//         if ( blockHeight > maxHeight )
//             maxHeight = blockHeight;
//     }
    int totWidth = 0;
#if BIDI_DEBUG > 0
    kdDebug( 6040 ) << "starting run.." << endl;
#endif
    while ( r ) {
        int width;
        if(r->vertical == PositionTop)
            r->vertical = m_height;
        else if(r->vertical == PositionBottom)
            r->vertical = m_height + maxHeight - r->box->height();
        else
            r->vertical += m_height + maxAscent - r->box->baseline();

        if(r->obj->isText())
            width = static_cast<RenderText *>(r->obj)->width(r->start, r->stop-r->start, m_firstLine);
        else {
            r->obj->calcWidth();
            width = r->obj->width()+r->obj->marginLeft()+r->obj->marginRight();
        }
	r->box->setWidth(width);
#if BIDI_DEBUG > 0
	kdDebug(6040) << "object="<< r->obj << " placing at vertical=" << r->vertical <<" width=" << r->box->width() <<endl;
#endif
        totWidth += width;
        r = sruns->next();
    }
    //kdDebug(6040) << "yPos of line=" << m_height << "  lineBoxHeight=" << maxHeight << endl;

    // now construct the reordered string out of the runs...

    r = sruns->first();
    int x = leftOffset(m_height);
    int availableWidth = lineWidth(m_height);
    switch(style()->textAlign()) {
    case LEFT:
	numSpaces = 0;
        break;
    case JUSTIFY:
        if(numSpaces != 0 && !current.atEnd() && !current.obj->isBR() )
            break;
	// fall through
    case TAAUTO:
	numSpaces = 0;
        // for right to left fall through to right aligned
	if ( endEmbed->basicDir == QChar::DirL )
	    break;
    case RIGHT:
        x += availableWidth - totWidth;
	numSpaces = 0;
        break;
    case CENTER:
    case KONQ_CENTER:
        int xd = (availableWidth - totWidth)/2;
        x += xd>0?xd:0;
	numSpaces = 0;
        break;
    }
    while ( r ) {
#if BIDI_DEBUG > 1
        kdDebug(6040) << "positioning " << r->obj->renderName() << "@" << r->obj << " start=" << r->start << " stop=" << r->stop << " x=" << x << " width=" << r->box->width() << " yPos=" << r->vertical << endl;
#endif
	int spaceAdd = 0;
	if ( numSpaces > 0 ) {
	    if ( r->obj->isText() ) {
		// get number of spaces in run
		int spaces = 0;
		for ( int i = r->start; i < r->stop; i++ )
		    if ( static_cast<RenderText *>(r->obj)->text()[i].direction() == QChar::DirWS )
			spaces++;
		if ( spaces > numSpaces ) // should never happen...
		    spaces = numSpaces;
		spaceAdd = (availableWidth - totWidth)*spaces/numSpaces;
		numSpaces -= spaces;
		totWidth += spaceAdd;
	    }
	}
	r->box->setXPos(x);
	r->box->setYPos(r->vertical);
	// FIXME: this assignment is probably redundant
//	r->box->setBaseline(baselinePosition( m_firstLine ));
	r->box->setFirstLineStyleBit(m_firstLine);
//	kdDebug(6040) << "start: " << r->start << " stop: " << r->stop << endl;
        if (r->box->isInlineTextBox())
            x += r->box->width() + spaceAdd;
        r->obj->position(r->box, r->start, r->stop - r->start, r->level%2, spaceAdd);
	if (r->box->isInlineTextBox())
	    r->box->setWidth(r->box->width() + spaceAdd);
        else
            x += r->box->width() + spaceAdd;
        r = sruns->next();
    }

    m_height += maxHeight;

    int bottomOfLine = lineBox->bottomOverflow();
    if ( bottomOfLine > m_height && bottomOfLine > m_overflowHeight )
        m_overflowHeight = bottomOfLine;
}

void RenderBlock::layoutInlineChildren( bool relayoutChildren )
{
    m_overflowHeight = 0;

    invalidateVerticalPositions();
#ifdef DEBUG_LAYOUT
    QTime qt;
    qt.start();
    kdDebug( 6040 ) << renderName() << " layoutInlineChildren( " << this <<" )" << endl;
#endif
#if BIDI_DEBUG > 1 || defined( DEBUG_LINEBREAKS )
    kdDebug(6041) << " ------- bidi start " << this << " -------" << endl;
#endif
    int toAdd = style()->borderBottomWidth();
    m_height = style()->borderTopWidth();

    emptyRun = true;

    m_height += paddingTop();
    toAdd += paddingBottom();

    // Clear out our line boxes.
    deleteLineBoxes();

    if(firstChild()) {
        // layout replaced elements
        RenderObject *o = first( this );
        while ( o ) {
            if(o->isReplaced() || o->isFloating() || o->isPositioned()) {
                //kdDebug(6041) << "layouting replaced or floating child" << endl;
                if (relayoutChildren || o->style()->width().isPercent() || o->style()->height().isPercent())
                    o->setLayouted(false);
                if( !o->layouted() )
                    o->layout();
                if ( o->isPositioned() )
                    o->containingBlock()->insertPositionedObject(  o );
            }
            else if(o->isText())
                static_cast<RenderText *>(o)->deleteTextBoxes();
            else if (o->isInlineFlow() /*&& !endOfInline*/)
                static_cast<RenderFlow*>(o)->deleteLineBoxes();
            o = Bidinext( this, o );
        }

        BidiContext *startEmbed;
        status = BidiStatus();
        if( style()->direction() == LTR ) {
            startEmbed = new BidiContext( 0, QChar::DirL );
            status.eor = QChar::DirL;
        } else {
            startEmbed = new BidiContext( 1, QChar::DirR );
            status.eor = QChar::DirR;
        }
        startEmbed->ref();

        context = startEmbed;
	adjustEmbeddding = true;
        BidiIterator start(this);
	adjustEmbeddding = false;
        BidiIterator end(this);

        m_firstLine = true;
        while( !end.atEnd() ) {
            QPtrList<BidiRun> runs;
            runs.setAutoDelete(true);
            sruns = &runs;

            start = end;

            end = findNextLineBreak(start);
            if( start.atEnd() ) break;
	    bidiReorderLine(start, end);

	    {
                if (sruns && sruns->count() > 0) {
                    InlineFlowBox* lineBox = constructLine(start, end);
                    if (lineBox) {
                        // Now we position all of our text boxes horizontally.
//                        computeHorizontalPositionsForLine(lineBox, context);

                        // Now position our text boxes vertically.
//                        computeVerticalPositionsForLine(lineBox);
			computeVerticalPositionsForLine(lineBox, context);

			// FIXME: needs porting of sruns to s*BidiRun stuff
                        //deleteBidiRuns(renderArena());
                    }
                }
	    }

            if( end == start || (end.obj && end.obj->isBR() && !start.obj->isBR() ) ) {
		adjustEmbeddding = true;
                ++end;
		adjustEmbeddding = false;
	    } else if(m_pre && end.current() == QChar('\n') ) {
		adjustEmbeddding = true;
                ++end;
		adjustEmbeddding = false;
            }

            newLine();
            m_firstLine = false;
        }
	sruns = 0;

	// clean up
	while ( context ) {
	    BidiContext *parent = context->parent;
	    delete context;
	    context = parent;
	}
    }
    // in case we have a float on the last line, it might not be positioned up to now.
    positionNewFloats();

    m_height += toAdd;

    // Always make sure this is at least our height.
    if (m_overflowHeight < m_height)
        m_overflowHeight = m_height;

#if BIDI_DEBUG > 1
    kdDebug(6041) << " ------- bidi end " << this << " -------" << endl;
#endif
    //kdDebug() << "RenderBlock::layoutInlineChildren time used " << qt.elapsed() << endl;
    //kdDebug(6040) << "height = " << m_height <<endl;
}

BidiIterator RenderBlock::findNextLineBreak(BidiIterator &start)
{
    int width = lineWidth(m_height);
    int w = 0;
    int tmpW = 0;
#ifdef DEBUG_LINEBREAKS
    kdDebug(6041) << "RenderFlow::findNextLineBreak: " << this << endl;
    kdDebug(6041) << "findNextLineBreak: line at " << m_height << " line width " << width << endl;
    kdDebug(6041) << "sol: " << start.obj << " " << start.pos << endl;
#endif


    // remove leading spaces
    while(!start.atEnd() && ( start.obj->isInlineFlow() || ( start.obj->style()->whiteSpace() != PRE &&
#ifndef QT_NO_UNICODETABLES
	      ( start.direction() == QChar::DirWS || start.obj->isFloatingOrPositioned() )
#else
	      ( start.current() == ' ' || start.obj->isFloatingOrPositioned() )
#endif
          ))) {
        RenderObject *o = start.obj;
        // add to floating objects...
        if(o->isFloating()) {
            insertFloatingObject(o);
            positionNewFloats();
            width = lineWidth(m_height);
        }
        else if (o->isPositioned()) {
// merge me
#if 0
            if (o->hasStaticX())
                o->setStaticX(style()->direction() == LTR ?
                              borderLeft()+paddingLeft() :
                              borderRight()+paddingRight());
            if (o->hasStaticY())
                o->setStaticY(m_height);
#endif
        }

        adjustEmbeddding = true;
        ++start;
        adjustEmbeddding = false;
    }
    if ( start.atEnd() )
        return start;

    BidiIterator lBreak = start;

    RenderObject *o = start.obj;
    RenderObject *last = o;
    int pos = start.pos;

    while( o ) {
#ifdef DEBUG_LINEBREAKS
        kdDebug(6041) << "new "<< o->renderName() << "@"<< o <<" width = " << w <<" tmpw = " << tmpW << endl;
#endif
        if(o->isBR()) {
            if( w + tmpW <= width ) {
                lBreak = o;
                //check the clear status
                m_clearStatus =  (EClear) (m_clearStatus | o->style()->clear());
            }
            goto end;
        } else if(o->isFloating()) {
            insertFloatingObject(o);
            // check if it fits in the current line.
            // If it does, position it now, otherwise, position
            // it after moving to next line (in newLine() func)
            if (o->width()+o->marginLeft()+o->marginRight()+w+tmpW <= width) {
                positionNewFloats();
                width = lineWidth(m_height);
            }
        } else if(o->isPositioned()) {
            // ignore
        } else if ( o->isText() ) {
	    RenderText *t = static_cast<RenderText *>(o);
	    int strlen = t->stringLength();
	    int len = strlen - pos;
	    QChar *str = t->text();
            bool appliedStartWidth = ( pos > 0 );

            if (style()->whiteSpace() == NOWRAP || t->style()->whiteSpace() == NOWRAP) {
                tmpW += t->maxWidth();
                pos = strlen;
                len = 0;
            } else {
                const Font *f = t->htmlFont( m_firstLine );
                // proportional font, needs a bit more work.
                int lastSpace = pos;
                bool isPre = style()->whiteSpace() == PRE;
                while(len) {
                    if( (isPre && str[pos] == '\n') ||
                        (!isPre && isBreakable( str, pos, strlen ) ) ) {
		    tmpW += t->width(lastSpace, pos - lastSpace, f);
                    if ( !appliedStartWidth ) {
                        tmpW += inlineWidth( o, true, false );
                        appliedStartWidth = true;
                    }
#ifdef DEBUG_LINEBREAKS
		    kdDebug(6041) << "found space: '" << QString( str, pos ).latin1() << "' +" << tmpW << " -> w = " << w << endl;
#endif
		    if ( !isPre && w + tmpW > width && w == 0 ) {
			int fb = floatBottom();
			int newLineWidth = lineWidth(fb);
			if(!w && m_height < fb && width < newLineWidth) {
			    m_height = fb;
			    width = newLineWidth;
#ifdef DEBUG_LINEBREAKS
			    kdDebug() << "RenderBlock::findNextLineBreak new position at " << m_height << " newWidth " << width << endl;
#endif
			}
		    }
		    if ( !isPre && w + tmpW > width )
			goto end;

		    lBreak.obj = o;
		    lBreak.pos = pos;

		    if( str[pos] == '\n' ) {
#ifdef DEBUG_LINEBREAKS
			kdDebug(6041) << "forced break sol: " << start.obj << " " << start.pos << "   end: " << lBreak.obj << " " << lBreak.pos << "   width=" << w << endl;
#endif
			return lBreak;
		    }
		    w += tmpW;
		    tmpW = 0;
		    lastSpace = pos;
		}
		pos++;
		len--;
                }
                // IMPORTANT: pos is > length here!
                tmpW += t->width(lastSpace, pos - lastSpace, f);
                if ( !appliedStartWidth )
                    tmpW += inlineWidth(o, true, false );
                tmpW += inlineWidth(o, false, true );
                if (!isPre && w + tmpW < width && pos && str[pos-1] != nbsp)
                    lBreak =  Bidinext( start.par, o );
            }
        } else if ( o->isReplaced() ) {
            tmpW += o->width()+o->marginLeft()+o->marginRight();

            if ( w + tmpW > width )
                goto end;

            if (o->style()->whiteSpace() != NOWRAP) {
                w += tmpW;
                tmpW = 0;
                lBreak = o;
                lBreak.pos = pos;
            }
        } else
            KHTMLAssert( false );

        if( w + tmpW > width+1 && style()->whiteSpace() == NORMAL /*&& o->style()->whiteSpace() != NOWRAP*/ ) {
            //kdDebug() << " too wide w=" << w << " tmpW = " << tmpW << " width = " << width << endl;
	    //kdDebug() << "start=" << start.obj << " current=" << o << endl;
            // if we have floats, try to get below them.
            int fb = floatBottom();
	    int newLineWidth = lineWidth(fb);
            if( !w && m_height < fb && width < newLineWidth ) {
                m_height = fb;
                width = newLineWidth;
#ifdef DEBUG_LINEBREAKS
                kdDebug() << "RenderBlock::findNextLineBreak new position at " << m_height << " newWidth " << width << endl;
#endif
            }
	    if( !w && w + tmpW > width+1 && (o != start.obj || (unsigned) pos != start.pos) ) {
		// getting below floats wasn't enough...
		//kdDebug() << "still too wide w=" << w << " tmpW = " << tmpW << " width = " << width << endl;
		lBreak = o;
                if (o == last)
                    lBreak.pos = pos;
                if (unsigned ( pos ) >= o->length())
                    lBreak = Bidinext(start.par, o);
            }
            goto end;
        }

        last = o;
        o = Bidinext( start.par, o );
        pos = 0;
    }

#ifdef DEBUG_LINEBREAKS
    kdDebug( 6041 ) << "end of par, width = " << width << " linewidth = " << w + tmpW << endl;
#endif
    if( w + tmpW <= width )
        lBreak = 0;

 end:

    if( lBreak == start && !lBreak.obj->isBR() ) {
        // we just add as much as possible
        if ( m_pre )
            lBreak = pos ? Bidinext(start.par, o) : o;
        else {
	    if( last != o ) {
		// better break between object boundaries than in the middle of a word
		lBreak = o;
	    } else {
		int w = 0;
		if( lBreak.obj->isText() )
		    w += static_cast<RenderText *>(lBreak.obj)->width(lBreak.pos, 1);
		else
		    w += lBreak.obj->width();
		while( lBreak.obj && w < width ) {
		    ++lBreak;
		    if( !lBreak.obj ) break;
		    if( lBreak.obj->isText() )
			w += static_cast<RenderText *>(lBreak.obj)->width(lBreak.pos, 1);
		    else
			w += lBreak.obj->width();
		}
	    }
        }
    }

    // make sure we consume at least one char/object.
    if( lBreak == start )
        ++lBreak;

#ifdef DEBUG_LINEBREAKS
    kdDebug(6041) << "regular break sol: " << start.obj << " " << start.pos << "   end: " << lBreak.obj << " " << lBreak.pos << "   width=" << w << "/" << width << endl;
#endif
    return lBreak;
}

// For --enable-final
#undef BIDI_DEBUG
#undef DEBUG_LINEBREAKS
#undef DEBUG_LAYOUT
