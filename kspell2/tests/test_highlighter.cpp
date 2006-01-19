/**
 * test_highlighter.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "highlighter.h"
#include "test_highlighter.h"
#include "test_highlighter.moc"
#include "dictionary.h"
#include "filter.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <QTextEdit>
#include <QAction>
#include <QMenu>
TestSpell::TestSpell()
	: QTextEdit()
{
    hl = new KSpell2::Highlighter( this );
}

void TestSpell::contextMenuEvent(QContextMenuEvent *e)
{
	kdDebug()<<"TestSpell::contextMenuEvent\n";
	QMenu *popup = createStandardContextMenu();
    QMenu *subMenu = new QMenu( popup );
	connect( subMenu, SIGNAL( triggered ( QAction* ) ),
    	this, SLOT( slotActivate( ) ) );
	QAction *action = new QAction( "active or not", popup );
	popup->insertSeparator();
	popup->insertItem( "Text Completion",subMenu );
	subMenu->addAction(action);
	popup->exec(e->globalPos());
	delete popup;
			
}

void TestSpell::slotActivate()
{
	kdDebug()<<"Activate or not highlight :"<<endl;
	hl->setActive(!hl->isActive());
}

int main( int argc, char** argv )
{
    KCmdLineArgs::init( argc, argv, "KSpell2Test", 0, 0, 0, 0);

    KApplication app;

    QTextEdit *test = new TestSpell();
    app.setMainWidget( test );
    test->show();

    return app.exec();
}
