/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <ktextedit.h>
#include <ksyntaxhighlighter.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "KSyntaxHighlighterTest", "KSyntaxHighlighterTest", "KSyntaxHighlighter test app", "1.0" );
    KApplication app;
    KTextEdit *edit = new KTextEdit();

	KSyntaxHighlighter *syntax = new KSyntaxHighlighter(edit, true/*color quoting*/, Qt::blue, Qt::red, Qt::yellow, Qt::green); 
    edit->resize( 600, 600 );
    edit->show();
    return app.exec();
}

