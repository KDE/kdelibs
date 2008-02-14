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
    printConversionInfo(imConversions, false);
    printConversionInfo(rgConversions, true);
    *cppStream << "};\n\n";

    int numBits = neededBits(types.size());
    *cppStream << "static inline const ConvInfo* getConversionInfo(bool immediate, OpType from, OpType to) {\n";
    *cppStream << "    return &conversions[((int)immediate << " << (2 * numBits) << ")"
               << " | ((int)from << " << numBits << ") | (int)to];\n";
    *cppStream << "};\n\n";

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
            dumpOpStructForVariant(variant, true, true);
        dumpOpStructForVariant(variant, false, c != variants.size() - 1);
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

    *cppStream << "const OpInstanceList* const opSpecializations[] = {\n";
    for (int o = 0; o < operationNames.size(); ++o) {
        *cppStream << "    op" << operationNames[o] << "Variants";
        if (o != (operationNames.size() - 1))
            *cppStream << ",";
        *cppStream << "\n";
    }
    *cppStream << "};\n\n";
}

void TableBuilder::issueError(const QString& err)
{
    std::cerr << err.toLocal8Bit().data() << "\n";
    exit(-1);
}

void TableBuilder::printConversionInfo(const QHash<QString, QHash<QString, ConversionInfo> >& table, bool last)
{
    int numBits = neededBits(types.size());
    int fullRange = 1 << numBits;
    for (int from = 0; from < fullRange; ++from) {
        for (int to = 0; to < fullRange; ++to) {
            if (from < types.size() && to < types.size()) {
                QString fromName = typeNames[from];
                QString toName   = typeNames[to];
                if (table[fromName].contains(toName)) {
                    const ConversionInfo& inf = table[typeNames[from]][typeNames[to]];
                    *cppStream << "    {Conv_" << inf.name << ",";
                    if (inf.checked)
                        *cppStream << "Cost_Checked}";
                    else
                        *cppStream << inf.cost << "}";
                } else if (from == to) {
                    *cppStream << "    {Conv_NoOp, 0}";
                } else {
                    *cppStream << "    {Conv_NoConversion, Cost_NoConversion}";
                }

                *cppStream << " /*" << fromName << " => " << toName << "*/";
            } else {
                *cppStream << "    {Conv_NoConversion, Cost_NoConversion}";
            }

            if (!last || from != (fullRange - 1) || to != (fullRange - 1))
                *cppStream << ",";

            *cppStream << "\n";
        } // for to..
    } // for from..
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

void TableBuilder::handleConversion(const QString& name, bool immediate, bool checked,
                                    const QString& from, const QString& to, int cost)
{
    conversionNames << name;
    ConversionInfo inf;
    inf.name    = name;
    inf.cost    = cost;
    inf.checked = checked;

    if (immediate)
        imConversions[from][to] = inf;
    else
        rgConversions[from][to] = inf;
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

void TableBuilder::handleImpl(const QString& fnName, const QString& code,
                              QStringList sig, QStringList paramNames)
{
    Operation op;
    op.name           = operationNames.last();
    op.implementAs    = code;
    op.parameters     = resolveSignature(sig);
    op.implParams     = op.parameters;
    op.implParamNames = paramNames;
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
    operations << op;
}

void TableBuilder::expandOperationVariants(const Operation& op, QList<bool>& paramIsIm)
{
    int pos = paramIsIm.size();
    if (pos < op.parameters.size()) {
        if (op.parameters[pos].im) {
            paramIsIm.append(true);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.removeLast();
        }

        if (op.parameters[pos].reg) {
            paramIsIm.append(false);
            expandOperationVariants(op, paramIsIm);
            paramIsIm.removeLast();
        }
        return;
    }

    // Have a full variant... Build a signature.
    QString sig = op.name;
    for (int p = 0; p < paramIsIm.size(); ++p) {
        sig += "_";
        sig += paramIsIm[p] ? "I" : "R";
        sig += op.parameters[p].name;
    }

    // We may need padding if we have an immediate align8 param..
    bool needsPad = false;
    for (int c = 0; c < paramIsIm.size(); ++c)
        needsPad |= (paramIsIm[c] & op.parameters[c].align8);

    OperationVariant var;
    var.sig = sig;
    var.op  = op;
    var.paramIsIm = paramIsIm;
    var.needsPadVariant = needsPad;

    // Build shuffle table --- tells which parameter gets
    // passed in which position of the instruction...
    // pad8/align ones go first.
    for (int c = 0; c < paramIsIm.size(); ++c)
        if (paramIsIm[c] & op.parameters[c].align8)
            var.shuffleTable << c;

    // Then the rest..
    for (int c = 0; c < paramIsIm.size(); ++c)
        if (!paramIsIm[c] || !op.parameters[c].align8)
            var.shuffleTable << c;

    variants << var;
    if (needsPad) { // we put the pad before, due to the fallthrough idiom..
        QString pSig = sig + "_Pad";
        variantNames << pSig;
        variantNamesForOp[op.name] << pSig;
    }
    variantNames << sig;
    variantNamesForOp[op.name] << sig;
}

void TableBuilder::dumpOpStructForVariant(const OperationVariant& variant, bool doPad, bool needsComma)
{
    *cppStream << "    {";
    *cppStream << "Op_" << variant.op.name << ", ";     // baseInstr..
    *cppStream << "OpByteCode_" << (doPad ? variant.sig + "_Pad" : variant.sig) << ", "; // byteCode op
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

    // Shuffle table..
    *cppStream << "{";
    for (int p = 0; p < numParams; ++p) {
        *cppStream << variant.shuffleTable[p];
        if (p != numParams - 1)
            *cppStream << ", ";
    }

    *cppStream << "}, ";

    // Whether this is a padded version..
    *cppStream << (doPad ? "true" : "false");

    if (needsComma)
        *cppStream << "},\n";
    else
        *cppStream << "}\n";
}


// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
