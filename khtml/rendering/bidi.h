/*
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
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
 */
#ifndef BIDI_H
#define BIDI_H

#include <qstring.h>

namespace khtml {
    class RenderArena;
    class RenderBlock;
    class RenderObject;
    class InlineBox;

    class BidiContext {
    public:
	BidiContext(unsigned char level, QChar::Direction embedding, BidiContext *parent = 0, bool override = false);
	~BidiContext();

	void ref() const;
	void deref() const;

	unsigned char level;
	bool override : 1;
	QChar::Direction dir : 5;
	QChar::Direction basicDir : 5;

	BidiContext *parent;


	// refcounting....
	mutable int count;
    };

    struct BidiRun {
	BidiRun(int _start, int _stop, RenderObject *_obj, BidiContext *context, QChar::Direction dir)
	    :  start( _start ), stop( _stop ), obj( _obj ), box(0), nextRun(0)
	{
	    if(dir == QChar::DirON) dir = context->dir;

	    level = context->level;

	    // add level of run (cases I1 & I2)
	    if( level % 2 ) {
		if(dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN)
		    level++;
	    } else {
		if( dir == QChar::DirR )
		    level++;
		else if( dir == QChar::DirAN || dir == QChar::DirEN)
		    level += 2;
	    }
	}

        void detach(RenderArena* renderArena);

        // Overloaded new operator.
        void* operator new(size_t sz, RenderArena* renderArena) throw();

        // Overridden to prevent the normal delete from being called.
        void operator delete(void* ptr, size_t sz);

private:
        // The normal operator new is disallowed.
        void* operator new(size_t sz) throw();

public:
	int start;
	int stop;

        RenderObject *obj;
        InlineBox* box;

	// explicit + implicit levels here
	uchar level;

        bool compact : 1;

        BidiRun* nextRun;
    };

    struct BidiIterator;
    struct BidiState;

};

#endif
