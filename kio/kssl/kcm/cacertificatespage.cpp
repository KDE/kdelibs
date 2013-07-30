/*  This file is part of the KDE project
    Copyright (C) 2010 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "cacertificatespage.h"
#include "displaycertdialog_p.h"

#include <kio_ksslcertificatemanager.h>
#include <kio_ksslcertificatemanager_p.h>
using namespace KIO;

#include <QDebug>
#include <kfiledialog.h>

#include <QList>
#include <QSslCertificate>
#include <QTreeWidgetItem>
#include <QStandardItemModel>

enum Columns {
    OrgCnColumn = 0,
    OrgUnitColumn,
    HiddenSortColumn
};


static QString nonemptyIssuer(const QSslCertificate &cert)
{
    QString issuerText;
    static const QSslCertificate::SubjectInfo fields[3] = {
                     QSslCertificate::Organization,
                     QSslCertificate::CommonName,
                     QSslCertificate::OrganizationalUnitName
    };
    for (int i = 0; i < 3; i++) {
#warning QT5 PORT TO NEW API
        issuerText = cert.issuerInfo(fields[i]).first();

        if (!issuerText.isEmpty()) {
            return issuerText;
        }
    }
    return issuerText;
}


class CaCertificateItem : public QTreeWidgetItem
{
public:
    CaCertificateItem(QTreeWidgetItem *parent, const QSslCertificate &cert, bool isEnabled)
     : QTreeWidgetItem(parent, m_type),
       m_cert(cert)
    {
       setEnabled(isEnabled);
    }

    QVariant data(int column, int role) const
    {
        switch (role) {
        case Qt::DisplayRole:
            switch (column) {
            case OrgCnColumn:
            case HiddenSortColumn: {
#warning QT5 PORT TO NEW API
                QString subjectText = m_cert.issuerInfo(QSslCertificate::CommonName).first();
                if (column == HiddenSortColumn) {
                    return subjectText.toLower();
                }
                return subjectText; }
            case OrgUnitColumn:
                return m_cert.issuerInfo(QSslCertificate::OrganizationalUnitName);
            }
        }

        return QTreeWidgetItem::data(column, role);
    }

    bool isEnabled() const
    {
        return data(OrgCnColumn, Qt::CheckStateRole).toInt() == Qt::Checked;
    }

    void setEnabled(bool enabled)
    {
        setData(OrgCnColumn, Qt::CheckStateRole, enabled ? Qt::Checked : Qt::Unchecked);
    }

    static const int m_type = QTreeWidgetItem::UserType;
    QSslCertificate m_cert;
};

CaCertificatesPage::CaCertificatesPage(QWidget *parent)
 : QWidget(parent),
   m_firstShowEvent(true),
   m_blockItemChanged(false)
{
    m_ui.setupUi(this);
    connect(m_ui.displaySelection, SIGNAL(clicked()), SLOT(displaySelectionClicked()));
    connect(m_ui.disableSelection, SIGNAL(clicked()), SLOT(disableSelectionClicked()));
    connect(m_ui.enableSelection, SIGNAL(clicked()), SLOT(enableSelectionClicked()));
    connect(m_ui.removeSelection, SIGNAL(clicked()), SLOT(removeSelectionClicked()));
    connect(m_ui.add, SIGNAL(clicked()), SLOT(addCertificateClicked()));
    connect(m_ui.treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            SLOT(itemChanged(QTreeWidgetItem*,int)));
    connect(m_ui.treeWidget, SIGNAL(itemSelectionChanged()),
            SLOT(itemSelectionChanged()));

    m_ui.treeWidget->setColumnCount(HiddenSortColumn + 1);
    m_ui.treeWidget->setColumnHidden(HiddenSortColumn, true);
}


void CaCertificatesPage::load()
{
    m_ui.treeWidget->clear();
    m_ui.treeWidget->sortByColumn(-1);  // disable during mass insertion
    m_knownCertificates.clear();

    m_systemCertificatesParent = new QTreeWidgetItem(m_ui.treeWidget);
    m_systemCertificatesParent->setText(0, i18n("System certificates"));
    // make system certificates come first in the sorted view
    m_systemCertificatesParent->setText(HiddenSortColumn, QLatin1String("a"));
    m_systemCertificatesParent->setExpanded(true);
    m_systemCertificatesParent->setFlags(m_systemCertificatesParent->flags() & ~Qt::ItemIsSelectable);

    m_userCertificatesParent = new QTreeWidgetItem(m_ui.treeWidget);
    m_userCertificatesParent->setText(0, i18n("User-added certificates"));
    m_userCertificatesParent->setText(HiddenSortColumn, QLatin1String("b"));
    m_userCertificatesParent->setExpanded(true);
    m_userCertificatesParent->setFlags(m_userCertificatesParent->flags() & ~Qt::ItemIsSelectable);

    QList<KSslCaCertificate> caCerts = _allKsslCaCertificates(KSslCertificateManager::self());
    // qDebug() << "# certs:" << caCerts.count();
    foreach (const KSslCaCertificate &caCert, caCerts) {
        addCertificateItem(caCert);
    }

    m_ui.treeWidget->sortByColumn(HiddenSortColumn, Qt::AscendingOrder);
}

void CaCertificatesPage::showEvent(QShowEvent *event)
{
    if (m_firstShowEvent) {
        // TODO use QTextMetrics
        m_ui.treeWidget->setColumnWidth(OrgCnColumn, 420);
        m_firstShowEvent = false;
    }
    QWidget::showEvent(event);
}

void CaCertificatesPage::save()
{
    QList<KSslCaCertificate> newState;

    KSslCaCertificate::Store store = KSslCaCertificate::SystemStore;
    QTreeWidgetItem *grandParent = m_systemCertificatesParent;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < grandParent->childCount(); j++) {

            QTreeWidgetItem *parentItem = grandParent->child(j);
            for (int k = 0; k < parentItem->childCount(); k++) {
                CaCertificateItem *item = static_cast<CaCertificateItem *>(parentItem->child(k));
                newState += KSslCaCertificate(item->m_cert, store, !item->isEnabled());
            }
        }
        store = KSslCaCertificate::UserStore;
        grandParent = m_userCertificatesParent;
    }

    // qDebug() << "# certs:" << newState.count();
    _setAllKsslCaCertificates(KSslCertificateManager::self(), newState);
    emit changed(false);
}


void CaCertificatesPage::defaults()
{
    //### is that all?
    load();
    emit changed(false);
}

// private slot
void CaCertificatesPage::itemSelectionChanged()
{
    // qDebug() << m_ui.treeWidget->selectionModel()->hasSelection();
    int numRemovable = 0;
    int numEnabled = 0;
    int numDisplayable = 0;
    foreach(const QTreeWidgetItem *twItem, m_ui.treeWidget->selectedItems()) {
        const CaCertificateItem *item = dynamic_cast<const CaCertificateItem *>(twItem);
        Q_ASSERT(item);
        if (item) {
            numDisplayable++;
            if (item->parent()->parent() == m_userCertificatesParent) {
                numRemovable++;
            }
            if (item->isEnabled()) {
                numEnabled++;
            }
        }
    }
    m_ui.displaySelection->setEnabled(numDisplayable);
    m_ui.removeSelection->setEnabled(numRemovable);
    m_ui.disableSelection->setEnabled(numEnabled);
    m_ui.enableSelection->setEnabled(numDisplayable > numEnabled); // the rest is disabled
}

// private slot
void CaCertificatesPage::displaySelectionClicked()
{
    QList<QSslCertificate> certs;
    foreach(const QTreeWidgetItem *twItem, m_ui.treeWidget->selectedItems()) {
        const CaCertificateItem *item = dynamic_cast<const CaCertificateItem *>(twItem);
        Q_ASSERT(item);
        if (item) {
            certs += item->m_cert;
        }
    }
    DisplayCertDialog dc(this);
    dc.setCertificates(certs);
    dc.exec();
}

// private slot
void CaCertificatesPage::disableSelectionClicked()
{
    enableDisableSelectionClicked(false);
}

// private slot
void CaCertificatesPage::enableSelectionClicked()
{
    enableDisableSelectionClicked(true);
}

void CaCertificatesPage::enableDisableSelectionClicked(bool isEnable)
{
    const bool prevBlockItemChanged = m_blockItemChanged;
    m_blockItemChanged = true;
    foreach(QTreeWidgetItem *twItem, m_ui.treeWidget->selectedItems()) {
        CaCertificateItem *item = dynamic_cast<CaCertificateItem *>(twItem);
        Q_ASSERT(item);
        if (item) {
            item->setEnabled(isEnable);
        }
    }
    emit changed(true);
    m_blockItemChanged = prevBlockItemChanged;
    // now make sure that the buttons are enabled as appropriate
    itemSelectionChanged();
}


// private slot
void CaCertificatesPage::removeSelectionClicked()
{
    bool didRemove = false;
    foreach(QTreeWidgetItem *twItem, m_ui.treeWidget->selectedItems()) {
        const CaCertificateItem *item = dynamic_cast<const CaCertificateItem *>(twItem);
        Q_ASSERT(item);
        if (!item || item->parent()->parent() != m_userCertificatesParent) {
            continue;
        }
        QTreeWidgetItem *parent = item->parent();
        m_knownCertificates.remove(item->m_cert.digest().toHex());
        delete item;
        didRemove = true;
        if (parent->childCount() == 0) {
            delete parent;
        }
    }
    if (didRemove) {
        emit changed(true);
    }
}

// private slot
void CaCertificatesPage::addCertificateClicked()
{
    const QStringList certFiles
      = KFileDialog::getOpenFileNames(QUrl(), QLatin1String("application/x-x509-ca-cert"),
                                      this, i18n("Pick Certificates"));

    QList<QSslCertificate> certs;
    foreach (const QString &certFile, certFiles) {
        // trying both formats is easiest to program and most user-friendly if somewhat sloppy
        const int prevCertCount = certs.count();
        certs += QSslCertificate::fromPath(certFile, QSsl::Pem);
        if (prevCertCount == certs.count()) {
            certs += QSslCertificate::fromPath(certFile, QSsl::Der);
        }
        if (prevCertCount == certs.count()) {
            // qDebug() << "failed to load certificate file" << certFile;
        }
    }

    bool didAddCertificates = false;
    foreach (const QSslCertificate &cert, certs) {
        KSslCaCertificate caCert(cert, KSslCaCertificate::UserStore, false);
        if (!addCertificateItem(caCert)) {
            // ### tell the user?
        } else {
            didAddCertificates = true;
        }
    }
    if (didAddCertificates) {
        emit changed(true);
    }
}

// private slot
void CaCertificatesPage::itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)

    if (m_blockItemChanged) {
        return;
    }
    // qDebug();
    // we could try to emit changed(false) if everything was changed back to status quo

    // a click on the checkbox of an unselected item first invokes itemSelectionChanged(),
    // then itemChanged(). we'll have to rerun the checks in itemSelectionChanged().
    itemSelectionChanged();
    emit changed(true);
}

static QTreeWidgetItem *findImmediateChild(QTreeWidgetItem *parent, const QString &issuerText)
{
    for (int i = 0; i < parent->childCount(); i ++) {
        QTreeWidgetItem *candidate = parent->child(i);
        if (candidate->text(OrgCnColumn) == issuerText) {
            return candidate;
        }
    }
    return 0;
}

bool CaCertificatesPage::addCertificateItem(const KSslCaCertificate &caCert)
{
    if (m_knownCertificates.contains(caCert.certHash)) {
        // qDebug() << "CaCertificatesPage::addCertificateItem(): refusing duplicate";
        return false;
    }
    const bool prevBlockItemChanged = m_blockItemChanged;
    m_blockItemChanged = true;
    QTreeWidgetItem *grandParent = caCert.store == KSslCaCertificate::SystemStore ?
                                            m_systemCertificatesParent : m_userCertificatesParent;
    const QString issuerOrganization = nonemptyIssuer(caCert.cert);

    QTreeWidgetItem *parent = findImmediateChild(grandParent, issuerOrganization);
    if (!parent) {
        parent = new QTreeWidgetItem(grandParent);
        parent->setText(OrgCnColumn, issuerOrganization);
        parent->setText(HiddenSortColumn, issuerOrganization.toLower());
        parent->setExpanded(true);
        parent->setFlags(parent->flags() & ~Qt::ItemIsSelectable);
    }

    (void) new CaCertificateItem(parent, caCert.cert, !caCert.isBlacklisted);
    m_knownCertificates.insert(caCert.certHash);
    m_blockItemChanged = prevBlockItemChanged;
    return true;
}

