/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _TEST_BASE_H_
#define _TEST_BASE_H_

#include <QtTest/QtTest>

class KTempDir;
namespace Soprano {
class Model;
}

class TestBase : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    virtual void init();
    virtual void cleanup();

private:
    KTempDir* m_tmpDir;
    Soprano::Model* m_model;
};

#endif
