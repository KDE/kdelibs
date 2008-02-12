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

TableBuilder::TableBuilder(QTextStream* inStream, QTextStream* hStream, QTextStream* cppStream):
    Parser(inStream), hStream(hStream), cppStream(cppStream)
{
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
    types << t;
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
    Operation op;
    op.name = name;
    operations << op;
}

void TableBuilder::handleImpl(const QString& fnName, QStringList sig)
{}

void TableBuilder::handleTile(const QString& fnName, QStringList sig)
{}



// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
