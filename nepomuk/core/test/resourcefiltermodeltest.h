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

#ifndef _RESOURCE_FILTER_MODEL_TEST_H_
#define _RESOURCE_FILTER_MODEL_TEST_H_

#include <QtCore/QObject>

class ResourceFilterModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEnsureResource();
    void testUpdateProperty1();
    void testUpdateProperty2();
    void testRemoveStatement();
    void testRemoveAllStatements();
    void testInstanceQuery();
};

#endif
