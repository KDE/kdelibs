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

#include <kunittest/tester.h>

class QString;

// change to SlotTester when it works
class KConfigCompiler_Test : public KUnitTest::Tester
{
public:
	void allTests();
public slots:
	void testExpectedOutput();
private:
	void performCompare(const QString &fileName, bool fail=false);
};

#endif

