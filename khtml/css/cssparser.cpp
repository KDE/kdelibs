/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *               1999 Waldo Bastian (bastian@kde.org)
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
#include "css_stylesheetimpl.h"

#include "css_stylesheet.h"
#include "css_rule.h"
#include "css_ruleimpl.h"
#include "css_valueimpl.h"

#include "dom_string.h"
#include "dom_exception.h"
using namespace DOM;

#include <stdio.h>

#include "khtmltokenizer.h"
#include "khtmlattrs.h"
#include "cssproperties.h"
#include "cssvalues.h"

// ### this file should disappear at some point!
#include "khtmlstyle.h"

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1
#include "cssproperties.c"
#include "cssvalues.c"

int DOM::getPropertyID(const char *tagStr, int len)
{
    const struct props *propsPtr = findProp(tagStr, len);
    if (!propsPtr)
        return 0;

    return propsPtr->id;
}

// ------------------------------------------------------------------------------------------------------

bool StyleBaseImpl::deleteMe()
{
    if(!m_parent && _ref <= 0) return true;
    return false;
}

void StyleBaseImpl::setParent(StyleBaseImpl *parent)
{
    m_parent = parent;
}


/*
 * parsing functions for stylesheets
 */

bool isspace(const QChar &c)
{
     return (c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r');
}

const QChar *
StyleBaseImpl::parseSpace(const QChar *curP, const QChar *endP)
{
  bool sc = false;     // possible start comment?
  bool ec = false;     // possible end comment?
  bool ic = false;     // in comment?

  while (curP < endP)
  {
      if (ic)
      {
          if (ec && (*curP == '/'))
              ic = false;
          else if (*curP == '*')
              ec = true;
          else
              ec = false;
      }
      else if (sc && (*curP == '*'))
      {
          ic = true;
      }
      else if (*curP == '/')
      {
          sc = true;
      }
      else if (!isspace(*curP))
      {
          return(curP);
      }
      else
      {
          sc = false;
      }
      curP++;
  }

  return(0);
}

/*
 * ParseToChar
 *
 * Search for an expected character.  Deals with escaped characters,
 * quoted strings, and pairs of braces/parens/brackets.
 */
const QChar *
StyleBaseImpl::parseToChar(const QChar *curP, const QChar *endP, QChar c, bool chkws)
{
    //printf("parsetochar: \"%s\" searching %c ws=%d\n", QString(curP, endP-curP).latin1(), c.latin1(), chkws);

    bool sq = false; /* in single quote? */
    bool dq = false; /* in double quote? */
    bool esc = false; /* escape mode? */

    while (curP < endP)
    {
        if (esc)
            esc = false;
        else if (*curP == '\\')
            esc = true;
        else if (dq && (*curP == '"'))
            dq = false;
        else if (sq && (*curP == '\''))
            sq = false;
        else if (*curP == '"')
            dq = true;
        else if (*curP == '\'')
            sq = true;
        else if (*curP == c)
            return(curP);
        else if (chkws && isspace(*curP))
            return(curP);
        else if (*curP == '{')
        {
            curP = parseToChar(curP + 1, endP, '}', false);
            if (!curP)
                return(0);
        }
        else if (*curP == '(')
        {
            curP = parseToChar(curP + 1, endP, ')', false);
            if (!curP)
                return(0);
        }
        else if (*curP == '[')
        {
            curP = parseToChar(curP + 1, endP, ']', false);
            if (!curP)
                return(0);
        }
        curP++;
    }

    return(0);
}

CSSRuleImpl *
StyleBaseImpl::parseAtRule(const QChar *&curP, const QChar *endP)
{
    curP++;
    const QChar *startP = curP;
    while( *curP != ' ' && *curP != '{' && *curP != '\'')
	curP++;

    QString rule(startP, curP-startP);
    rule = rule.lower();

    printf("rule = '%s'\n", rule.ascii());

    if(rule == "import")
    {
	// ### load stylesheet and pass it over
	startP = curP++;
	curP = parseToChar(startP, endP, ';', false);
	printf("url = %s\n", QString(startP, curP - startP).ascii());
    }
    else if(rule == "charset")
    {
	// ### invoke decoder
	startP = curP++;
	curP = parseToChar(startP, endP, ';', false);
	printf("charset = %s\n", QString(startP, curP - startP).ascii());
    }
    else if(rule == "font-face")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	printf("font rule = %s\n", QString(startP, curP - startP).ascii());	
    }
    else if(rule == "media")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	printf("media rule = %s\n", QString(startP, curP - startP).ascii());	
    }
    else if(rule == "page")
    {
	startP = curP++;
	curP = parseToChar(startP, endP, '}', false);
	printf("page rule = %s\n", QString(startP, curP - startP).ascii());	
    }

	
    return 0;
}

CSSSelector *
StyleBaseImpl::parseSelector2(const QChar *curP, const QChar *endP)
{
    CSSSelector *cs = new CSSSelector();
    QString selecString( curP, endP - curP );

printf("selectString = \"%s\"\n", selecString.ascii());

    if (*curP == '#')
    {
	cs->tag = -1;
	cs->attr = ATTR_ID;
	cs->value = QString( curP + 1, endP - curP -1 );
    }
    else if (*curP == '.')
    {
	cs->tag = -1;
	cs->attr = ATTR_CLASS;
	cs->value = QString( curP + 1, endP - curP -1 );
    }
    else if (*curP == ':')
    {
	cs->tag = -1;
	cs->attr = ATTR_CLASS;
	cs->value = QString( curP, endP - curP );
    }
    else
    {
        const QChar *startP = curP;
	QString tag;
        while (curP < endP)
        {
            if (*curP =='#')
            {
                tag = QString( startP, curP-startP );
                QString tmp( curP + 1, endP - curP - 1);
                cs->attr = ATTR_ID;
		cs->value = tmp;
                break;
            }
            else if (*curP == '.')
            {
                tag = QString( startP, curP - startP );
                QString tmp( curP + 1, endP - curP - 1);
		cs->attr = ATTR_CLASS;
		cs->match = CSSSelector::List;
                cs->value = tmp;
                break;
            }
            else if (*curP == ':')
            {
                tag = QString( startP, curP - startP );
                QString tmp( curP, endP - curP );
                cs->value = tmp;
                break;
            }
            else if (*curP == '[')
            {
                tag = QString( startP, curP - startP );
		curP++;
		printf("tag = %s\n", tag.ascii());
		const QChar *equal = parseToChar(curP, endP, '=', false);
		QString attr;
		if(!equal)
		{
		    attr = QString( curP, endP - curP );
		    cs->match = CSSSelector::Set;
		}
		else
		{
		    // check relation: = / ~= / |=
		    if(*(equal-1) == '~')
		    {
			attr = QString( curP, equal - curP - 1 );
			cs->match = CSSSelector::List;
		    }
		    else if(*(equal-1) == '|')
		    {
			attr = QString( curP, equal - curP - 1 );
			cs->match = CSSSelector::Hyphen;
		    }
		    else
		    {
			attr = QString(curP, equal - curP );
			cs->match = CSSSelector::Exact;
		    }
		}
		cs->attr = getAttrID(attr.ascii(), attr.length());
		if(equal)
		{
		    equal++;
		    if(*equal == '\'')
		    {
			equal++;
			while(*endP != '\'' && endP > equal)
			    endP--;
		    }
		    else if(*equal == '\"')
		    {
			equal++;
			while(*endP != '\"' && endP > equal)
			    endP--;
		    }
		    else
			endP--;
		    cs->value = QString(equal, endP - equal);
		}
		break;
	    }
	    else
            {
                curP++;
            }
        }
        if (curP == endP)
        {
            tag = QString( startP, curP - startP );
        }
	if(tag == "*")
	    cs->tag = -1;
	else
	    cs->tag = getTagID(tag.lower().data(), tag.length());
   }
   if (cs->tag == 0)
   {
       delete cs;
       return(0);
   }
   printf("[Selector: tag=%d Attribute=%d relation=%d value=%s]\n", cs->tag, cs->attr, cs->match, cs->value.ascii());
   return(cs);
}

CSSSelector *
StyleBaseImpl::parseSelector1(const QChar *curP, const QChar *endP)
{
    //printf("selector1 is \'%s\'\n", QString(curP, endP-curP).latin1());

    CSSSelector *selecStack=0;

    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);

    const QChar *startP = curP;
    while (curP <= endP)
    {
        if ((curP == endP) || isspace(*curP) || *curP == '+' || *curP == '>')
        {
            CSSSelector *cs = parseSelector2(startP, curP);
            if (cs)
            {
                cs->tagHistory = selecStack;
                selecStack = cs;
            }
	    else
	    {
		// invalid selector, delete
		delete selecStack;
		return 0;
	    }
		
            curP = parseSpace(curP, endP);
            if (!curP)
                return(selecStack);

	    if(*curP == '+')
	    {
		cs->relation = CSSSelector::Sibling;
		curP++;
		curP = parseSpace(curP, endP);
	    }
	    else if(*curP == '>')
	    {
		cs->relation = CSSSelector::Child;
		curP++;
		curP = parseSpace(curP, endP);
	    }
	    if(cs)
		cs->print();
            startP = curP;
        }
        else
        {
            curP++;
        }
    }
    return(selecStack);
}

QList<CSSSelector> *
StyleBaseImpl::parseSelector(const QChar *curP, const QChar *endP)
{
    //printf("selector is \'%s\'\n", QString(curP, endP-curP).latin1());

    QList<CSSSelector> *slist  = 0;
    const QChar *startP;

    while (curP < endP)
    {
        startP = curP;
        curP = parseToChar(curP, endP, ',', false);
        if (!curP)
            curP = endP;

        CSSSelector *selector = parseSelector1(startP, curP);
        if (selector)
        {
            if (!slist)
            {
                slist = new QList<CSSSelector>;
		slist->setAutoDelete(true);
            }
            slist->append(selector);
        }
	else
	{
	    // invalid selector, delete
	    delete slist;
	    return 0;
	}
        curP++;
    }
    return slist;
}

CSSProperty *
StyleBaseImpl::parseProperty(const QChar *curP, const QChar *endP)
{
    bool important = false;
    const QChar *colon;
    // Get rid of space in front of the declaration

    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);

    // Search for the required colon or white space
    colon = parseToChar(curP, endP, ':', true);
    if (!colon)
        return(0);

    QString propName( curP, colon - curP );

printf("Property-name = \"%s\"\n", propName.data());
    // May have only reached white space before
    if (*colon != ':')
    {
        // Search for the required colon
        colon = parseToChar(curP, endP, ':', false);
        if (!colon)
            return(0);
    }
    // remove space in front of the value
    curP = parseSpace(colon+1, endP);
    if (!curP)
        return(0);

    // search for !important
    const QChar *exclam = parseToChar(curP, endP, '!', false);
    if(exclam)
    {
	const QChar *imp = parseSpace(exclam+1, endP);
	QString s(imp, endP - imp);
	s.lower();
	if(!s.contains("important"))
	    return 0;
	important = true;
	endP = exclam - 1;
	printf("important property!\n");
    }

    // remove space after the value;
    while (endP > curP)
    {
        if (!isspace(*(endP-1)))
            break;
        endP--;
    }


    QString propVal( curP , endP - curP );
printf("Property-value = \"%s\"\n", propVal.data());

    const struct props *propPtr = findProp(propName.lower().ascii(), propName.length());
    if (!propPtr)
    {
         printf("Unknown property\n");
         return (0);
    }

    CSSValueImpl *val = parseValue(curP, endP, propPtr->id);

    if(!val) return 0;

    CSSProperty *prop = new CSSProperty();
    prop->m_id = propPtr->id;

    prop->setValue(val);
    prop->m_bImportant = important;

    return(prop);
}

QList<CSSProperty> *
StyleBaseImpl::parseProperties(const QChar *curP, const QChar *endP)
{
    QList<CSSProperty> *propList=0;

    while (curP < endP)
    {
        const QChar *startP = curP;
        curP = parseToChar(curP, endP, ';', false);
        if (!curP)
            curP = endP;

        CSSProperty *prop = parseProperty(startP, curP);
        if (prop)
        {
            if (!propList)
            {
                propList = new QList<CSSProperty>;
		propList->setAutoDelete(true);
            }
            propList->append(prop);
        }
        curP++;
    }
    return propList;
}

CSSValueImpl *StyleBaseImpl::parseValue(const QChar *curP, const QChar *endP, int propId)
{
    printf("parseValue!\n");
    QString value(curP, endP - curP);
    value = value.lower();
    const char *val = value.ascii();

    if(!strcmp(val, "inherit"))
    {
	// inherited value
	return new CSSInheritedValueImpl();
    }

    switch(propId)
    {
    case CSS_PROP_AZIMUTH:
    case CSS_PROP_BACKGROUND_ATTACHMENT:
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BACKGROUND_IMAGE:
    case CSS_PROP_BACKGROUND_POSITION:
    case CSS_PROP_BACKGROUND_REPEAT:
    case CSS_PROP_BORDER_COLLAPSE:
    case CSS_PROP_CAPTION_SIDE:
    case CSS_PROP_CLEAR:
    case CSS_PROP_CLIP:
    case CSS_PROP_CONTENT:
    case CSS_PROP_COUNTER_INCREMENT:
    case CSS_PROP_COUNTER_RESET:
    case CSS_PROP_CUE_AFTER:
    case CSS_PROP_CUE_BEFORE:
    case CSS_PROP_CURSOR:
    case CSS_PROP_DIRECTION:
    case CSS_PROP_DISPLAY:
    case CSS_PROP_ELEVATION:
    case CSS_PROP_EMPTY_CELLS:
    case CSS_PROP_FLOAT:
    case CSS_PROP_FONT_SIZE_ADJUST:
    case CSS_PROP_FONT_STRETCH:
    case CSS_PROP_FONT_STYLE:
    case CSS_PROP_FONT_VARIANT:
    case CSS_PROP_FONT_WEIGHT:
    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_LINE_HEIGHT:
    case CSS_PROP_LIST_STYLE_IMAGE:
    case CSS_PROP_LIST_STYLE_POSITION:
    case CSS_PROP_LIST_STYLE_TYPE:
    case CSS_PROP_MAX_HEIGHT:
    case CSS_PROP_MAX_WIDTH:
    case CSS_PROP_OUTLINE_COLOR:
    case CSS_PROP_OUTLINE_STYLE:
    case CSS_PROP_OUTLINE_WIDTH:
    case CSS_PROP_OVERFLOW:
    case CSS_PROP_PAGE:
    case CSS_PROP_PAGE_BREAK_AFTER:
    case CSS_PROP_PAGE_BREAK_BEFORE:
    case CSS_PROP_PAGE_BREAK_INSIDE:
    case CSS_PROP_PITCH:
    case CSS_PROP_PLAY_DURING:
    case CSS_PROP_POSITION:
    case CSS_PROP_QUOTES:
    case CSS_PROP_SIZE:
    case CSS_PROP_SPEAK:
    case CSS_PROP_SPEAK_HEADER:
    case CSS_PROP_SPEAK_NUMERAL:
    case CSS_PROP_SPEAK_PUNCTUATION:
    case CSS_PROP_SPEECH_RATE:
    case CSS_PROP_TABLE_LAYOUT:
    case CSS_PROP_TEXT_ALIGN:
    case CSS_PROP_TEXT_DECORATION:
    case CSS_PROP_TEXT_TRANSFORM:
    case CSS_PROP_UNICODE_BIDI:
    case CSS_PROP_VERTICAL_ALIGN:
    case CSS_PROP_VISIBILITY:
    case CSS_PROP_VOICE_FAMILY:
    case CSS_PROP_VOLUME:
    case CSS_PROP_WHITE_SPACE:
    case CSS_PROP_WORD_SPACING:
    case CSS_PROP_Z_INDEX:
    {
	printf("parseValue: value = %s\n", val);
	const struct css_value *cssval = findValue(val, value.length());
	if (cssval)
	{
	    printf("got value %d\n", cssval->id);
	    CSSPrimitiveValueImpl *v = new CSSPrimitiveValueImpl(cssval->id);
	    return v;
	    // ### FIXME: should check if the identifier makes sense with the property
	}
    }	
    default:
	break;
    }

    // we don't have an identifier.

    switch(propId)
    {
// ident only properties
    case CSS_PROP_BACKGROUND_ATTACHMENT:
    case CSS_PROP_BACKGROUND_REPEAT:
    case CSS_PROP_BORDER_COLLAPSE:
    case CSS_PROP_BORDER_TOP_STYLE:
    case CSS_PROP_BORDER_RIGHT_STYLE:
    case CSS_PROP_BORDER_BOTTOM_STYLE:
    case CSS_PROP_BORDER_LEFT_STYLE:
    case CSS_PROP_CAPTION_SIDE:
    case CSS_PROP_CLEAR:
    case CSS_PROP_DIRECTION:
    case CSS_PROP_DISPLAY:
    case CSS_PROP_EMPTY_CELLS:
    case CSS_PROP_FLOAT:
    case CSS_PROP_FONT_STRETCH:
    case CSS_PROP_FONT_STYLE:
    case CSS_PROP_FONT_VARIANT:
    case CSS_PROP_FONT_WEIGHT:
    case CSS_PROP_LIST_STYLE_POSITION:
    case CSS_PROP_LIST_STYLE_TYPE:
    case CSS_PROP_OUTLINE_STYLE:
    case CSS_PROP_OVERFLOW:
    case CSS_PROP_PAGE:
    case CSS_PROP_PAGE_BREAK_AFTER:
    case CSS_PROP_PAGE_BREAK_BEFORE:
    case CSS_PROP_PAGE_BREAK_INSIDE:
    case CSS_PROP_PAUSE_AFTER:
    case CSS_PROP_PAUSE_BEFORE:
    case CSS_PROP_POSITION:
    case CSS_PROP_SPEAK:
    case CSS_PROP_SPEAK_HEADER:
    case CSS_PROP_SPEAK_NUMERAL:
    case CSS_PROP_SPEAK_PUNCTUATION:
    case CSS_PROP_TABLE_LAYOUT:
    case CSS_PROP_TEXT_TRANSFORM:
    case CSS_PROP_UNICODE_BIDI:
    case CSS_PROP_VISIBILITY:
    case CSS_PROP_WHITE_SPACE:
	break;

// special properties (css_extensions)
    case CSS_PROP_AZIMUTH:
	// CSS2Azimuth
    case CSS_PROP_BACKGROUND_POSITION:
	// CSS2BackgroundPosition
    case CSS_PROP_BORDER_SPACING:
	// CSS2BorderSpacing
    case CSS_PROP_CURSOR:
	// CSS2Cursor
    case CSS_PROP_PLAY_DURING:
	// CSS2PlayDuring
    case CSS_PROP_TEXT_SHADOW:
	// list of CSS2TextShadow
	break;

// colors || inherit
    case CSS_PROP_BACKGROUND_COLOR:
    case CSS_PROP_BORDER_TOP_COLOR:
    case CSS_PROP_BORDER_RIGHT_COLOR:
    case CSS_PROP_BORDER_BOTTOM_COLOR:
    case CSS_PROP_BORDER_LEFT_COLOR:
    case CSS_PROP_COLOR:
    case CSS_PROP_OUTLINE_COLOR:
    {
	QString value(curP, endP - curP);
	QColor c;
	setNamedColor(c, value);
	printf("color is: %d, %d, %d\n", c.red(), c.green(), c.blue());
	return new CSSPrimitiveValueImpl(c);
    }
  break;

// uri || inherit
    case CSS_PROP_BACKGROUND_IMAGE:
    case CSS_PROP_CUE_AFTER:
    case CSS_PROP_CUE_BEFORE:
    case CSS_PROP_LIST_STYLE_IMAGE:
    {
	DOMString value(curP, endP - curP);
	return new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_URI);
	break;
    }

// length
    case CSS_PROP_BORDER_TOP_WIDTH:
    case CSS_PROP_BORDER_RIGHT_WIDTH:
    case CSS_PROP_BORDER_BOTTOM_WIDTH:
    case CSS_PROP_BORDER_LEFT_WIDTH:
    case CSS_PROP_MARKER_OFFSET:
    case CSS_PROP_LETTER_SPACING:
    case CSS_PROP_OUTLINE_WIDTH:
    case CSS_PROP_WORD_SPACING:
	return parseUnit(curP, endP, LENGTH);
	break;

// length, percent
    case CSS_PROP_PADDING_TOP:
    case CSS_PROP_PADDING_RIGHT:
    case CSS_PROP_PADDING_BOTTOM:
    case CSS_PROP_PADDING_LEFT:
    case CSS_PROP_TEXT_INDENT:
    case CSS_PROP_BOTTOM:
    case CSS_PROP_FONT_SIZE:
    case CSS_PROP_HEIGHT:
    case CSS_PROP_LEFT:
    case CSS_PROP_MARGIN_TOP:
    case CSS_PROP_MARGIN_RIGHT:
    case CSS_PROP_MARGIN_BOTTOM:
    case CSS_PROP_MARGIN_LEFT:
    case CSS_PROP_MAX_HEIGHT:
    case CSS_PROP_MAX_WIDTH:
    case CSS_PROP_MIN_HEIGHT:
    case CSS_PROP_MIN_WIDTH:
    case CSS_PROP_RIGHT:
    case CSS_PROP_TOP:
    case CSS_PROP_VERTICAL_ALIGN:
    case CSS_PROP_WIDTH:
	return parseUnit(curP, endP, LENGTH | PERCENT );
	break;

// angle
    case CSS_PROP_ELEVATION:
	parseUnit(curP, endP, ANGLE);
	break;
	
// number
    case CSS_PROP_FONT_SIZE_ADJUST:
    case CSS_PROP_ORPHANS:
    case CSS_PROP_PITCH_RANGE:
    case CSS_PROP_RICHNESS:
    case CSS_PROP_SPEECH_RATE:
    case CSS_PROP_STRESS:
    case CSS_PROP_WIDOWS:
    case CSS_PROP_Z_INDEX:
	parseUnit(curP, endP, NUMBER);
	break;

// length, percent, number
    case CSS_PROP_LINE_HEIGHT:
	parseUnit(curP, endP, LENGTH | PERCENT | NUMBER);
	break;

// number, percent
    case CSS_PROP_VOLUME:
	parseUnit(curP, endP, PERCENT | NUMBER);
	break;
	
// frequency
    case CSS_PROP_PITCH:
	parseUnit(curP, endP, FREQUENCY);
	break;

// string
    case CSS_PROP_TEXT_ALIGN:
	// only for table columns.
	break;

// rect
    case CSS_PROP_CLIP:
	// rect, ident
	break;
	
// lists
    case CSS_PROP_CONTENT:
	// list of string, uri, counter, attr, i
    case CSS_PROP_COUNTER_INCREMENT:
	// list of CSS2CounterIncrement
    case CSS_PROP_COUNTER_RESET:
	// list of CSS2CounterReset
    case CSS_PROP_FONT_FAMILY:
	// list of strings and ids
    case CSS_PROP_QUOTES:
	// list of strings or i
    case CSS_PROP_SIZE:
	// ### look up
    case CSS_PROP_TEXT_DECORATION:
	// list of ident
    case CSS_PROP_VOICE_FAMILY:
	// list of strings and i
	break;
	
// shorthand properties
    case CSS_PROP_BACKGROUND:
    case CSS_PROP_BORDER:
    case CSS_PROP_BORDER_COLOR:
    case CSS_PROP_BORDER_STYLE:
    case CSS_PROP_BORDER_TOP:
    case CSS_PROP_BORDER_RIGHT:
    case CSS_PROP_BORDER_BOTTOM:
    case CSS_PROP_BORDER_LEFT:
    case CSS_PROP_BORDER_WIDTH:
    case CSS_PROP_CUE:
    case CSS_PROP_FONT:
    case CSS_PROP_LIST_STYLE:
    case CSS_PROP_MARGIN:
    case CSS_PROP_OUTLINE:
    case CSS_PROP_PADDING:
    case CSS_PROP_PAUSE:
	break;
    default:
	printf("illegal property!\n");
    }

    return 0;
}	

CSSValueImpl *
StyleBaseImpl::parseUnit(const QChar * curP, const QChar *endP, int allowedUnits)
{
    const QChar *split = endP;
    // splt up number and unit
    while( (*split < '0' || *split > '9') && *split != '.' && split > curP)
	split--;
    split++;

    QString s(curP, split-curP);
    printf("number = %s\n", s.ascii());

    bool isInt = false;
    if(s.find('.') == -1) isInt = true;

    bool ok;
    float value = s.toFloat(&ok);
    if(!ok) return 0;

    if(split > endP) // no unit
    {
	if(!(allowedUnits & (NUMBER | INTEGER)))
	    return 0;

	if(allowedUnits & NUMBER)
	    return new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_NUMBER);
	
	if(allowedUnits & INTEGER && isInt) // ### DOM CSS doesn't seem to define something for integer
	    return new CSSPrimitiveValueImpl(value, CSSPrimitiveValue::CSS_NUMBER);

	if(allowedUnits & LENGTH && value == 0)
	    return new CSSPrimitiveValueImpl(0, CSSPrimitiveValue::CSS_UNKNOWN);
	
	return 0;
    }

    CSSPrimitiveValue::UnitTypes type = CSSPrimitiveValue::CSS_UNKNOWN;
    int unit = 0;

    switch(split->lower().latin1())
    {
    case '%':
	type = CSSPrimitiveValue::CSS_PERCENTAGE;
	unit = PERCENT;
    case 'e':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_EMS;
	    unit = LENGTH;
	}
	else if(split->latin1() == 'x' || split->latin1() == 'X')
	{
	    type = CSSPrimitiveValue::CSS_EXS;
	    unit = LENGTH;
	}
	break;
    case 'p':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'x' || split->latin1() == 'X')
	{
	    type = CSSPrimitiveValue::CSS_PX;
	    unit = LENGTH;
	}
	else if(split->latin1() == 't' || split->latin1() == 'T')
	{
	    type = CSSPrimitiveValue::CSS_PT;
	    unit = LENGTH;
	}
	else if(split->latin1() == 'c' || split->latin1() == 'C')
	{
	    type = CSSPrimitiveValue::CSS_PC;
	    unit = LENGTH;
	}
	break;
    case 'c':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_CM;
	    unit = LENGTH;
	}
	break;
    case 'm':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'm' || split->latin1() == 'M')
	{
	    type = CSSPrimitiveValue::CSS_MM;
	    unit = LENGTH;
	}
	else if(split->latin1() == 's' || split->latin1() == 'S')
	{
	    type = CSSPrimitiveValue::CSS_MS;
	    unit = TIME;
	}
	break;
    case 'i':
	split++;
	if(split > endP) break;
	if(split->latin1() == 'n' || split->latin1() == 'N')
	{
	    type = CSSPrimitiveValue::CSS_IN;
	    unit = LENGTH;
	}
	break;
    case 'd':
	type = CSSPrimitiveValue::CSS_DEG;
	unit = ANGLE;
	break;
    case 'r':
	type = CSSPrimitiveValue::CSS_RAD;
	unit = ANGLE;
	break;
    case 'g':
	type = CSSPrimitiveValue::CSS_GRAD;
	unit = ANGLE;
	break;
    case 's':
	type = CSSPrimitiveValue::CSS_S;
	unit = TIME;
	break;
    case 'h':
        type = CSSPrimitiveValue::CSS_HZ;
	unit = FREQUENCY;
	break;
    case 'k':
	type = CSSPrimitiveValue::CSS_KHZ;
	unit = FREQUENCY;
	break;
    }

    if(unit & allowedUnits)
    {
	printf("found allowed number %f, unit %d\n", value, type);
	return new CSSPrimitiveValueImpl(value, type);
    }

    return 0;
}

CSSStyleRuleImpl *
StyleBaseImpl::parseStyleRule(const QChar *&curP, const QChar *endP)
{
    //printf("style rule is \'%s\'\n", QString(curP, endP-curP).latin1());

    const QChar *startP;
    QList<CSSSelector> *slist;
    QList<CSSProperty> *plist;

    startP = curP;
    curP = parseToChar(startP, endP, '{', false);
    if (!curP)
        return(0);
    //printf("selector is \'%s\'\n", QString(curP, endP-curP).latin1());

    slist = parseSelector(startP, curP );

    curP++; // need to get past the '{' from above

    startP = curP;
    curP = parseToChar(startP, endP, '}', false);
    if (!curP)
    {
        delete slist;
        return(0);
    }

    plist = parseProperties(startP, curP );

    curP++; // need to get past the '}' from above

    if (!plist || !slist)
    {
        // Useless rule
        delete slist;
        delete plist;
	printf("bad style rule\n");
        return 0;
    }

    // return the newly created rule
    CSSStyleRuleImpl *rule = new CSSStyleRuleImpl(this);
    CSSStyleDeclarationImpl *decl = new CSSStyleDeclarationImpl(rule, plist);

    rule->setSelector(slist);
    rule->setDeclaration(decl);
    // ### set selector and value
    return rule;
}

CSSRuleImpl *
StyleBaseImpl::parseRule(const QChar *&curP, const QChar *endP)
{
    curP = parseSpace(curP, endP);
    CSSRuleImpl *rule = 0;

    //printf("parse rule: current = %c\n", curP->latin1());

    if (*curP == '@')
	rule = parseAtRule(curP, endP);
    else
	rule = parseStyleRule(curP, endP);

    curP++;
    return rule;
}

// ------------------------------------------------------------------------------

StyleListImpl::~StyleListImpl()
{
    StyleBaseImpl *n;

    if(!m_lstChildren) return;

    for( n = m_lstChildren->first(); n != 0; n = m_lstChildren->next() )
    {
	n->setParent(0);
	if(n->deleteMe()) delete n;
    }
    delete m_lstChildren;
}

// --------------------------------------------------------------------------------

CSSSelector::CSSSelector(void)
: tag(0), tagHistory(0)
{
    attr = 0;
    match = Exact;
    relation = Descendant;
}

CSSSelector::~CSSSelector(void)
{
    if (tagHistory)
    {
        delete tagHistory;
    }
}

void CSSSelector::print(void)
{
    printf("[Selector: tag = %d, attr = \"%d\", value = \"%s\" relation = %d\n",
    	tag, attr, value.data(), (int)relation);
}

// ----------------------------------------------------------------------------

CSSProperty::~CSSProperty()
{
    if(m_value) m_value->deref();
}

void CSSProperty::setValue(CSSValueImpl *val)
{
    if(m_value) m_value->deref();
    m_value = val;
    if(m_value) m_value->ref();
}

CSSValueImpl *CSSProperty::value()
{
    return m_value;
}
