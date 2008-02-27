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

#include <QString>
#include <QList>
#include <QHash>

struct Type
{
    QString name;
    QString nativeName;
    bool im, reg, align8;

    bool operator==(const Type& other) const {
        return name == other.name;
    }
};

struct ConversionInfo
{
    QString name;
    QString impl;
    int  cost; // for w/in tile for immediate, for external for reg
    bool checked;
    bool mayThrow;
    Type from;
    Type to;
    int codeLine;

    ConversionInfo(): cost(0), checked(false), mayThrow(false)
    {} //Can be called for trivial conversion
};

// Actually, a specialization, but who cares?
struct Operation
{
    QString name;
    QString retType;
    QList<Type>  parameters;
    int          cost;
    int          codeLine;
    bool         overload;

    QString implementAs;
    QList<Type> implParams;
    QStringList implParamNames;
};

struct OperationVariant
{
    QString   sig;
    Operation op;
    QList<bool> paramIsIm;
    QList<int>  paramOffsets;
    int         size;
    bool        needsPadVariant;
};

class TableBuilder: public Parser
{
public:
    TableBuilder(QTextStream* inStream, QTextStream* hStream, QTextStream* cppStream,
                 QTextStream* mStream);

    void generateCode();
private:
    void issueError(const QString& err);

    virtual void handleType(const QString& type, const QString& nativeName, bool im, bool rg, bool al8);
    virtual void handleConversion(const QString& name, const QString& runtimeRoutine, int codeLine,
                                  bool immediate, bool checked, bool mayThrow,
                                  const QString& from, const QString& to, int cost);

    virtual void handleOperation(const QString& name);
    virtual void handleImpl(const QString& fnName, const QString& code, bool overload,
                            int codeLine, int cost, const QString& retType, QStringList sig,
                            QStringList paramNames);
    virtual void handleTile(const QString& fnName, QStringList sig);

    void printConversionInfo(const QHash<QString, QHash<QString, ConversionInfo> >& table, bool last);

    void printConversionRoutine(const ConversionInfo& conversion);

    void printCode(QTextStream* out, int baseIdent, const QString& code, int baseLine);

    // Enumerates all r/i/pad variants; plus computes the shuffle table.
    void expandOperationVariants(const Operation& op, QList<bool>& paramIsIm);

    void dumpOpStructForVariant(const OperationVariant& variant, bool doPad,
                                bool hasPadVariant, bool needsComma);

    void generateVariantImpl(const OperationVariant& variant);

    // issues error if there is a problem..
    QList<Type> resolveSignature(const QStringList& in);

    QTextStream* hStream;
    QTextStream* cppStream;
    QTextStream* mStream;

    QTextStream& mInd(int ind);

    QHash<QString, Type> types;
    QStringList          typeNames;

    QStringList        operationNames;
    QHash<QString, QString> operationRetTypes; // uglily enough specified on the impl. I suck.
    QList<Operation>   operations;
    QHash<QString, Operation> implementations;

    QStringList  variantNames;
    QList<OperationVariant> variants;
    QHash<QString, QStringList> variantNamesForOp;

    QStringList conversionNames;
    QList<ConversionInfo> imConversionList;
    QList<ConversionInfo> rgConversionList;

    QHash<QString, QHash<QString, ConversionInfo> > imConversions;
    QHash<QString, QHash<QString, ConversionInfo> > rgConversions;
};

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
