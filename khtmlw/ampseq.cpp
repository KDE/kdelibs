// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
*/

#include "ampseq.h"

ampseq_s AmpSequences[] = {
	{"lt",     '<'},		
	{"LT",     '<'},
	{"gt",     '>'},
	{"GT",     '>'},
	{"amp",    '&'},	// 5
	{"AMP",    '&'},		 
	{"quot",   '\"'},
	{"QUOT",   '\"'},
	{"Agrave", '\300'},
	{"Aacute", '\301'},	// 10
	{"Acirc",  '\302'},
	{"Atilde", '\303'},
	{"Auml",   '\304'},
	{"Aring",  '\305'},
	{"AElig",  '\306'},	// 15
	{"Ccedil", '\307'},
	{"Egrave", '\310'},
	{"Eacute", '\311'},
	{"Ecirc",  '\312'},
	{"Euml",   '\313'},	// 20
	{"Igrave", '\314'},
	{"Iacute", '\315'},
	{"Icirc",  '\316'},
	{"Iuml",   '\317'},
	{"ETH",    '\320'},	// 25
	{"Ntilde", '\321'},
	{"Ograve", '\322'},
	{"Oacute", '\323'},
	{"Ocirc",  '\324'},
	{"Otilde", '\325'},	// 30
	{"Ouml",   '\326'},
	{"Oslash", '\330'},
	{"Ugrave", '\331'},
	{"Uacute", '\332'},
	{"Ucirc",  '\333'},	// 35
	{"Uuml",   '\334'},
	{"Yacute", '\335'},
	{"THORN",  '\336'},
	{"szlig",  '\337'},
	{"agrave", '\340'},	// 40
	{"aacute", '\341'},
	{"acirc",  '\342'},
	{"atilde", '\343'},
	{"auml",   '\344'},
	{"aring",  '\345'},	// 45
	{"aelig",  '\346'},
	{"ccedil", '\347'},
	{"egrave", '\350'},
	{"eacute", '\351'},
	{"ecirc",  '\352'},	// 50
	{"euml",   '\353'},
	{"igrave", '\354'},
	{"iacute", '\355'},
	{"icirc",  '\356'},
	{"iuml",   '\357'},	// 55
	{"eth",    '\360'},
	{"ntilde", '\361'},
	{"ograve", '\362'},
	{"oacute", '\363'},
	{"ocirc",  '\364'},	// 60
	{"otilde", '\365'},
	{"ouml",   '\366'},
	{"oslash", '\370'},
	{"ugrave", '\371'},
	{"uacute", '\372'},	// 65
	{"ucirc",  '\373'},
	{"uuml",   '\374'},
	{"yacute", '\375'},
	{"thorn",  '\376'},
	{"yuml",   '\377'},	// 70
	{"copy",   '\251'},
	{"reg",    '\256'},
	{"nbsp",   ' '},
	{"laqou",  '\253'},
	{"raqou",  '\272'},	// 75
	{"shy",    '\255'},
	{"not",    '\254'},
	{"sup2",   '\260'},
	{"sup3",   '\261'},
	{"pound",  '\245'},	// 80
	{"yen",    '\247'},
	{"cent",   '\244'},
};
/*
	{"deg",    '\258'},
	{"plusmn", '\259'},
	{"brvbar", '\248'},
*/
