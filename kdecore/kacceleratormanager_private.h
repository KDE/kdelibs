/*  This file is part of the KDE project
    Copyright (C) 2002 Matthias H�zer-Klpfel <mhk@kde.org>

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


#ifndef KACCELMANAGER_PRIVATE_H
#define KACCELMANAGER_PRIVATE_H


#include <qstring.h>

#include <qobject.h>

class QStackedWidget;

/**
 * A string class handling accelerators.
 *
 * This class contains a string and knowledge about accelerators.
 * It keeps a list weights, telling how valuable each character
 * would be as an accelerator.
 *
 * @author Matthias H�zer-Klpfel <mhk@kde.org>
*/

class KAccelString
{
public:

  KAccelString() : m_pureText(), m_accel(-1) {}
  KAccelString(const QString &input, int initalWeight=-1);

  void calculateWeights(int initialWeight);

  const QString &pure() const { return m_pureText; };
  QString accelerated() const;

  int accel() const { return m_accel; };
  void setAccel(int accel) { m_accel = accel; };

  int originalAccel() const { return m_orig_accel; }
  QString originalText() const { return m_origText; }

  QChar accelerator() const;

  int maxWeight(int &index, const QString &used);

  bool operator == (const KAccelString &c) const { return m_pureText == c.m_pureText && m_accel == c.m_accel && m_orig_accel == c.m_orig_accel; }


private:

  int stripAccelerator(QString &input);

  void dump();

  QString        m_pureText,  m_origText;
  int            m_accel, m_orig_accel;
  QVector<int> m_weight;

};


typedef QList<KAccelString> KAccelStringList;


/**
 * This class encapsulates the algorithm finding the 'best'
 * distribution of accelerators in a hierarchy of widgets.
 *
 * @author Matthias H�zer-Klpfel <mhk@kde.org>
*/

class KAccelManagerAlgorithm
{
public:

  enum {
    // Default control weight
    DEFAULT_WEIGHT = 50,
    // Additional weight for first character in string
    FIRST_CHARACTER_EXTRA_WEIGHT = 50,
    // Additional weight for the beginning of a word
    WORD_BEGINNING_EXTRA_WEIGHT = 50,
    // Additional weight for the dialog buttons (large, we basically never want these reassigned)
    DIALOG_BUTTON_EXTRA_WEIGHT = 300,
    // Additional weight for a 'wanted' accelerator
    WANTED_ACCEL_EXTRA_WEIGHT = 150,
    // Default weight for an 'action' widget (ie, pushbuttons)
    ACTION_ELEMENT_WEIGHT = 50,
    // Default weight for group boxes (low priority)
    GROUP_BOX_WEIGHT = -2000,
    // Default weight for menu titles
    MENU_TITLE_WEIGHT = 250,
    // Additional weight for KDE standard accelerators
    STANDARD_ACCEL = 300
  };

  static void findAccelerators(KAccelStringList &result, QString &used);

};


/**
 * This class manages a popup menu. It will notice if entries have been
 * added or changed, and will recalculate the accelerators accordingly.
 *
 * This is necessary for dynamic menus like for example in kicker.
 *
 * @author Matthias H�zer-Klpfel <mhk@kde.org>
*/

class KPopupAccelManager : public QObject
{
  Q_OBJECT

public:

  static void manage(QMenu *popup);


protected:

  KPopupAccelManager(QMenu *popup);


private slots:

  void aboutToShow();


private:

  void calculateAccelerators();

  void findMenuEntries(KAccelStringList &list);
  void setMenuEntries(const KAccelStringList &list);

  QMenu       *m_popup;
  KAccelStringList m_entries;
  int              m_count;

};


class QWidgetStackAccelManager : public QObject
{
  Q_OBJECT

public:

  static void manage(QStackedWidget *popup);


protected:

  QWidgetStackAccelManager(QStackedWidget *popup);


private slots:

    void currentChanged(int child);
    bool eventFilter ( QObject * watched, QEvent * e );

private:

  void calculateAccelerators();

  QStackedWidget     *m_stack;
  KAccelStringList m_entries;

};


#endif
