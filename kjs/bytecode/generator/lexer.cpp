/*
 *  A utilitity for building various tables and specializations for the
 *  KJS Frostbyte bytecode
 *
 *  Copyright (C) 2007, 2008 Maks Orlovich (maksim@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "lexer.h"

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

//### FIXME:appropriate constant from private use or such
const unsigned short EndOfFileChar = 0x21AA; //injection or surjection arrow? I keep forgetting

Lexer::Lexer(QTextStream* _stream): stream(_stream), charLoaded(false), lineNum(0)
{
    keywords["type"]        = Type;
    keywords["conversion" ] = Conversion;
    keywords["register"]    = Register;
    keywords["immediate"]   = Immediate;
    keywords["checked"]     = Checked;
    keywords["operation"]   = Operation;
    keywords["costs"]       = Costs;
    keywords["impl"]        = Impl;
    keywords["tile"]        = Tile;
    keywords["as"]          = As;
    keywords["align8"]      = Align8;
    lineNum = 0;
}

Lexer::Token Lexer::lexComment()
{
    QChar in = getNext();
    if (in == '/') {
        // Single-line comment -- read until the end of line (or file)
        do {
            in = getNext();
        } while (in != '\n' && in != EndOfFileChar);
    } else if (in == '*') {
        // Multi-line comment --- scan until  */
        do {
            in = getNext();
            if (in == EndOfFileChar)
                return Token(Error, "Unterminated multiline comment");
        }
        while (!(in == '*' && peekNext() == '/'));
        getNext(); // Eat the /
    } else {
        return Token(Error, QLatin1String("/ can only start comments, but is followed by: ") + in);
    }

    // Wee. Worked fine. Recurse to get next stuff
    return nextToken();
}

Lexer::Token Lexer::nextToken()
{
    QChar begin;

    // Skip any whitespace characters..
    do {
        begin = getNext();
    }
    while (begin.isSpace());

    if (begin == EndOfFileChar)
        return Token(EndOfFile);

    // Check for simple chars..
    if (begin == '{')
        return Token(LBrace);
    else if (begin == '}')
        return Token(RBrace);
    if (begin == '(')
        return Token(LParen);
    else if (begin == ')')
        return Token(RParen);
    else if (begin == ':')
        return Token(Colon);
    else if (begin == ';')
        return Token(SemiColon);
    else if (begin == '*')
        return Token(Star);
    else if (begin == ',')
        return Token(Comma);


    // =>
    if (begin == '=') {
        QChar c2 = getNext();
        if (c2 == '>')
            return Token(Arrow);
        else
            return Token(Error, QLatin1String("- not part of ->"));
    }

    // Check for comments..
    if (begin == '/')
        return lexComment();

    // Numbers
    if (begin >= '0' && begin <= '9') {
        QString text = begin;
        while (peekNext().isNumber())
            text += getNext();
        return Token(Number, text);
    }

    // Code..
    if (begin == '[') {
        QChar next = getNext();
        if (next != '[')
            return Token(Error, QLatin1String("[ continued with:") + next);
        QString text;
        while(true) {
            QChar letter = getNext();
            if (letter == EndOfFileChar)
                return Token(Error, QLatin1String("Unterminated code fragment"));
            if (letter == ']' && peekNext() == ']') {
                getNext(); //Eat 2nd ']'
                return Token(Code, text);
            }
            text += letter;
        }
    }

    // Identifiers
    if (begin.isLetter()) {
        QString text = begin;
        while (peekNext().isLetterOrNumber() || peekNext() == '_')
            text += getNext();

        TokenType t = Ident;
        if (keywords.contains(text))
            t = keywords[text];
        return Token(t, text);
    } else {
        return Token(Error, QLatin1String("Invalid start of token:") + begin);
    }
}

QChar Lexer::peekNext()
{
    if (charLoaded)
        return nextChar;

    if (stream->atEnd())
        return EndOfFileChar;

    *stream >> nextChar;
    if (nextChar == '\n')
        ++lineNum;
    charLoaded = true;
    return nextChar;
}

QChar Lexer::getNext()
{
    if (charLoaded) {
        charLoaded = false;
        return nextChar;
    }

    if (stream->atEnd()) {
        // Make sure to let peekNext know.
        nextChar   = EndOfFileChar;
        charLoaded = true;
        return EndOfFileChar;
    }

    QChar in;
    *stream >> in;
    if (in == '\n')
        ++lineNum;

    return in;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
