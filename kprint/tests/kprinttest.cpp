/* This file is part of the KDE libraries
    Copyright (C) 2000 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdio.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include "kprintdialog.h"

#include <qprinter.h>

static const char *description = I18N_NOOP("Test program for KPrintDialog");

static const char *version = "1.0";

int main( int argc, char *argv[] )
{
   KCmdLineArgs::init(argc, argv, "kprinttest", description, version);

   KApplication a;

   KPrintDialog *dlg = new KPrintDialog(0, true);

   int result = dlg->exec();
   fprintf(stderr, "Result = %d\n", result);

   QPrinter bla;
   bla.setup();

   return 0;
}
