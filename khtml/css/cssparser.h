/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 *
 * $Id$
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
 */
#ifndef _CSS_cssparser_h_
#define _CSS_cssparser_h_

#include <qstring.h>
#include <dom/dom_string.h>

namespace DOM {
    class StyleListImpl;
    class CSSStyleSheetImpl;
    class CSSRuleImpl;
    class CSSStyleRuleImpl;
    class DocumentImpl;
    class CSSValueImpl;
    class CSSValueListImpl;
    class CSSPrimitiveValueImpl;
    class CSSStyleDeclarationImpl;
    class CSSProperty;
    class CSSRuleListImpl;


    struct ParseString {
	unsigned short *string;
	int length;
    };

    struct Value;
    class ValueList;

    struct Function {
	ParseString name;
	ValueList *args;
    };

    struct Value {
	int id;
	union {
	    double fValue;
	    int iValue;
	    ParseString string;
	    struct Function *function;
	};
	enum {
	    Operator = 0x100000,
	    Function = 0x100001,
	    Q_EMS     = 0x100002
	};

	int unit;
    };

    static inline QString qString( const ParseString &ps ) {
	return QString( (QChar *)ps.string, ps.length );
    }
    static inline DOMString domString( const ParseString &ps ) {
	return DOMString( (QChar *)ps.string, ps.length );
    }

    class ValueList {
    public:
	ValueList();
	~ValueList();
	void addValue( const Value &val );
	Value *current() { return currentValue < numValues ? values + currentValue : 0; }
	Value *next() { ++currentValue; return current(); }
        bool isLast() const { return currentValue+1 >= numValues; }
	Value *values;
	int numValues;
	int maxValues;
	int currentValue;
    };

    class CSSParser
    {
    public:
	CSSParser( bool strictParsing = true );
	~CSSParser();

	void parseSheet( DOM::CSSStyleSheetImpl *sheet, const DOM::DOMString &string );
	DOM::CSSRuleImpl *parseRule( DOM::CSSStyleSheetImpl *sheet, const DOM::DOMString &string );
	bool parseValue( DOM::CSSStyleDeclarationImpl *decls, int id, const DOM::DOMString &string,
			 bool _important, bool _nonCSSHint );
	bool parseDeclaration( DOM::CSSStyleDeclarationImpl *decls, const DOM::DOMString &string,
			       bool _nonCSSHint );

	static CSSParser *current() { return currentParser; }


	DOM::DocumentImpl *document() const;

	void addProperty( int propId, CSSValueImpl *value, bool important );
	bool hasProperties() const { return numParsedProperties > 0; }
	CSSStyleDeclarationImpl *createStyleDeclaration( CSSStyleRuleImpl *rule );
	void clearProperties();

	bool parseValue( int propId, bool important, int expected=1 );
	bool parseShortHand( const int *properties, int numProperties, bool important );
	bool parse4Values( const int *properties, bool important );
	bool parseContent( int propId, bool important );
	bool parseShape( int propId, bool important );
	bool parseFont(bool important);

        // returns the found property
        // 0 if nothing found (or ok == false)
        // @param forward if true, it parses the next in the list
        CSSPrimitiveValueImpl *parseBackgroundPositionXY( int propId, bool forward, bool &ok );
	CSSValueListImpl *parseFontFamily();
	CSSPrimitiveValueImpl *parseColor();

    public:
	bool strict;
	bool important;
	bool nonCSSHint;
	unsigned int id;
	DOM::StyleListImpl* styleElement;
	DOM::CSSRuleImpl *rule;
	ValueList *valueList;
	CSSProperty **parsedProperties;
	int numParsedProperties;
	int maxParsedProperties;
	bool inParseShortHand;
	unsigned int defaultNamespace;
	static CSSParser *currentParser;

	// tokenizer methods and data
    public:
	int lex( void *yylval );
	int token() { return yyTok; }
	unsigned short *text( int *length);
	int lex();
    private:
	int yyparse();
        void runParser(int length);

	unsigned short *data;
	unsigned short *yytext;
	unsigned short *yy_c_buf_p;
	unsigned short yy_hold_char;
	int yy_last_accepting_state;
	unsigned short *yy_last_accepting_cpos;
        int block_nesting;
	int yyleng;
	int yyTok;
	int yy_start;
    };

} // namespace
#endif
