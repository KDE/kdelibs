/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KACTIVELABEL_H
#define KACTIVELABEL_H

#include <kde3support_export.h>
#include <ktextbrowser.h>

class K3ActiveLabelPrivate;
 /**
  * Label with support for selection and clickable links.
  *
  * @deprecated Use QLabel. (QLabel::setOpenExternalLinks(true) for labels with hyperlinks;
  * QLabel::setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard) for labels whose text
  * should be selectable by user.)
  *
  * @author Waldo Bastian (bastian@kde.org)
  */
class KDE3SUPPORT_EXPORT K3ActiveLabel : public KTextBrowser
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit K3ActiveLabel(QWidget * parent = 0);

    /**
     * Constructor.
     */
    explicit K3ActiveLabel(const QString & text, QWidget * parent = 0);

    virtual ~K3ActiveLabel();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    virtual void focusInEvent( QFocusEvent* fe );
    virtual void focusOutEvent( QFocusEvent* fe );
    virtual void keyPressEvent ( QKeyEvent * e );
    virtual bool event(QEvent *e);
private:
    K3ActiveLabelPrivate *const d;
};

#endif
