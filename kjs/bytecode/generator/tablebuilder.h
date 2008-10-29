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

#ifndef TABLE_BUILDER_H
#define TABLE_BUILDER_H

#include "parser.h"

#include <iostream>
#include <vector>

using std::ostream;
using std::vector;

struct Type
{
    string name;
    string nativeName;
    bool im, reg, align8;

    bool operator==(const Type& other) const {
        return name == other.name;
    }
};

struct ConversionInfo
{
    string name;
    string impl;
    int  cost;   // for w/in tile for immediate, for external for reg
    unsigned flags;
    Type from;
    Type to;
    int codeLine;

    ConversionInfo(): cost(0), flags(Conv_NoFlags)
    {} //Can be called for trivial conversion
};

// Actually, a specialization, but who cares?
struct Operation
{
    string name;
    string retType;
    vector<Type> parameters;
    int          cost;
    int          codeLine;
    bool         overload;
    bool         endsBB;
    bool         isTile;

    string implementAs;
    vector<Type> implParams;
    StringList   implParamNames;
    HintList     implHints;
};

struct OperationVariant
{
    string   sig;
    Operation op;
    vector<bool> paramIsIm;
    vector<int>  paramOffsets;
    int         size;
    bool        needsPadVariant;
};

class TableBuilder: public Parser
{
public:
    TableBuilder(istream* inStream, ostream* hStream, ostream* cppStream,
                 ostream* mStream);

    void generateCode();
private:
    void issueError(const string& err);

    virtual void handleType(const string& type, const string& nativeName, bool im, bool rg, bool al8);
    virtual void handleConversion(const string& runtimeRoutine, int codeLine,
                                  unsigned flags, const string& from, const string& to,
                                  int tileCost, int registerCost);

    virtual void handleOperation(const string& name, bool endsBB);
    virtual void handleImpl(const string& fnName, const string& code, bool overload,
                            int codeLine, int cost, const string& retType, StringList sig,
                            StringList paramNames, HintList hints);
    virtual void handleTile(const string& fnName, StringList sig);

    void printConversionInfo(map<string, map<string, ConversionInfo> >& table, bool last);

    void printConversionRoutine(const ConversionInfo& conversion);

    void printCode(ostream* out, int baseIdent, const string& code, int baseLine);

    // Enumerates all r/i/pad variants; plus computes the shuffle table.
    void expandOperationVariants(const Operation& op, vector<bool>& paramIsIm);

    void dumpOpStructForVariant(const OperationVariant& variant, bool doPad,
                                bool hasPadVariant, bool needsComma);

    void generateVariantImpl(const OperationVariant& variant);

    // issues error if there is a problem..
    vector<Type> resolveSignature(const StringList& in);

    ostream* hStream;
    ostream* cppStream;
    ostream* mStream;

    ostream& mInd(int ind);

    map<string, Type> types;
    StringList        typeNames;

    StringList          operationNames;
    vector<bool>        operationEndBB;
    map<string, string> operationRetTypes; // uglily enough specified on the impl. I suck.
    vector<Operation>   operations;
    map<string, Operation> implementations;

    StringList  variantNames;
    vector<OperationVariant> variants;
    map<string, StringList>  variantNamesForOp;

    StringList conversionNames;
    vector<ConversionInfo> imConversionList;
    vector<ConversionInfo> rgConversionList;

    map<string, map<string, ConversionInfo> > imConversions;
    map<string, map<string, ConversionInfo> > rgConversions;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
