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
 * BC - Status (2000-09-30): MCOPConfig
 *
 * Is guaranteed to stay binary compatible. d ptr provided.
 */

#ifndef MCOPCONFIG_H
#define MCOPCONFIG_H

#include <string>
#include <vector>

namespace Arts {

class MCOPConfigPrivate;
class MCOPConfig {
private:
	MCOPConfigPrivate *d;  // unused
protected:
	std::string filename;

public:
	MCOPConfig(const std::string& filename);

	std::string readEntry(const std::string& key,
								const std::string& defaultValue = "");
	std::vector<std::string> *readListEntry(const std::string& key);
};

};
#endif /* MCOPCONFIG_H */
