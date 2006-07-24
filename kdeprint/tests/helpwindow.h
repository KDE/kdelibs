/****************************************************************************
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <kmainwindow.h>
#include <q3textbrowser.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdir.h>

class QAction;
class QComboBox;
class QMenu;

class HelpWindow : public KMainWindow
{
    Q_OBJECT
public:
    HelpWindow( const QString& home_,  const QString& path, QWidget* parent = 0 );
    ~HelpWindow();

private Q_SLOTS:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void textChanged();
    void about();
    void aboutQt();
    void openFile();
    void newWindow();
    void print();

    void pathSelected( const QString & );
	void histChosen( QAction* );
	void bookmChosen( QAction* );
    void addBookmark();
    
private:
    void readHistory();
    void readBookmarks();
    
    Q3TextBrowser* browser;
    QComboBox *pathCombo;
	QAction *backwardAction;
	QAction *forwardAction;
    QString selectedURL;
    QStringList history, bookmarks;
    QMap<QAction*, QString> mHistory, mBookmarks;
    QMenu *hist, *bookm;

};





#endif

