/*
    Tests for KConfig Compiler

    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KCONFIGCOMPILER_TEST_H
#define KCONFIGCOMPILER_TEST_H

#include <QObject>

class QString;

class KConfigCompiler_Test : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void testExpectedOutput();
private:
	void performCompare(const QString &fileName, bool fail=false);
};

#endif

