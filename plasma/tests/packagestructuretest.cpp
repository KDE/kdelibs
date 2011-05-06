/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                            *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "packagestructuretest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "plasma/package.h"
#include "plasma/packagestructure.h"
#include "plasma/applet.h"

class NoPrefixes : public Plasma::PackageStructure
{
public:
    explicit NoPrefixes()
        : Plasma::PackageStructure(0, "StructureLess")
    {
        setContentsPrefixPaths(QStringList());
        addDirectoryDefinition("bin", "bin", "bin");
    }
};


void PackageStructureTest::init()
{
    ps = Plasma::Applet::packageStructure();
}

void PackageStructureTest::emptyContentsPrefix()
{
    Plasma::PackageStructure::Ptr structure(new NoPrefixes);
    Plasma::Package package("/", structure);
    QString path(package.filePath("bin", "ls"));
    qDebug() << path;
    QCOMPARE(path, QString("/bin/ls"));
}

void PackageStructureTest::type()
{
    QCOMPARE(ps->type(), QString("Plasmoid"));
}

void PackageStructureTest::directories()
{
    QList<const char*> dirs;
    dirs << "animations" << "config" << "data" << "images" << "scripts" << "translations" << "ui";

    QList<const char*> psDirs = ps->directories();

    QCOMPARE(dirs.count(), psDirs.count());

    for (int i = 0; i < psDirs.count(); ++i) {
        QVERIFY(qstrcmp(dirs[i], psDirs[i]) == 0);
    }
}

void PackageStructureTest::requiredDirectories()
{
    QList<const char*> dirs;
    QCOMPARE(ps->requiredDirectories(), dirs);
}

void PackageStructureTest::files()
{
    QList<const char*> files;
    files << "defaultconfig" << "mainconfigui" << "mainconfigxml" << "mainscript";

    QList<const char*> psFiles = ps->files();

    //for (int i = 0; i < psFiles.count(); ++i) {
    //    qDebug() << psFiles[i];
    //}
    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::requiredFiles()
{
    QList<const char*> files;
    files << "mainscript";

    QList<const char*> psFiles = ps->requiredFiles();

    QCOMPARE(files.count(), psFiles.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(files[i], psFiles[i]);
    }
}

void PackageStructureTest::path()
{
    QCOMPARE(ps->path("images"), QString("images"));
    QCOMPARE(ps->path("mainscript"), QString("code/main"));
}

void PackageStructureTest::name()
{
    QCOMPARE(ps->name("config"), i18n("Configuration Definitions"));
    QCOMPARE(ps->name("mainscript"), i18n("Main Script File"));
}

void PackageStructureTest::required()
{
    QVERIFY(ps->isRequired("mainscript"));
}

void PackageStructureTest::mimeTypes()
{
    QStringList mimeTypes;
    mimeTypes << "image/svg+xml" << "image/png" << "image/jpeg";
    QCOMPARE(ps->mimeTypes("images"), mimeTypes);
}

void PackageStructureTest::read()
{
    QString structurePath = QString(KDESRCDIR) + "/plasmoidpackagerc";
    KConfig config(structurePath, KConfig::SimpleConfig);
    Plasma::PackageStructure structure;
    structure.read(&config);

    // check some names
    QCOMPARE(structure.name("config"), i18n("Configuration Definitions"));
    QCOMPARE(structure.name("mainscript"), i18n("Main Script File"));

    // check some paths
    QCOMPARE(structure.path("images"), QString("images"));
    QCOMPARE(structure.path("mainscript"), QString("code/main"));

    // compare files
    QList<const char *> files;
    files << "mainconfiggui" << "mainconfigxml" << "mainscript";

    QList<const char *> psFiles = structure.files();

    QCOMPARE(psFiles.count(), files.count());
    for (int i = 0; i < files.count(); ++i) {
        QCOMPARE(psFiles[i], files[i]);
    }

    // compare required files
    QList<const char *> reqFiles = structure.requiredFiles();
    QCOMPARE(reqFiles.count(), 1);
    QCOMPARE(reqFiles[0], "mainscript");

    // compare directories
    QList <const char *> dirs;
    dirs << "config" << "configui" << "images" << "scripts";
    QList <const char *> psDirs = structure.directories();
    QCOMPARE(psDirs.count(), dirs.count());
    for (int i = 0; i < dirs.count(); i++) {
        QCOMPARE(psDirs[i], dirs[i]);
    }
    QCOMPARE(structure.requiredDirectories().size(), 0);
}

void PackageStructureTest::write()
{
    QString file1 = QDir::homePath() + "/.kde-unit-test/packagerc";
    QString file2 = QString(KDESRCDIR) + "/plasmoidpackagerc";

    KConfig config(file1, KConfig::SimpleConfig);
    ps->write(&config);

    // check type
    QCOMPARE(config.group("").readEntry("Type", QString()), QString("Plasmoid"));

    // check groups
    QStringList groups;
    groups << "images" << "config" << "data" << "defaultconfig" << "scripts"
           << "mainconfigui" << "mainconfigxml" << "mainscript"
           << "translations" << "ui" << "animations";
    groups.sort();

    QStringList actualGroups = config.groupList();
    actualGroups.sort();
    //kDebug() << actualGroups;
    QCOMPARE(actualGroups, groups);

    // check scripts
    KConfigGroup scripts = config.group("scripts");
    QCOMPARE(scripts.readEntry("Path", QString()), QString("code"));
    QCOMPARE(scripts.readEntry("Name", QString()), QString("Executable Scripts"));
    QCOMPARE(scripts.readEntry("Mimetypes", QStringList()), QStringList() << "text/plain");
    QCOMPARE(scripts.readEntry("Directory", false), true);
    QCOMPARE(scripts.readEntry("Required", false), false);
}

QTEST_KDEMAIN(PackageStructureTest, NoGUI)

//#include "packagestructuretest.moc"

