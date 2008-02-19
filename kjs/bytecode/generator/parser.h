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

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <QtCore/QString>
#include <QtCore/QStringList>

class Parser
{
public:
    Parser(QTextStream* stream);

    void parse();
private:
    // Note: signatures here are just a list of strings;
    // the last one is the return type

    virtual void handleType(const QString& type, const QString& nativeName, bool im, bool rg, bool al8) = 0;
    virtual void handleConversion(const QString& name, const QString& code, int codeLine,
                                  bool immediate, bool checked, bool mayThrow,
                                  const QString& from, const QString& to, int cost) = 0;
    virtual void handleOperation(const QString& name) = 0;
    virtual void handleImpl(const QString& fnName, const QString& code, int codeLine, int cost,
                            const QString& retType, QStringList sig, QStringList paramNames) = 0;
    virtual void handleTile(const QString& fnName, QStringList sig) = 0;

    QString matchIdentifier();
    QString matchCode(int& lineOut);
    int     matchNumber();
    void    match(Lexer::TokenType t);

    // Checks whether an optional flag is next..
    bool checkFlag(Lexer::TokenType t);

    void parseType();
    void parseConversion();
    void parseOperation();
    void parseImpl();
    void parseTile();

    bool tokenLoaded;
    Lexer::Token nextToken;

    void issueError(const QString& msg);

    bool   hadError;

    Lexer* lexer;

    Lexer::Token peekNext();
    Lexer::Token getNext();
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

