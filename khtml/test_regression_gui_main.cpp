/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 *
 * Icons: Copyright (C) 2006 The Aquaosk-2.0 Team
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

int main(int argc, char *argv[])
{
    KCmdLineOptions options;

	KCmdLineArgs::init(argc, argv, "testregressiongui", 0, ki18n("TestRegressionGui"),
								   "1.0", ki18n("GUI for the khtml regression tester"));

	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	TestRegressionWindow mainWindow;
	mainWindow.show();

	return app.exec();
}

// vim:ts=4:tw=4:noet
