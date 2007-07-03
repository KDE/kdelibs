/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2000 Rik Hemsley <rik@kde.org>
   Copyright (C) 2003 Zack Rusin  <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef K3SPELLDLG_H
#define K3SPELLDLG_H

#include <kde3support_export.h>
#include <kdialog.h>

class QStringList;
class QLabel;
class Q3ListViewItem;
class QPushButton;
class QProgressBar;
class KListWidget;
class KLineEdit;

//Possible result codes
enum KS_RESULT {
  KS_CANCEL=     0,
  KS_REPLACE=    1,
  KS_REPLACEALL= 2,
  KS_IGNORE=     3,
  KS_IGNOREALL=  4,
  KS_ADD=        5,
  KS_STOP=       7,
  KS_SUGGEST=    8,
  KS_CONFIG=     9
};

/**
 * @deprecated, use sonnet instead
 */
class KDE3SUPPORT_EXPORT_DEPRECATED K3SpellDlg : public KDialog
{
  Q_OBJECT

  class K3SpellDlgPrivate;
  K3SpellDlgPrivate *d;
  KLineEdit *editbox;
  QProgressBar *progbar;
  KListWidget *listbox;
  QStringList *sugg;
  QPushButton *qpbrep, *qpbrepa;
  QLabel *wordlabel;
  QString word, newword;
  bool progressbar;

public:
  explicit K3SpellDlg( QWidget *parent,
                      bool _progressbar = false, bool _modal = false );
  ~K3SpellDlg();

  QString replacement() const;

  /**
   * Change the misspelled word and suggested replacements
   *  and enable the disabled buttons on the dialog box.
   * (Buttons are disabled by standby().)
   **/
  void init( const QString& _word, QStringList* _sugg );
  void init( const QString& _word, QStringList* _sugg,
             const QString& context );

  void standby();

  public Q_SLOTS:
  /**
   * Adjust the progress bar to @p p percent.
   **/
  void slotProgress( unsigned int p );

protected:
  virtual void closeEvent( QCloseEvent * e );
  void done( int i );

 Q_SIGNALS:
    /**
      This signal is emitted when a button is pressed.
      */
  void command( int );

  void ready( bool );

protected Q_SLOTS:
  void ignore();
  void add();
  void ignoreAll();
  void cancel();
  void replace();
  void replaceAll();
  void suggest();
  void stop();
  void slotConfigChanged();

  void textChanged( const QString & );

  void slotSelectionChanged( Q3ListViewItem* item );
};

#endif

