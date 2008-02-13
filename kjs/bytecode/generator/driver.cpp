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

#include <QFile>
#include <QTextStream>
#include <iostream>

QTextStream* makeStream(const QString& fileName, QIODevice::OpenModeFlag flags)
{
    QFile* f = new QFile(fileName);
    if (!f->open(flags)) {
        std::cerr << "Unable to open:" << fileName.toLocal8Bit().data() << "\n";
        return 0;
    }

    return new QTextStream(f);
}

// Reads and copies out stuff until the @generate line
static void copyUntilGen(QTextStream* in, QTextStream* out)
{
    while (!in->atEnd()) {
        QString line = in->readLine();
        if (line.endsWith("@generate"))
            return;
        else
            *out << line << "\n";
    }
}

// Copies out stuff after the @generate, to end
static void copyUntilEnd(QTextStream* in, QTextStream* out)
{
    while (!in->atEnd()) {
        QString line = in->readLine();
        *out << line << "\n";
    }
}

int main(int argc, char* argv[])
{
    QTextStream* def   = makeStream("codes.def",      QIODevice::ReadOnly);
    QTextStream* hIn   = makeStream("opcodes.h.in",   QIODevice::ReadOnly);
    QTextStream* cppIn = makeStream("opcodes.cpp.in", QIODevice::ReadOnly);
    QTextStream* mchIn = makeStream("machine.cpp.in", QIODevice::ReadOnly);

    QTextStream* hOut   = makeStream("opcodes.h",  QIODevice::WriteOnly);
    QTextStream* cppOut = makeStream("opcodes.cpp", QIODevice::WriteOnly);
    QTextStream* mchOut = makeStream("machine.cpp", QIODevice::WriteOnly);

    if (!def || !hIn || !cppIn || !mchIn || !hOut || !cppOut || !mchOut)
        return -1;

    copyUntilGen(hIn, hOut);
    copyUntilGen(cppIn, cppOut);
    copyUntilGen(mchIn, mchOut);

    TableBuilder build(def, hOut, cppOut, mchOut);
    build.generateCode();

    copyUntilEnd(hIn, hOut);
    copyUntilEnd(cppIn, cppOut);
    copyUntilEnd(mchIn, cppOut);

    delete hOut;
    delete cppOut;

    return 0;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
