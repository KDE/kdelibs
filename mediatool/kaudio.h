// -*-C++-*-

class KAudio
{
public:
  /// Create an Audio player
  KAudio();
  /// Play the Wav last set via setFilename(filename) or play(filename)
  bool play();
  /// Set filename as current Wav name and play it
  bool play(char *filename);
  bool setFilename(char *filename);
  bool stop();
  /** Query Server status. 0 means OK. You MUST check server status after
      creating a KAudio object */
  int  serverStatus();

private:
  bool		ServerContacted;
  char		*WAVname;
  MdCh_FNAM*	FnamChunk;
  MdCh_KEYS*	KeysChunk;
  MdCh_IHDR*	IhdrChunk;
};
