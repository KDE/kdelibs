/* 
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    Based on the Chimera CSS implementation of 
                       John Kilburg <john@cs.unlv.edu>

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Cascading Style Sheets / 1
// $Id$

#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>                     

#include "khtmltoken.h"
#include "khtmlstyle.h"


static const int UNDEFINED= -1;

//
// Classes for internal use only
//
/////////////////////////////////

//
// The following file includes all CSS_PROP_* defines
// CSS properties range from CSS_PROP_MIN to (and including) CSS_PROP_MAX
// Combined property-keywords are ranging from CSS_PROP_MAX+1 to
// (and including) CSS_PROP_TOTAL-1
#include "kcssprop.h"

//
// The following file defines the function
//     const struct props *findProp(const char *word, int len)
//
// with 'props->id' a CSS property in the range from CSS_PROP_MIN to
// (and including) CSS_PROP_TOTAL-1
#include "kcssprop.c"

class CSSProperty
{
public:
    int   propId;
#if 0
    enum contentEnum { contEnum, contAbsValue, contRelValue, contPercent,
    contEm, contEx, contPx };
    int   contentId;   
    // contentId determines how contentValue should be interpreted
    int   contentValue;
    const char *contentString;
#else
    QString value;
#endif
    int   weight;
};

class CSSSelector 
{
public:
    CSSSelector(void);
    ~CSSSelector(void);
    void print(void);
    int          tag;
    QString      id;
    QString      klass;
    CSSSelector *tagHistory;
    CSSPropList *propList;
};

CSSSelector::CSSSelector(void)
: tag(-1), id(0), klass(0), tagHistory(0), propList(0)
{
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
    printf("[Selector: tag = %d, id = \"%s\", class = \"%s\"\n",
    	tag, id.data(), klass.data());
}

CSSStyleSheet::CSSStyleSheet(const HTMLSettings *_settings)
{
    settings = _settings;
}

CSSStyleSheet::~CSSStyleSheet()
{
}
    
CSSStyle *
CSSStyleSheet::newStyle(CSSStyle *parentStyle)
{
    CSSStyle *newStyle = new CSSStyle();
    
    if (parentStyle)
    {
        newStyle->font = parentStyle->font;
        newStyle->text = parentStyle->text;
        newStyle->list = parentStyle->list;
    }
    else
    {
        newStyle->font.decoration = CSSStyleFont::decNormal;
        newStyle->font.color = settings->fontBaseColor;
        newStyle->font.family = settings->fontBaseFace;
        newStyle->font.style = CSSStyleFont::stNormal;
        newStyle->font.weight = CSSStyleFont::Normal;
        newStyle->font.size = settings->fontBaseSize;
        newStyle->font.fp = 0;

        newStyle->text.transform = CSSStyleText::transNormal;
        newStyle->text.halign = CSSStyleText::halLeft;
        newStyle->text.indent = 0;
        newStyle->text.height = CSSStyleText::heightNormal;

        newStyle->list.type = CSSStyleList::liDisc;
    }

    newStyle->text.valign = CSSStyleText::valBaseline;
    newStyle->box.marginF = 0;
    newStyle->box.marginP = UNDEFINED;
    newStyle->box.paddingF = 0;
    newStyle->box.paddingP = UNDEFINED;
    newStyle->box.borderColor = newStyle->font.color;
    newStyle->box.border = 2;
    newStyle->box.borderStyle = CSSStyleBox::borderNone;
    newStyle->bgtransparant = true;
    newStyle->width = UNDEFINED;
    newStyle->width_percent = UNDEFINED;
    newStyle->height = UNDEFINED;
    newStyle->height_percent = UNDEFINED;
    
    return newStyle;
}

void
CSSStyleSheet::getStyle(int tagID, HTMLStackElem *tagStack, 
                        CSSStyle *styleElem, 
                        const char *klass, const char *id)
{
}

void
CSSStyleSheet::addStyle(CSSStyle *currentStyle, const char *CSSString)
{
}

const char *
CSSStyleSheet::parseSpace(const char *curP, const char *endP)
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
const char *
CSSStyleSheet::parseToChar(const char *curP, const char *endP, int c, bool chkws)
{
    bool sq = false; /* in single quote? */
    bool dq = false; /* in double quote? */
    bool esc = false; /* escape mode? */

    while (curP < endP)
    {
        if (esc) 
            esc = false;
        else if (*curP == '\\') 
            esc = true;
        else if (dq && (*curP != '"'))
            dq = false;
        else if (sq && (*curP != '\''))
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

const char *
CSSStyleSheet::parseAt(const char *curP, const char *endP)
{
//    const char *startP = curP;
    while (curP < endP)
    {
        if (*curP == '{')
        {
            curP = parseToChar(curP + 1, endP, '}', false);
            return curP;
        }
        else if (*curP == ';')
        {
            // check if [startP, curP] contains an include...
            // if so, emit a signal
            curP++;
            return curP;
        }
        curP++;
    }

    return(0);
}

CSSSelector *
CSSStyleSheet::parseSelector2(const char *curP, const char *endP)
{
    CSSSelector *cs = new CSSSelector();
    QString selecString(curP+1, endP-curP);

    if (*curP == '#') cs->id = selecString;
    else if (*curP == '.') cs->klass = selecString;
    else if (*curP == ':') cs->klass = ":" + selecString;
    else
    {
        const char *startP = curP;
        while (curP < endP)
        {
            if (*curP =='#') 
            {
                QString tag(startP, curP-startP+1);
                QString tmp(curP+1, endP-curP);
                cs->tag = getTagID(tag.lower().data(), tag.length());
                cs->id = tmp;
                break;
            }
            else if (*curP == '.') 
            {
                QString tag(startP, curP-startP+1);
                QString tmp(curP+1, endP-curP);
                cs->tag = getTagID(tag.lower().data(), tag.length());
                cs->klass = tmp;
                break;
            }
            else if (*curP == ':')
            {
                QString tag(startP, curP-startP+1);
                QString tmp(curP+1, endP-curP);
                cs->tag = getTagID(tag.lower().data(), tag.length());
                cs->klass = ":" + tmp;
                break;
            }
            else
            {
                curP++;
            }
        }
        if (curP == endP)
        {
            QString tag(startP,curP-startP+1);
            cs->tag = getTagID(tag.lower().data(), tag.length());
        }
   }
   cs->print();
   if (cs->tag == 0)
   {
       delete cs;
       return(0);
   }
   return(cs);
}

CSSSelector *
CSSStyleSheet::parseSelector1(const char *curP, const char *endP)
{
    CSSSelector *selecStack=0;

    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);
    
    const char *startP = curP;
    while (curP <= endP)
    {
        if ((curP == endP) || isspace(*curP))
        {
            CSSSelector *cs = parseSelector2(startP, curP);
            if (cs)
            {
                cs->tagHistory = selecStack;
                selecStack = cs;
            }
            curP = parseSpace(curP, endP);
            if (!curP)
                return(selecStack);

            startP = curP;
        }
        else
        {
            curP++;
        }
    }
    return(selecStack);
}

CSSSelecList *
CSSStyleSheet::parseSelector(const char *curP, const char *endP)
{
    CSSSelecList *slist  = 0;
    const char *startP;

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
                slist = new CSSSelecList();
            }
            slist->append(selector);
        }
        curP++;
    }
    return(slist);
}        

CSSProperty *
CSSStyleSheet::parseProperty(const char *curP, const char *endP)
{
    const char *colon;
    // Get rid of space in front of the declaration
    
    curP = parseSpace(curP, endP);
    if (!curP)
        return(0);
        
    // Search for the required colon or white space
    colon = parseToChar(curP, endP, ':', true);
    if (!colon)
        return(0);
        
    QString propName(curP, colon-curP+1);
    
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

    // remove space after the value;
    while (endP > curP)
    {
    
        if (!isspace(*(endP-1)))
            break;
        endP--;
    }

    QString propVal(curP, endP-curP+1);
printf("Property-value = \"%s\"\n", propVal.data());

    const struct props *propPtr = findProp(propName.lower().data(), propName.length());
    if (!propPtr)
    {
         printf("Unknown property\n");
         return (0);
    }
    CSSProperty *prop = new CSSProperty();
    prop->propId = propPtr->id;
    prop->value = propVal.data();    

    return(prop);
}

CSSPropList *
CSSStyleSheet::parseProperties(const char *curP, const char *endP)
{
    CSSPropList *propList=0;

    while (curP < endP)
    {
        const char *startP = curP;
        curP = parseToChar(curP, endP, ';', false);
        if (!curP)
            curP = endP;
        
        CSSProperty *prop = parseProperty(startP, curP);
        if (prop)
        {
            if (!propList)
            {
                propList = new CSSPropList();
            }
            propList->append(prop);
        }
        curP++;
    }
    return(propList);
}

const char *
CSSStyleSheet::parseRule(const char *curP, const char *endP)
{
    const char *startP;
    CSSSelecList *slist;
    CSSPropList *plist;

    startP = curP;
    curP = parseToChar(startP, endP, '{', false);
    if (!curP)
        return(0);

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
        return(curP);
    }

    // Add rule to our data structures...
    // WABA: To be done 
    return(curP);
}

void
CSSStyleSheet::parseSheet(const char *src, int len)
{
    const char *curP = src;
    const char *endP = src+len;
    
    curP = parseSpace(curP, endP);
    while (curP && (curP < endP))
    {
        if (*curP == '@')
        {
            curP = parseAt(curP, endP);
        }
        else
        {
            curP = parseRule(curP, endP);
        }
        
        if (curP)
            curP = parseSpace(curP, endP);
    }
}

void
CSSStyleSheet::test(void)
{
    char buf[40000];
    
    int fd = open("/home/waba/test.css", O_RDONLY);

    if (fd < 0)
    {
        perror("Couldn't open /home/waba/test.css:");
        return;
    }
    
    int len = read(fd, buf, 40000);
 
    close(fd);

    parseSheet(buf, len);
}
