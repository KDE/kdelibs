/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
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


#ifndef KURLEDIT_H
#define KURLEDIT_H

#include <qevent.h>
#include <qwidget.h>

#include <kurl.h>

class QLineEdit;
class QString;
class QTimer;
class QToolButton;


class KURLEdit : public QWidget
{
  Q_OBJECT

public:
  KURLEdit( QWidget *parent=0, const char *name=0 );
  KURLEdit( const QString& url, QWidget *parent=0, const char *name=0 );
  ~KURLEdit();

  void 			setURL(const QString& url);
  QString 		url( bool omitProtocol=true ) const;

  void 			setEnableVerify( bool enable );
  bool 			verifyEnabled() const { return (myVerifyTimer != 0L); }

  bool 			exists();

  virtual QSize 	sizeHint() const;

signals:
  // forwards from LineEdit
  void 			urlChanged(const QString&);
  void 			returnPressed();


protected:
  virtual void 		resizeEvent( QResizeEvent * );

  QLineEdit *		myEdit;
  QToolButton *		myButton;


private:
  void 			init();

  KURL 			myURL;
  bool 			myDoVerify;
  bool 			myIsDirty;
  bool 			myExists;
  QTimer *		myVerifyTimer;


protected slots:
  void 			slotOpenDialog();
  void 			slotTextChanged( const QString& text );
  virtual void 		slotDoVerify();


};

#endif // KURLEDIT_H
