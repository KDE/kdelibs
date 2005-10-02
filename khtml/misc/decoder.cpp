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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
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


enum MIB
{
    MibLatin1  = 4,
    Mib8859_8  = 85,
    MibUtf8    = 106,
    MibUtf16   = 1015,
    MibUtf16BE = 1013,
    MibUtf16LE = 1014
};

static bool is16Bit(QTextCodec* codec)
{
    switch (codec->mibEnum())
    {
    case MibUtf16:
    case MibUtf16BE:
    case MibUtf16LE:
        return true;
    default:
        return false;
    }
}

Decoder::Decoder()
{
    // latin1
    m_codec = QTextCodec::codecForMib(MibLatin1);
    m_decoder = m_codec->makeDecoder();
    enc = 0;
    m_type = DefaultEncoding;
    body = false;
    beginning = true;
    visualRTL = false;
    m_autoDetectLanguage = SemiautomaticDetection;
    kc = NULL;
}

Decoder::~Decoder()
{
    delete m_decoder;
    if (kc)
        delete kc;
}

void Decoder::setEncoding(const char *_encoding, EncodingType type)
{
#ifdef DECODE_DEBUG
    kdDebug(6005) << "setEncoding " << _encoding << " " << type << endl;
#endif
    enc = _encoding;
#ifdef DECODE_DEBUG
    kdDebug(6005) << "old encoding is:" << m_codec->name() << endl;
#endif
    enc = enc.toLower();
#ifdef DECODE_DEBUG
    kdDebug(6005) << "requesting:" << enc << endl;
#endif
    if(enc.isNull() || enc.isEmpty())
        return;

#ifdef APPLE_CHANGES
    QTextCodec *codec = (type == EncodingFromMetaTag || type == EncodingFromXMLHeader)
        ? QTextCodec::codecForNameEightBitOnly(enc)
        : QTextCodec::codecForName(enc);
    if (codec) {
        enc = codec->name();
        visualRTL = codec->usesVisualOrdering();
    }
#else
    if(enc == "visual") // hebrew visually ordered
        enc = "iso8859-8";
    bool b;
    QTextCodec *codec = KGlobal::charsets()->codecForName(enc, b);
    if (!b)
        codec = 0;

    if (type == EncodingFromMetaTag || type  == EncodingFromXMLHeader) {
        //Sometimes the codec specified is absurd, i.e. UTF-16 despite
        //us decoding a meta tag as ASCII. In that case, ignore it.
        if (codec && is16Bit(codec))
            codec = 0;
    }

    if (codec && codec->mibEnum() == Mib8859_8)  {
        //We do NOT want to use Qt's QHebrewCodec, since it tries to reorder itself.
        codec = QTextCodec::codecForName("iso8859-8-i");
	
        // visually ordered unless one of the following
        if( !(enc == "iso-8859-8-i" || enc == "iso_8859-8-i"
                || enc == "csiso88598i" || enc == "logical") )
        visualRTL = true;
    }
#endif

    if( codec ) { // in case the codec didn't exist, we keep the old one (fixes some sites specifying invalid codecs)
        m_codec = codec;
        m_type = type;
        delete m_decoder;
        m_decoder = m_codec->makeDecoder();
    }

#ifdef DECODE_DEBUG
    kdDebug(6005) << "Decoder::encoding used is" << m_codec->name() << endl;
#endif
}

const char *Decoder::encoding() const
{
    return enc;
}

// Other browsers allow comments in the head section, so we need to also.
// It's important not to look for tags inside the comments.
static void skipComment(const char *&ptr, const char *pEnd)
{
    const char *p = ptr;
    // Allow <!-->; other browsers do.
    if (*p == '>') {
        p++;
    } else {
        while (p != pEnd) {
            if (*p == '-') {
                // This is the real end of comment, "-->".
                if (p[1] == '-' && p[2] == '>') {
                    p += 3;
                    break;
                }
                // This is the incorrect end of comment that other browsers allow, "--!>".
                if (p[1] == '-' && p[2] == '!' && p[3] == '>') {
                    p += 4;
                    break;
                }
            }
            p++;
        }
    }
    ptr = p;
}

// Returns the position of the encoding string.
static int findXMLEncoding(const QByteArray &str, int &encodingLength)
{
    int len = str.length();

    int pos = str.find("encoding");
    if (pos == -1)
        return -1;
    pos += 8;

    // Skip spaces and stray control characters.
    while (pos < len && str[pos] <= ' ')
        ++pos;

    //Bail out if nothing after
    if (pos >= len)
        return -1;

    // Skip equals sign.
    if (str[pos] != '=')
        return -1;
    ++pos;

    // Skip spaces and stray control characters.
    while (pos < len && str[pos] <= ' ')
        ++pos;

    //Bail out if nothing after
    if (pos >= len)
        return -1;

    // Skip quotation mark.
    char quoteMark = str[pos];
    if (quoteMark != '"' && quoteMark != '\'')
        return -1;
    ++pos;

    // Find the trailing quotation mark.
    int end = pos;
    while (end < len && str[end] != quoteMark)
        ++end;

    if (end >= len)
        return -1;

    encodingLength = end - pos;
    return pos;
}

QString Decoder::decode(const char *data, int len)
{
    // Check for UTF-16 or UTF-8 BOM mark at the beginning, which is a sure sign of a Unicode encoding.
    int bufferLength = buffer.length();
    const int maximumBOMLength = 10;
    if (beginning && bufferLength + len >= maximumBOMLength) {
        // If the user has chosen utf16 we still need to auto-detect the endianness
        if ((m_type != UserChosenEncoding) || (m_codec->mibEnum() == MibUtf16)) {
            // Extract the first three bytes.
            // Handle the case where some of bytes are already in the buffer.
            const uchar *udata = (const uchar *)data;
            uchar c1 = bufferLength >= 1 ? (uchar)buffer[0] : *udata++;
            uchar c2 = bufferLength >= 2 ? (uchar)buffer[1] : *udata++;
            uchar c3 = bufferLength >= 3 ? (uchar)buffer[2] : *udata++;

            // Check for the BOM
            const char *autoDetectedEncoding;
            if ((c1 == 0xFE && c2 == 0xFF) || (c1 == 0xFF && c2 == 0xFE)) {
                autoDetectedEncoding = "ISO-10646-UCS-2";
            } else if (c1 == 0xEF && c2 == 0xBB && c3 == 0xBF) {
                autoDetectedEncoding = "UTF-8";
            } else if (c1 == 0x00 || c2 == 0x00) {
                uchar c4 = bufferLength >= 4 ? (uchar)buffer[3] : *udata++;
                uchar c5 = bufferLength >= 5 ? (uchar)buffer[4] : *udata++;
                uchar c6 = bufferLength >= 6 ? (uchar)buffer[5] : *udata++;
                uchar c7 = bufferLength >= 7 ? (uchar)buffer[6] : *udata++;
                uchar c8 = bufferLength >= 8 ? (uchar)buffer[7] : *udata++;
                uchar c9 = bufferLength >= 9 ? (uchar)buffer[8] : *udata++;
                uchar c10 = bufferLength >= 10 ? (uchar)buffer[9] : *udata++;
                int nul_count_even = (c2 != 0) + (c4 != 0) + (c6 != 0) + (c8 != 0) + (c10 != 0);
                int nul_count_odd = (c1 != 0) + (c3 != 0) + (c5 != 0) + (c7 != 0) + (c9 != 0);
                if ((nul_count_even == 0 && nul_count_odd == 5) ||
                    (nul_count_even == 5 && nul_count_odd == 0))
                    autoDetectedEncoding = "ISO-10646-UCS-2";
                else
                    autoDetectedEncoding = 0;
            } else {
                autoDetectedEncoding = 0;
            }

            // If we found a BOM, use the encoding it implies.
            if (autoDetectedEncoding != 0) {
                m_type = AutoDetectedEncoding;
                m_codec = QTextCodec::codecForName(autoDetectedEncoding);
                assert(m_codec);
                enc = m_codec->name();
                delete m_decoder;
                m_decoder = m_codec->makeDecoder();
                if (m_codec->mibEnum() == MibUtf16 && c2 == 0x00)
                {
                  // utf16LE, we need to put the decoder in LE mode
                  char reverseUtf16[3] = {0xFF, 0xFE, 0x00};
                  m_decoder->toUnicode(reverseUtf16, 2);
                }
            }
        }
        beginning = false;
    }

    // this is not completely efficient, since the function might go
    // through the html head several times...

    bool lookForMetaTag = m_type == DefaultEncoding && !body;

    if (lookForMetaTag) {
#ifdef DECODE_DEBUG
        kdDebug(6005) << "looking for charset definition" << endl;
#endif
        { // extra level of braces to keep indenting matching original for better diff'ing
#ifdef APPLE_CHANGES
            buffer.append(data, len);
#else
            if(!is16Bit(m_codec)) {  // utf16
                // replace '\0' by spaces, for buggy pages
                char *d = const_cast<char *>(data);
                int i = len - 1;
                while(i >= 0) {
                    if(d[i] == 0) d[i] = ' ';
                    i--;
                }
            }
            buffer += QByteArray(data, len);
#endif
            // we still don't have an encoding, and are in the head
            // the following tags are allowed in <head>:
            // SCRIPT|STYLE|META|LINK|OBJECT|TITLE|BASE
            int invalid = 0; // invalid head tag count
#ifdef APPLE_CHANGES
            const char *ptr = buffer.latin1();
            const char *pEnd = ptr + buffer.length();
#else
            const char *ptr = buffer.data();
            const char *pEnd = ptr + buffer.length();
#endif
            while(ptr != pEnd)
            {
                if(*ptr == '<') {
                    bool end = false;
                    ptr++;

                    // Handle comments.
                    if (ptr[0] == '!' && ptr[1] == '-' && ptr[2] == '-') {
                        ptr += 3;
                        skipComment(ptr, pEnd);
                        continue;
                    }

                    // Handle XML header, which can have encoding in it.
                    if (ptr[0] == '?' && ptr[1] == 'x' && ptr[2] == 'm' && ptr[3] == 'l') {
                        const char *end = ptr;
                        while (*end != '>' && *end != '\0') end++;
                        if (*end == '\0')
                            break;
                        QByteArray str(ptr, end - ptr + 1); //+1 as it must include the \0 terminator
                        int len;
                        int pos = findXMLEncoding(str, len);
                        if (pos != -1) {
                            setEncoding(str.mid(pos, len), EncodingFromXMLHeader);
                            if (m_type == EncodingFromXMLHeader)
                                goto found;
                        }
                    }

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
                        QByteArray str( ptr, (end-ptr)+1);
                        str = str.toLower();
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
			    int endpos = pos;
			    while( endpos < str.length() &&
                                   (str[endpos] != ' ' && str[endpos] != '"' && str[endpos] != '\''
                                    && str[endpos] != ';' && str[endpos] != '>') )
				endpos++;
			    enc = str.mid(pos, endpos-pos);
#ifdef DECODE_DEBUG
			    kdDebug( 6005 ) << "Decoder: found charset: " << enc.data() << endl;
#endif
			    setEncoding(enc, EncodingFromMetaTag);
			    if( m_type == EncodingFromMetaTag ) goto found;

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
                    case ID_BODY:
                    case (ID_HEAD+ID_CLOSE_TAG):
                        body = true;
#ifdef DECODE_DEBUG
			kdDebug( 6005 ) << "Decoder: no charset found. Id=" << id << endl;
#endif
                        goto found;
                    default:
                        // Invalid tag in head. Let's be a little tolerant
                        invalid++;
                        if (invalid > 2)  {
                            body = true;
#ifdef DECODE_DEBUG
                            kdDebug( 6005 ) << "Decoder: no charset found. Id=" << id << endl;
#endif
                            goto found;
                        }
                    }
                }
                else
                    ptr++;
            }
            if (invalid > 0) {
                body = true;
                goto found;
            }
            return QString::null;
        }
    }

 found:
    if (m_type == DefaultEncoding)
    {
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
            enc = 0; //Reset invalid codec we tried, so we get back to latin1 fallback.
            break;
        }

#ifdef DECODE_DEBUG
        kdDebug( 6005 ) << "Decoder: auto detect encoding is " << enc.data() << endl;
#endif
        if ( !enc.isEmpty() )
            setEncoding( enc.data(), AutoDetectedEncoding);
    }


    // if we still haven't found an encoding latin1 will be used...
    // this is according to HTML4.0 specs
    if (!m_codec)
    {
        if(enc.isEmpty()) enc = "iso8859-1";
        m_codec = QTextCodec::codecForName(enc);
        // be sure not to crash
        if(!m_codec) {
            m_codec = QTextCodec::codecForMib(MibLatin1);
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
#warning "is this still needed?"
        if(!is16Bit(m_codec)) // utf16
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

QByteArray Decoder::automaticDetectionForArabic( const unsigned char* ptr, int size )
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

QByteArray Decoder::automaticDetectionForBaltic( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9E ) )
             return "cp1257";

        if ( ptr[ i ] == 0xA1 || ptr[ i ] == 0xA5 )
            return "iso-8859-13";
    }

    return "iso-8859-13";
}

QByteArray Decoder::automaticDetectionForCentralEuropean(const unsigned char* ptr, int size )
{
    QByteArray charset = QByteArray();
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

QByteArray Decoder::automaticDetectionForCyrillic( const unsigned char* ptr, int size, AutoDetectLanguage _language )
{
    QByteArray charset = QByteArray();
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

QByteArray Decoder::automaticDetectionForGreek( const unsigned char* ptr, int size )
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

QByteArray Decoder::automaticDetectionForHebrew( const unsigned char* ptr, int size )
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

QByteArray Decoder::automaticDetectionForJapanese( const unsigned char* ptr, int size )
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

QByteArray Decoder::automaticDetectionForTurkish( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] == 0x80 || ( ptr[ i ] >= 0x82 && ptr[ i ] <= 0x8C ) || ( ptr[ i ] >= 0x91 && ptr[ i ] <= 0x9C ) || ptr[ i ] == 0x9F ) {
            return "cp1254";
        }
    }

    return "iso-8859-9";
}

QByteArray Decoder::automaticDetectionForWesternEuropean( const unsigned char* ptr, int size )
{
    for ( int i = 0; i < size; ++i ) {
        if ( ptr[ i ] >= 0x80 && ptr[ i ] <= 0x9F )
            return "cp1252";
    }

    return "iso-8859-1"; //"iso-8859-15"; Which better at default ?
}


// -----------------------------------------------------------------------------
#undef DECODE_DEBUG
