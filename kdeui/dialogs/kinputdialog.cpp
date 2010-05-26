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

#include <QtGui/QDoubleValidator>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <kcombobox.h>
#include <kcompletion.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <knuminput.h>
#include <kstandardguiitem.h>
#include <ktextedit.h>

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QString &value, QWidget *parent,
                                        QValidator *validator, const QString &mask )
    : KDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setMargin(0);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_lineEdit = new KLineEdit(value, frame);
    m_lineEdit->setClearButtonShown(true);
    layout->addWidget(m_lineEdit);

    m_lineEdit->setFocus();
    m_label->setBuddy(m_lineEdit);

    layout->addStretch();

    if (validator)
        m_lineEdit->setValidator(validator);

    if (!mask.isEmpty())
        m_lineEdit->setInputMask(mask);

    connect(m_lineEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotEditTextChanged(const QString&)));

    setMainWidget(frame);
    slotEditTextChanged(value);
    setMinimumWidth(350);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QString &value, QWidget *parent )
    : KDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel | User1);
    setButtonGuiItem(User1, KStandardGuiItem::clear());
    setDefaultButton(Ok);
    setModal(true);
    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setMargin(0);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_textEdit = new KTextEdit(frame);
    m_textEdit->insertPlainText(value);
    layout->addWidget(m_textEdit, 10);

    m_textEdit->setFocus();
    m_label->setBuddy(m_textEdit);

    connect(this, SIGNAL(user1Clicked()), m_textEdit, SLOT(clear()));
    connect(this, SIGNAL(user1Clicked()), m_textEdit, SLOT(setFocus()));
    setMainWidget(frame);
    setMinimumWidth(400);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        int value, int minValue, int maxValue, int step, int base,
                                        QWidget *parent )
    : KDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel);
    setModal(true);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_intSpinBox = new KIntSpinBox(minValue, maxValue, step, value, frame, base);
    layout->addWidget(m_intSpinBox);

    layout->addStretch();
    layout->setMargin(0);

    m_intSpinBox->setFocus();
    setMainWidget(frame);
    setMinimumWidth(300);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        double value, double minValue, double maxValue, double step, int decimals,
                                        QWidget *parent )
    : KDialog( parent ),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel);
    setModal(true);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_doubleSpinBox = new QDoubleSpinBox(frame);
    m_doubleSpinBox->setRange(minValue, maxValue);
    m_doubleSpinBox->setSingleStep(step);
    m_doubleSpinBox->setValue(value);
    m_doubleSpinBox->setDecimals(decimals);

    layout->addWidget(m_doubleSpinBox);

    layout->addStretch();
    layout->setMargin(0);

    m_doubleSpinBox->setFocus();
    setMainWidget(frame);
    setMinimumWidth(300);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QStringList &list, int current, bool editable, QWidget *parent )
    : KDialog(parent),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    if (editable) {
        m_comboBox = new KComboBox(editable, frame);
        m_lineEdit = new KLineEdit(frame);
        m_lineEdit->setClearButtonShown(true);
        m_comboBox->setLineEdit(m_lineEdit);
        m_comboBox->insertItems(0, list);
        m_comboBox->setCurrentIndex(current);
        layout->addWidget(m_comboBox);

        connect(m_comboBox, SIGNAL(editTextChanged(const QString&)),
                SLOT(slotUpdateButtons(const QString&)));
        slotUpdateButtons(m_comboBox->currentText());
        m_comboBox->setFocus();
    } else {
        m_listBox = new KListWidget(frame);
        m_listBox->addItems(list);
        m_listBox->setCurrentRow(current);
        layout->addWidget(m_listBox, 10);
        connect(m_listBox, SIGNAL(executed(QListWidgetItem*)),
                SLOT(accept()));
        m_listBox->setFocus();
    }

    layout->addStretch();
    layout->setMargin(0);
    setMainWidget(frame);
    setMinimumWidth(320);
}

KInputDialogHelper::KInputDialogHelper( const QString &caption, const QString &label,
                                        const QStringList &list, const QStringList &select, bool multiple,
                                        QWidget *parent )
    : KDialog( parent ),
      m_label(0), m_lineEdit(0), m_intSpinBox(0),
      m_doubleSpinBox(0), m_comboBox(0)
{
    setCaption(caption);
    setButtons(Ok | Cancel);
    setModal(true);

    QWidget *frame = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(frame);

    m_label = new QLabel(label, frame);
    m_label->setWordWrap(true); 
   layout->addWidget(m_label);

    m_listBox = new KListWidget(frame);
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
        connect(m_listBox, SIGNAL(executed(QListWidgetItem*)), SLOT(accept()));

        if (!select.isEmpty()) {
            QString text = select.first();

            const QList<QListWidgetItem*> matches = m_listBox->findItems(text, Qt::MatchCaseSensitive|Qt::MatchExactly);
            if (!matches.isEmpty())
                m_listBox->setCurrentItem(matches.first());
        }
    }

    m_listBox->setFocus();

    layout->addStretch();
    layout->setMargin(0);
    setMainWidget(frame);
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

    enableButton(Ok, on);
}

void KInputDialogHelper::slotUpdateButtons( const QString &text )
{
    enableButton(Ok, !text.isEmpty());
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

KListWidget *KInputDialogHelper::listBox() const
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

    bool _ok = (dlg.exec() == KDialog::Accepted);

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

    bool _ok = (dlg.exec() == KDialog::Accepted);

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

    bool _ok = (dlg.exec() == KDialog::Accepted);

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

    bool _ok = (dlg.exec() == KDialog::Accepted);

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
        dlg.connect(dlg.listBox(), SIGNAL(executed(QListWidgetItem*)), &dlg, SLOT(accept()));

    bool _ok = (dlg.exec() == KDialog::Accepted);

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

    bool _ok = (dlg.exec() == KDialog::Accepted);

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

#include "kinputdialog_p.moc"

/* vim: set ai et sw=2 ts=2
*/
