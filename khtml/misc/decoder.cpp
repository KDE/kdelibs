/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@kde.org)
    Copyright (C) 2003 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2003 Apple Computer, Inc.

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

#undef DECODE_DEBUG
//#define DECODE_DEBUG

#include <assert.h>

#include "decoder.h"
#include "guess_ja.h"

using namespace khtml;

#include "htmlhashes.h"

#include <qregexp.h>
#include <qtextcodec.h>

#include <kglobal.h>
#include <kcharsets.h>

#include <ctype.h>
#include <kdebug.h>
#include <klocale.h>



Decoder::Decoder()
{
    // latin1
    m_codec = QTextCodec::codecForMib(4);
    m_decoder = m_codec->makeDecoder();
    enc = 0;
    body = false;
    beginning = true;
    visualRTL = false;
    haveEncoding = false;
    m_autoDetectLanguage = SemiautomaticDetection;
    kc = NULL;
}

Decoder::~Decoder()
{
    delete m_decoder;
    if (kc)
        delete kc;
}

void Decoder::setEncoding(const char *_encoding, bool force)
{
#ifdef DECODE_DEBUG
    kdDebug(6005) << "setEncoding " << _encoding << " " << force << endl;
#endif
    enc = _encoding;

    QTextCodec *old = m_codec;
#ifdef DECODE_DEBUG
    kdDebug(6005) << "old encoding is:" << m_codec->name() << endl;
#endif
    enc = enc.lower();
#ifdef DECODE_DEBUG
    kdDebug(6005) << "requesting:" << enc << endl;
#endif
    if(enc.isNull() || enc.isEmpty())
        return;
    if(enc == "visual") // hebrew visually ordered
        enc = "iso8859-8";
    bool b;
    m_codec = KGlobal::charsets()->codecForName(enc, b);
    if(m_codec->mibEnum() == 11)  {
        // iso8859-8 (visually ordered)
        m_codec = QTextCodec::codecForName("iso8859-8-i");
        visualRTL = true;
    }
    if( !b ) // in case the codec didn't exist, we keep the old one (fixes some sites specifying invalid codecs)
	m_codec = old;
    else
	haveEncoding = force;
    delete m_decoder;
    m_decoder = m_codec->makeDecoder();
    if (m_codec->mibEnum() == 1000) // utf 16
        haveEncoding = false; // force auto detection
#ifdef DECODE_DEBUG
    kdDebug(6005) << "Decoder::encoding used is" << m_codec->name() << endl;
#endif
}

const char *Decoder::encoding() const
{
    return enc;
}

QString Decoder::decode(const char *data, int len)
{
    // this is not completely efficient, since the function might go
    // through the html head several times...

    if(!haveEncoding && !body) {
#ifdef DECODE_DEBUG
        kdDebug(6005) << "looking for charset definition" << endl;
#endif
        // check for UTF-16
        uchar * uchars = (uchar *) data;
        if( uchars[0] == 0xfe && uchars[1] == 0xff ||
            uchars[0] == 0xff && uchars[1] == 0xfe ) {
            enc = "ISO-10646-UCS-2";
            haveEncoding = true;
            m_codec = QTextCodec::codecForMib(1000);
            delete m_decoder;
            m_decoder = m_codec->makeDecoder();
        } else {

            if(m_codec->mibEnum() != 1000) {  // utf16
                // replace '\0' by spaces, for buggy pages
                char *d = const_cast<char *>(data);
                int i = len - 1;
                while(i >= 0) {
                    if(d[i] == 0) d[i] = ' ';
                    i--;
                }
            }
            buffer += QCString(data, len+1);

            // we still don't have an encoding, and are in the head
            // the following tags are allowed in <head>:
            // SCRIPT|STYLE|META|LINK|OBJECT|TITLE|BASE

            const char *ptr = buffer.data();
            while(*ptr != '\0')
            {
                if(*ptr == '<') {
                    bool end = false;
                    ptr++;
                    if(*ptr == '/') ptr++, end=true;
                    char tmp[20];
                    int len = 0;
                    while (
                        ((*ptr >= 'a') && (*ptr <= 'z') ||
                         (*ptr >= 'A') && (*ptr <= 'Z') ||
                         (*ptr >= '0') && (*ptr <= '9'))
                        && len < 19 )
                    {
                        tmp[len] = tolower( *ptr );
                        ptr++;
                        len++;
                    }
		    tmp[len] = 0;
                    int id = khtml::getTagID(tmp, len);
                    if(end) id += ID_CLOSE_TAG;

                    switch( id ) {
                    case ID_META:
                    {
                        // found a meta tag...
                        //ptr += 5;
                        const char * end = ptr;
                        while(*end != '>' && *end != '\0') end++;
                        if ( *end == '\0' ) break;
                        QCString str( ptr, (end-ptr)+1);
                        str = str.lower();
                        int pos = 0;
                        //if( (pos = str.find("http-equiv", pos)) == -1) break;
                        //if( (pos = str.find("content-type", pos)) == -1) break;
			while( pos < ( int ) str.length() ) {
			    if( (pos = str.find("charset", pos)) == -1) break;
			    pos += 7;
                            // skip whitespace..
			    while(  pos < (int)str.length() && str[pos] <= ' ' ) pos++;
                            if ( pos == ( int )str.length()) break;
                            if ( str[pos++] != '=' ) continue;
                            while ( pos < ( int )str.length() &&
                                    ( str[pos] <= ' ' ) || str[pos] == '=' || str[pos] == '"' || str[pos] == '\'')
				pos++;

                            // end ?
                            if ( pos == ( int )str.length() ) break;
			    uint endpos = pos;
			    while( endpos < str.length() &&
                                   (str[endpos] != ' ' && str[endpos] != '"' && str[endpos] != '\''
                                    && str[endpos] != ';' && str[endpos] != '>') )
				endpos++;
			    enc = str.mid(pos, endpos-pos);
#ifdef DECODE_DEBUG
			    kdDebug( 6005 ) << "Decoder: found charset: " << enc.data() << endl;
#endif
			    setEncoding(enc, true);
			    if( haveEncoding ) goto found;

                            if ( endpos >= str.length() || str[endpos] == '/' || str[endpos] == '>' ) break;

			    pos = endpos + 1;
			}
		    }
                    case ID_SCRIPT:
                    case (ID_SCRIPT+ID_CLOSE_TAG):
                    case ID_NOSCRIPT:
                    case (ID_NOSCRIPT+ID_CLOSE_TAG):
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
                    case 0:
                    case (0 + ID_CLOSE_TAG ):
                        break;
                    default:
                        body = true;
#ifdef DECODE_DEBUG
			kdDebug( 6005 ) << "Decoder: no charset found. Id=" << id << endl;
#endif
                        goto found;
                    }
                }
                else
                    ptr++;
            }
            return QString::null;
        }
    }

 found:
    if ( !haveEncoding ) {
#ifdef DECODE_DEBUG
	kdDebug( 6005 ) << "Decoder: use auto-detect (" << strlen(data) << ")" << endl;
#endif

        switch ( m_autoDetectLanguage) {
        case Decoder::Arabic:
            enc = automaticDetectionForArabic( (const unsigned char*) data, len );
            break;
        case Decoder::Baltic:
            enc = automaticDetectionForBaltic( (const unsigned char*) data, len );
            break;
        case Decoder::CentralEuropean:
            enc = automaticDetectionForCentralEuropean( (const unsigned char*) data, len );
            break;
        case Decoder::Russian:
        case Decoder::Ukrainian:
            enc = automaticDetectionForCyrillic( (const unsigned char*) data, len, m_autoDetectLanguage );
            break;
        case Decoder::Greek:
            enc = automaticDetectionForGreek( (const unsigned char*) data, len );
            break;
        case Decoder::Hebrew:
            enc = automaticDetectionForHebrew( (const unsigned char*) data, len );
            break;
        case Decoder::Japanese:
            enc = automaticDetectionForJapanese( (const unsigned char*) data, len );
            break;
        case Decoder::Turkish:
            enc = automaticDetectionForTurkish( (const unsigned char*) data, len );
            break;
        case Decoder::WesternEuropean:
            enc = automaticDetectionForWesternEuropean( (const unsigned char*) data, len );
            break;
        case Decoder::SemiautomaticDetection:
        case Decoder::Chinese:
        case Decoder::Korean:
        case Decoder::Thai:
        case Decoder::Unicode:
            // huh. somethings broken in this code ### FIXME
            break;
        }

#ifdef DECODE_DEBUG
        kdDebug( 6005 ) << "Decoder: auto detect encoding is " << enc.data() << endl;
#endif
        if ( !enc.isEmpty() )
            setEncoding( enc.data(), true );
    }


    // if we still haven't found an encoding latin1 will be used...
    // this is according to HTML4.0 specs
    if (!m_codec)
    {
        if(enc.isEmpty()) enc = "iso8859-1";
        m_codec = QTextCodec::codecForName(enc);
        // be sure not to crash
        if(!m_codec) {
            m_codec = QTextCodec::codecForMib(4);
            enc = "iso8859-1";
        }
        delete m_decoder;
        m_decoder = m_codec->makeDecoder();
    }
    QString out;

    if(!buffer.isEmpty() && enc != "ISO-10646-UCS-2") {
        out = m_decoder->toUnicode(buffer, buffer.length());
        buffer = "";
    } else {
        if(m_codec->mibEnum() != 1000) // utf16
        {
            // ### hack for a bug in QTextCodec. It cut's the input stream
            // in case there are \0 in it. ZDNET has them inside... :-(
            char *d = const_cast<char *>(data);
            int i = len - 1;
            while(i >= 0) {
                if(*(d+i) == 0) *(d+i) = ' ';
                i--;
            }
        }
        out = m_decoder->toUnicode(data, len);
    }

    return out;
}

QString Decoder::flush() const
{
    return m_decoder->toUnicode(buffer, buffer.length());
}

QCString Decoder::automaticDetectionForArabic( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) || ptr[ i ] == 0xA1 || ptr[ i ] == 0xA2 || ptr[ i ] == 0xA3
             || ( ptr[ i ] >= 0xA5 && ptr[ i ] <= 0xAB ) || ( ptr[ i ] >= 0xAE && ptr[ i ] <= 0xBA )
             || ptr[ i ] == 0xBC || ptr[ i ] == 0xBD || ptr[ i ] == 0xBE || ptr[ i ] == 0xC0
             || ( ptr[ i ] >= 0xDB && ptr[ i ] <= 0xDF ) || ( ptr[ i ] >= 0xF3 ) ) {
            return "cp1256";
        }
    }

    return "iso-8859-6";
}

QCString Decoder::automaticDetectionForBaltic( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9E ) )
             return "cp1257";

        if ( ptr[ i ] == 0xA1 || ptr[ i ] == 0xA5 )
            return "iso-8859-13";
    }

    return "iso-8859-13";
}

QCString Decoder::automaticDetectionForCentralEuropean(const unsigned char* ptr, int size )
{
    QCString charset = QCString();
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) {
            if ( ptr[ i ] == 0x81 || ptr[ i ] == 0x83 || ptr[ i ] == 0x90 || ptr[ i ] == 0x98 )
                return "ibm852";

            if ( i + 1 > size )
                return "cp1250";
            else { // maybe ibm852 ?
                charset = "cp1250";
                continue;
            }
        }
        if ( ptr[ i ] == 0xA5 || ptr[ i ] == 0xAE || ptr[ i ] == 0xBE || ptr[ i ] == 0xC3 || ptr[ i ] == 0xD0 || ptr[ i ] == 0xE3 || ptr[ i ] == 0xF0 ) {
            if ( i + 1 > size )
                return "iso-8859-2";
            else {  // maybe ibm852 ?
                if ( charset.isNull() )
                    charset = "iso-8859-2";
                continue;
            }
        }
    }

    if ( charset.isNull() )
        charset = "iso-8859-3";

    return charset.data();
}

QCString Decoder::automaticDetectionForCyrillic( const unsigned char* ptr, int size, AutoDetectLanguage _language )
{
    QCString charset = QCString();
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F ) {
            if ( ptr[ i ] == 0x98 ) {
                if ( _language == Russian )
                    return "koi8-r";
                else if ( _language == Ukrainian )
                    return "koi8-u";
            }

            if ( i + 1 > size )
                return "cp1251";
            else { // maybe koi8-r or koi8-u ?
                charset = "cp1251";
                continue;
            }
        }
        else {
            if ( i + 1 > size )
                return "iso-8859-5";
            else {  // maybe koi8-r (koi8-u) or cp1251 ?
                if ( charset.isNull() )
                    charset = "iso-8859-5";
                continue;
            }
        }
    }

    if ( charset.isNull() )
        charset = "iso-8859-5";

    return charset.data();
}

QCString Decoder::automaticDetectionForGreek( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x87 ) || ptr[ i ] == 0x89 || ptr[ i ] == 0x8B
             || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x97 ) || ptr[ i ] == 0x99 || ptr[ i ] == 0x9B || ptr[ i ] == 0xA4
             || ptr[ i ] == 0xA5 || ptr[ i ] == 0xAE ) {
            return "cp1253";
        }
    }

    return "iso-8859-7";
}

QCString Decoder::automaticDetectionForHebrew( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x89 ) || ptr[ i ] == 0x8B
             || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x99 ) || ptr[ i ] == 0x9B || ptr[ i ] == 0xA1 || ( ptr[ i ] >= 0xBF && ptr[ i ] <= 0xC9 )
             || ( ptr[ i ] >= 0xCB && ptr[ i ] <= 0xD8 ) ) {
            return "cp1255";
        }

        if ( ptr[ i ] == 0xDF )
            return "iso-8859-8-i";
    }

    return "iso-8859-8-i";
}

QCString Decoder::automaticDetectionForJapanese( const unsigned char* ptr, int size )
{
    if (!kc)
        kc = new JapaneseCode();

    switch ( kc->guess_jp( (const char*)ptr, size ) ) {
    case JapaneseCode::JIS:
        return "jis7";
    case JapaneseCode::EUC:
        return "eucjp";
    case JapaneseCode::SJIS:
        return "sjis";
     case JapaneseCode::UTF8:
        return "utf8";
    default:
        break;
    }

    return "";
}

QCString Decoder::automaticDetectionForTurkish( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x8C ) || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x9C ) || ptr[ i ] == 0x9F ) {
            return "cp1254";
        }
    }

    return "iso-8859-9";
}

QCString Decoder::automaticDetectionForWesternEuropean( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F )
            return "cp1252";
    }

    return "iso-8859-1"; //"iso-8859-15"; Which better at default ?
}


// -----------------------------------------------------------------------------
#undef DECODE_DEBUG
