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

#ifndef LEXER_H
#define LEXER_H

#include <QtCore/QHash>
#include <QtCore/QString>

class QTextStream;

class Lexer
{
public:
    enum TokenType {
        Ident,
        Number,
        LBrace, // {
        RBrace, // }
        Colon,  // :
        SemiColon, // ;
        Star,      // *
        Arrow,     // =>
        Error,
        EndOfFile,
        // Keywords:
        Type,
        Conversion,
        Register,
        Immediate,
        Checked,
        Operation,
        Costs,
        Impl,
        Tile,
        As
    };

    struct Token {
        TokenType type;
        QString   value;

        Token() : type(Error), value("Uninitialized token") {}
        Token(TokenType t): type(t) {}
        Token(TokenType t, const QString& v): type(t), value(v) {}

        QString toString(Lexer* lex)
        {
            switch (type) {
            case LBrace:
                return "'{'";
            case RBrace:
                return "'}'";
            case Colon:
                return "':'";
            case SemiColon:
                return "';'";
            case Star:
                return "'*'";
            case Arrow:
                return "'=>'";
            case EndOfFile:
                return "<End of File>";
            case Error:
                return "<Lex Error:" + value + ">";
            case Ident:
            case Number:
                return value;
            default: {
                    // keywords
                    QHashIterator<QString, TokenType> iter(lex->keywords);
                    while (iter.hasNext()) {
                        if (iter.peekNext().value() == type)
                            return iter.peekNext().key();
                        iter.next();
                    }
                    return "???";
                } // default :
            } // switch(type)
        }
    };

    Lexer(QTextStream* _stream);

    Token nextToken();
private:
    friend class Token;
    Token lexComment();

    QChar peekNext();
    QChar getNext();

    QTextStream* stream;

    bool  charLoaded;
    QChar nextChar;

    QHash<QString, TokenType> keywords;
    };

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
