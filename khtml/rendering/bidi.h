// $Id$

#ifndef BIDI_H
#define BIDI_H

#include <qstring.h>

namespace khtml {
    class RenderFlow;
    class RenderObject;

    class BidiContext {
    public:
	BidiContext(unsigned char level, QChar::Direction embedding, BidiContext *parent = 0, bool override = false);
	~BidiContext();

	void ref() const;
	void deref() const;

	unsigned char level;
	bool override : 1;
	QChar::Direction dir : 5;

	BidiContext *parent;


	// refcounting....
	mutable int count;
    };

    struct BidiRun {
	BidiRun(int _start, int _stop, RenderObject *_obj, BidiContext *context, QChar::Direction dir) {
	    start = _start;
	    stop = _stop;
	    obj = _obj;
	    if(dir == QChar::DirON) dir = context->dir;

	    level = context->level;

	    // add level of run (cases I1 & I2)
	    if( level % 2 ) {
		if(dir == QChar::DirL || dir == QChar::DirAN)
		    level++;
	    } else {
		if( dir == QChar::DirR )
		    level++;
		else if( dir == QChar::DirAN )
		    level += 2;
	    }

	    yOffset = 0;
	    width = 0;
	}

	int start;
	int stop;
	RenderObject *obj;

	// explicit + implicit levels here
	uchar level;
	int yOffset;
	int width;
    };

    // an iterator which goes through a BidiParagraph
    class BidiIterator
    {
    public:
	BidiIterator();
	BidiIterator(RenderFlow *par);
	BidiIterator(RenderFlow *par, RenderObject *_obj, int _pos = 0);

	BidiIterator(const BidiIterator &it);
	BidiIterator &operator = (const BidiIterator &it);

	void operator ++ ();

	bool atEnd();

	const QChar &current();
	QChar::Direction direction();

	RenderFlow *par;
	RenderObject *obj;
	unsigned int pos;

    };

    struct BidiStatus {
	BidiStatus() {
	    eor = QChar::DirON;
	    lastStrong = QChar::DirON;
	    last = QChar:: DirON;
	}
	QChar::Direction eor 		: 5;
	QChar::Direction lastStrong 	: 5;
	QChar::Direction last		: 5;
    };

};

#endif
