/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _RESOURCE_DATA_TEST_H_
#define _RESOURCE_DATA_TEST_H_

#include "testbase.h"

#include <kmetadata/resourcedata.h>

class ResourceDataTest : public TestBase
{
    Q_OBJECT

 private Q_SLOTS:
    void testResourceData();
    void testPropertyTypes();
    void testTypeHandling();
};

#endif
