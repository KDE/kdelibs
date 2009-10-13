/* This file is part of the KDE libraries
    Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kurlrequester.h"

#include <kcombobox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>
#include <kprotocolmanager.h>
#include <khbox.h>
#include <kstandardshortcut.h>
#include <kdebug.h>

#include <QEvent>
#include <QDrag>
#include <QMimeData>
#include <QAction>
#include <QApplication>

class KUrlDragPushButton : public KPushButton
{
public:
    KUrlDragPushButton( QWidget *parent)
        : KPushButton( parent)
    {
        setDragEnabled( true );
    }
    ~KUrlDragPushButton() {}

    void setURL( const KUrl& url )
    {
        m_urls.clear();
        m_urls.append( url );
    }

protected:
    virtual QDrag *dragObject()
    {
        if (m_urls.isEmpty())
            return 0;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        m_urls.populateMimeData(mimeData);
        drag->setMimeData(mimeData);
        return drag;
    }

private:
    KUrl::List m_urls;

};


class KUrlRequester::KUrlRequesterPrivate
{
public:
    KUrlRequesterPrivate(KUrlRequester *parent)
        : m_parent(parent),
          edit(0),
          combo(0),
          fileDialogMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly)
    {
    }

    ~KUrlRequesterPrivate()
    {
        delete myCompletion;
        delete myFileDialog;
    }

    void init();

    void setText( const QString& text ) {
        if ( combo )
        {
            if (combo->isEditable())
            {
               combo->setEditText( text );
            }
            else
            {
               int i = combo->findText( text );
               if ( i == -1 )
               {
                  combo->addItem( text );
                  combo->setCurrentIndex( combo->count()-1 );
               }
               else
               {
                  combo->setCurrentIndex( i );
               }
            }
        }
        else
        {
            edit->setText( text );
        }
    }

    void connectSignals( QObject *receiver )
    {
        QObject *sender;
        if ( combo )
            sender = combo;
        else
            sender = edit;

        if (combo )
            connect( sender, SIGNAL( editTextChanged( const QString& )),
                     receiver, SIGNAL( textChanged( const QString& )));
        else
            connect( sender, SIGNAL( textChanged( const QString& )),
                     receiver, SIGNAL( textChanged( const QString& )));

        connect( sender, SIGNAL( returnPressed() ),
                 receiver, SIGNAL( returnPressed() ));
        connect( sender, SIGNAL( returnPressed( const QString& ) ),
                 receiver, SIGNAL( returnPressed( const QString& ) ));
    }

    void setCompletionObject( KCompletion *comp )
    {
        if ( combo )
            combo->setCompletionObject( comp );
        else
            edit->setCompletionObject( comp );
    }

    QString text() const {
        return combo ? combo->currentText() : edit->text();
    }

    /**
     * replaces ~user or $FOO, if necessary
     */
    KUrl url() const {
        const QString txt = text();
        KUrlCompletion *comp;
        if ( combo )
            comp = qobject_cast<KUrlCompletion*>(combo->completionObject());
        else
            comp = qobject_cast<KUrlCompletion*>(edit->completionObject());

        if ( comp )
            return KUrl( comp->replacedPath( txt ) );
        else
            return KUrl( txt );
    }

    // slots
    void _k_slotUpdateUrl();
    void _k_slotOpenDialog();
    void _k_slotFileDialogFinished();

    KUrl m_startDir;
    KUrlRequester *m_parent;
    KLineEdit *edit;
    KComboBox *combo;
    KFile::Modes fileDialogMode;
    QString fileDialogFilter;
    KEditListBox::CustomEditor editor;
    KUrlDragPushButton *myButton;
    KFileDialog *myFileDialog;
    KUrlCompletion *myCompletion;
    Qt::WindowModality fileDialogModality;
};



KUrlRequester::KUrlRequester( QWidget *editWidget, QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate(this))
{
    // must have this as parent
    editWidget->setParent( this );
    d->combo = qobject_cast<KComboBox*>( editWidget );
    d->edit = qobject_cast<KLineEdit*>( editWidget );
    if ( d->edit ) {
        d->edit->setClearButtonShown( true );
    }

    d->init();
}


KUrlRequester::KUrlRequester( QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate(this))
{
    d->init();
}


KUrlRequester::KUrlRequester( const KUrl& url, QWidget *parent)
  : KHBox( parent),d(new KUrlRequesterPrivate(this))
{
    d->init();
    setUrl( url );
}

KUrlRequester::~KUrlRequester()
{
    delete d;
}


void KUrlRequester::KUrlRequesterPrivate::init()
{
    m_parent->setMargin(0);
    m_parent->setSpacing(-1); // use default spacing

    myFileDialog = 0L;
    fileDialogModality = Qt::ApplicationModal;

    if ( !combo && !edit ) {
        edit = new KLineEdit( m_parent );
        edit->setClearButtonShown( true );
    }

    QWidget *widget = combo ? (QWidget*) combo : (QWidget*) edit;

    myButton = new KUrlDragPushButton(m_parent);
    myButton->setIcon(KIcon("document-open"));
    int buttonSize = widget->sizeHint().height();
    myButton->setFixedSize(buttonSize, buttonSize);
    myButton->setToolTip(i18n("Open file dialog"));

    m_parent->connect(myButton, SIGNAL(pressed()), SLOT(_k_slotUpdateUrl()));

    widget->installEventFilter( m_parent );
    m_parent->setFocusProxy( widget );
    m_parent->setFocusPolicy(Qt::StrongFocus);

    connectSignals( m_parent );
    m_parent->connect(myButton, SIGNAL(clicked()), m_parent, SLOT(_k_slotOpenDialog()));

    myCompletion = new KUrlCompletion();
    setCompletionObject( myCompletion );

    QAction* openAction = new QAction(m_parent);
    openAction->setShortcut(KStandardShortcut::Open);
    m_parent->connect(openAction, SIGNAL(triggered(bool)), SLOT( _k_slotOpenDialog() ));
}

void KUrlRequester::setUrl( const KUrl& url )
{
    d->setText( url.pathOrUrl() );
}

void KUrlRequester::setPath( const QString& path )
{
    d->setText( path );
}

void KUrlRequester::setText(const QString& text)
{
    d->setText(text);
}

void KUrlRequester::setStartDir(const KUrl& startDir)
{
    d->m_startDir = startDir;
    if (startDir.isLocalFile())
        d->myCompletion->setDir(startDir.toLocalFile());
}

void KUrlRequester::changeEvent(QEvent *e)
{
   if (e->type()==QEvent::WindowTitleChange) {
     if (d->myFileDialog) {
        d->myFileDialog->setCaption(windowTitle());
     }
   }
   KHBox::changeEvent(e);
}

KUrl KUrlRequester::url() const
{
    return d->url();
}

KUrl KUrlRequester::startDir() const
{
    return d->m_startDir;
}

QString KUrlRequester::text() const
{
    return d->text();
}

void KUrlRequester::KUrlRequesterPrivate::_k_slotOpenDialog()
{
    if ( myFileDialog )
        if ( myFileDialog->isVisible() )
        {
            //The file dialog is already being shown, raise it and exit
            myFileDialog->raise();
            myFileDialog->activateWindow();
            return;
        }

    if ( ((fileDialogMode & KFile::Directory) && !(fileDialogMode & KFile::File)) ||
         /* catch possible fileDialog()->setMode( KFile::Directory ) changes */
         (myFileDialog && (myFileDialog->mode() & KFile::Directory) &&
                          (myFileDialog->mode() & (KFile::File | KFile::Files)) == 0) )
    {
        const KUrl openUrl = (!m_parent->url().isEmpty() && !m_parent->url().isRelative() )
          ? m_parent->url() : m_startDir;

        /* FIXME We need a new abstract interface for using KDirSelectDialog in a non-modal way */

        KUrl newurl;
        if (fileDialogMode & KFile::LocalOnly)
            newurl = KFileDialog::getExistingDirectory( openUrl, m_parent);
        else
            newurl = KFileDialog::getExistingDirectoryUrl( openUrl, m_parent);

        if ( newurl.isValid() )
        {
            m_parent->setUrl( newurl );
            emit m_parent->urlSelected( url() );
        }
    }
    else
    {
        emit m_parent->openFileDialog( m_parent );

        //Creates the fileDialog if it doesn't exist yet
        KFileDialog *dlg = m_parent->fileDialog();

        if ( !url().isEmpty() && !url().isRelative() ) {
          KUrl u( url() );
          // If we won't be able to list it (e.g. http), then don't try :)
          if ( KProtocolManager::supportsListing( u ) )
              dlg->setSelection( u.url() );
        } else {
          dlg->setUrl(m_startDir);
        }

        //Update the file dialog window modality
        if ( dlg->windowModality() != fileDialogModality )
            dlg->setWindowModality(fileDialogModality);

        if ( fileDialogModality == Qt::NonModal )
        {
            dlg->show();
        } else {
            dlg->exec();
        }
    }
}

void KUrlRequester::KUrlRequesterPrivate::_k_slotFileDialogFinished()
{
    if ( !myFileDialog )
        return;

    if ( myFileDialog->result() == QDialog::Accepted )
    {
        KUrl newUrl = myFileDialog->selectedUrl();
        if ( newUrl.isValid() )
        {
            m_parent->setUrl( newUrl );
            emit m_parent->urlSelected( url() );
        }
    }
}

void KUrlRequester::setMode( KFile::Modes mode)
{
    Q_ASSERT( (mode & KFile::Files) == 0 );
    d->fileDialogMode = mode;
    if ( (mode & KFile::Directory) && !(mode & KFile::File) )
        d->myCompletion->setMode( KUrlCompletion::DirCompletion );

    if (d->myFileDialog) {
        d->myFileDialog->setMode(d->fileDialogMode);
    }
}

KFile::Modes KUrlRequester::mode() const
{
    return d->fileDialogMode;
}

void KUrlRequester::setFilter(const QString &filter)
{
    d->fileDialogFilter = filter;
    if (d->myFileDialog) {
        d->myFileDialog->setFilter(d->fileDialogFilter);
    }
}

QString KUrlRequester::filter( ) const
{
    return d->fileDialogFilter;
}

KFileDialog * KUrlRequester::fileDialog() const
{
    if (!d->myFileDialog) {
        QWidget *p = parentWidget();
        d->myFileDialog = new KFileDialog(QString(), d->fileDialogFilter, p);
        d->myFileDialog->setMode(d->fileDialogMode);
        d->myFileDialog->setCaption(windowTitle());
        d->myFileDialog->setWindowModality(d->fileDialogModality);
        connect(d->myFileDialog, SIGNAL(finished()), SLOT(_k_slotFileDialogFinished()));
    }

    return d->myFileDialog;
}

void KUrlRequester::clear()
{
    d->setText( QString() );
}

KLineEdit * KUrlRequester::lineEdit() const
{
    return d->edit;
}

KComboBox * KUrlRequester::comboBox() const
{
    return d->combo;
}

void KUrlRequester::KUrlRequesterPrivate::_k_slotUpdateUrl()
{
    KUrl u( KUrl::fromPath( QDir::currentPath() + '/' ), url().url() );
    myButton->setURL( u );
}

bool KUrlRequester::eventFilter( QObject *obj, QEvent *ev )
{
    if ( ( d->edit == obj ) || ( d->combo == obj ) )
    {
        if (( ev->type() == QEvent::FocusIn ) || ( ev->type() == QEvent::FocusOut ))
            // Forward focusin/focusout events to the urlrequester; needed by file form element in khtml
            QApplication::sendEvent( this, ev );
    }
    return QWidget::eventFilter( obj, ev );
}

KPushButton * KUrlRequester::button() const
{
    return d->myButton;
}

KUrlCompletion *KUrlRequester::completionObject() const
{
    return d->myCompletion;
}

void KUrlRequester::setClickMessage(const QString& msg)
{
    if(d->edit)
        d->edit->setClickMessage(msg);
}

QString KUrlRequester::clickMessage() const
{
    if(d->edit)
        return d->edit->clickMessage();
    else
        return QString();
}

Qt::WindowModality KUrlRequester::fileDialogModality() const
{
    return d->fileDialogModality;
}

void KUrlRequester::setFileDialogModality(Qt::WindowModality modality)
{
    d->fileDialogModality = modality;
}

const KEditListBox::CustomEditor &KUrlRequester::customEditor()
{
    setSizePolicy(QSizePolicy( QSizePolicy::Preferred,
                               QSizePolicy::Fixed));

    KLineEdit *edit = d->edit;
    if ( !edit && d->combo )
        edit = qobject_cast<KLineEdit*>( d->combo->lineEdit() );

#ifndef NDEBUG
    if ( !edit ) {
        kWarning() << "KUrlRequester's lineedit is not a KLineEdit!??\n";
    }
#endif

    d->editor.setRepresentationWidget(this);
    d->editor.setLineEdit(edit);
    return d->editor;
}

KUrlComboRequester::KUrlComboRequester( QWidget *parent)
  : KUrlRequester( new KComboBox(false), parent), d(0)
{
}

#include "kurlrequester.moc"
