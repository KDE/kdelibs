/***************************************************************************
 * formfile.cpp
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "formfile.h"

//#include <QByteArray>
//#include <QBuffer>
//#include <QRegExp>
#include <QFile>
//#include <QMetaObject>
#include <QMetaEnum>
//#include <QKeyEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
//#include <QStackedLayout>
#include <QSizePolicy>
#include <QApplication>

#include <kdebug.h>
#include <kurl.h>
#include <kpushbutton.h>
#include <kurlcombobox.h>
#include <kdiroperator.h>
#include <kshell.h>
#include <kicon.h>
#include <kurlbar.h>
//#include <kaction.h>
#include <kactioncollection.h>
//#include <kmessagebox.h>

using namespace Kross;

namespace Kross {

    /// \internal implementation of the customized KFileDialog
    class FormFileWidgetImpl : public KFileDialog
    {
        public:
            FormFileWidgetImpl(QWidget* parent, const QString& startDirOrVariable)
                : KFileDialog(KUrl(startDirOrVariable), "", parent, 0)
            {
                setModal( false );
                setParent( parent, windowFlags() & ~Qt::WindowType_Mask );
                setGeometry(0, 0, width(), height());
                setFocusProxy( locationEdit );
                //setMinimumSize( QSize(width(), height()) );
                setMinimumSize( QSize(480,360) );

                if( layout() )
                    layout()->setMargin(0);
                if( parent->layout() )
                    parent->layout()->addWidget(this);

                if( KFileDialog::okButton() )
                    KFileDialog::okButton()->setVisible(false);
                if( KFileDialog::cancelButton() )
                    KFileDialog::cancelButton()->setVisible(false);

                KFileDialog::setMode( KFile::File | KFile::LocalOnly );

                if( actionCollection() ) {
                    QAction* a = actionCollection()->action("toggleSpeedbar");
                    if( a && a->isCheckable() && a->isChecked() ) a->toggle();
                    a = actionCollection()->action("toggleBookmarks");
                    if( a && a->isCheckable() && a->isChecked() ) a->toggle();
                }
            }

            virtual ~FormFileWidgetImpl() {}

            QString selectedFile() const
            {
                KUrl selectedUrl;
                QString locationText = locationEdit->currentText();
                if( locationText.contains( '/' ) ) { // relative path? -> prepend the current directory
                    KUrl u( ops->url(), KShell::tildeExpand(locationText) );
                    selectedUrl = u.isValid() ? u : selectedUrl = ops->url();
                }
                else { // simple filename -> just use the current URL
                    selectedUrl = ops->url();
                }

                QFileInfo fi( selectedUrl.path(), locationEdit->currentText() );
                return fi.absoluteFilePath();
            }

            virtual void accept()
            {
                kDebug() << "FormFileWidget::accept m_file=" << selectedFile() << endl;
                setResult( QDialog::Accepted );
                //emit fileSelected( selectedFile() );
                //KFileDialog::accept();
            }

            virtual void reject()
            {
                kDebug() << "FormFileWidget::reject" << endl;
                /*TODO
                for(QWidget* parent = parentWidget(); parent; parent = parent->parentWidget()) {
                    FormDialog* dialog = qobject_cast<FormDialog*>(parent);
                    if( dialog ) {
                        dialog->reject();
                        break;
                    }
                    if( parent == QApplication::activeModalWidget() || parent == QApplication::activeWindow() )
                        break;
                }
                //KFileDialog::reject();
                */
            }
    };

    /// \internal d-pointer class.
    class FormFileWidget::Private
    {
        public:
            FormFileWidgetImpl* impl;

            QString startDirOrVariable;
            KFileDialog::OperationMode mode;
            QString currentFilter;
            QString filter;
            QString currentMimeFilter;
            QStringList mimeFilter;
            QString selectedFile;

            Private(const QString& startDirOrVariable) : impl(0), startDirOrVariable(startDirOrVariable) {}
    };

}

FormFileWidget::FormFileWidget(QWidget* parent, const QString& startDirOrVariable)
    : QWidget(parent)
    , d( new Private(startDirOrVariable) )
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

FormFileWidget::~FormFileWidget()
{
    delete d;
}

void FormFileWidget::setMode(const QString& mode)
{
    QMetaEnum e = metaObject()->enumerator( metaObject()->indexOfEnumerator("Mode") );
    d->mode = (KFileDialog::OperationMode) e.keysToValue(mode.toLatin1());
    if( d->impl )
        d->impl->setOperationMode(d->mode);
}

QString FormFileWidget::currentFilter() const
{
    return d->impl ? d->impl->currentFilter() : d->currentFilter;
}

void FormFileWidget::setFilter(QString filter)
{
    filter.replace(QRegExp("([^\\\\]{1,1})/"), "\\1\\/"); // escape '/' chars else KFileDialog assumes they are mimetypes :-/
    d->filter = filter;
    if( d->impl )
        d->impl->setFilter(d->filter);
}

QString FormFileWidget::currentMimeFilter() const
{
    return d->impl ? d->impl->currentMimeFilter() : d->currentMimeFilter;
}

void FormFileWidget::setMimeFilter(const QStringList& filter)
{
    d->mimeFilter = filter;
    if( d->impl )
        d->impl->setMimeFilter(d->mimeFilter);
}

QString FormFileWidget::selectedFile() const
{
    return d->impl ? d->impl->selectedFile() : d->selectedFile;
}

void FormFileWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if( ! d->impl ) {
        d->impl = new FormFileWidgetImpl(this, d->startDirOrVariable);
        d->impl->setOperationMode(d->mode);
        if( d->mimeFilter.count() > 0 )
            d->impl->setMimeFilter(d->mimeFilter);
        else if( ! d->filter.isEmpty() )
            d->impl->setFilter(d->filter);
        QObject::connect(d->impl, SIGNAL(fileSelected(const QString&)), this, SIGNAL(fileSelected(const QString&)));
        QObject::connect(d->impl, SIGNAL(fileHighlighted(const QString&)), this, SIGNAL(fileHighlighted(const QString&)));
        QObject::connect(d->impl, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
        QObject::connect(d->impl, SIGNAL(filterChanged(const QString&)), this, SIGNAL(filterChanged(const QString&)));
        d->impl->show();
    }
}

void FormFileWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    /*
    if( d->impl ) {
        d->selectedFile = selectedFile();
        d->currentFilter = currentFilter();
        d->currentMimeFilter = currentMimeFilter();
        delete d->impl; d->impl = 0;
    }
    */
}

#include "formfile.moc"
