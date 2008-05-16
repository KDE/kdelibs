/*
    Tests for Kopete::Message::parseEmoticons

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>
    Copyright (c) 2006      by Michael Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KEMOTICON_TEST_H
#define KEMOTICON_TEST_H

#include <QObject>

/**
 * QTestLib automatic test to test Emoticon parsing.
 */
class KEmoticonTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEmoticonParser();
};

#endif


