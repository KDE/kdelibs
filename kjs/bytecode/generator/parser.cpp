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

QString Parser::matchCode()
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Code)
        return tok.value;
    issueError("Expected code, got:" + tok.toString(lexer));
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

bool Parser::checkFlag(Lexer::TokenType t)
{
    if (peekNext().type == t) {
        getNext();
        return true;
    }
    return false;
}

void Parser::issueError(const QString& msg)
{
    qWarning() << "Parse error:" << msg << "at about line:" << lexer->lineNumber();
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

    if (nativeName == "const")
        nativeName += " " + matchIdentifier();

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

static QString capitalized(const QString& in)
{
    return in[0].toUpper() + in.mid(1);
}

void Parser::parseConversion()
{
    // conversion from =>  to { clauses .. }
    // clause := tile costs number; || impl checked? mayThrow? code; || register ident costs number;
    match(Lexer::Conversion);
    QString from = matchIdentifier();
    match(Lexer::Arrow);
    QString to = matchIdentifier();

    match(Lexer::LBrace);

    // impl clause info..
    bool implMayThrow = false, immChecked = false;
    QString code;

    // tile clause info
    int tileCost = 0;

    // register clause info
    bool hasRegister = false;
    QString registerIdent;
    int registerCost = 0;

    while (peekNext().type != Lexer::RBrace) {
        switch (peekNext().type) {
        case Lexer::Impl:
            match(Lexer::Impl);
            immChecked   = checkFlag(Lexer::Checked);
            implMayThrow = checkFlag(Lexer::MayThrow);
            code = matchCode();
            break;
        case Lexer::Tile:
            match(Lexer::Tile);
            match(Lexer::Costs);
            tileCost = matchNumber();
            match(Lexer::SemiColon);
            break;
        case Lexer::Register:
            hasRegister = true;
            match(Lexer::Register);
            registerIdent = matchIdentifier();
            match(Lexer::Costs);
            registerCost  = matchNumber();
            match(Lexer::SemiColon);
            break;
        default:
            issueError("Invalid start of a clause within conversion block:" + peekNext().toString(lexer));
        }
    }

    match(Lexer::RBrace);

    if (hasRegister)
        handleConversion(registerIdent, "", false, false, false, from, to, registerCost);

    // Computer name, from type sig
    QString name = "I" + capitalized(from) + "_" + capitalized(to);
    handleConversion(name, code, true, immChecked, implMayThrow, from, to, tileCost);
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
    // impl identifier identifier ( paramList? ) code
    // paramList := ident ident
    // paramList := ident ident , paramList

    match(Lexer::Impl);

    QString ret = matchIdentifier();
    QString fn  = matchIdentifier();
    match(Lexer::LParen);

    // Parse parameter types and names, if any..
    QStringList paramSigs;
    QStringList paramNames;
    while (peekNext().type != Lexer::RParen) {
        paramSigs  << matchIdentifier();
        paramNames << matchIdentifier();
        if (peekNext().type != Lexer::Comma)
            break;
        getNext(); // Eat the comma..
        // Make sure we have an ident next, and not an rparen..
        if (peekNext().type != Lexer::Ident)
            issueError("Parameter signature in impl doesn't start with an identifier!");
    }
    match(Lexer::RParen);

    int cost = 0;
    if (peekNext().type == Lexer::Costs) {
        getNext();
        cost = matchNumber();
    }

    QString code = matchCode();


    handleImpl(fn, code, cost, ret, paramSigs, paramNames);
}

void Parser::parseTile()
{
    // tile signature as identifier;
    match(Lexer::Tile);

    QStringList paramSigs;
    match(Lexer::LParen);
    while (peekNext().type != Lexer::RParen) {
        paramSigs  << matchIdentifier();
        if (peekNext().type != Lexer::Comma)
            break;
        getNext(); // Eat the comma..
        // Make sure we have an ident next, and not an rparen..
        if (peekNext().type != Lexer::Ident)
            issueError("Parameter signature in tile doesn't start with an identifier!");
    }

    match(Lexer::RParen);

    match(Lexer::As);
    QString     fn = matchIdentifier();
    handleTile(fn, paramSigs);
    match(Lexer::SemiColon);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
