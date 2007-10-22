/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kminstancepage.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmvirtualmanager.h"
#include "kmmanager.h"
#include "kprinterpropertydialog.h"
#include "kprinter.h"
#include "kmtimer.h"

#include <QtGui/QLayout>
#include <QtCore/QRegExp>
#include <QtGui/QPushButton>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdebug.h>

KMInstancePage::KMInstancePage(QWidget *parent)
        : QWidget(parent)
{
    m_view = new KListWidget(this);
    m_printer = 0;

    initActions();

    QHBoxLayout *main_ = new QHBoxLayout(this);
    main_->setMargin(0);
    main_->setSpacing(0);
    main_->addWidget(m_view);
    QVBoxLayout *sub_ = new QVBoxLayout();
    sub_->setMargin(0);
    sub_->setSpacing(0);
    main_->addLayout(sub_);
    for (QList<QPushButton*>::Iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
        if (*it)
            sub_->addWidget(*it);
        else
            sub_->addSpacing(10);
    sub_->addStretch(1);

    this->setWhatsThis(i18n("Define/Edit here instances for the current selected "
                            "printer. An instance is a combination of a real "
                            "(physical) printer and a set of predefined options. "
                            "For a single InkJet printer, you could define different "
                            "print formats like <i>DraftQuality</i>, <i>PhotoQuality</i> "
                            "or <i>TwoSided</i>. Those instances appear as normal "
                            "printers in the print dialog and allow you to quickly "
                            "select the print format you want."));
}

KMInstancePage::~KMInstancePage()
{
}

void KMInstancePage::addButton(const QString& txt, const QString& pixmap, const char *receiver)
{
    QPushButton *btn = new QPushButton(this);
    btn->setText(txt);
    btn->setIcon(KIcon(pixmap));
    btn->setFlat(true);
    connect(btn, SIGNAL(clicked()), receiver);
    m_buttons.append(btn);
}

void KMInstancePage::initActions()
{
    addButton(i18n("New..."), "document-new", SLOT(slotNew()));
    addButton(i18n("Copy..."), "edit-copy", SLOT(slotCopy()));
    addButton(i18n("Remove"), "user-trash", SLOT(slotRemove()));
    m_buttons.append(0);
    addButton(i18n("Set as Default"), "exec", SLOT(slotDefault()));
    addButton(i18n("Settings"), "configure", SLOT(slotSettings()));
    m_buttons.append(0);
    addButton(i18n("Test..."), "document-print", SLOT(slotTest()));
}

void KMInstancePage::setPrinter(KMPrinter *p)
{
    QString oldText = m_view->currentItem() ? m_view->currentItem()->text() : QString();

    m_view->clear();
    m_printer = p;
    //bool ok = (p && !p->isSpecial());
    bool ok = (p != 0);
    if (ok) {
        QList<KMPrinter*> list;
        KMFactory::self()->virtualManager()->virtualList(list, p->name());
        QListIterator<KMPrinter*> it(list);
        while (it.hasNext()) {
            KMPrinter *printer(it.next());
            QStringList pair = printer->name().split('/', QString::SkipEmptyParts);

            QListWidgetItem* item = new QListWidgetItem();
            item->setIcon(SmallIcon((printer->isSoftDefault() ? "exec" : "document-print")));
            item->setText(pair.count() > 1 ? pair[1] : i18n("(Default)"));
            m_view->addItem(item);
        }
        m_view->model()->sort(0);
    }

    for (QList<QPushButton*>::ConstIterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
        if (*it)
            (*it)->setEnabled(ok);

    //iif (!oldText.isEmpty())
    //{
    QListWidgetItem *item = m_view->findItems(oldText, Qt::MatchContains).value(0);
    if (!item)
        item = m_view->findItems(i18n("(Default)"), Qt::MatchContains).value(0);
    if (item)
        item->setSelected(true);
    //}
}

void KMInstancePage::slotNew()
{
    KMTimer::self()->hold();

    bool ok(false);
    QString name = KInputDialog::getText(i18n("Instance Name"), i18n("Enter name for new instance (leave untouched for default):"),
                                         i18n("(Default)"), &ok, this);
    if (ok) {
        if (name.indexOf(QRegExp("[/\\s]")) != -1)
            KMessageBox::error(this, i18n("Instance name must not contain any spaces or slashes."));
        else {
            if (name == i18n("(Default)"))
                name.clear();
            KMFactory::self()->virtualManager()->create(m_printer, name);
            setPrinter(m_printer);
        }
    }

    KMTimer::self()->release();
}

void KMInstancePage::slotRemove()
{
    if (!m_view->currentItem()) return;
    KMTimer::self()->hold();
    bool reload(false);


    QString src = m_view->currentItem()->text();
    if (!src.isEmpty()) {
        QString msg = (src != i18n("(Default)") ? i18n("Do you really want to remove instance %1?", src) : i18n("You can not remove the default instance. However all settings of %1 will be discarded. Continue?", src));
        if (KMessageBox::warningContinueCancel(this, msg, QString(), KStandardGuiItem::del()) == KMessageBox::Continue) {
            if (src == i18n("(Default)"))
                src.clear();
            reload = KMFactory::self()->virtualManager()->isDefault(m_printer, src);
            KMFactory::self()->virtualManager()->remove(m_printer, src);
            setPrinter(m_printer);
        }
    }

    KMTimer::self()->release(reload);
}

void KMInstancePage::slotCopy()
{
    if (!m_view->currentItem()) return;
    KMTimer::self()->hold();


    QString src = m_view->currentItem()->text();
    if (!src.isEmpty()) {
        bool ok(false);
        QString name = KInputDialog::getText(i18n("Instance Name"), i18n("Enter name for new instance (leave untouched for default):"),
                                             i18n("(Default)"), &ok, this);
        if (ok) {
            if (name.indexOf(QRegExp("[/\\s]")) != -1)
                KMessageBox::error(this, i18n("Instance name must not contain any spaces or slashes."));
            else {
                if (src == i18n("(Default)"))
                    src.clear();
                if (name == i18n("(Default)"))
                    name.clear();
                KMFactory::self()->virtualManager()->copy(m_printer, src, name);
                setPrinter(m_printer);
            }
        }
    }

    KMTimer::self()->release();
}

void KMInstancePage::slotSettings()
{
    if (!m_view->currentItem())
        return;
    KMTimer::self()->hold();

    QString src = m_view->currentItem()->text();
    if (!src.isEmpty()) {
        if (src == i18n("(Default)")) src.clear();
        KMPrinter *pr = KMFactory::self()->virtualManager()->findInstance(m_printer, src);
        if (!pr)
            KMessageBox::error(this, i18n("Unable to find instance %1." ,  m_view->currentItem()->text()));
        else if (!pr->isSpecial() && !KMFactory::self()->manager()->completePrinterShort(pr))
            KMessageBox::error(this, i18n("Unable to retrieve printer information. Message from printing system: %1." ,  KMFactory::self()->manager()->errorMsg()));
        else {
            int oldAppType = KMFactory::self()->settings()->application;
            KMFactory::self()->settings()->application = -1;
            KPrinterPropertyDialog::setupPrinter(pr, this);
            KMFactory::self()->settings()->application = oldAppType;
            if (pr->isEdited()) { // printer edited, need to save changes
                pr->setDefaultOptions(pr->editedOptions());
                pr->setEditedOptions(QMap<QString, QString>());
                pr->setEdited(false);
                KMFactory::self()->virtualManager()->triggerSave();
            }
        }
    } else
        KMessageBox::error(this, i18n("The instance name is empty. Please select an instance."));

    KMTimer::self()->release();
}

void KMInstancePage::slotDefault()
{
    if (!m_view->currentItem())
        return;
    KMTimer::self()->hold();

    QString src = m_view->currentItem()->text();
    if (!src.isEmpty()) {
        if (src == i18n("(Default)"))
            src.clear();
        KMFactory::self()->virtualManager()->setAsDefault(m_printer, src);
        setPrinter(m_printer);
    }

    KMTimer::self()->release(true);
}

void KMInstancePage::slotTest()
{
    if (!m_view->currentItem())
        return;
    KMTimer::self()->hold();

    QString src = m_view->currentItem()->text();
    if (!src.isEmpty()) {
        if (src == i18n("(Default)"))
            src.clear();
        KMPrinter *mpr = KMFactory::self()->virtualManager()->findInstance(m_printer, src);
        if (!mpr)
            KMessageBox::error(this, i18n("Internal error: printer not found."));
        else if (KMessageBox::warningContinueCancel(this, i18n("You are about to print a test page on %1. Do you want to continue?", mpr->printerName()), QString(), KGuiItem(i18n("Print Test Page")), KStandardGuiItem::cancel(), "printTestPage") == KMessageBox::Continue) {
            if (!KMFactory::self()->virtualManager()->testInstance(mpr))
                KMessageBox::error(this, i18n("Unable to send test page to %1.", mpr->printerName()));
            else
                KMessageBox::information(this, i18n("Test page successfully sent to printer %1.", mpr->printerName()));
        }
    }

    KMTimer::self()->release(false);
}
#include "kminstancepage.moc"
