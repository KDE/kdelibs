/**
 * test_dialog.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */
#ifndef TEST_DIALOG_H
#define TEST_DIALOG_H

#include "kspell_dialog.h"
#include "kspell_broker.h"

#include <qobject.h>

class TestDialog : public QObject
{
    Q_OBJECT
public:
    TestDialog();

public slots:
    void check( const QString& buffer );
    void doneChecking( const QString& );
private:
    KSpell2::Broker *m_broker;
};

#endif
