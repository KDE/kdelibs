/* This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color collection

#include "kcolorcollection.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <qsavefile.h>
#include <qstandardpaths.h>

//BEGIN KColorCollectionPrivate
class KColorCollectionPrivate
{
public:
    KColorCollectionPrivate(const QString&);
    KColorCollectionPrivate(const KColorCollectionPrivate&);
    ~KColorCollectionPrivate() {}
    struct ColorNode
  {
        ColorNode(const QColor &c, const QString &n)
            : color(c), name(n) {}
        QColor color;
        QString name;
    };
    QList<ColorNode> colorList;

    QString name;
    QString desc;
    KColorCollection::Editable editable;
};

KColorCollectionPrivate::KColorCollectionPrivate(const QString &_name)
    : name(_name)
{
    if (name.isEmpty()) return;

    QString filename = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QLatin1String("colors/") + name);
  if (filename.isEmpty()) return;

  QFile paletteFile(filename);
  if (!paletteFile.exists()) return;
  if (!paletteFile.open(QIODevice::ReadOnly)) return;

  // Read first line
  // Expected "GIMP Palette"
  QString line = QString::fromLocal8Bit(paletteFile.readLine());
  if (line.contains(QLatin1String(" Palette"))) return;

  while( !paletteFile.atEnd() )
  {
     line = QString::fromLocal8Bit(paletteFile.readLine());
     if (line[0] == QLatin1Char('#'))
     {
        // This is a comment line
        line = line.mid(1); // Strip '#'
        line = line.trimmed(); // Strip remaining white space..
        if (!line.isEmpty())
        {
                desc += line + QLatin1Char('\n'); // Add comment to description
        }
     }
     else
     {
        // This is a color line, hopefully
        line = line.trimmed();
        if (line.isEmpty()) continue;
        int r, g, b;
        int pos = 0;
        if (sscanf(line.toLatin1().constData(), "%d %d %d%n", &r, &g, &b, &pos) >= 3)
        {
           r = qBound(0, r, 255);
           g = qBound(0, g, 255);
           b = qBound(0, b, 255);
           QString name = line.mid(pos).trimmed();
                colorList.append(ColorNode(QColor(r, g, b), name));
        }
     }
  }
}

KColorCollectionPrivate::KColorCollectionPrivate(const KColorCollectionPrivate& p)
    : colorList(p.colorList), name(p.name), desc(p.desc), editable(p.editable)
{
}
//END KColorCollectionPrivate

QStringList KColorCollection::installedCollections()
{
  QStringList paletteDirs = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation,
                                                      QLatin1String("colors"),
                                                      QStandardPaths::LocateDirectory);

  QStringList paletteList;
  Q_FOREACH(const QString& dir, paletteDirs) {
      paletteList += QDir(dir).entryList(QDir::NoDotAndDotDot);
  }
  paletteList.removeDuplicates();

  return paletteList;
}

KColorCollection::KColorCollection(const QString &name)
{
    d = new KColorCollectionPrivate(name);
}

KColorCollection::KColorCollection(const KColorCollection &p)
{
    d = new KColorCollectionPrivate(*p.d);
}

KColorCollection::~KColorCollection()
{
  // Need auto-save?
    delete d;
}

bool
KColorCollection::save()
{
   QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/colors/")
                                                                                       + d->name;
   QSaveFile sf(filename);
   if (!sf.open(QIODevice::WriteOnly)) return false;

   QTextStream str ( &sf );

   QString description = d->desc.trimmed();
   description = QLatin1Char('#') + description.split(QLatin1Char('\n'), QString::KeepEmptyParts).join(QLatin1String("\n#"));

   str << QLatin1String("KDE RGB Palette\n");
   str << description << QLatin1Char('\n');
   foreach (const KColorCollectionPrivate::ColorNode &node, d->colorList)
   {
       int r,g,b;
       node.color.getRgb(&r, &g, &b);
       str << r << " " << g << " " << b << " " << node.name << "\n";
   }

   return sf.commit();
}

QString KColorCollection::description() const
{
    return d->desc;
}

void KColorCollection::setDescription(const QString &desc)
{
    d->desc = desc;
}

QString KColorCollection::name() const
{
    return d->name;
}

void KColorCollection::setName(const QString &name)
{
    d->name = name;
}

KColorCollection::Editable KColorCollection::editable() const
{
    return d->editable;
}

void KColorCollection::setEditable(Editable editable)
{
    d->editable = editable;
}

int KColorCollection::count() const
{
    return (int) d->colorList.count();
}

KColorCollection&
KColorCollection::operator=( const KColorCollection &p)
{
  if (&p == this) return *this;
    d->colorList = p.d->colorList;
    d->name = p.d->name;
    d->desc = p.d->desc;
    d->editable = p.d->editable;
  return *this;
}

QColor
KColorCollection::color(int index) const
{
    if ((index < 0) || (index >= count()))
	return QColor();

    return d->colorList[index].color;
}

int
KColorCollection::findColor(const QColor &color) const
{
    for (int i = 0; i < d->colorList.size(); ++i)
  {
        if (d->colorList[i].color == color)
        return i;
  }
  return -1;
}

QString
KColorCollection::name(int index) const
{
  if ((index < 0) || (index >= count()))
	return QString();

  return d->colorList[index].name;
}

QString KColorCollection::name(const QColor &color) const
{
    return name(findColor(color));
}

int
KColorCollection::addColor(const QColor &newColor, const QString &newColorName)
{
    d->colorList.append(KColorCollectionPrivate::ColorNode(newColor, newColorName));
    return count() - 1;
}

int
KColorCollection::changeColor(int index,
                      const QColor &newColor,
                      const QString &newColorName)
{
    if ((index < 0) || (index >= count()))
	return -1;

  KColorCollectionPrivate::ColorNode& node = d->colorList[index];
  node.color = newColor;
  node.name  = newColorName;

  return index;
}

int KColorCollection::changeColor(const QColor &oldColor,
                          const QColor &newColor,
                          const QString &newColorName)
{
    return changeColor( findColor(oldColor), newColor, newColorName);
}

