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

#include "playtofile.h"
#include "flowsystem.h"
#include "stdsynthmodule.h"
#include "convert.h"
#include "stdio.h"

using namespace Arts;
using namespace std;

namespace Arts {
	class PlayToFile_impl : virtual public PlayToFile_skel,
							virtual public StdSynthModule
	{
		typedef unsigned char uchar;
		unsigned char *outblock;
		unsigned long maxsamples;
		string _filename;
		FILE *out;
	public:
		PlayToFile_impl()  : _filename("-")
		{
		}
		void filename(const string& newfilename) {
			_filename = newfilename;
			filename_changed(_filename);
		}

		string filename() { return _filename; }

		void streamInit() {
			maxsamples = 0;
			outblock = 0;
			if(_filename == "-")
				out = stdout;
			else
				out = fopen(_filename.c_str(),"w");
		}
		void streamEnd() {
			if(outblock)
			{
				delete[] outblock;
				outblock = 0;
			}
			if(out != stdout)
			{
				fclose(out);
			}
		}

		void calculateBlock(unsigned long samples)
		{
			if(samples > maxsamples)
			{
				maxsamples = samples;

				if(outblock) delete[] outblock;
				outblock = new uchar[maxsamples * 4]; // 2 channels, 16 bit
			}

			convert_stereo_2float_i16le(samples,left,right, outblock);
			fwrite(outblock, 1, 4 * samples,out);
		}

		/*
		 * this is the most tricky part here - since we will run in a context
		 * where no audio hardware will play the "give me more data role",
		 * we'll have to request things ourselves (requireFlow() tells the
		 * flowsystem that more signal flow should happen, so that
		 * calculateBlock will get called
		 */
		void goOn()
		{
			_node()->requireFlow();
		}
	};
	REGISTER_IMPLEMENTATION(PlayToFile_impl);
};
