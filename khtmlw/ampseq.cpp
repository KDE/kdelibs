// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

    with some minor changes by Norman Markgarf

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
#define  Nrm (0)         // normal font face
#define  Sym (1)         // 'symbol' font face

// --> 127

ampseq_s AmpSequences[] = {
        {"AElig"   	,   0, 198},	// 9
	{"AMP"   	,   0,  38},
	{"Aacute"   	,   0, 193},
	{"Acirc"   	,   0, 194},
	{"Agrave"   	,   0, 192},
	{"Alpha"        , Sym,  65},
	{"Aring"   	,   0, 197},
	{"Atilde"   	,   0, 195},
	{"Auml"   	,   0, 196},
	{"Beta"         , Sym,  66},
	{"Ccedil"   	,   0, 199},
	{"Chi"          , Sym,  87},
	{"Delta"        , Sym,  68},
	{"ETH"   	,   0, 208},
	{"Eacute"   	,   0, 201},
	{"Ecirc"   	,   0, 202},
	{"Egrave"   	,   0, 200},
	{"Epsilon"      , Sym,  69},
	{"Eta"          , Sym,  71},
	{"Euml"   	,   0, 203},
	{"GT"   	,   0,  62},
	{"Gamma"        , Sym,  67},
	{"Iacute"   	,   0, 205},
	{"Icirc"   	,   0, 206},
	{"Igrave"   	,   0, 204},
	{"Iota"         , Sym,  73},
	{"Iuml"   	,   0, 207},
	{"Kappa"        , Sym,  74},
	{"LT"   	,   0,  60},
	{"Lambda"       , Sym,  75},
	{"Mu"           , Sym,  76},
	{"Ntilde"   	,   0, 209},
	{"Nu"           , Sym,  77},
	{"Oacute"   	,   0, 211},
	{"Ocirc"   	,   0, 212},
	{"Ograve"   	,   0, 210},
	{"Omega"        , Sym,  89},
	{"Omicron"      , Sym,  79},
	{"Oslash"   	,   0, 216},
	{"Otilde"   	,   0, 213},
	{"Ouml"   	,   0, 214},
	{"Phi"          ,   0,  86},
	{"Pi"           , Sym,  80},
	{"Psi"          , Sym,  88},
	{"QUOT"   	,   0,  34},
	{"Rho"          , Sym,  81},
	{"Sigma"        , Sym,  83},
	{"THORN"   	,   0, 222},
	{"Tau"          , Sym,  84},
	{"Theta"        , Sym,  72},
	{"Uacute"   	,   0, 218},
	{"Ucirc"   	,   0, 219},
	{"Ugrave"   	,   0, 217},
	{"Upsilon"      , Sym,  85},
	{"Uuml"   	,   0, 220},
	{"Xi"           , Sym,  78},
	{"Yacute"   	,   0, 221},
	{"Zeta"         , Sym,  70},
	{"acute"   	,   0, 180},
	{"aacute"   	,   0, 225},
	{"acirc"   	,   0, 226},
	{"aelig"   	,   0, 230},
	{"agrave"   	,   0, 224},
	{"amp"   	,   0,  38},
	{"aring"   	,   0, 229},
	{"atilde"   	,   0, 227},
	{"auml"   	,   0, 228},
	{"brvbar"   	,   0, 166},
	{"ccedil"   	,   0, 231},
	{"cedil"   	,   0, 184},
	{"cent"   	,   0, 162},
	{"copy"   	,   0, 169},
	{"curren"   	,   0, 164},
	{"deg"   	,   0, 176},
	{"divide"   	,   0, 247},
	{"eacute"   	,   0, 233},
	{"ecirc"   	,   0, 234},
	{"egrave"   	,   0, 232},
	{"eth"   	,   0, 240},
	{"euml"   	,   0, 235},
	{"frac12"   	,   0, 189},
	{"frac14"   	,   0, 188},
	{"frac34"   	,   0, 190},
	{"gt"   	,   0,  62},
	{"iacute"   	,   0, 237},
	{"icirc"   	,   0, 238},
	{"iexcl"   	,   0, 161},
	{"igrave"   	,   0, 236},
	{"iquest"   	,   0, 191},
	{"iuml"   	,   0, 239},
	{"laqou"   	,   0, 171},
	{"lt"   	,   0,  60},
	{"macr"   	,   0, 175},
	{"micro"   	,   0, 181},
	{"middot"   	,   0, 183},
	{"nbsp"   	,   0,  32},
	{"not"   	,   0, 172},
	{"ntilde"   	,   0, 241},
	{"oacute"   	,   0, 243},
	{"ocirc"   	,   0, 244},
	{"ograve"   	,   0, 242},
	{"ordf"   	,   0, 170},
	{"ordm"   	,   0, 186},
	{"oslash"   	,   0, 248},
	{"otilde"   	,   0, 245},
	{"ouml"   	,   0, 246},
	{"para"   	,   0, 182},
	{"plusmn"   	,   0, 177},
	{"pound"   	,   0, 163},
	{"quot"   	,   0,  34},
	{"raqou"   	,   0, 186},
	{"reg"   	,   0, 174},
	{"sect"   	,   0, 167},
	{"shy"   	,   0, 173},
	{"sup1"   	,   0, 185},
	{"sup2"   	,   0, 178},
	{"sup3"   	,   0, 179},
	{"szlig"   	,   0, 223},
	{"thorn"   	,   0, 254},
	{"times"   	,   0, 215},
	{"uacute"   	,   0, 250},
	{"ucirc"   	,   0, 251},
	{"ugrave"   	,   0, 249},
	{"uml"   	,   0, 168},
	{"uuml"   	,   0, 252},
	{"yacute"   	,   0, 253},
	{"yen"   	,   0, 165},	// 135 - 9 = 126
};

char*   AmpSeqFontFaces[]={
        {""},           // use the standard font as set by setStandardFont()
        {"symbol"}, 
};

