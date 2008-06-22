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

#include <cstdlib>
#include <wtf/ASCIICType.h>

/**
 This is a fairly straightforward affair. It's written in a
 recursive descent style, but the language is actually regular.

 The error recovert is even simpler: we just exit. That's it.
*/

Parser::Parser(istream* stream): tokenLoaded(false), hadError(false), lexer(new Lexer(stream))
{}

string Parser::matchIdentifier()
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Ident)
        return tok.value;
    issueError("Expected identifier, got:" + tok.toString(lexer));
    return "";
}

string Parser::matchCode(int& lineOut)
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Code) {
        lineOut = tok.lineNum;
        return tok.value;
    }
    issueError("Expected code, got:" + tok.toString(lexer));
    return "";
}

int Parser::matchNumber()
{
    Lexer::Token tok = getNext();
    if (tok.type == Lexer::Number)
        return std::atol(tok.value.c_str());
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

void Parser::issueError(const string& msg)
{
    std::cerr << "Parse error:" << msg << "at about line:" << lexer->lineNumber();
    std::exit(-1);
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
    while (tok.type == Lexer::Conversion || tok.type == Lexer::Operation || tok.type == Lexer::Jump) {
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

    string name = matchIdentifier();
    match(Lexer::Colon);
    string nativeName = matchIdentifier();

    if (nativeName == "const")
        nativeName += " " + matchIdentifier();

    while (peekNext().type == Lexer::Scope) {
        getNext();
        nativeName += "::" + matchIdentifier();
    }

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

static string capitalized(const string& in)
{
    return WTF::toASCIIUpper(in[0]) + in.substr(1);
}

void Parser::parseConversion()
{
    // conversion from =>  to { clauses .. }
    // clause := tile costs number; || impl checked? mayThrow? code; || register ident costs number;
    match(Lexer::Conversion);
    string from = matchIdentifier();
    match(Lexer::Arrow);
    string to = matchIdentifier();

    match(Lexer::LBrace);

    // impl clause info..
    bool implMayThrow = false, immChecked = false;
    string code;
    int     codeLine;

    // tile clause info
    int tileCost = 0;

    // register clause info
    bool hasRegister = false;
    string registerIdent;
    int registerCost = 0;

    while (peekNext().type != Lexer::RBrace) {
        switch (peekNext().type) {
        case Lexer::Impl:
            match(Lexer::Impl);
            immChecked   = checkFlag(Lexer::Checked);
            implMayThrow = checkFlag(Lexer::MayThrow);
            code = matchCode(codeLine);
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
        handleConversion(registerIdent, code, codeLine, false, false, false, from, to, registerCost);

    // Computer name, from type sig
    string name = "I" + capitalized(from) + "_" + capitalized(to);
    handleConversion(name, code, codeLine, true, immChecked, implMayThrow, from, to, tileCost);
}

void Parser::parseOperation()
{
    // operation identifier { ... },  where ... is a list of impl or tile statements.

    bool jump = false;
    if (peekNext().type == Lexer::Operation) {
        match(Lexer::Operation);
    } else {
        jump = true;
        match(Lexer::Jump);
    }

    handleOperation(matchIdentifier(), jump);

    match(Lexer::LBrace);
    Lexer::Token tok = peekNext();
    while (tok.type == Lexer::Tile || tok.type == Lexer::Impl || tok.type == Lexer::Overload) {
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
    // impl identifier identifier? ( paramList? ) code
    // paramList := ident ident
    // paramList := ident ident , paramList

    bool overload = getNext().type == Lexer::Overload;

    string ret = matchIdentifier();

    string fn;
    if (peekNext().type == Lexer::Ident)
        fn = matchIdentifier();
    match(Lexer::LParen);

    // Parse parameter types and names, if any..
    StringList paramSigs;
    StringList paramNames;
    HintList   paramHints;
    while (peekNext().type != Lexer::RParen) {
        paramSigs.push_back (matchIdentifier());

        if (peekNext().type == Lexer::NoImm) {
            getNext();
            paramHints.push_back(NoImm);
        } else {
            paramHints.push_back(NoHint);
        }
        
        paramNames.push_back(matchIdentifier());

        
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

    int codeLine;
    string code = matchCode(codeLine);

    handleImpl(fn, code, overload, codeLine, cost, ret, paramSigs, paramNames, paramHints);
}

void Parser::parseTile()
{
    // tile signature as identifier;
    match(Lexer::Tile);

    StringList paramSigs;
    match(Lexer::LParen);
    while (peekNext().type != Lexer::RParen) {
        paramSigs.push_back(matchIdentifier());
        if (peekNext().type != Lexer::Comma)
            break;
        getNext(); // Eat the comma..
        // Make sure we have an ident next, and not an rparen..
        if (peekNext().type != Lexer::Ident)
            issueError("Parameter signature in tile doesn't start with an identifier!");
    }

    match(Lexer::RParen);

    match(Lexer::As);
    string     fn = matchIdentifier();
    handleTile(fn, paramSigs);
    match(Lexer::SemiColon);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
