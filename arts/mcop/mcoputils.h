    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

/*
 * BC - Status (2000-09-30): MCOPUtils
 *
 * Collection class for all kinds of utility functions. BC, since never
 * instanciated.
 */

#ifndef MCOPUTILS_H
#define MCOPUTILS_H

#include <string>
#include <vector>

namespace Arts {

class MCOPUtils {
public:
	/**
	 * Returns the full pathname to a file in the mcop directory which
	 * is called "name". It will also care that no other characters than
	 * A-Z,a-z,0-9,-,_ occur.
	 * 
	 * The result is something like /tmp/mcop-<username>/name, the directory
	 * will be created when necessary.
	 */
	static std::string createFilePath(std::string name);

	/**
	 * Returns the fully qualified hostname, such as "www.kde.org" (of course
	 * this may fail due to misconfiguration).
	 *
	 * The result is "localhost" if nothing at all can be found out.
	 */
	static std::string getFullHostname();

	/**
	 * Returns configuration values from .mcoprc
	 */
	static std::string readConfigEntry(const std::string& key,
										const std::string& defaultValue = "");

	/**
	 * Makes an interface ID from string - if the given string is already
	 * known, the returned IID will be the one returned last time. If not,
	 * a new IID is generated
	 */
	static unsigned long makeIID(const std::string& interfaceName);

	/**
	 * Returns the directories for the trader to look into
	 */
	static const std::vector<std::string> *traderPath();

	/**
	 * Returns the directories from where extensions can be loaded
	 */
	static const std::vector<std::string> *extensionPath();

	/**
	 * Returns the full path of the ~/.mcop directory
	 */
	static std::string mcopDirectory();

	/**
	 * Tokenization of a key = value1, "value 2", "value III" line into the key
	 * and the values.
	 */
	static bool tokenize(const std::string& line, std::string& key,
					     std::vector<std::string>& values);
};

};
#endif /* MCOPUTILS_H */
