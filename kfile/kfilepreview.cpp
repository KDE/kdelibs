/* This file is part of the KDE libraries
    Copyright (C) 1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  2000 Werner Trobin <wtrobin@carinthia.com>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kfilepreview.h>
#include <kfilepreview.moc>

KFilePreview::KFilePreview(KFileView *view, QWidget *parent, const char *name)
  : QSplitter(parent, name), KFileView()
{
    if ( view )
        init( view );
    else
        init( new KFileIconView( (QSplitter*) this, "left" ));
}


KFilePreview::KFilePreview(QWidget *parent, const char *name) :
                           QSplitter(parent, name), KFileView()
{
    init( new KFileIconView((QSplitter*)this, "left") );
}

void KFilePreview::init( KFileView *view )
{
    left = 0L;
    setFileView( view );

    preview=new QWidget((QSplitter*)this, "preview");
    QString tmp=i18n("Sorry, no preview available.");
    QLabel *l=new QLabel(tmp, preview);
    l->setMinimumSize(l->sizeHint());
    l->move(10, 5);
    preview->setMinimumWidth(l->sizeHint().width()+20);
    setResizeMode(preview, QSplitter::KeepSize);
    deleted=false;
    previewMode=false;
    setViewName( i18n("Preview") );
}

KFilePreview::~KFilePreview() {
    if(!deleted && preview) {
        delete preview;
        preview=0L;
    }
}

void KFilePreview::setFileView( KFileView *view )
{
    ASSERT( view );

    delete left;
    view->widget()->reparent( this, QPoint(0,0) );
    view->KFileView::setViewMode(All);
    view->setOperator(this);
    left=view;
}

// this url parameter is useless... it's the url of the current directory.
// what for?
void KFilePreview::setPreviewWidget(const QWidget *w, const KURL &)
{
    left->setOnlyDoubleClickSelectsFiles( onlyDoubleClickSelectsFiles() );

    if(w!=0L) {
        previewMode=true;
        connect(this, SIGNAL(showPreview(const KURL &)),
                w, SLOT(showPreview(const KURL &)));
    }
    else {
        previewMode=false;
        preview->hide();
        return;
    }

    if(preview) {
        deleted=true;
        delete preview;
    }
    preview=const_cast<QWidget*>(w);
    preview->reparent((QSplitter*)this, 0, QPoint(0, 0), true);
    preview->resize(preview->sizeHint());
    preview->show();
}

void KFilePreview::insertItem(KFileViewItem *item) {
    left->insertItem(item);
}

void KFilePreview::clearView() {
    left->clearView();
    if(preview)
        emit showPreview( KURL() );
}

void KFilePreview::updateView(bool b) {
    left->updateView(b);
    if(preview)
        preview->repaint(b);
}

void KFilePreview::updateView(const KFileViewItem *i) {
    left->updateView(i);
}

void KFilePreview::removeItem(const KFileViewItem *i) {
    left->removeItem(i);
    KFileView::removeItem( i );
}

// huh, I don't think we need this....
void KFilePreview::clear() {
    KFileView::clear();
    left->KFileView::clear();
    if(preview)
        preview->erase();
}

void KFilePreview::clearSelection() {
    left->clearSelection();
}

bool KFilePreview::isSelected( const KFileViewItem *i ) const
{
    return left->isSelected( i );
}

void KFilePreview::setSelectionMode(KFile::SelectionMode sm) {
    left->setSelectionMode( sm );
}

void KFilePreview::setSelected(const KFileViewItem *item, bool enable) {
    left->setSelected( item, enable );
}

void KFilePreview::selectDir(const KFileViewItem* item) {
    sig->activateDir(item);
}

void KFilePreview::highlightFile(const KFileViewItem* item) {
    if ( item )
        emit showPreview( item->url() );
    else { // item = 0 -> multiselection mode
        const KFileViewItemList *items = selectedItems();
        if ( items->count() == 1 )
            emit showPreview( items->getFirst()->url() );
        else
            emit showPreview( KURL() );
    }
        

    sig->highlightFile(item);
    // the preview widget appears and takes some space of the left view,
    // so we may have to scroll to make the current item visible
    left->ensureItemVisible(item);
}

void KFilePreview::selectFile(const KFileViewItem* item) {
    sig->activateFile(item);
}

void KFilePreview::activatedMenu(const KFileViewItem *item) {
    sig->activateMenu(item);
}

void KFilePreview::ensureItemVisible(const KFileViewItem *item) {
    left->ensureItemVisible(item);
}
