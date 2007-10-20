/*
  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#ifndef KINPUTDIALOG_P_H
#define KINPUTDIALOG_P_H

class QLabel;
class QValidator;

class KComboBox;
class QDoubleSpinBox;
class KIntSpinBox;
class KLineEdit;
class KListWidget;
class KTextEdit;

#include <kdialog.h>

/**
 * @author Nadeem Hasan <nhasan@kde.org>
 */
class KInputDialogHelper : public KDialog
{
  Q_OBJECT

  public:

    KInputDialogHelper( const QString &caption, const QString &label,
                        const QString &value, QWidget *parent,
                        QValidator *validator, const QString &mask );
    KInputDialogHelper( const QString &caption, const QString &label,
                        const QString &value, QWidget *parent );
    KInputDialogHelper( const QString &caption, const QString &label, int value,
                        int minValue, int maxValue, int step, int base, QWidget *parent );
    KInputDialogHelper( const QString &caption, const QString &label, double value,
                        double minValue, double maxValue, double step, int decimals,
                        QWidget *parent );
    KInputDialogHelper( const QString &caption, const QString &label,
                        const QStringList &list, int current, bool editable, QWidget *parent );
    KInputDialogHelper( const QString &caption, const QString &label,
                        const QStringList &list, const QStringList &select, bool editable,
                        QWidget *parent );

    ~KInputDialogHelper();

    KLineEdit *lineEdit() const;
    KIntSpinBox *intSpinBox() const;
    QDoubleSpinBox *doubleSpinBox() const;
    KComboBox *comboBox() const;
    KListWidget *listBox() const;
    KTextEdit *textEdit() const;

  private Q_SLOTS:
    void slotEditTextChanged( const QString& );
    void slotUpdateButtons( const QString& );

  private:
    QLabel *m_label;
    KLineEdit *m_lineEdit;
    KIntSpinBox *m_intSpinBox;
    QDoubleSpinBox *m_doubleSpinBox;
    KComboBox *m_comboBox;
    KListWidget *m_listBox;
    KTextEdit *m_textEdit;
};

#endif // KINPUTDIALOG_P_H

/* vim: set ai et sw=2 ts=2
*/
