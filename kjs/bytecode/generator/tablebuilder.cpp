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
#include <QTextStream>
#include <QDebug>
#include <stdlib.h>
#include <iostream>

class Enum {
public:
    Enum(const QString& name, const QString& prefix, QStringList values):
        name(name), prefix(prefix), values(values)
    {}

    void printDeclaration(QTextStream* hStream)
    {
        *hStream << "enum " << name << " {\n";
        for (int p = 0; p < values.size(); ++p) {
            *hStream << "    " << prefix << values[p];
            if (p != (values.size() - 1))
                *hStream << ",";
            *hStream << "\n";
        }
        *hStream << "};\n";
        *hStream << "extern const char* const " << name << "Vals[];\n\n";
    }

    void printDefinition(QTextStream* hStream)
    {
        *hStream << "const char* const " << name << "Vals[] = {\n";
        for (int p = 0; p < values.size(); ++p) {
            *hStream << "    \"" << prefix << values[p] << "\"";
            if (p != (values.size() - 1))
                *hStream << ",";
            *hStream << "\n";
        }
        *hStream << "};\n\n";
    }
private:
    QString name;
    QString prefix;
    QStringList values;
};

TableBuilder::TableBuilder(QTextStream* inStream, QTextStream* hStream,
                           QTextStream* cppStream, QTextStream* mStream):
    Parser(inStream), hStream(hStream), cppStream(cppStream), mStream(mStream)
{
    // Builtin stuff...
    conversionNames << "NoConversion" << "NoOp";

    // Special ones for stuff that might not fit into immediate..
    conversionNames << "I_R_Uint32_Value" << "I_R_Number_Value";
}

// # of bits store 'vals' values, e.g. 3 for 8, etc.
static int neededBits(int vals)
{
    int bits = 1;
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
    for (int t = 0; t < typeNames.size(); ++t) {
        const Type& type = types[typeNames[t]];
        *cppStream << (type.align8 ? "true": "false");
        if (t != typeNames.size() - 1)
            *cppStream << ",";
        *cppStream << " //" << type.name << "\n";
    }

    *cppStream << "};\n\n";

    // Conversion ops. Those go entirely in the .cpp
    Enum convOps("ConvOp", "Conv_", conversionNames);
    convOps.printDeclaration(cppStream);
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
    foreach (const ConversionInfo& inf, imConversionList)
        printConversionRoutine(inf);

    *cppStream << "static void emitImmediateConversion(ConvOp convType, OpValue* original, OpValue& out)\n{\n";
    *cppStream << "    out.immediate = true;\n";
    *cppStream << "    switch(convType) {\n";
    *cppStream << "    case Conv_NoOp:\n";
    *cppStream << "        out = *original;\n";
    *cppStream << "        break;\n";
    foreach (const ConversionInfo& inf, imConversionList) {
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
    *cppStream << "        printf(\"Unable to handle in-place conversion:%s\\n\", ConvOpVals[convType]);\n";
    *cppStream << "        CRASH();\n";
    *cppStream << "    }\n";
    *cppStream << "}\n\n";

    // Operations
    Enum opNamesEnum("OpName", "Op_", operationNames);
    opNamesEnum.printDeclaration(hStream);
    opNamesEnum.printDefinition (cppStream);

    // Enumerate all the variants..
    foreach (const Operation& op, operations) {
        QList<bool> parIm;
        expandOperationVariants(op, parIm);
    }

    // Now we have all our bytecode names... Whee.
    Enum opByteCodesEnum("OpByteCode", "OpByteCode_", variantNames);
    opByteCodesEnum.printDeclaration(hStream);
    opByteCodesEnum.printDefinition (cppStream);

    // We can now emit the actual tables...

    // ... first descriptors for each bytecode op..
    *cppStream << "const Op opsForOpCodes[] = {\n";
    for (int c = 0; c < variants.size(); ++c) {
        const OperationVariant& variant = variants[c];
        if (variant.needsPadVariant)
            dumpOpStructForVariant(variant, true, variant.needsPadVariant, true);
        dumpOpStructForVariant(variant, false, variant.needsPadVariant, c != variants.size() - 1);
    }
    *cppStream << "};\n\n";

    // then variant tables for each main op..
    foreach (const QString& opName, operationNames) {
        *cppStream << "static const Op* const op" << opName << "Variants[] = {";
        QStringList variants = variantNamesForOp[opName];
        for (int v = 0; v < variants.size(); ++v) {
            *cppStream << "&opsForOpCodes[OpByteCode_" << variants[v] << "], ";
        }
        *cppStream << "0};\n";
    }
    *cppStream << "\n";

    *cppStream << "const Op* const* const opSpecializations[] = {\n";
    for (int o = 0; o < operationNames.size(); ++o) {
        *cppStream << "    op" << operationNames[o] << "Variants";
        if (o != (operationNames.size() - 1))
            *cppStream << ",";
        *cppStream << "\n";
    }
    *cppStream << "};\n\n";

    // Now, generate the VM loop.
    mInd(8) << "OpByteCode op = *reinterpret_cast<OpByteCode*>(block + pc);\n";
    mInd(8) << "switch (op) {\n";
    foreach (const OperationVariant& var, variants) {
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

void TableBuilder::issueError(const QString& err)
{
    std::cerr << err.toLocal8Bit().data() << "\n";
    exit(-1);
}

void TableBuilder::printConversionInfo(const QHash<QString, QHash<QString, ConversionInfo> >& table, bool reg)
{
    int numBits = neededBits(types.size());
    int fullRange = 1 << numBits;
    for (int from = 0; from < fullRange; ++from) {
        for (int to = 0; to < fullRange; ++to) {
            if (from < types.size() && to < types.size()) {
                QString fromName = typeNames[from];
                QString toName   = typeNames[to];

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
                } else if (table[fromName].contains(toName) && representable) {
                    // We skip immediate conversions for things that can't be immediate, as
                    // we don't have exec there..
                    const ConversionInfo& inf = table[fromName][toName];
                    *cppStream << "    {Conv_" << inf.name << ",";
                    if (inf.checked)
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
    *hStream << "inline " << conversion.to.nativeName << " convert" << conversion.name
         << "(ExecState* exec, " << conversion.from.nativeName << " in)\n";
    *hStream << "{\n";
    *hStream << "    (void)exec;\n";
    printCode(hStream, 4, conversion.impl);
    *hStream << "}\n\n";
}

void TableBuilder::handleType(const QString& type, const QString& nativeName, bool im, bool rg, bool al8)
{
    typeNames << type;
    Type t;
    t.name = type;
    t.nativeName = nativeName;
    t.im     = im;
    t.reg    = rg;
    t.align8 = al8;
    types[type] = t;
}

void TableBuilder::handleConversion(const QString& name, const QString& code,
                                    bool immediate, bool checked, bool mayThrow,
                                    const QString& from, const QString& to, int cost)
{
    conversionNames << name;
    ConversionInfo inf;
    inf.name    = name;
    inf.cost    = cost;
    inf.checked = checked;
    inf.mayThrow = mayThrow;
    inf.impl     = code;
    inf.from     = types[from];
    inf.to       = types[to];

    if (immediate) {
        imConversions[from][to] = inf;
        imConversionList << inf;
    } else {
        rgConversions[from][to] = inf;
    }
}

void TableBuilder::handleOperation(const QString& name)
{
    operationNames << name;
}

QList<Type> TableBuilder::resolveSignature(const QStringList& in)
{
    QList<Type> sig;
    foreach (const QString& type, in) {
        if (types.contains(type))
            sig.append(types[type]);
        else
            issueError("Unknown type:" + type);
    }
    return sig;
}

void TableBuilder::handleImpl(const QString& fnName, const QString& code, int cost,
                              QStringList sig, QStringList paramNames)
{
    Operation op;
    op.name           = operationNames.last();
    op.implementAs    = code;
    op.parameters     = resolveSignature(sig);
    op.implParams     = op.parameters;
    op.implParamNames = paramNames;
    op.cost           = cost;
    operations << op;
    if (!fnName.isEmpty())
        implementations[fnName] = op;
}

void TableBuilder::handleTile(const QString& fnName, QStringList sig)
{
    if (!implementations.contains(fnName))
        issueError("Unknown implementation name " + fnName + " in a tile definition");
    const Operation& impl = implementations[fnName];

    Operation op;
    op.name        = operationNames.last();
    op.implementAs = impl.implementAs;
    op.parameters  = resolveSignature(sig);
    op.implParams     = impl.implParams;
    op.implParamNames = impl.implParamNames;
    op.cost           = impl.cost;
    // Now also include the cost of inline conversions.
    for (int p = 0; p < op.parameters.size(); ++p)
        op.cost += imConversions[op.parameters[p].name][op.implParams[p].name].cost;

    operations << op;
}

void TableBuilder::expandOperationVariants(const Operation& op, QList<bool>& paramIsIm)
{
    int numParams = op.parameters.size();
    if (paramIsIm.size() < numParams) {
        if (op.parameters[paramIsIm.size()].im) {
            paramIsIm.append(true);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.removeLast();
        }

        if (op.parameters[paramIsIm.size()].reg) {
            paramIsIm.append(false);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.removeLast();
        }
        return;
    }

    // Have a full variant... Build a signature.

    QString sig = op.name;
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
        var.paramOffsets.append(0);

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

    variants << var;
    if (needsPad) { // we put the pad before, due to the fallthrough idiom..
        QString pSig = sig + "_Pad";
        variantNames << pSig;
        variantNamesForOp[op.name] << pSig;
    }
    variantNames << sig;
    variantNamesForOp[op.name] << sig;
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
    *cppStream << (hasPadVariant ? "true" : "false");

    if (needsComma)
        *cppStream << "},\n";
    else
        *cppStream << "}\n";
}

QTextStream& TableBuilder::mInd(int ind)
{
    for (int i = 0; i < ind; ++i)
        *mStream << ' ';
    return *mStream;
}

void TableBuilder::printCode(QTextStream* out, int baseIdent, const QString& code)
{
    QStringList lines = code.split("\n");

    if (!lines.isEmpty() && lines.first().trimmed().isEmpty())
        lines.removeFirst();
    if (!lines.isEmpty() && lines.last().trimmed().isEmpty())
        lines.removeLast();

    // Compute "leading" whitespace.
    int minWhiteSpace = 100000;
    foreach(const QString& line, lines) {
        if (line.trimmed().isEmpty())
            continue;

        int ws = 0;
        while (ws < line.length() && line[ws].isSpace())
            ++ws;
        if (ws < minWhiteSpace)
            minWhiteSpace = ws;
    }

    // Print out w/it stripped..
    foreach(const QString& line, lines) {
        if (line.length() < minWhiteSpace)
            *out << "\n";
        else {
            for (int c = 0; c < baseIdent; ++c)
                *out << ' ';
            *out << line.mid(minWhiteSpace) << "\n";
        }
    }
}

void TableBuilder::generateVariantImpl(const OperationVariant& variant)
{
    mInd(16) << "pc += " << variant.size << ";\n";
    mInd(16) << "Addr localPC = pc;\n";
    int numParams = variant.paramIsIm.size();
    for (int p = 0; p < numParams; ++p) {
        const Type& type  = variant.op.parameters[p];
        bool        inReg = !variant.paramIsIm[p];
        int negPos = variant.paramOffsets[p] - variant.size;

        bool wideArg = !inReg && type.align8;

        QString accessString = QString::fromLatin1("reinterpret_cast<const %1*>(&block[localPC + %2])->%3").
            arg(wideArg ? "WideArg" : "NarrowArg", QString::number(negPos),
                inReg ? QString::fromLatin1("regVal") : (type.name + "Val"));

        if (inReg) // Need to indirect..
            accessString = "localStore[" + accessString + "]." + type.name + "Val";

        mInd(16) << variant.op.implParams[p].nativeName << " " << variant.op.implParamNames[p]
                     << " = ";
        if (type == variant.op.implParams[p]) {
            // We don't need a conversion, just fetch it directly into name..
            *mStream << accessString << ";\n";
        } else {
            ConversionInfo conv;
            conv = imConversions[type.name][variant.op.implParams[p].name];
            *mStream << "convert" << conv.name << "(exec, " << accessString << ");\n";

            if (conv.mayThrow) {
                // Check for an exception being raised..
                mInd(16) << "if (pc != localPC) // Exception\n";
                mInd(20) << "continue;\n";
            }
        }
    }

    // Print out the impl code..
    printCode(mStream, 16, variant.op.implementAs);
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
