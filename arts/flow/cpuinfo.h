    /*

    Copyright (C) 2001 Malte Starostik <malte.starostik@t-online.de>

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

#ifndef _arts_cpuinfo_h
#define _arts_cpuinfo_h

namespace Arts {

	class CpuInfo
	{
	public:
		enum Flags
		{
			CpuMMX   = 0x001, // Pentium MMX
			CpuEMMX  = 0x002, // Cyrix Extended MMX
			Cpu3DNow = 0x004, // AMD 3DNow!
			CpuSSE   = 0x008  // Pentium III SSE
		};
		static Flags flags();
	
	private:
		static void initialize();
		
	private:
		static bool s_initialized;
		static int s_flags;
	};

	CpuInfo::Flags CpuInfo::flags()
	{
		if (!s_initialized)
			initialize();
		return static_cast<Flags>(s_flags);
	}
};

#endif // _arts_cpuinfo_h
