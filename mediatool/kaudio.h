// -*-C++-*-

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
