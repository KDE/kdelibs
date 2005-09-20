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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//-----------------------------------------------------------------------------
// KDE color palette 

#include "kpalette.h"

#include <qfile.h>
#include <qtextstream.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ksavefile.h>
#include <kstringhandler.h>

QStringList
KPalette::getPaletteList()
{
  QStringList paletteList;
  KGlobal::dirs()->findAllResources("config", "colors/*", false, true, paletteList);

  int strip = strlen("colors/");
  for(QStringList::Iterator it = paletteList.begin();
      it != paletteList.end();
      it++)
  { 
      (*it) = (*it).mid(strip); 
  }

  return paletteList;
}

KPalette::KPalette(const QString &name)
 : mName(name)
{
  if (mName.isEmpty()) return;

  QString filename = locate("config", "colors/"+mName);
  if (filename.isEmpty()) return;

  QFile paletteFile(filename);
  if (!paletteFile.exists()) return;
  if (!paletteFile.open(QIODevice::ReadOnly)) return;

  // Read first line
  // Expected "GIMP Palette"
  QString line = QString::fromLocal8Bit(paletteFile.readLine());
  if (line.indexOf(" Palette") == -1) return;

  while( !paletteFile.atEnd() )
  {
     line = QString::fromLocal8Bit(paletteFile.readLine());
     if (line[0] == '#') 
     {
        // This is a comment line
        line = line.mid(1); // Strip '#' 
        line = line.trimmed(); // Strip remaining white space..
        if (!line.isEmpty())
        {
           mDesc += line+"\n"; // Add comment to description
        }
     }
     else
     {
        // This is a color line, hopefully
        line = line.trimmed();
        if (line.isEmpty()) continue;
        int r, g, b;
        int pos = 0;
        if (sscanf(line.ascii(), "%d %d %d%n", &r, &g, &b, &pos) >= 3)
        {
           r = qBound(0, r, 255);
           g = qBound(0, g, 255);
           b = qBound(0, b, 255);
           QString name = line.mid(pos).trimmed();
           mColorList.append(ColorNode(QColor(r, g, b), name));
        }
     }
  }
}

KPalette::KPalette(const KPalette &p)
 : mColorList(p.mColorList), mName(p.mName), 
   mDesc(p.mDesc), mEditable(p.mEditable)
{}

KPalette::~KPalette()
{
  // Need auto-save?
}

bool
KPalette::save()
{
   QString filename = locateLocal("config", "colors/"+mName);
   KSaveFile sf(filename);
   if (sf.status() != 0) return false;

   QTextStream *str = sf.textStream();

   QString description = mDesc.trimmed();
   description = "#"+QStringList::split("\n", description, true).join("\n#");

   (*str) << "KDE RGB Palette\n";   
   (*str) << description << "\n";
   foreach (ColorNode node, mColorList)
   {
       int r,g,b;
       node.color.getRgb(&r, &g, &b);
       (*str) << r << " " << g << " " << b << " " << node.name << "\n";
   }
   return sf.close();
}


KPalette&
KPalette::operator=( const KPalette &p)
{
  if (&p == this) return *this;
  mColorList = p.mColorList;
  mName = p.mName;
  mDesc = p.mDesc;
  mEditable = p.mEditable; 
  return *this;
}

QColor
KPalette::color(int index) const
{
  if ((index < 0) || (index >= nrColors()))
	return QColor();

  return mColorList[index].color;
}

int
KPalette::findColor(const QColor &color) const
{
  for (int i = 0; i < mColorList.size(); ++i)
  {
     if (mColorList[i].color == color)
        return i;
  }
  return -1;
}

QString
KPalette::colorName(int index) const
{
  if ((index < 0) || (index >= nrColors()))
	return QString::null;

  return mColorList[index].name;
}

int
KPalette::addColor(const QColor &newColor, const QString &newColorName)
{
  mColorList.append(ColorNode(newColor, newColorName));
  return nrColors()-1;
}

int
KPalette::changeColor(int index, 
                      const QColor &newColor, 
                      const QString &newColorName)
{
  if ((index < 0) || (index >= nrColors()))
	return -1;

  ColorNode& node = mColorList[index];
  node.color = newColor;
  node.name  = newColorName;
  
  return index;
}
