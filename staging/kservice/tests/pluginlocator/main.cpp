/*
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

#include "plugintest.h"

int main(int argc, char **argv)
{
    QCommandLineParser *parser = new QCommandLineParser;
    PluginTest app(argc, argv, parser);

    const QString description = "PluginLocator test app";
    const char version[] = "1.0";

    app.setApplicationVersion(version);
    parser->addVersionOption();
    parser->setApplicationDescription(description);

    parser->addOption(QCommandLineOption(QStringList() << "s" << "show",
                          QStringLiteral("Show plugins"),
                          QStringLiteral("name")));

    return app.exec();
}

