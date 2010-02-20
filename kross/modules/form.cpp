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

#include <QtCore/QByteRef>
#include <QtCore/QBuffer>
#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QArgument>
#include <QtCore/QMetaEnum>
#include <QtGui/QKeyEvent>
#include <QtGui/QDialog>
#include <QtGui/QBoxLayout>
#include <QtGui/QStackedLayout>
#include <QtGui/QSizePolicy>
#include <QtGui/QApplication>
#include <QtGui/QProgressBar>
//#include <QtGui/QProgressDialog>
#include <QtGui/QTextBrowser>
#include <QUiLoader>
#include <QTextCursor>
#include <QTextBlock>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kpushbutton.h>
//#include <kurlcombobox.h>
//#include <kdiroperator.h>
//#include <kshell.h>
#include <kicon.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kparts/part.h>
//#include <kio/netaccess.h>
//#include <klocale.h>
//#include <kmimetype.h>
//#include <kstandarddirs.h>
#include <kfilewidget.h>
#include <kurlcombobox.h>
#include <kshell.h>
#include <widgets/ksqueezedtextlabel.h>

extern "C"
{
    KDE_EXPORT QObject* krossmodule()
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

    d->filewidget = new KFileWidget(KUrl(startDirOrVariable), this);
    layout->addWidget( d->filewidget );
    //QMetaObject::invokeMethod(d->filewidget, "toggleSpeedbar", Q_ARG(bool,false));
    //KFileDialog::setMode( KFile::File | KFile::LocalOnly );

    // slotOk() emits accepted, accept() emits fileSelected()
    QObject::connect(d->filewidget, SIGNAL(fileSelected(const QString&)), this, SLOT(slotFileSelected(const QString&)));

    QObject::connect(d->filewidget, SIGNAL(fileSelected(const QString&)), this, SIGNAL(fileSelected(const QString&)));
    QObject::connect(d->filewidget, SIGNAL(fileHighlighted(const QString&)), this, SIGNAL(fileHighlighted(const QString&)));
    QObject::connect(d->filewidget, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    QObject::connect(d->filewidget, SIGNAL(filterChanged(const QString&)), this, SIGNAL(filterChanged(const QString&)));

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
    KFileWidget::OperationMode m = (KFileWidget::OperationMode) e.keysToValue( mode.toLatin1() );
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

void FormFileWidget::slotFileSelected( const QString & fn )
{
    //kDebug()<<fn;
    d->filename = fn;
}

QString FormFileWidget::selectedFile() const
{
    if ( d->filewidget->operationMode() != KFileWidget::Saving ) {
      d->filewidget->accept();
    } else {
      //kDebug()<<d->filename<<d->filewidget->operationMode();
      if ( d->filename.isEmpty() ) {
        // make KFileWidget create an url for us (including extension if necessary)
        QObject::connect(d->filewidget, SIGNAL(accepted()), d->filewidget, SLOT(accept()));
        d->filewidget->slotOk();
        QObject::disconnect(d->filewidget, SIGNAL(accepted()), d->filewidget, SLOT(accept()));
      }
    }
    //kDebug()<<d->filename;
    KUrl url( d->filename );
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

    setCaption(caption);
    KDialog::setButtons(KDialog::Ok|KDialog::Cancel);
    setFaceType(KPageDialog::Plain);
    enableButton(KDialog::Ok, false);
    //setWindowModality(Qt::WindowModal);
    setModal(false); //true);
    setMinimumWidth(540);
    setMinimumHeight(400);

    QWidget* widget = new QWidget( mainWidget() );
    KPageWidgetItem* item = KPageDialog::addPage(widget, QString());
    item->setHeader(labelText);
    //item->setIcon( KIcon(iconname) );
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
            enableButton(KDialog::Cancel, false);
            emit canceled();
        }
        return;
    }
    KPageDialog::done(r);
}

int FormProgressDialog::exec()
{
    enableButton(KDialog::Ok, true);
    enableButton(KDialog::Cancel, false);
    if( d->bar->isVisible() )
        d->bar->setValue( d->bar->maximum() );
    return KDialog::exec();
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
            KDialog::ButtonCode buttoncode;
            QHash<QString, KPageWidgetItem*> items;
    };

}

FormDialog::FormDialog(const QString& caption)
    : KPageDialog( /*0, Qt::WShowModal | Qt::WDestructiveClose*/ )
    , d( new Private() )
{
    setCaption(caption);
    KDialog::setButtons(KDialog::Ok);
    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            this, SLOT(slotCurrentPageChanged(KPageWidgetItem*)));
}

FormDialog::~FormDialog()
{
    kWarning()<<"dtor";
    delete d;
}

bool FormDialog::setButtons(const QString& buttons)
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = metaObject()->enumerator(i);
    int v = e.keysToValue( buttons.toUtf8() );
    if( v < 0 )
        return false;
    KDialog::setButtons( (KDialog::ButtonCode) v );
    return true;
}

bool FormDialog::setButtonText(const QString& button, const QString& text)
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = metaObject()->enumerator(i);
    int v = e.keysToValue( button.toUtf8() );
    if( v < 0 )
        return false;
    KDialog::setButtonText( (KDialog::ButtonCode) v, text);
    return true;
}

bool FormDialog::setFaceType(const QString& facetype)
{
    int i = KPageView::staticMetaObject.indexOfEnumerator("FaceType");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = KPageView::staticMetaObject.enumerator(i);
    int v = e.keysToValue( facetype.toUtf8() );
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
    QWidget* widget = new QWidget( dialog->mainWidget() );
    QVBoxLayout* boxlayout = new QVBoxLayout(widget);
    boxlayout->setSpacing(0);
    boxlayout->setMargin(0);
    widget->setLayout(boxlayout);

    KPageWidgetItem* item = dialog->addPage(widget, name);
    item->setHeader(header.isNull() ? name : header);
    if( ! iconname.isEmpty() )
        item->setIcon( KIcon(iconname) );
    //d->items.insert(name, item);

    return item;
}

QWidget* FormDialog::addPage(const QString& name, const QString& header, const QString& iconname)
{
    return d->items.insert(name, formAddPage((KPageDialog*)this,name,header,iconname)).value()->widget();
}

void FormDialog::setMainWidget(QWidget *newMainWidget)
{
    KDialog::setMainWidget(newMainWidget);
}

QString FormDialog::result()
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    if( i < 0 ) {
        kWarning() << "Kross::FormDialog::setButtons No such enumerator \"ButtonCode\"";
        return QString();
    }
    QMetaEnum e = metaObject()->enumerator(i);
    return e.valueToKey(d->buttoncode);
}

void FormDialog::slotButtonClicked(int button)
{
    d->buttoncode = (KDialog::ButtonCode) button;
    KDialog::slotButtonClicked(button);
}

void FormDialog::slotCurrentPageChanged(KPageWidgetItem* current)
{
    Q_UNUSED(current);
    //kDebug() << "FormDialog::slotCurrentPageChanged current=" << current->name();
    //foreach(QWidget* widget, current->widget()->findChildren< QWidget* >("")) widget->setFocus();
}


namespace Kross {
    /// \internal d-pointer class.
    class FormAssistant::Private
    {
        public:
            KDialog::ButtonCode buttoncode;
            QHash<QString, KPageWidgetItem*> items;
    };
}
FormAssistant::FormAssistant(const QString& caption)
    : KAssistantDialog( /*0, Qt::WShowModal | Qt::WDestructiveClose*/ )
    , d( new Private() )
{
    setCaption(caption);
    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

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
    showButton(KDialog::Help, show);
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
        kWarning() << "Kross::FormAssistant::setButtons No such enumerator \"AssistantButtonCode\"";
        return QString();
    }
    QMetaEnum e = metaObject()->enumerator(i);
    return e.valueToKey(FormAssistant::AssistantButtonCode(int(d->buttoncode)));
}

void FormAssistant::slotButtonClicked(int button)
{
    d->buttoncode = (KDialog::ButtonCode) button;
    KDialog::slotButtonClicked(button);
}

void FormAssistant::slotCurrentPageChanged(KPageWidgetItem* current)
{
    Q_UNUSED(current);
    //kDebug() << "FormAssistant::slotCurrentPageChanged current=" << current->name();
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
    return QObject::tr(str.toUtf8());
}
QString FormModule::tr(const QString& str, const QString& comment)
{
    return QObject::tr(str.toUtf8(),comment.toUtf8());
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
        QString i18nd=e.attribute("comment").isEmpty()?QObject::tr(e.text().toUtf8()):QObject::tr(e.text().toUtf8(),e.attribute("comment").toUtf8());
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
        kDebug() << QString("Kross::FormModule::createWidgetFromUIFile: There exists no such file \"%1\"").arg(filename);
        return false;
    }
    if( ! file.open(QFile::ReadOnly) ) {
        kDebug() << QString("Kross::FormModule::createWidgetFromUIFile: Failed to open the file \"%1\"").arg(filename);
        return false;
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
        kWarning() << QString("Kross::FormModule::loadPart: No such library \"%1\"").arg(name);
        return 0;
    }
    KParts::ReadOnlyPart* part = factory->create< KParts::ReadOnlyPart >( parent );
    if( ! part ) {
        kWarning() << QString("Kross::FormModule::loadPart: Library \"%1\" is not a KPart").arg(name);
        return 0;
    }
    if( url.isValid() )
        part->openUrl(url);
    if( parent && parent->layout() && part->widget() )
        parent->layout()->addWidget( part->widget() );
    return part;
}

#include "form.moc"
