/*
   This file is part of the KDE libraries
   Copyright (c) 2007 Thiago Macieira <thiago@kde.org>

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

#include <kconfiggroup.h>

#include <QtCore/QMutableStringListIterator>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <kconfiggroup_p.h>

/**
 * Try to read a GUI type from config group @p cg at key @p key.
 * @p input is the default value and also indicates the type to be read.
 * @p output is to be set with the value that has been read.
 *
 * @returns true if something was handled (even if output was set to clear or default)
 *          or false if nothing was handled (e.g., Core type)
 */
static bool readEntryGui(const KConfigGroup *cg, const char *pKey, const QVariant &input,
                         QVariant &output)
{
    const QString errString = QString::fromLatin1("\"%1\" - conversion from \"%3\" to %2 failed")
                              .arg(pKey).arg(QVariant::typeToName(input.type()));
    const QString formatError = QString::fromLatin1(" (wrong format: expected '%1' items, read '%2')");

    // set in case of failure
    output = input;

    switch (input.type()) {
    case QVariant::Color: {
        const QString color = cg->readEntry(pKey, QString());
        if (color.isEmpty()) {
            output = QColor();  // empty color
            return true;
        } else if (color.at(0) == '#') {
            QColor col;
            col.setNamedColor(color);
            output = col;
            return true;
        } else {
            const QStringList list = cg->readEntry(pKey, QStringList());
            const int count = list.count();

            if (count != 3 && count != 4) {
                if (count == 1 && list.first() == QLatin1String("invalid")) {
                    output = QColor(); // return what was stored
                    return true;
                }

                kcbError() << errString.arg(cg->readEntry(pKey))
                           << formatError.arg("3' or '4").arg(count)
                           << endl;
                return true;    // return default
            }

            int temp[4];
            // bounds check components
            for(int i = 0; i < count; i++) {
                bool ok;
                const int j = temp[i] = list.at(i).toInt(&ok);
                if (!ok) { // failed to convert to int
                    kcbError() << errString.arg(cg->readEntry(pKey))
                               << " (integer conversion failed)"
                               << endl;
                    return true; // return default
                }
                if (j < 0 || j > 255) {
                    static const char *const components[6] = {
                        "red", "green", "blue", "alpha"
                    };
                    const QString boundsError = QLatin1String(" (bounds error: %1 component %2)");
                    kcbError() << errString.arg(cg->readEntry(pKey))
                               << boundsError.arg(components[i]).arg(j < 0? "< 0": "> 255")
                               << endl;
                    return true; // return default
                }
            }
            QColor aColor(temp[0], temp[1], temp[2]);
            if (count == 4)
                aColor.setAlpha(temp[3]);

            if (!aColor.isValid())
                kcbError() << errString.arg(cg->readEntry(pKey)) << endl;
            else
                output = aColor;
            return true;
        }
    }

    case QVariant::Font:
    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::Icon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Pen:
        // we may want to handle these in the future

    default:
        break;
    }

    return false;               // not handled
}

/**
 * Try to write a GUI type @p prop to config group @p cg at key @p key.
 *
 * @returns true if something was handled (even if an empty value was written)
 *          or false if nothing was handled (e.g., Core type)
 */
static bool writeEntryGui(KConfigGroup *cg, const char *pKey, const QVariant &prop,
                          KConfigBase::WriteConfigFlags pFlags)
{
    switch (prop.type()) {
    case QVariant::Color: {
        QList<int> list;
        const QColor rColor = prop.value<QColor>();

        if (!rColor.isValid()) {
            cg->writeEntry(pKey, "invalid", pFlags);
            return true;
        }
        list.insert(0, rColor.red());
        list.insert(1, rColor.green());
        list.insert(2, rColor.blue());
        if (rColor.alpha() != 255)
            list.insert(3, rColor.alpha());

        cg->writeEntry( pKey, list, pFlags );
        return true;
    }
    case QVariant::Font:
        cg->writeEntry( pKey, prop.toString(), pFlags );
        return true;

    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::Icon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Pen:
        // we may want to handle one of these in the future
        break;

    default:
        break;
    }

    return false;
}

static int initKConfigGroupGui()
{
    _kde_internal_KConfigGroupGui.readEntryGui = readEntryGui;
    _kde_internal_KConfigGroupGui.writeEntryGui = writeEntryGui;
    return 42;                  // because 42 is nicer than 1 or 0
}

#ifdef Q_CONSTRUCTOR_FUNCTION
Q_CONSTRUCTOR_FUNCTION(initKConfigGroupGui)
#else
static int dummyKConfigGroupGui = initKConfigGroupGui();
#endif
