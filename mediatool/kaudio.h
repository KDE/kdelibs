// -*-C++-*-
/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Esken (esken@kde.org)

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

class KAudio
{
public:
  /// Create an Audio player
  KAudio();
  /// Play the Wav last set via setFilename(filename) or play(filename)
  bool play();
  /// Set filename as current media name and play it
  bool play(char *filename);
  /// Set the "current" Filename. This file can be played later with ::play()
  bool setFilename(char *filename);
  /// If true is given, every play call is synced directly.
  void setAutosync(bool autosync);
  /// Stop current media
  bool stop();
  /** Sync media. This effectively blocks the calling process until the
      media is played fully */
  void sync();
  /** Query Server status. 0 means OK. You MUST check server status after
      creating a KAudio object */
  int  serverStatus();

private:
  bool		ServerContacted;
  bool		autosync;
  char		*WAVname;
  MdCh_FNAM*	FnamChunk;
  MdCh_KEYS*	KeysChunk;
  MdCh_IHDR*	IhdrChunk;
  MdCh_STAT*	StatChunk;
};
