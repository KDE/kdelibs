/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias Hölzer-Klüpfel <mhk@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#ifndef __KACCELMANAGER_PRIVATE_H__
#define __KACCELMANAGER_PRIVATE_H__


#include <qstring.h>
#include <qarray.h>
#include <qvaluelist.h>
#include <qobject.h>


/**
 * A string class handling accelerators.
 *
 * This class contains a string and knowledge about accelerators.
 * It keeps a list weights, telling how valuable each character
 * would be as an accelerator.
 *
 * @author Matthias Hölzer-Klüpfel <mhk@kde.org>
 * @version $Id$
*/

class KAccelString
{
public:

  KAccelString() : m_pureText(), m_accel(-1) {};
  KAccelString(const QString &input, int initalWeight=-1);

  void calculateWeights(int initialWeight);

  const QString &pure() const { return m_pureText; };
  QString accelerated() const;

  int accel() const { return m_accel; };
  void setAccel(int accel) { m_accel = accel; };

  QChar accelerator() const;

  int maxWeight(int &index, const QString &used);

  bool operator == (const KAccelString &c) const { return m_pureText == c.m_pureText && m_accel == c.m_accel && orig_accel == c.orig_accel; }


private:

  int stripAccelerator(QString &input);

  void dump();

  QString        m_pureText;
  int            m_accel, orig_accel;
  QMemArray<int> m_weight;

};


typedef QValueList<KAccelString> KAccelStringList;


/**
 * This class encapsulates the algorithm finding the 'best'
 * distribution of accelerators in a hierarchy of widgets.
 *
 * @author Matthias Hölzer-Klüpfel <mhk@kde.org>
 * @version $Id$
*/

class KAccelManagerAlgorithm
{
public:

  static const int DEFAULT_WEIGHT;

  static const int FIRST_CHARACTER_EXTRA_WEIGHT;
  static const int WORD_BEGINNING_EXTRA_WEIGHT;
  static const int WANTED_ACCEL_EXTRA_WEIGHT;
  static const int DIALOG_BUTTON_EXTRA_WEIGHT;

  static const int ACTION_ELEMENT_WEIGHT;
  static const int GROUP_BOX_WEIGHT;
  static const int MENU_TITLE_WEIGHT;

  static void findAccelerators(KAccelStringList &result, QString &used);

};


class QPopupMenu;


/**
 * This class manages a popup menu. It will notice if entries have been
 * added or changed, and will recalculate the accelerators accordingly.
 *
 * This is necessary for dynamic menues like for example in kicker.
 *
 * @author Matthias Hölzer-Klüpfel <mhk@kde.org>
 * @version $Id$
*/

class KPopupAccelManager : public QObject
{
  Q_OBJECT

public:

  static void manage(QPopupMenu *popup);


protected:

  KPopupAccelManager(QPopupMenu *popup);


private slots:

  void aboutToShow();


private:

  void calculateAccelerators();

  void findMenuEntries(KAccelStringList &list);
  void setMenuEntries(const KAccelStringList &list);

  QPopupMenu       *m_popup;
  KAccelStringList m_entries;
  int              m_count;

};


#endif
