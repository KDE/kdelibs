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

KFilePreview::KFilePreview(QWidget *parent, const char *name) :
                           QSplitter(parent, name), KFileView(), preview(0L) {

    // only default stuff for now
    KFileIconView *files = new KFileIconView((QSplitter*)this, "left");
    files->KFileView::setViewMode(All);
    //files->setAlignMode(QIconView::East);
    left=files;
    files->setOperator(this);

    preview=new QWidget((QSplitter*)this, "preview");
    QString tmp=i18n("Sorry, no preview available.");
    QLabel *l=new QLabel(tmp, preview);
    l->setMinimumSize(l->sizeHint());
    l->move(10, 5);
    preview->setMinimumWidth(l->sizeHint().width()+20);
    setResizeMode(preview, QSplitter::KeepSize);
}

KFilePreview::~KFilePreview() {
}

void KFilePreview::setPreviewWidget(QWidget *w) {

    if(!w)
        return;
    if(preview)
        delete preview;

    preview=w;
    connect(this, SIGNAL(showPreview(const KURL &)),
            preview, SLOT(showPreview(const KURL &)));
    preview->recreate((QSplitter*)this, 0, QPoint(0, 0), true);
}

void KFilePreview::insertItem(KFileViewItem *item) {
    left->insertItem(item);
}

void KFilePreview::clearView() {
    left->clearView();
    if(preview)
        preview->erase();
}

void KFilePreview::updateView(bool b) {
    left->updateView(b);
    if(preview)
        preview->repaint(b);
}

void KFilePreview::updateView(const KFileViewItem *i) {
    left->updateView(i);
}

void KFilePreview::clear() {
    KFileView::clear();
    left->KFileView::clear();
    if(preview)
        preview->erase();
}

void KFilePreview::clearSelection() {
    left->clearSelection();
}

void KFilePreview::setSelectMode(KFileView::SelectionMode sm) {
    left->setSelectMode( sm );
}

void KFilePreview::highlightItem(const KFileViewItem *) {
    // todo?
}

void KFilePreview::selectDir(const KFileViewItem* item) {
    sig->activateDir(item);
}

void KFilePreview::highlightFile(const KFileViewItem* item) {
    sig->highlightFile(item);
}

void KFilePreview::selectFile(const KFileViewItem* item) {
    sig->activateFile(item);
    //emit showPreview();???
}

void KFilePreview::activatedMenu(const KFileViewItem *item) {
    sig->activateMenu(item);
}
