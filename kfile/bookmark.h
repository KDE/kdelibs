/* This file is part of the KDE libraries
    Copyright (C) 1996, 1997, 1998 Martin R. Jones <mjones@kde.org>
      
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
//-----------------------------------------------------------------------------
//
// KDE HTML Bookmarks
//
// (c) Martin R. Jones 1996
//

#ifndef __BOOKMARK_H__
#define __BOOKMARK_H__

#include <qobject.h>
#include <qtextstream.h>
#include "booktoken.h"

class KBookmark
{
public:
	enum { URL, Folder };

	KBookmark();
	KBookmark( const char *_text, const char *_url );

	void clear();

	void setText( const char *_text )	{	text = _text; }
	void setURL( const char *_url )	{	url = _url; }
	void setType( int _type )	{	type = _type; }

	const char *getText()	{	return text; }
	const char *getURL()	{	return url; }
	int getType()	{	return type; }

	QList<KBookmark> &getChildren() 	{ return children; }

private:
	QString text;
	QString url;
	int type;
	QList<KBookmark> children;
};

class KBookmarkManager : public QObject
{
	Q_OBJECT
public:
	KBookmarkManager();

	void setTitle( const char *t )
		{	title = t; }

	void read( const char *filename );
	void write( const char *filename );

	void add( const char *_text, const char *_url );
	// rich
	bool remove(int);
	bool moveUp(int);
	bool moveDown(int);
	void reread();
	void rename(int, const char *);
	/**
	 * Overloaded to reread the last file loaded
	 */
	void write();

	KBookmark *getBookmark( int id );
	KBookmark *getRoot()	{	return &root; }

private:
	const char *parse( BookmarkTokenizer *ht, KBookmark *parent, const char *_end);
	void	writeFolder( QTextStream &stream, KBookmark *parent );
	KBookmark *findBookmark( KBookmark *parent, int id, int &currId );

signals:
	void changed();

private:
	KBookmark root;
	QString title;
	// rich
        QString myFilename;
};

#endif	// __BOOKMARK_H__

