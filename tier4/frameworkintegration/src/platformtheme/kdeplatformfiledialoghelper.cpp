/*  This file is part of the KDE libraries
 *  Copyright 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License or ( at
 *  your option ) version 3 or, at the discretion of KDE e.V. ( which shall
 *  act as a proxy as in section 14 of the GPLv3 ), any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kdeplatformfiledialoghelper.h"
#include <kfilefiltercombo.h>
#include <kfilewidget.h>
#include <klocalizedstring.h>
#include <kdiroperator.h>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QTextStream>
#include <QEventLoop>

namespace
{

/*
 * Map a Qt filter string into a KDE one.
 */
static QString qt2KdeFilter(const QStringList &f)
{
    QString               filter;
    QTextStream           str(&filter, QIODevice::WriteOnly);
    QStringList           list(f);
    list.replaceInStrings("/", "\\/");
    QStringList::const_iterator it(list.begin()), end(list.end());
    bool                  first=true;

    for (; it!=end; ++it)
    {
        int ob=it->lastIndexOf('('),
        cb=it->lastIndexOf(')');

        if (-1!=cb && ob<cb)
        {
            if (first)
                first=false;
            else
                str << '\n';
            str << it->mid(ob+1, (cb-ob)-1) << '|' << it->mid(0, ob);
        }
    }

    return filter;
}

/*
 * Map a KDE filter string into a Qt one.
 */
static QString kde2QtFilter(const QStringList &list, const QString &kde)
{
    QStringList::const_iterator it(list.begin()), end(list.end());
    int                   pos;
    QString               sel;

    for (; it!=end; ++it) {
        if (-1!=(pos=it->indexOf(kde)) && pos>0 &&
            ('('==(*it)[pos-1] || ' '==(*it)[pos-1]) &&
            it->length()>=kde.length()+pos &&
            (')'==(*it)[pos+kde.length()] || ' '==(*it)[pos+kde.length()]))
        {
            return *it;
        }
    }
    return QString();
}
}

KDEPlatformFileDialog::KDEPlatformFileDialog()
    : QDialog()
    , m_fileWidget(new KFileWidget(QUrl(), this))
{
    setLayout(new QVBoxLayout);
    connect(m_fileWidget, SIGNAL(selectionChanged()), SLOT(selectionChanged()));
    connect(m_fileWidget, SIGNAL(fileHighlighted(QUrl)), SLOT(selectionChanged()));
    connect(m_fileWidget, SIGNAL(filterChanged(QString)), SIGNAL(filterSelected(QString)));
    connect(m_fileWidget, SIGNAL(accepted()), SLOT(accept()));
    layout()->addWidget(m_fileWidget);

    m_buttons = new QDialogButtonBox(this);
    m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttons, SIGNAL(accepted()), m_fileWidget, SLOT(slotOk()));
    connect(m_buttons, SIGNAL(rejected()), m_fileWidget, SLOT(slotCancel()));
    connect(m_buttons, SIGNAL(rejected()), SLOT(reject()));
    layout()->addWidget(m_buttons);

    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(!m_fileWidget->selectedUrls().isEmpty());
}

void KDEPlatformFileDialog::selectionChanged()
{
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(!m_fileWidget->dirOperator()->selectedItems().isEmpty());
}

QUrl KDEPlatformFileDialog::directory()
{
    return m_fileWidget->baseUrl();
}

QList<QUrl> KDEPlatformFileDialog::selectedFiles()
{
    QList<QUrl> ret;
    KFileItemList items = m_fileWidget->dirOperator()->selectedItems();
    Q_FOREACH(const KFileItem& item, items) {
        ret += item.url();
    }
    return ret;
}

void KDEPlatformFileDialog::selectFile(const QUrl &filename)
{
    m_fileWidget->setUrl(filename);
}

QString KDEPlatformFileDialog::selectedNameFilter()
{
    return m_fileWidget->filterWidget()->currentFilter();
}

void KDEPlatformFileDialog::selectNameFilter(const QString &filter)
{
    m_fileWidget->filterWidget()->setCurrentFilter(filter);
}

void KDEPlatformFileDialog::setDirectory(const QUrl &directory)
{
    m_fileWidget->setStartDir(directory);
}

////////////////////////////////////////////////

KDEPlatformFileDialogHelper::KDEPlatformFileDialogHelper()
    : QPlatformFileDialogHelper()
    , m_dialog(new KDEPlatformFileDialog)
{
    connect(m_dialog, SIGNAL(currentChanged(QUrl)), SIGNAL(currentChanged(QUrl)));
    connect(m_dialog, SIGNAL(directoryEntered(QUrl)), SIGNAL(directoryEntered(QUrl)));
    connect(m_dialog, SIGNAL(fileSelected(QUrl)), SIGNAL(fileSelected(QUrl)));
    connect(m_dialog, SIGNAL(filesSelected(QList<QUrl>)), SIGNAL(filesSelected(QList<QUrl>)));
    connect(m_dialog, SIGNAL(filterSelected(QString)), SIGNAL(filterSelected(QString)));
    connect(m_dialog, SIGNAL(accepted()), SIGNAL(accept()));
    connect(m_dialog, SIGNAL(rejected()), SIGNAL(reject()));
}

KDEPlatformFileDialogHelper::~KDEPlatformFileDialogHelper()
{
    delete m_dialog;
}

void KDEPlatformFileDialogHelper::initializeDialog()
{
    m_dialog->m_fileWidget->setOperationMode(options()->acceptMode()==QFileDialogOptions::AcceptOpen ? KFileWidget::Opening : KFileWidget::Saving);
    if (options()->windowTitle().isEmpty())
        m_dialog->setWindowTitle(options()->acceptMode() == QFileDialogOptions::AcceptOpen ? i18n("Opening...") : i18n("Saving..."));
    else
        m_dialog->setWindowTitle(options()->windowTitle());
    setDirectory(options()->initialDirectory());

    QStringList filters = options()->mimeTypeFilters();
    if (!filters.isEmpty())
        m_dialog->m_fileWidget->setMimeFilter(filters);

    QStringList nameFilters = options()->nameFilters();
    if (!nameFilters.isEmpty())
        m_dialog->m_fileWidget->setFilter(qt2KdeFilter(nameFilters));
}

void KDEPlatformFileDialogHelper::exec()
{
}

void KDEPlatformFileDialogHelper::hide()
{
    m_dialog->hide();
}

bool KDEPlatformFileDialogHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    initializeDialog();
    m_dialog->setWindowFlags(windowFlags);
    m_dialog->setModal(windowModality != Qt::NonModal);
    if (windowModality == Qt::NonModal)
        m_dialog->show();
    else
        m_dialog->exec();
    return true;
}

QList<QUrl> KDEPlatformFileDialogHelper::selectedFiles() const
{
    return m_dialog->selectedFiles();
}

QString KDEPlatformFileDialogHelper::selectedNameFilter() const
{
    return kde2QtFilter(options()->nameFilters(), m_dialog->selectedNameFilter());
}

QUrl KDEPlatformFileDialogHelper::directory() const
{
    return m_dialog->directory();
}

void KDEPlatformFileDialogHelper::selectFile(const QUrl& filename)
{
    m_dialog->selectFile(filename);
}

void KDEPlatformFileDialogHelper::setDirectory(const QUrl& directory)
{
    m_dialog->setDirectory(directory);
}

void KDEPlatformFileDialogHelper::selectNameFilter(const QString& filter)
{
    m_dialog->selectNameFilter(qt2KdeFilter(QStringList(filter)));
}

void KDEPlatformFileDialogHelper::setFilter()
{
}

bool KDEPlatformFileDialogHelper::defaultNameFilterDisables() const
{
    return false;
}
