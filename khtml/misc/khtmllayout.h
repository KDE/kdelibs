/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@kde.org)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$

    This widget holds some useful definitions needed for layouting Elements
*/
#ifndef HTML_LAYOUT_H
#define HTML_LAYOUT_H


/*
 * this namespace contains definitions for various types needed for
 * layouting.
 */
namespace khtml
{

    const int UNDEFINED = -1;

    // alignment
    enum VAlign { VNone=0, Bottom, VCenter, Top, Baseline };
    enum HAlign { HDefault, Left, HCenter, Right, HNone = 0 };

    /*
     * %multiLength and %Length
     */
    enum LengthType { Variable = 0, Relative, Percent, Fixed, Static };
    class Length
    {
    public:
	Length() : _length(0) {}
        Length(LengthType t) { l.value = 0; l.type = t; }
        Length(int v, LengthType t) { l.value = v; l.type = t; }

        bool operator==(const Length& o) const
            { return _length == o._length; }
        bool operator!=(const Length& o) const
            { return _length != o._length ; }

	/*
	 * works only for Fixed and Percent, returns -1 otherwise
	 */
	int width(int maxWidth) const
	    {
		switch(l.type)
		{
		case Fixed:
		    return l.value;
		case Percent:
		    return maxWidth*l.value/100;
		case Variable:
		    return maxWidth;
		default:
		    return -1;
		}
	    }
	/*
	 * returns the minimum width value which could work...
	 */
	int minWidth(int maxWidth) const
	    {
		switch(l.type)
		{
		case Fixed:
		    return l.value;
		case Percent:
		    return maxWidth*l.value/100;
		case Variable:
		default:
		    return 0;
		}
	    }
        bool isVariable() const { return (l.type == Variable); }
        bool isRelative() const { return (l.type == Relative); }
        bool isPercent() const { return (l.type == Percent); }
        bool isFixed() const { return (l.type == Fixed); }
        bool isStatic() const { return (l.type == Static); }

        int value() const { return l.value; }
        LengthType type() const { return l.type; }
        union {
          struct {
            int value : 29;
            LengthType type : 3;
          } l;
          Q_UINT32 _length;
       };
    };

};

#if __GNUC__ -  0 < 3 // stupid stl_relops.h
inline bool operator!=(khtml::LengthType __x, khtml::LengthType __y) {
  return !(__x == __y);
}
#endif

#endif
