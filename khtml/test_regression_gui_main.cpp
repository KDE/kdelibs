/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <kapplication.h>
#include <kcmdlineargs.h>

#include "test_regression_gui_window.h"

static KCmdLineOptions options[] =
{
	KCmdLineLastOption
};

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc, argv, "testregressiongui", "TestRegressionGui",
								   "GUI for the khtml regression tester", "1.0");

	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	TestRegressionWindow mainWindow;
	mainWindow.show();

	return app.exec();
}

/*
TODO
----
* Fix progress bars in "do not run html/js tests" mode
* Offer option to save log to disk
* Offer Pause/Continue buttons to control regression testing flow
* Offer Right-Mouse-Button "Add to ignore"/"Remove from ignore" in context menu

MAYBE-TODO
----------
* Offer an integrated "index.html" viewer, after the regression testing is done
* Offer a way to change the location where the regression testing output is stored
*/

// vim:ts=4:tw=4:noet
