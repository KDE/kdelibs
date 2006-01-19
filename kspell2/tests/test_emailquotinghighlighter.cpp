/**
 * test_highlighter.cpp
 *
 * Copyright (C)  2006  Laurent Montel <montel@kde.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "highlighter.h"
#include "dictionary.h"
#include "filter.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <QTextEdit>

int main( int argc, char** argv )
{
    KCmdLineArgs::init( argc, argv, "KSpell2Test", 0, 0, 0, 0);

    KApplication app;

    QTextEdit *test = new QTextEdit();
    KSpell2::Highlighter *hl = new KSpell2::KEMailQuotingSyntaxHighlighter( test ,Qt::red, Qt::blue, Qt::yellow, Qt::green);
    Q_UNUSED( hl );
    app.setMainWidget( test );
    test->show();

    return app.exec();
}
