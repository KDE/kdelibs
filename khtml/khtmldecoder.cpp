/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@mpi-hd.mpg.de)

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
// KDE HTML Widget -- decoder for input stream
// $Id$

#include <khtmldecoder.h>
#include <khtmltags.h>

#include <ctype.h>
#include <stdio.h> // for the printf debug messages

int getTagID(const char *tagStr, int len);

KHTMLDecoder::KHTMLDecoder()
{
    codec = QTextCodec::codecForName("ISO 8859-1");
printf("INIT HTML Codec name= %s\n", codec->name());
    enc = 0;
    currentCodec = 0;
    body = false;
    beginning = true;
}
KHTMLDecoder::~KHTMLDecoder()
{
}

void KHTMLDecoder::setEncoding(const char *_encoding)
{
    enc = _encoding;
}

const char *KHTMLDecoder::encoding() const 
{
    return enc;
}

QString KHTMLDecoder::decode(const char *data)
{
    // this is not completely efficient, since the function might go
    // through the html head several times...
    
    if(enc.isEmpty() && !body) {
	buffer += data;

	// check for UTF-16
	uchar * uchars = (uchar *) buffer.data();
	if( uchars[0] == 0xfe && uchars[1] == 0xff ||
	    uchars[0] == 0xff && uchars[1] == 0xfe ) {
	    enc = "utf16";
	    codec = QTextCodec::codecForName(enc);
	} else {

	    // we still don't have an encoding, and are in the head
	    // the following tags are allowed in <head>:
	    // SCRIPT|STYLE|META|LINK|OBJECT|TITLE|BASE
	    
	    const char *ptr = buffer.data();
	    while(*ptr != '\0')
	    {
		if(*ptr == '<') {
		    ptr++;
		    char tmp[20];
		    int len = 0;
		    while (  
			((*ptr >= 'a') && (*ptr <= 'z')) ||
			((*ptr >= 'A') && (*ptr <= 'Z')) ||
			((*ptr >= '0') && (*ptr <= '9'))
			&& len < 19 ) 
		    {
			tmp[len] = tolower( *ptr );
			ptr++;
			len++;
		    }
		    int id = getTagID(tmp, len);
		    
		    switch( id ) {
		    case ID_META:
		    {
			// found a meta tag...
			//ptr += 5;
			const char * end = ptr;
			while(*end != '>' && *end != '\0') end++;
			if ( *end == '\0' ) break;
			QCString str( ptr, (end-ptr));
			str = str.lower();
			int pos = 0;
			if( (pos = str.find("http-equiv", pos)) == -1) break;
			if( (pos = str.find("content-type", pos)) == -1) break;
			if( (pos = str.find("charset", pos)) == -1) break;
			pos += 7;
			while( (str[pos] == ' ' || str[pos] == '=' 
				|| str[pos] == '"') 
			       && pos < (int)str.length()) 
			    pos++;
			
			uint endpos = pos;
			while( (str[endpos] != ' ' || str[endpos] != '"' 
				|| str[endpos] != '>')
			       && endpos < str.length() )
			    endpos++;
			
			enc = str.mid(pos, endpos-pos);
			printf("KHTMLDecoder: found charset: %s\n", enc.data());
			codec = QTextCodec::codecForName(enc);
			goto found;
		    }
		    case ID_SCRIPT:
		    case (ID_SCRIPT+ID_CLOSE_TAG):
		    case ID_STYLE:
		    case (ID_STYLE+ID_CLOSE_TAG):
		    case ID_LINK:
		    case (ID_LINK+ID_CLOSE_TAG):
		    case ID_OBJECT:
		    case (ID_OBJECT+ID_CLOSE_TAG):
		    case ID_TITLE:
		    case (ID_TITLE+ID_CLOSE_TAG):
		    case ID_BASE:
		    case (ID_BASE+ID_CLOSE_TAG):
		    case ID_HTML:
		    case ID_HEAD:
			break;
		    default:
			body = true;
			goto found;
		    }
		}
		else
		    ptr++;
	    }
	    return 0;
	}
    }

 found:
    // if we still haven't found an encoding latin1 will be used...
    // this is according to HTML4.0 specs
    QString out;

    if(!buffer.isEmpty()) {
	out = codec->toUnicode(buffer);
	buffer = "";
    }
    else 
	out = codec->toUnicode(data);

    // the hell knows, why the output does sometimes have a QChar::null at
    // the end...
    if(out[out.length()-1] == QChar::null)
	out.truncate(out.length() - 1);
    return out;
}

