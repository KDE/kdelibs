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

#include "kinputdialog.h"
#include "kinputdialog_p.h"

#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QListWidget>

#include <kcombobox.h>
#include <kcompletion.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kstandardguiitem.h>
#include <ktextedit.h>

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QString &value, QWidget *parent,
                                        QValidator *validator, const QString &mask )
    : QDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0), m_listBox(0)
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_lineEdit = new KLineEdit(value, this);
    m_lineEdit->setClearButtonShown(true);
    layout->addWidget(m_lineEdit);

    m_lineEdit->setFocus();
    m_label->setBuddy(m_lineEdit);

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    if (validator)
        m_lineEdit->setValidator(validator);

    if (!mask.isEmpty())
        m_lineEdit->setInputMask(mask);

    connect(m_lineEdit, SIGNAL(textChanged(QString)),
            SLOT(slotEditTextChanged(QString)));

    slotEditTextChanged(value);
    setMinimumWidth(350);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QString &value, QWidget *parent )
    : QDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0)
{
    setWindowTitle(caption);
    setModal(true);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_textEdit = new KTextEdit(this);
    m_textEdit->insertPlainText(value);
    layout->addWidget(m_textEdit, 10);

    m_textEdit->setFocus();
    m_label->setBuddy(m_textEdit);

    m_buttonBox = new QDialogButtonBox(this);
    QPushButton *clearButton = new QPushButton(m_buttonBox);
    KGuiItem::assign(clearButton, KStandardGuiItem::clear());
    m_buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    connect(clearButton, SIGNAL(clicked()), m_textEdit, SLOT(clear()));
    connect(clearButton, SIGNAL(clicked()), m_textEdit, SLOT(setFocus()));
    setMinimumWidth(400);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        int value, int minValue, int maxValue, int step, int base,
                                        QWidget *parent )
    : QDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0), m_listBox(0)
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_intSpinBox = new KIntSpinBox(minValue, maxValue, step, value, this, base);
    layout->addWidget(m_intSpinBox);

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    m_intSpinBox->setFocus();
    setMinimumWidth(300);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        double value, double minValue, double maxValue, double step, int decimals,
                                        QWidget *parent )
    : QDialog( parent ),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0), m_listBox(0)
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_doubleSpinBox = new QDoubleSpinBox(this);
    m_doubleSpinBox->setRange(minValue, maxValue);
    m_doubleSpinBox->setSingleStep(step);
    m_doubleSpinBox->setValue(value);
    m_doubleSpinBox->setDecimals(decimals);

    layout->addWidget(m_doubleSpinBox);

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    m_doubleSpinBox->setFocus();
    setMinimumWidth(300);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QStringList &list, int current, bool editable, QWidget *parent )
    : QDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0), m_listBox(0)
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    if (editable) {
        m_comboBox = new KComboBox(editable, this);
        m_lineEdit = new KLineEdit(this);
        m_lineEdit->setClearButtonShown(true);
        m_comboBox->setLineEdit(m_lineEdit);
        m_comboBox->insertItems(0, list);
        m_comboBox->setCurrentIndex(current);
        layout->addWidget(m_comboBox);

        connect(m_comboBox, SIGNAL(editTextChanged(QString)),
                SLOT(slotUpdateButtons(QString)));
        m_comboBox->setFocus();
    } else {
        m_listBox = new QListWidget(this);
        m_listBox->addItems(list);
        m_listBox->setCurrentRow(current);
        layout->addWidget(m_listBox, 10);
        connect(m_listBox, SIGNAL(itemActivated(QListWidgetItem*)),
                SLOT(accept()));
        m_listBox->setFocus();
    }

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    if (editable) {
        slotUpdateButtons(m_comboBox->currentText());
    }
    setMinimumWidth(320);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QStringList &list, const QStringList &select, bool multiple,
                                        QWidget *parent )
    : QDialog( parent ),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0), m_buttonBox(0), m_listBox(0)
{
    setWindowTitle(caption);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    m_label = new QLabel(label, this);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_listBox = new QListWidget(this);
    m_listBox->addItems(list);
    layout->addWidget(m_listBox);

    if (multiple) {
        m_listBox->setSelectionMode(QAbstractItemView::ExtendedSelection);

        for (QStringList::ConstIterator it = select.begin(); it != select.end(); ++it) {
            const QList<QListWidgetItem*> matches = m_listBox->findItems(*it, Qt::MatchCaseSensitive|Qt::MatchExactly);
            if (!matches.isEmpty())
                m_listBox->setCurrentItem(matches.first());
        }
    } else {
        connect(m_listBox, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(accept()));

        if (!select.isEmpty()) {
            QString text = select.first();

            const QList<QListWidgetItem*> matches = m_listBox->findItems(text, Qt::MatchCaseSensitive|Qt::MatchExactly);
            if (!matches.isEmpty())
                m_listBox->setCurrentItem(matches.first());
        }
    }

    m_listBox->setFocus();

    layout->addStretch();

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(m_buttonBox);

    setMinimumWidth(320);
}

KInputDialogHelper::~KInputDialogHelper()
{
}

void KInputDialogHelper::slotEditTextChanged( const QString &text )
{
    bool on;

    if (m_lineEdit->validator()) {
        QString str = m_lineEdit->text();
        int index = m_lineEdit->cursorPosition();
        on = (m_lineEdit->validator()->validate(str, index) == QValidator::Acceptable);
    } else {
        on = !text.trimmed().isEmpty();
    }

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(on);
}

void KInputDialogHelper::slotUpdateButtons( const QString &text )
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

KLineEdit *KInputDialogHelper::lineEdit() const
{
    return m_lineEdit;
}

KIntSpinBox *KInputDialogHelper::intSpinBox() const
{
    return m_intSpinBox;
}

QDoubleSpinBox *KInputDialogHelper::doubleSpinBox() const
{
    return m_doubleSpinBox;
}

KComboBox *KInputDialogHelper::comboBox() const
{
    return m_comboBox;
}

QListWidget *KInputDialogHelper::listBox() const
{
    return m_listBox;
}

KTextEdit *KInputDialogHelper::textEdit() const
{
    return m_textEdit;
}


// KInputDialog namespace

namespace KInputDialog {

QString getText( const QString &caption,
                 const QString &label, const QString &value, bool *ok, QWidget *parent,
                 QValidator *validator, const QString &mask,
                 const QString &whatsThis,const QStringList &completionList )
{
    KInputDialogHelper dlg(caption, label, value, parent, validator, mask);

    if (!whatsThis.isEmpty())
        dlg.lineEdit()->setWhatsThis(whatsThis);

    if (!completionList.isEmpty()) {
        KCompletion *comp=dlg.lineEdit()->completionObject();
        for (QStringList::const_iterator it = completionList.constBegin(); it != completionList.constEnd(); ++it)
            comp->addItem(*it);
    }

    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    QString result;
    if (_ok)
        result = dlg.lineEdit()->text();

    // A validator may explicitly allow leading and trailing whitespace
    if (!validator)
        result = result.trimmed();

    return result;
}

QString getMultiLineText( const QString &caption,
                          const QString &label, const QString &value, bool *ok,
                          QWidget *parent )
{
    KInputDialogHelper dlg(caption, label, value, parent);
    dlg.textEdit()->setAcceptRichText(false);
    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    QString result;
    if (_ok)
        result = dlg.textEdit()->toPlainText();

    return result;
}

int getInteger( const QString &caption, const QString &label,
                int value, int minValue, int maxValue, int step, int base, bool *ok,
                QWidget *parent )
{
    KInputDialogHelper dlg(caption, label, value, minValue, maxValue, step, base, parent);

    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    int result = 0;
    if (_ok)
        result = dlg.intSpinBox()->value();

    return result;
}

int getInteger( const QString &caption, const QString &label,
                int value, int minValue, int maxValue, int step, bool *ok,
                QWidget *parent )
{
    return getInteger(caption, label, value, minValue, maxValue, step, 10, ok, parent);
}

double getDouble( const QString &caption, const QString &label,
                  double value, double minValue, double maxValue, double step, int decimals,
                  bool *ok, QWidget *parent )
{
    KInputDialogHelper dlg(caption, label, value, minValue, maxValue, step, decimals, parent);

    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    double result = 0;
    if (_ok)
        result = dlg.doubleSpinBox()->value();

    return result;
}

double getDouble( const QString &caption, const QString &label,
                  double value, double minValue, double maxValue, int decimals,
                  bool *ok, QWidget *parent )
{
    return getDouble(caption, label, value, minValue, maxValue, 0.1, decimals, ok, parent);
}

QString getItem( const QString &caption, const QString &label,
                 const QStringList &list, int current, bool editable, bool *ok,
                 QWidget *parent )
{
    KInputDialogHelper dlg(caption, label, list, current, editable, parent);

    if (!editable)
        dlg.connect(dlg.listBox(), SIGNAL(itemActivated(QListWidgetItem*)), &dlg, SLOT(accept()));

    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    QString result;
    if (_ok) {
        if (editable)
            result = dlg.comboBox()->currentText();
        else if( dlg.listBox()->currentItem())
            result = dlg.listBox()->currentItem()->text();
    }

    return result;
}

QStringList getItemList( const QString &caption,
                         const QString &label, const QStringList &list, const QStringList &select,
                         bool multiple, bool *ok, QWidget *parent )
{
    KInputDialogHelper dlg(caption, label, list, select, multiple, parent);

    bool _ok = (dlg.exec() == QDialog::Accepted);

    if (ok)
        *ok = _ok;

    QStringList result;
    if (_ok) {
        for (int i=0 ; i < dlg.listBox()->count() ; i++) {

            QListWidgetItem* item = dlg.listBox()->item(i);

            if (item->isSelected())
                result.append(item->text());
        }
    }

    return result;
}

}

#include "moc_kinputdialog_p.cpp"

/* vim: set ai et sw=2 ts=2
*/
