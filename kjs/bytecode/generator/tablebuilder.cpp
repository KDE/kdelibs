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
#include "tablebuilder.h"
#include <stdlib.h>
#include <iostream>
#include "assert.h"
#include <cctype>
#include <cstdio>
#include <wtf/ASCIICType.h>

using namespace std;

class Enum {
public:
    Enum(const string& name, const string& prefix, StringList values):
        name(name), prefix(prefix), values(values)
    {}

    void printDeclaration(ostream* hStream)
    {
        *hStream << "enum " << name << " {\n";
        for (unsigned p = 0; p < values.size(); ++p) {
            *hStream << "    " << prefix << values[p] << ",\n";
        }
        *hStream << "    " << prefix << "NumValues\n";
        *hStream << "};\n";
        *hStream << "extern const char* const " << name << "Vals[];\n\n";
    }

    void printDefinition(ostream* hStream)
    {
        *hStream << "const char* const " << name << "Vals[] = {\n";
        for (unsigned p = 0; p < values.size(); ++p) {
            *hStream << "    \"" << prefix << values[p] << "\"";
            if (p != (values.size() - 1))
                *hStream << ",";
            *hStream << "\n";
        }
        *hStream << "};\n\n";
    }
private:
    string name;
    string prefix;
    StringList values;
};

static string strReplace(string where, string from, string to) {
    string res = where;
    size_t pos;
    while ((pos = res.find(from)) != string::npos) {
        res = res.substr(0, pos) + to + res.substr(pos + from.length());
    }
    return res;
}

TableBuilder::TableBuilder(istream* inStream, ostream* hStream,
                           ostream* cppStream, ostream* mStream):
    Parser(inStream), hStream(hStream), cppStream(cppStream), mStream(mStream)
{
    // Builtin stuff...
    conversionNames.push_back("NoConversion");
    conversionNames.push_back("NoOp");

    // Special ones for stuff that might not fit into immediate..
    // ### TODO: eventually, auto-spills will be better, and will
    // permit to throttle VM size
    conversionNames.push_back("I_R_Int32_Value");
    conversionNames.push_back("I_R_Number_Value");
}

// # of bits store 'vals' values, e.g. 3 for 8, etc.
static unsigned neededBits(unsigned vals)
{
    unsigned bits = 1;
    while ((1 << bits) < vals)
        ++bits;
    return bits;
}

void TableBuilder::generateCode()
{
    parse();

    // Types... First we just want to list them
    Enum typesEnum("OpType", "OpType_", typeNames);
    typesEnum.printDeclaration(hStream);
    typesEnum.printDefinition (cppStream);

    // Also, print out the width array...
    *cppStream << "const bool opTypeIsAlign8[] = {\n";
    for (unsigned t = 0; t < typeNames.size(); ++t) {
        const Type& type = types[typeNames[t]];
        *cppStream << (type.align8 ? "true": "false");
        if (t != typeNames.size() - 1)
            *cppStream << ",";
        *cppStream << " //" << type.name << "\n";
    }

    *cppStream << "};\n\n";

    // Conversion ops. Those go entirely in the .cpp
    Enum convOps("ConvOp", "Conv_", conversionNames);
    convOps.printDeclaration(hStream);
    convOps.printDefinition (cppStream);

    *cppStream << "struct ConvInfo {\n";
    *cppStream << "    ConvOp routine;\n";
    *cppStream << "    int    costCode;\n";
    *cppStream << "};\n\n";

    // For conversion info, we use upper bit for register/immediate (immediate is set),
    // and then enough bits for the from/to types as the index.
    *cppStream << "static const ConvInfo conversions[] = {\n";
    printConversionInfo(rgConversions, true);
    printConversionInfo(imConversions, false);
    *cppStream << "};\n\n";

    int numBits = neededBits(types.size());
    *cppStream << "static inline const ConvInfo* getConversionInfo(bool immediate, OpType from, OpType to)\n{\n";
    *cppStream << "    return &conversions[((int)immediate << " << (2 * numBits) << ")"
               << " | ((int)from << " << numBits << ") | (int)to];\n";
    *cppStream << "}\n\n";

    // Conversion helpers..
    for (unsigned c = 0; c < imConversionList.size(); ++c)
        printConversionRoutine(imConversionList[c]);

    *cppStream << "static bool emitImmediateConversion(ConvOp convType, OpValue* original, OpValue& out)\n{\n";
    *cppStream << "    out.immediate = true;\n";
    *cppStream << "    switch(convType) {\n";
    *cppStream << "    case Conv_NoOp:\n";
    *cppStream << "        out = *original;\n";
    *cppStream << "        break;\n";
    for (unsigned c = 0; c < imConversionList.size(); ++c) {
        const ConversionInfo& inf = imConversionList[c];
        *cppStream << "    case Conv_" << inf.name << ":\n";
        *cppStream << "        out.type = OpType_" << inf.to.name << ";\n";
        *cppStream << "        out.value." << (inf.to.align8 ? "wide" : "narrow")
                   << "." << inf.to.name << "Val = "
                   << "convert" << inf.name << "(0, "
                   << "original->value." << (inf.from.align8 ? "wide" : "narrow")
                   << "." << inf.from.name << "Val);\n";

        *cppStream << "        break;\n";
    }

    *cppStream << "    default:\n";
    *cppStream << "        return false;\n";
    *cppStream << "    }\n";
    *cppStream << "    return true;\n";    
    *cppStream << "}\n\n";

    // Similar helper for simple register conversions..
    *cppStream << "static bool emitSimpleRegisterConversion(CompileState* comp, ConvOp convType, OpValue* original, OpValue& out)\n{\n";
    *cppStream << "    switch(convType) {\n";
    *cppStream << "    case Conv_NoOp:\n";
    *cppStream << "        out = *original;\n";
    *cppStream << "        break;\n";
    for (unsigned c = 0; c < rgConversionList.size(); ++c) {
        const ConversionInfo& inf = rgConversionList[c];
        *cppStream << "    case Conv_" << inf.name << ":\n";
        *cppStream << "        CodeGen::emitOp(comp, Op_" << inf.name << ", &out, original);\n";
        *cppStream << "        break;\n";
    }
    *cppStream << "    default:\n";
    *cppStream << "        return false;\n";
    *cppStream << "    }\n";
    *cppStream << "    return true;\n";
    *cppStream << "}\n\n";

    // Operations
    Enum opNamesEnum("OpName", "Op_", operationNames);
    opNamesEnum.printDeclaration(hStream);
    opNamesEnum.printDefinition (cppStream);

    // Enumerate all the variants..
    for (unsigned c = 0; c < operations.size(); ++c) {
        vector<bool> parIm;
        expandOperationVariants(operations[c], parIm);
    }

    // Return types for each..
    *cppStream << "static const OpType opRetTypes[] = {\n";
    for (unsigned c = 0; c < operationNames.size(); ++c) {
        *cppStream << "     OpType_" << operationRetTypes[operationNames[c]];
        if (c  != operationNames.size() - 1)
            *cppStream << ",";
        *cppStream << " //" << operationNames[c] << "\n";
    }
    *cppStream << "};\n\n";

    // Now we have all our bytecode names... Whee.
    Enum opByteCodesEnum("OpByteCode", "OpByteCode_", variantNames);
    opByteCodesEnum.printDeclaration(hStream);
    opByteCodesEnum.printDefinition (cppStream);

    // We can now emit the actual tables...

    // ... first descriptors for each bytecode op..
    *cppStream << "const Op opsForOpCodes[] = {\n";
    for (unsigned c = 0; c < variants.size(); ++c) {
        const OperationVariant& variant = variants[c];
        if (variant.needsPadVariant)
            dumpOpStructForVariant(variant, true, variant.needsPadVariant, true);
        dumpOpStructForVariant(variant, false, variant.needsPadVariant, c != variants.size() - 1);
    }
    *cppStream << "};\n\n";

    // then variant tables for each main op..
    for (unsigned c = 0; c < operationNames.size(); ++c) {
        const string& opName = operationNames[c];
        *cppStream << "static const Op* const op" << opName << "Variants[] = {";
        StringList variants = variantNamesForOp[opName];
        for (unsigned v = 0; v < variants.size(); ++v) {
            *cppStream << "&opsForOpCodes[OpByteCode_" << variants[v] << "], ";
        }
        *cppStream << "0};\n";
    }
    *cppStream << "\n";

    *cppStream << "const Op* const* const opSpecializations[] = {\n";
    for (unsigned o = 0; o < operationNames.size(); ++o) {
        *cppStream << "    op" << operationNames[o] << "Variants";
        if (o != (operationNames.size() - 1))
            *cppStream << ",";
        *cppStream << "\n";
    }
    *cppStream << "};\n\n";

    // Now, generate the VM loop.
    mInd(8) << "OpByteCode op = *reinterpret_cast<const OpByteCode*>(pc);\n";
    mInd(8) << "switch (op) {\n";
    for (unsigned c = 0; c < variants.size(); ++c) {
        const OperationVariant& var = variants[c];
        if (var.needsPadVariant) {
            mInd(12) << "case OpByteCode_" + var.sig + "_Pad:\n";
            mInd(16) << "pc += 4;\n";
            mInd(16) << "// Fallthrough\n";
        }

        mInd(12) << "case OpByteCode_" + var.sig + ": {\n";
        generateVariantImpl(var);
        mInd(12) << "}\n";
        mInd(12) << "break;\n\n";
    }

    mInd(8) << "}\n\n";
}

void TableBuilder::issueError(const string& err)
{
    std::cerr << err << "\n";
    exit(-1);
}

void TableBuilder::printConversionInfo(map<string, map<string, ConversionInfo> >& table, bool reg)
{
    unsigned numBits = neededBits(types.size());
    unsigned fullRange = 1 << numBits;
    for (unsigned from = 0; from < fullRange; ++from) {
        for (unsigned to = 0; to < fullRange; ++to) {
            if (from < types.size() && to < types.size()) {
                string fromName = typeNames[from];
                string toName   = typeNames[to];

                // For register conversion, we need it to be possible for source + dest to be in
                // registers. For immediate, we only require source, since dest will just go
                // into local value.
                bool representable;
                if (reg)
                    representable = types[fromName].reg && types[toName].reg;
                else
                    representable = types[fromName].im;

                if (from == to) {
                    *cppStream << "    {Conv_NoOp, 0}";
                } else if (table[fromName].find(toName) != table[fromName].end() && representable) {
                    // We skip immediate conversions for things that can't be immediate, as
                    // we don't have exec there..
                    const ConversionInfo& inf = table[fromName][toName];
                    *cppStream << "    {Conv_" << inf.name << ",";
                    if (inf.flags & Conv_Checked)
                        *cppStream << "Cost_Checked}";
                    else
                        *cppStream << (reg ? inf.cost : 0) << "}";
                } else {
                    *cppStream << "    {Conv_NoConversion, Cost_NoConversion}";
                }

                *cppStream << " /*" << fromName << " => " << toName << "*/";
            } else {
                *cppStream << "    {Conv_NoConversion, Cost_NoConversion}";
            }

            if (reg || from != (fullRange - 1) || to != (fullRange - 1))
                *cppStream << ",";

            *cppStream << "\n";
        } // for to..
    } // for from..
}

void TableBuilder::printConversionRoutine(const ConversionInfo& conversion)
{
    *hStream << "ALWAYS_INLINE " << conversion.to.nativeName << " convert" << conversion.name
         << "(ExecState* exec, " << conversion.from.nativeName << " in)\n";
    *hStream << "{\n";
    *hStream << "    (void)exec;\n";
    printCode(hStream, 4, conversion.impl, conversion.codeLine);
    *hStream << "}\n\n";
}

void TableBuilder::handleType(const string& type, const string& nativeName, bool im, bool rg, bool al8)
{
    typeNames.push_back(type);
    Type t;
    t.name = type;
    t.nativeName = nativeName;
    t.im     = im;
    t.reg    = rg;
    t.align8 = al8;
    types[type] = t;
}

static string capitalized(const string& in)
{
    return WTF::toASCIIUpper(in[0]) + in.substr(1);
}

void TableBuilder::handleConversion(const string& code, int codeLine,
                                    unsigned flags, const string& from, const string& to,
                                    int tileCost, int registerCost)
{
    // Compute the conversion names. The register one (if any) would also create an operation.
    string immName = "I" + capitalized(from) + "_" + capitalized(to);
    string regName = "R" + capitalized(from) + "_" + capitalized(to);

    // Register immediate conversion
    conversionNames.push_back(immName);
    ConversionInfo inf;
    inf.name    = immName;
    inf.cost    = tileCost;
    inf.flags   = flags;
    inf.impl     = code;
    inf.codeLine = codeLine;
    inf.from     = types[from];
    inf.to       = types[to];

    imConversions[from][to] = inf;
    imConversionList.push_back(inf);

    // ... and, if it exists, register one.
    if (flags & Conv_HaveReg) {
        conversionNames.push_back(regName);
        inf.name = regName;
        inf.cost = registerCost;
        inf.flags &= ~Conv_Checked; // 'checked' makes no sense here
        rgConversions[from][to] = inf;
        rgConversionList.push_back(inf);
        
        // We also generate the corresponding bytecode routine, using
        // the immediate conversion helper we'll emit in it.
        handleOperation(regName, false);

        StringList sig;
        sig.push_back(from);
        StringList names;
        names.push_back("in");
        HintList hints;
        hints.push_back(NoHint);

        string code = inf.to.nativeName + " out = convertI" + inf.name.substr(1) + "(exec, in);\n";
        code += "$$ = out;\n";
        handleImpl("", code, false, codeLine, 0, to, sig, names, hints);
    }
}

void TableBuilder::handleOperation(const string& name, bool endsBB)
{
    operationNames.push_back(name);
    operationEndBB.push_back(endsBB);
}

vector<Type> TableBuilder::resolveSignature(const StringList& in)
{
    vector<Type> sig;
    for (unsigned c = 0; c < in.size(); ++c) {
        const string& type = in[c];
        if (types.find(type) != types.end())
            sig.push_back(types[type]);
        else
            issueError("Unknown type:" + type);
    }
    return sig;
}

void TableBuilder::handleImpl(const string& fnName, const string& code, bool ol, int codeLine, int cost,
                              const string& retType, StringList sig, StringList paramNames, HintList hints)
{
    // If the return type isn't 'void', we prepend a destination register as a parameter in the encoding.
    // emitOp will convert things as needed
    StringList extSig;
    StringList extParamNames;
    HintList   extHints;
    if (retType != "void") {
        extSig.push_back("reg");
        extParamNames.push_back("fbDestReg");
        extHints.push_back(NoHint);
    }

    for (unsigned c = 0; c < sig.size(); ++c) {
        extSig.push_back(sig[c]);
        extParamNames.push_back(paramNames[c]);
        extHints.push_back(hints[c]);
    }

    Operation op;
    op.name           = operationNames.back();
    op.retType        = retType;
    op.overload       = ol;
    operationRetTypes[op.name] = retType;
    op.isTile         = false;
    op.implementAs    = code;
    op.codeLine       = codeLine;
    op.parameters     = resolveSignature(extSig);
    op.implParams     = op.parameters;
    op.implParamNames = extParamNames;
    op.implHints      = extHints;
    op.cost           = cost;
    op.endsBB         = operationEndBB.back();
    operations.push_back(op);
    if (!fnName.empty())
        implementations[fnName] = op;
}

void TableBuilder::handleTile(const string& fnName, StringList sig)
{
    if (implementations.find(fnName) == implementations.end())
        issueError("Unknown implementation name " + fnName + " in a tile definition");
    const Operation& impl = implementations[fnName];

    // Add in a return reg if need be
    StringList extSig;
    if (impl.retType != "void")
        extSig.push_back("reg");

    for (unsigned c = 0; c < sig.size(); ++c)
        extSig.push_back(sig[c]);

    Operation op;
    op.name        = operationNames.back();
    op.isTile      = true;
    op.implementAs = impl.implementAs;
    op.codeLine    = impl.codeLine;
    op.retType     = impl.retType;
    op.overload    = impl.overload; // if original required precise matching, so did the tile.
    op.parameters  = resolveSignature(extSig);
    op.implParams     = impl.implParams;
    op.implParamNames = impl.implParamNames;
    // ### not sure we want this here
    op.implHints     = impl.implHints;
    op.endsBB        = impl.endsBB;
    
    op.cost           = impl.cost;
    // Now also include the cost of inline conversions.
    for (unsigned p = 0; p < op.parameters.size(); ++p)
        op.cost += imConversions[op.parameters[p].name][op.implParams[p].name].cost;

    operations.push_back(op);
}

void TableBuilder::expandOperationVariants(const Operation& op, vector<bool>& paramIsIm)
{
    int numParams = op.parameters.size();
    if (paramIsIm.size() < numParams) {
        int paramPos = paramIsIm.size();
        bool hasIm  = op.parameters[paramPos].im;
        bool hasReg = op.parameters[paramPos].reg;

        bool genIm  = hasIm;
        bool genReg = hasReg;

        // Don't generate non-register variants for tiles when possible.
        if (op.isTile && hasReg)
            genIm = false;

        // There may be hints saying not to generate some version
        if (op.implHints[paramPos] & NoImm)
            genIm = false;

        if (op.implHints[paramPos] & NoReg)
            genReg = false;
        
        if (genIm) {
            paramIsIm.push_back(true);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.pop_back();
        }

        if (genReg) {
            paramIsIm.push_back(false);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.pop_back();
        }
        return;
    }

    // Have a full variant... Build a signature.

    string sig = op.name;
    for (int p = 0; p < numParams; ++p) {
        sig += "_";
        sig += paramIsIm[p] ? "I" : "R";
        sig += op.parameters[p].name;
    }

    // We may need padding if we have an immediate align8 param..
    bool needsPad = false;
    for (int c = 0; c < numParams; ++c)
        needsPad |= (paramIsIm[c] & op.parameters[c].align8);

    OperationVariant var;
    var.sig = sig;
    var.op  = op;
    var.paramIsIm = paramIsIm;
    var.needsPadVariant = needsPad;

    // Build offset table, giving param positions..
    while (var.paramOffsets.size() < numParams) // no setSize in QList..
        var.paramOffsets.push_back(0);

    int pos = 4;
    // pad8/align ones go first.
    for (int c = 0; c < numParams; ++c) {
        if (paramIsIm[c] & op.parameters[c].align8) {
            var.paramOffsets[c] = pos;
            pos += 8;
        }
    }

    // Then the rest..
    for (int c = 0; c < numParams; ++c) {
        if (!paramIsIm[c] || !op.parameters[c].align8) {
            var.paramOffsets[c] = pos;
            pos += 4;
        }
    }
    var.size = pos;

    variants.push_back(var);
    if (needsPad) { // we put the pad before, due to the fallthrough idiom..
        string pSig = sig + "_Pad";
        variantNames.push_back(pSig);
        variantNamesForOp[op.name].push_back(pSig);
    }
    variantNames.push_back(sig);
    variantNamesForOp[op.name].push_back(sig);
}

void TableBuilder::dumpOpStructForVariant(const OperationVariant& variant, bool doPad,
                                          bool hasPadVariant, bool needsComma)
{
    *cppStream << "    {";
    *cppStream << "Op_" << variant.op.name << ", ";     // baseInstr..
    *cppStream << "OpByteCode_" << (doPad ? variant.sig + "_Pad" : variant.sig) << ", "; // byteCode op
    *cppStream << variant.op.cost << ", "; // uhm, cost. doh.
    int numParams = variant.op.parameters.size();
    *cppStream << numParams << ", "; // # of params

    // Param types.
    *cppStream << "{";
    for (int p = 0; p < numParams; ++p) {
        *cppStream << "OpType_" << variant.op.parameters[p].name;
        if (p != numParams - 1)
            *cppStream << ", ";
    }
    *cppStream << "}, ";

    // Immediate flag..
    *cppStream << "{";
    for (int p = 0; p < numParams; ++p) {
        *cppStream << (variant.paramIsIm[p] ? "true" : "false");
        if (p != numParams - 1)
            *cppStream << ", ";
    }
    *cppStream << "}, ";

    // Return type.
    *cppStream << "OpType_" << variant.op.retType << ", ";

    int adjust = doPad ? 4 : 0; // padded version has 4 extra bytes,
                                // between the opcode and the first arg.
    // Size..
    *cppStream << (variant.size + adjust) << ", ";

    // Offset table..
    *cppStream << "{";
    for (int p = 0; p < numParams; ++p) {
        *cppStream << (variant.paramOffsets[p] + adjust);
        if (p != numParams - 1)
            *cppStream << ", ";
    }

    *cppStream << "}, ";

    // Whether this is a padded version..
    *cppStream << (doPad ? "true" : "false") << ", ";

    // And whether a padded version exists.
    *cppStream << (hasPadVariant ? "true" : "false") << ", ";

    // Whether this is an overload, requiring precise matching
    *cppStream << (variant.op.overload ? "true" : "false") << ", ";

    // Whether this ends a basic block.
    *cppStream << (variant.op.endsBB ? "true" : "false");

    if (needsComma)
        *cppStream << "},\n";
    else
        *cppStream << "}\n";
}

ostream& TableBuilder::mInd(int ind)
{
    for (int i = 0; i < ind; ++i)
        *mStream << ' ';
    return *mStream;
}

static bool isWhitespaceString(const string& str)
{
    for (unsigned c = 0; c < str.length(); ++c) {
        if (!WTF::isASCIISpace(str[c]))
            return false;
    }

    return true;
}

StringList splitLines(const string& in)
{
    StringList lines;
    string     curLine;
    for (unsigned c = 0; c < in.length(); ++c) {
        if (in[c] == '\n') {
            lines.push_back(curLine);
            curLine = "";
        } else {
            curLine += in[c];
        }
    }
    return lines;
}

void TableBuilder::printCode(ostream* out, int baseIdent, const string& code, int baseLine)
{
    StringList lines = splitLines(code);

    if (!lines.empty() && isWhitespaceString(lines.front())) {
        ++baseLine;
        lines.erase(lines.begin());
    }

    if (!lines.empty() && isWhitespaceString(lines.back()))
        lines.pop_back();

    *out << "#line " << baseLine << " \"codes.def\"\n";

    // Compute "leading" whitespace.
    unsigned minWhiteSpace = 100000;
    for (unsigned c = 0; c < lines.size(); ++c) {
        const string& line = lines[c];
        if (isWhitespaceString(line))
            continue;

        unsigned ws = 0;
        while (ws < line.length() && WTF::isASCIISpace(line[ws]))
            ++ws;
        if (ws < minWhiteSpace)
            minWhiteSpace = ws;
    }

    // Print out w/it stripped..
    for (unsigned c = 0; c < lines.size(); ++c) {
        const string& line = lines[c];
        if (line.length() < minWhiteSpace)
            *out << "\n";
        else {
            for (int c = 0; c < baseIdent; ++c)
                *out << ' ';
            *out << line.substr(minWhiteSpace) << "\n";
        }
    }
}

void TableBuilder::generateVariantImpl(const OperationVariant& variant)
{
    mInd(16) << "pc += " << variant.size << ";\n";
    mInd(16) << "const unsigned char* localPC = pc;\n";
    int numParams = variant.paramIsIm.size();
    for (int p = 0; p < numParams; ++p) {
        const Type& type  = variant.op.parameters[p];
        bool        inReg = !variant.paramIsIm[p];
        int negPos = variant.paramOffsets[p] - variant.size;

        bool wideArg = !inReg && type.align8;

        char negPosStr[64];
        std::sprintf(negPosStr, "%d", negPos);

        string accessString = "reinterpret_cast<const ";
        accessString += wideArg ? "WideArg" : "NarrowArg";
        accessString += "*>(localPC ";
        accessString += negPosStr;
        accessString += ")->";
        accessString += inReg ? string("regVal") : type.name + "Val";
        if (inReg) { // Need to indirect... The register value is actually an offset, so a bit of casting, too.
            accessString = "reinterpret_cast<const LocalStorageEntry*>(reinterpret_cast<const unsigned char*>(localStore) + " + accessString + ")->val." + type.name + "Val";
        }

        mInd(16) << variant.op.implParams[p].nativeName << " " << variant.op.implParamNames[p]
                     << " = ";
        if (type == variant.op.implParams[p]) {
            // We don't need a conversion, just fetch it directly into name..
            *mStream << accessString << ";\n";
        } else {
            ConversionInfo conv;
            conv = imConversions[type.name][variant.op.implParams[p].name];
            *mStream << "convert" << conv.name << "(exec, " << accessString << ");\n";

            if (conv.flags & Conv_MayThrow) {
                // Check for an exception being raised, or perhaps a reload request
                mInd(16) << "if (pc != localPC) // {// Exception or reload\n";
                //mInd(20) << "if (exec->h
                mInd(20) << "continue;\n";
            }
        }
    }

    // Replace $$ with destination register
    string storeCode = "localStore[fbDestReg].val." + variant.op.retType + "Val";
    string code = variant.op.implementAs;
    code = strReplace(code, "$$", storeCode);

    // Print out the impl code..
    printCode(mStream, 16, code, variant.op.codeLine);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
