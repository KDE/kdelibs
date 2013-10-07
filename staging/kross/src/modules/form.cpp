/***************************************************************************
 * form.cpp
 * This file is part of the KDE project
 * copyright (C)2006-2007 by Sebastian Sauer (mail@dipe.org)
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

#include "form.h"
#include <QDomDocument>

#include <QByteRef>
#include <QBuffer>
#include <QRegExp>
#include <QFile>
#include <QArgument>
#include <QMetaEnum>
#include <QAction>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QDialog>
#include <QBoxLayout>
#include <QStackedLayout>
#include <QSizePolicy>
#include <QApplication>
#include <QProgressBar>
//#include <QProgressDialog>
#include <QPushButton>
#include <QTextBrowser>
#include <QUiLoader>
#include <QTextCursor>
#include <QTextBlock>
#include <QTime>
#include <QUrl>

#include <QDebug>
#include <klocalizedstring.h>
//#include <kurlcombobox.h>
//#include <kdiroperator.h>
#include <kmessagebox.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kparts/part.h>
//#include <kio/netaccess.h>
//#include <klocalizedstring.h>
//#include <kmimetype.h>
//
#include <kfilewidget.h>
#include <kurlcombobox.h>
#include <ksqueezedtextlabel.h>

extern "C"
{
    Q_DECL_EXPORT QObject* krossmodule()
    {
        return new Kross::FormModule();
    }
}

using namespace Kross;

/*********************************************************************************
 * FormList
 */

FormListView::FormListView(QWidget* parent) : QListWidget(parent) {}
FormListView::~FormListView() {}
void FormListView::clear() { QListWidget::clear(); }
void FormListView::remove(int index) { delete QListWidget::item(index); }
void FormListView::addItem(const QString& text) { QListWidget::addItem(text); }
int FormListView::count() { return QListWidget::count(); }
int FormListView::current() { return QListWidget::currentRow(); }
void FormListView::setCurrent(int row) { QListWidget::setCurrentRow(row); }
QString FormListView::text(int row) {
    QListWidgetItem *item = QListWidget::item(row);
    return item ? item->text() : QString();
}

/*********************************************************************************
 * FormFileWidget
 */

namespace Kross {

    /// \internal d-pointer class.
    class FormFileWidget::Private
    {
        public:
            KFileWidget* filewidget;
            QString filename;
    };

}

FormFileWidget::FormFileWidget(QWidget* parent, const QString& startDirOrVariable)
    : QWidget(parent), d(new Private())
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    d->filewidget = new KFileWidget(QUrl(startDirOrVariable), this);
    layout->addWidget( d->filewidget );
    //QMetaObject::invokeMethod(d->filewidget, "toggleSpeedbar", Q_ARG(bool,false));
    //KFileDialog::setMode( KFile::File | KFile::LocalOnly );

    // slotOk() emits accepted, accept() emits fileSelected()
    QObject::connect(d->filewidget, SIGNAL(fileSelected(QUrl)), this, SLOT(slotFileSelected(QUrl)));
    QObject::connect(d->filewidget, SIGNAL(fileHighlighted(QUrl)), this, SIGNAL(slotFileHighlighted(QUrl)));
    QObject::connect(d->filewidget, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    QObject::connect(d->filewidget, SIGNAL(filterChanged(QString)), this, SIGNAL(filterChanged(QString)));

//     d->impl->setOperationMode(d->mode);
//     if( d->mimeFilter.count() > 0 )
//         d->impl->setMimeFilter(d->mimeFilter);
//     else if( ! d->filter.isEmpty() )
//         d->impl->setFilter(d->filter);

    if( parent && parent->layout() )
        parent->layout()->addWidget(this);
    setMinimumSize( QSize(480,360) );
}

FormFileWidget::~FormFileWidget()
{
    delete d;
}

void FormFileWidget::setMode(const QString& mode)
{
    QMetaEnum e = metaObject()->enumerator( metaObject()->indexOfEnumerator("Mode") );
    KFileWidget::OperationMode m = (KFileWidget::OperationMode) e.keysToValue(mode.toLatin1().constData());
    d->filewidget->setOperationMode(m);
}

QString FormFileWidget::currentFilter() const
{
    return d->filewidget->currentFilter();
}

void FormFileWidget::setFilter(const QString &filter)
{
    QString f = filter;
    f.replace(QRegExp("([^\\\\]{1,1})/"), "\\1\\/"); // escape '/' chars else KFileDialog assumes they are mimetypes :-/
    d->filewidget->setFilter(f);
}

QString FormFileWidget::currentMimeFilter() const
{
    return d->filewidget->currentMimeFilter();
}

void FormFileWidget::setMimeFilter(const QStringList& filter)
{
    d->filewidget->setMimeFilter(filter);
}

void FormFileWidget::slotFileSelected(const QUrl & fn)
{
    //qDebug()<<fn;
    d->filename = fn.toString();
    emit fileSelected(fn.toString());
}

void FormFileWidget::slotFileHighlighted(const QUrl& fn)
{
    emit fileHighlighted(fn.toString());
}

QString FormFileWidget::selectedFile() const
{
    if ( d->filewidget->operationMode() != KFileWidget::Saving ) {
      d->filewidget->accept();
    } else {
      //qDebug()<<d->filename<<d->filewidget->operationMode();
      if ( d->filename.isEmpty() ) {
        // make KFileWidget create an url for us (including extension if necessary)
        QObject::connect(d->filewidget, SIGNAL(accepted()), d->filewidget, SLOT(accept()));
        d->filewidget->slotOk();
        QObject::disconnect(d->filewidget, SIGNAL(accepted()), d->filewidget, SLOT(accept()));
      }
    }
    //qDebug()<<d->filename;
    QUrl url = QUrl::fromLocalFile( d->filename );
    return url.path(); // strip file:// at least python chokes on it
}

/*********************************************************************************
 * FormProgressDialog
 */

namespace Kross {
    /// \internal d-pointer class.
    class FormProgressDialog::Private
    {
        public:
            QTextBrowser* browser;
            QProgressBar* bar;
            bool gotCanceled;
            QTime time;
            void update() {
                if( time.elapsed() >= 1000 ) {
                    time.restart();
                    qApp->processEvents();
                }
            }
    };
}

FormProgressDialog::FormProgressDialog(const QString& caption, const QString& labelText) : KPageDialog(), d(new Private)
{
    d->gotCanceled = false;
    d->time.start();

    setWindowTitle(caption);
    setFaceType(KPageDialog::Plain);
    buttonBox()->button(QDialogButtonBox::Ok)->setEnabled(false);
    //setWindowModality(Qt::WindowModal);
    setModal(false); //true);
    setMinimumWidth(540);
    setMinimumHeight(400);

    QWidget* widget = new QWidget( this );
    KPageWidgetItem* item = KPageDialog::addPage(widget, QString());
    item->setHeader(labelText);
    //item->setIcon( QIcon::fromTheme(iconname) );
    widget = item->widget();
    QVBoxLayout* layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    widget->setLayout(layout);

    d->browser = new QTextBrowser(this);
    d->browser->setHtml(labelText);
    layout->addWidget(d->browser);

    d->bar = new QProgressBar(this);
    //d->bar->setFormat("%v");
    d->bar->setVisible(false);
    layout->addWidget(d->bar);

    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );
    show();
    qApp->processEvents();
}

FormProgressDialog::~FormProgressDialog()
{
    delete d;
}

void FormProgressDialog::setValue(int progress)
{
    if( progress < 0 ) {
        if( d->bar->isVisible() ) {
            d->bar->setVisible(false);
            d->bar->setValue(0);
            qApp->processEvents();
        }
        return;
    }
    if( ! d->bar->isVisible() )
        d->bar->setVisible(true);
    d->bar->setValue(progress);
    d->update();
}

void FormProgressDialog::setRange(int minimum, int maximum)
{
    d->bar->setRange(minimum, maximum);
}

void FormProgressDialog::setText(const QString& text)
{
    d->browser->setHtml(text);
    d->update();
}

void FormProgressDialog::addText(const QString& text)
{
    QTextCursor cursor( d->browser->document()->end() );
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertHtml(text);
    d->browser->moveCursor(QTextCursor::End);
    d->browser->ensureCursorVisible();
    d->update();
}

void FormProgressDialog::done(int r)
{
    if( r == Rejected && ! d->gotCanceled ) {
        if( KMessageBox::messageBox(this, KMessageBox::WarningContinueCancel, i18n("Cancel?")) == KMessageBox::Continue ) {
            d->gotCanceled = true;
            buttonBox()->button(QDialogButtonBox::Cancel)->setEnabled(false);
            emit canceled();
        }
        return;
    }
    KPageDialog::done(r);
}

int FormProgressDialog::exec()
{
    buttonBox()->button(QDialogButtonBox::Ok)->setEnabled(true);
    buttonBox()->button(QDialogButtonBox::Cancel)->setEnabled(false);
    if( d->bar->isVisible() )
        d->bar->setValue( d->bar->maximum() );
    return QDialog::exec();
}

bool FormProgressDialog::isCanceled()
{
    return d->gotCanceled;
}

/*********************************************************************************
 * FormDialog
 */

namespace Kross {

    /// \internal d-pointer class.
    class FormDialog::Private
    {
        public:
            QDialogButtonBox::StandardButton buttoncode;
            QHash<QString, KPageWidgetItem*> items;
    };

}

FormDialog::FormDialog(const QString& caption)
    : KPageDialog( /*0, Qt::WShowModal | Qt::WDestructiveClose*/ )
    , d( new Private() )
{
    setWindowTitle(caption);
    buttonBox()->setStandardButtons(QDialogButtonBox::Ok);
    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

    connect(buttonBox(), SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton*)));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            this, SLOT(slotCurrentPageChanged(KPageWidgetItem*)));
}

FormDialog::~FormDialog()
{
    qWarning()<<"dtor";
    delete d;
}

bool FormDialog::setButtons(const QString& buttons)
{
    int i = buttonBox()->metaObject()->indexOfEnumerator("StandardButton");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = buttonBox()->metaObject()->enumerator(i);
    int v = e.keysToValue(buttons.toUtf8().constData());
    if( v < 0 )
        return false;
    buttonBox()->setStandardButtons((QDialogButtonBox::StandardButton) v);
    return true;
}

bool FormDialog::setButtonText(const QString& button, const QString& text)
{
    int i = buttonBox()->metaObject()->indexOfEnumerator("StandardButton");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = buttonBox()->metaObject()->enumerator(i);
    int v = e.keysToValue(button.toUtf8().constData());
    if( v < 0 )
        return false;
    QPushButton *pushButton = buttonBox()->button((QDialogButtonBox::StandardButton) v);
    if (!pushButton)
        return false;
    pushButton->setText(text);
    return true;
}

bool FormDialog::setFaceType(const QString& facetype)
{
    int i = KPageView::staticMetaObject.indexOfEnumerator("FaceType");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = KPageView::staticMetaObject.enumerator(i);
    int v = e.keysToValue(facetype.toUtf8().constData());
    if( v < 0 )
        return false;
    KPageDialog::setFaceType( (KPageDialog::FaceType) v );
    return true;
}

QString FormDialog::currentPage() const
{
    KPageWidgetItem* item = KPageDialog::currentPage();
    return item ? item->name() : QString();
}

bool FormDialog::setCurrentPage(const QString& name)
{
    if( ! d->items.contains(name) )
        return false;
    KPageDialog::setCurrentPage( d->items[name] );
    return true;
}

QWidget* FormDialog::page(const QString& name) const
{
    return d->items.contains(name) ? d->items[name]->widget() : 0;
}

//shared by FormDialog and FormAssistant
static KPageWidgetItem* formAddPage(KPageDialog* dialog, const QString& name, const QString& header, const QString& iconname)
{
    QWidget* widget = new QWidget( dialog );
    QVBoxLayout* boxlayout = new QVBoxLayout(widget);
    boxlayout->setSpacing(0);
    boxlayout->setMargin(0);
    widget->setLayout(boxlayout);

    KPageWidgetItem* item = dialog->addPage(widget, name);
    item->setHeader(header.isNull() ? name : header);
    if( ! iconname.isEmpty() )
        item->setIcon( QIcon::fromTheme(iconname) );
    //d->items.insert(name, item);

    return item;
}

QWidget* FormDialog::addPage(const QString& name, const QString& header, const QString& iconname)
{
    return d->items.insert(name, formAddPage((KPageDialog*)this,name,header,iconname)).value()->widget();
}

QString FormDialog::result()
{
    int i = buttonBox()->metaObject()->indexOfEnumerator("StandardButton");
    if( i < 0 ) {
        qWarning() << "Kross::FormDialog::setButtons No such enumerator \"StandardButton\"";
        return QString();
    }
    QMetaEnum e = buttonBox()->metaObject()->enumerator(i);
    return e.valueToKey(d->buttoncode);
}

void FormDialog::slotButtonClicked(QAbstractButton *button)
{
    d->buttoncode = buttonBox()->standardButton(button);
}

void FormDialog::slotCurrentPageChanged(KPageWidgetItem* current)
{
    Q_UNUSED(current);
    //qDebug() << "FormDialog::slotCurrentPageChanged current=" << current->name();
    //foreach(QWidget* widget, current->widget()->findChildren< QWidget* >("")) widget->setFocus();
}


namespace Kross {
    /// \internal d-pointer class.
    class FormAssistant::Private
    {
        public:
            QDialogButtonBox::StandardButton buttoncode;
            QHash<QString, KPageWidgetItem*> items;
    };
}
FormAssistant::FormAssistant(const QString& caption)
    : KAssistantDialog( /*0, Qt::WShowModal | Qt::WDestructiveClose*/ )
    , d( new Private() )
{
    setWindowTitle(caption);
    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

    connect(buttonBox(), SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton*)));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            this, SLOT(slotCurrentPageChanged(KPageWidgetItem*)));
    /* unlike boost qt does not support defining of slot call order!
    connect(this, SIGNAL(user2Clicked()), this, SIGNAL (nextClicked()));
    connect(this, SIGNAL(user3Clicked()), this, SIGNAL (backClicked()));
    */
}

FormAssistant::~FormAssistant()
{
    delete d;
}

void FormAssistant::showHelpButton(bool show)
{
    QPushButton *helpButton = buttonBox()->button(QDialogButtonBox::Help);
    if (helpButton) {
        helpButton->setVisible(show);
    }
}

void FormAssistant::back()
{
    emit backClicked();
    KAssistantDialog::back();
}
void FormAssistant::next()
{
    emit nextClicked();
    KAssistantDialog::next();
}

QString FormAssistant::currentPage() const
{
    KPageWidgetItem* item = KPageDialog::currentPage();
    return item ? item->name() : QString();
}

bool FormAssistant::setCurrentPage(const QString& name)
{
    if( ! d->items.contains(name) )
        return false;
    KPageDialog::setCurrentPage( d->items[name] );
    return true;
}

QWidget* FormAssistant::page(const QString& name) const
{
    return d->items.contains(name) ? d->items[name]->widget() : 0;
}

QWidget* FormAssistant::addPage(const QString& name, const QString& header, const QString& iconname)
{
    return d->items.insert(name, formAddPage((KPageDialog*)this,name,header,iconname)).value()->widget();
}

bool FormAssistant::isAppropriate (const QString& name) const
{
    return d->items.contains(name) && KAssistantDialog::isAppropriate(d->items[name]);
}
void FormAssistant::setAppropriate (const QString& name, bool appropriate)
{
    if (!d->items.contains(name))
        return;

    KAssistantDialog::setAppropriate(d->items[name],appropriate);
}
bool FormAssistant::isValid (const QString& name) const
{
    return d->items.contains(name) && KAssistantDialog::isValid(d->items[name]);
}
void FormAssistant::setValid (const QString& name, bool enable)
{
    if (!d->items.contains(name))
        return;

    KAssistantDialog::setValid(d->items[name],enable);
}

QString FormAssistant::result()
{
    int i = metaObject()->indexOfEnumerator("AssistantButtonCode");
    if( i < 0 ) {
        qWarning() << "Kross::FormAssistant::setButtons No such enumerator \"AssistantButtonCode\"";
        return QString();
    }
    QMetaEnum e = metaObject()->enumerator(i);
    return e.valueToKey(FormAssistant::AssistantButtonCode(int(d->buttoncode)));
}

void FormAssistant::slotButtonClicked(QAbstractButton *button)
{
    d->buttoncode = buttonBox()->standardButton(button);
}

void FormAssistant::slotCurrentPageChanged(KPageWidgetItem* current)
{
    Q_UNUSED(current);
    //qDebug() << "FormAssistant::slotCurrentPageChanged current=" << current->name();
    //foreach(QWidget* widget, current->widget()->findChildren< QWidget* >("")) widget->setFocus();
}

/*********************************************************************************
 * FormModule
 */

namespace Kross {

    /// \internal extension of the QUiLoader class.
    class UiLoader : public QUiLoader
    {
        public:
            UiLoader() : QUiLoader() {}
            virtual ~UiLoader() {}

            /*
            virtual QAction* createAction(QObject* parent = 0, const QString& name = QString())
            {
            }

            virtual QActionGroup* createActionGroup(QObject* parent = 0, const QString& name = QString())
            {
            }

            virtual QLayout* createLayout(const QString& className, QObject* parent = 0, const QString& name = QString())
            {
            }

            virtual QWidget* createWidget(const QString& className, QWidget* parent = 0, const QString& name = QString())
            {
            }
            */
    };

    /// \internal d-pointer class.
    class FormModule::Private
    {
        public:
    };

}

FormModule::FormModule()
    : QObject()
    , d( new Private() )
{
}

FormModule::~FormModule()
{
    delete d;
}

QWidget* FormModule::activeModalWidget()
{
    return QApplication::activeModalWidget();
}

QWidget* FormModule::activeWindow()
{
    return QApplication::activeWindow();
}

QString FormModule::showMessageBox(const QString& dialogtype, const QString& caption, const QString& message, const QString& details)
{
    KMessageBox::DialogType type;
    if(dialogtype == "Error") {
        if( ! details.isNull() ) {
            KMessageBox::detailedError(0, message, details, caption);
            return QString();
        }
        type = KMessageBox::Error;
    }
    else if(dialogtype == "Sorry") {
        if( ! details.isNull() ) {
            KMessageBox::detailedSorry(0, message, details, caption);
            return QString();
        }
        type = KMessageBox::Sorry;
    }
    else if(dialogtype == "QuestionYesNo") type = KMessageBox::QuestionYesNo;
    else if(dialogtype == "WarningYesNo") type = KMessageBox::WarningYesNo;
    else if(dialogtype == "WarningContinueCancel") type = KMessageBox::WarningContinueCancel;
    else if(dialogtype == "WarningYesNoCancel") type = KMessageBox::WarningYesNoCancel;
    else if(dialogtype == "QuestionYesNoCancel") type = KMessageBox::QuestionYesNoCancel;
    else /*if(dialogtype == "Information")*/ type = KMessageBox::Information;
    switch( KMessageBox::messageBox(0, type, message, caption) ) {
        case KMessageBox::Ok: return "Ok";
        case KMessageBox::Cancel: return "Cancel";
        case KMessageBox::Yes: return "Yes";
        case KMessageBox::No: return "No";
        case KMessageBox::Continue: return "Continue";
        default: break;
    }
    return QString();
}

QWidget* FormModule::showProgressDialog(const QString& caption, const QString& labelText)
{
    return new FormProgressDialog(caption, labelText);
}

QWidget* FormModule::createDialog(const QString& caption)
{
    return new FormDialog(caption);
}

QWidget* FormModule::createAssistant(const QString& caption)
{
    return new FormAssistant(caption);
}

QObject* FormModule::createLayout(QWidget* parent, const QString& layout)
{
    QLayout* l = 0;
    if( layout == "QVBoxLayout" )
        l = new QVBoxLayout();
    else if( layout == "QHBoxLayout" )
        l = new QHBoxLayout();
    else if( layout == "QStackedLayout" )
        l = new QStackedLayout();
    if( parent && l )
        parent->setLayout(l);
    return l;
}

QWidget* FormModule::createWidget(const QString& className)
{
    UiLoader loader;
    QWidget* widget = loader.createWidget(className);
    return widget;
}

QWidget* FormModule::createWidget(QWidget* parent, const QString& className, const QString& name)
{
    UiLoader loader;
    QWidget* widget = loader.createWidget(className, parent, name);
    if( parent && parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}


QString FormModule::tr(const QString& str)
{
    return tr(str.toUtf8().constData());
}
QString FormModule::tr(const QString& str, const QString& comment)
{
    return tr(str.toUtf8().constData(), comment.toUtf8().constData());
}

QWidget* FormModule::createWidgetFromUI(QWidget* parent, const QString& xml)
{
    QUiLoader loader;

    QDomDocument doc("mydocument");
    doc.setContent(xml.toUtf8());

    QDomNodeList strings=doc.elementsByTagName("string");
    int i=strings.size();
    while(--i>=0)
    {
        QDomElement e=strings.at(i).toElement();
        QString i18nd=e.attribute("comment").isEmpty()? tr(e.text()) : tr(e.text(), e.attribute("comment"));
        if (i18nd==e.text())
            continue;
        QDomNode n = e.firstChild();
        while (!n.isNull())
        {
            QDomNode nn=n.nextSibling();
            if (n.isCharacterData())
                e.removeChild(n);
            n = nn;
        }
        e.appendChild(e.ownerDocument().createTextNode(i18nd));
    }

    QByteArray ba = doc.toByteArray();
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadOnly);

    QWidget* widget = loader.load(&buffer, parent);
    if( widget && parent && parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}

QWidget* FormModule::createWidgetFromUIFile(QWidget* parent, const QString& filename)
{
    QFile file(filename);
    if( ! file.exists() ) {
        // qDebug() << QString("Kross::FormModule::createWidgetFromUIFile: There exists no such file \"%1\"").arg(filename);
        return 0;
    }
    if( ! file.open(QFile::ReadOnly) ) {
        // qDebug() << QString("Kross::FormModule::createWidgetFromUIFile: Failed to open the file \"%1\"").arg(filename);
        return 0;
    }
    const QString xml = file.readAll();
    file.close();
    return createWidgetFromUI(parent, xml);
}

QWidget* FormModule::createFileWidget(QWidget* parent, const QString& startDirOrVariable)
{
    FormFileWidget* widget = new FormFileWidget(parent, startDirOrVariable);
    if( parent && parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}

QWidget* FormModule::createListView(QWidget* parent)
{
    FormListView* widget = new FormListView(parent);
    if( parent && parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}

QAction* FormModule::createAction(QObject* parent)
{
    return new QAction(parent);
}

QObject* FormModule::loadPart(QWidget* parent, const QString& name, const QUrl& url)
{
    //name e.g. "libkghostview"
    KPluginFactory* factory = KPluginLoader( name.toLatin1() ).factory();
    if( ! factory ) {
        qWarning() << QString("Kross::FormModule::loadPart: No such library \"%1\"").arg(name);
        return 0;
    }
    KParts::ReadOnlyPart* part = factory->create< KParts::ReadOnlyPart >( parent );
    if( ! part ) {
        qWarning() << QString("Kross::FormModule::loadPart: Library \"%1\" is not a KPart").arg(name);
        return 0;
    }
    if( url.isValid() )
        part->openUrl(url);
    if( parent && parent->layout() && part->widget() )
        parent->layout()->addWidget( part->widget() );
    return part;
}

