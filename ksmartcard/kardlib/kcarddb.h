/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef _KCARDDB_H
#define _KCARDDB_H


class KConfig;
class QString;


class KCardDB {
public:

	KCardDB();
	~KCardDB();

	/*
	 *   This gets the name of the module that handles
	 *   this type of card based on the ATR.  Returns
	 *   QString::null if no handler exists.
	 */
	const QString getModuleName(const QString ATR);

	/*
	 *   Removes all cards from the database that are handled
	 *   by the handler "module".  Returns 0 on success.
	 */
	int removeByHandler(const QString module);

	/*
	 *   Removes a given ATR/card from the database.  Returns
	 *   0 on success.
	 */
	int removeCard(const QString ATR);

	/*
	 *   This adds an ATR to the database with the given
	 *   module name as the handler.  Returns 0 on success.
	 */
	int addHandler(const QString ATR, const QString module);

	/*
	 *   This launches the card selector application which
	 *   prompts the user to select the proper module to
	 *   handle the card he has inserted.
	 */
	static int launchSelector(const QString slot, const QString ATR);

	/*
	 *   Tries to make an educated guess of a handler to
	 *   use for a given ATR.  Returns QString::null if it
	 *   can't come up with anything reasonable.
	 */
	static QString guessHandler(const QString ATR);

private:
	class KCardDBPrivate;
	KCardDBPrivate *d;

	KConfig *cfg;
};


#endif

