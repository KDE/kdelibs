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
#include "parser.h"
#include <QDebug>
#include <QFile>

#include <stdlib.h>

/**
 This is a fairly straightforward affair. It's written in a
 recursive descent style, but the language is actually regular.

 The error recovert is even simpler: we just exit. That's it.
*/

Parser::Parser(QTextStream* stream): tokenLoaded(false), hadError(false), lexer(new Lexer(stream))
{}

QString Parser::matchIdentifier()
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Ident)
        return tok.value;
    issueError("Expected identifier, got:" + tok.toString(lexer));
    return "";
}

int Parser::matchNumber()
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Number)
        return tok.value.toInt();
    issueError("Expected number, got:" + tok.toString(lexer));
    return 0;
}

void Parser::match(Lexer::TokenType t)
{
    Lexer::Token tok = getNext();
    if (tok.type != t)
        issueError("Expected " + Lexer::Token(t).toString(lexer) + " got:" + tok.toString(lexer));
}

void Parser::issueError(const QString& msg)
{
    qWarning() << "Parse error:" << msg;
    exit(-1);
}

Lexer::Token Parser::peekNext()
{
    if (!tokenLoaded) {
        nextToken   = lexer->nextToken();
        tokenLoaded = true;
    }

    return nextToken;
}

Lexer::Token Parser::getNext()
{
    if (tokenLoaded) {
        tokenLoaded = false;
        return nextToken;
    }

    return lexer->nextToken();
}

void Parser::parse()
{
    Lexer::Token tok = peekNext();

    // The types are first..
    while (tok.type == Lexer::Type) {
        parseType();
        tok = peekNext();
    }

    // Now we may have conversions or operations
    while (tok.type == Lexer::Conversion || tok.type == Lexer::Operation) {
        if (tok.type == Lexer::Conversion)
            parseConversion();
        else
            parseOperation();
        tok = peekNext();
    }

    match(Lexer::EndOfFile);
}

void Parser::parseType()
{
    //type identifier:  nativeName *? (immediate | register | align8)*;
    match(Lexer::Type);

    QString name = matchIdentifier();
    match(Lexer::Colon);
    QString nativeName = matchIdentifier();
    if (peekNext().type == Lexer::Star) {
        nativeName += "*";
        getNext();
    }

    bool im = false, rg = false, al8 = false;

    Lexer::Token tok = getNext();
    while (tok.type != Lexer::SemiColon) {
        switch (tok.type) {
        case Lexer::Immediate:
            im = true;
            break;
        case Lexer::Register:
            rg = true;
            break;
        case Lexer::Align8:
            al8 = true;
            break;
        default:
            issueError("Unexpected type decoration, got:" + tok.toString(lexer));
        }

        tok = getNext();
    }

    handleType(name, nativeName, im, rg, al8);
}

QStringList Parser::parseSignature()
{
    QStringList sig;

    // identifier ( * identifier)
    sig << matchIdentifier();
    while (peekNext().type == Lexer::Star) {
        getNext();
        sig << matchIdentifier();
    }

    return sig;
}

void Parser::parseConversion()
{
    // conversion [register | immediate] checked? identifier:name identifier:from => identifier: to costs number ;
    match(Lexer::Conversion);

    bool immediate;
    Lexer::Token convType = getNext();
    if (convType.type == Lexer::Immediate)
        immediate = true;
    else if (convType.type == Lexer::Register)
        immediate = false;
    else
        issueError("conversion must be followed by immediate or register");

    bool checked = false;
    if (peekNext().type == Lexer::Checked) {
        checked = true;
        getNext(); // eat it.
    }

    QString name = matchIdentifier();

    match(Lexer::Colon);

    QString from = matchIdentifier();
    match(Lexer::Arrow);
    QString to = matchIdentifier();

    match(Lexer::Costs);

    int cost = matchNumber();
    match(Lexer::SemiColon);

    handleConversion(name, immediate, checked, from, to, cost);
}

void Parser::parseOperation()
{
    // operation identifier { ... },  where ... is a list of impl or tile statements.
    match(Lexer::Operation);
    handleOperation(matchIdentifier());

    match(Lexer::LBrace);
    Lexer::Token tok = peekNext();
    while (tok.type == Lexer::Tile || tok.type == Lexer::Impl) {
        if (tok.type == Lexer::Tile)
            parseTile();
        else
            parseImpl();
        tok = peekNext();
    }
    match(Lexer::RBrace);
}

void Parser::parseImpl()
{
    // impl identifier : signature;
    match(Lexer::Impl);

    QString     fn  = matchIdentifier();
    match(Lexer::Colon);
    QStringList sig = parseSignature();
    handleImpl(fn, sig);
    match(Lexer::SemiColon);
}

void Parser::parseTile()
{
    // tile signature as identifier;
    match(Lexer::Tile);

    QStringList sig = parseSignature();
    match(Lexer::As);
    QString     fn  = matchIdentifier();
    handleTile(fn, sig);

    match(Lexer::SemiColon);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
