
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "kaudio.h"


KAudio::KAudio()
{
  struct sockaddr_in socket_addr;
  int curState = 1;
  ServerContacted = false;
  bool l_b_contactError = false;
  unsigned short int port = 5007;	// !!! Change this
  const char* host = "127.0.0.1"; //"faui08j";	// !!! and this

  // Create socket
  sockFD = ::socket( AF_INET, SOCK_STREAM, 0 );
  if ( sockFD < 0 ) {
    cerr << "Unable to create socket\n";
    l_b_contactError = true;
  }
  // Set socket parameters
  if ( !l_b_contactError  &&
       fcntl( sockFD, F_SETFD, FD_CLOEXEC ) < 0 ) {
    cerr << "Unable to set socket parameter close-on-exex\n";
    l_b_contactError = true;
  }
  if ( !l_b_contactError && setsockopt( sockFD, SOL_SOCKET, SO_REUSEADDR,
				       &curState, sizeof(curState) ) < 0 ) {
    cerr << "Unable to set socket option SO_REUSEADDR\n";
    l_b_contactError = true;
  }
  if ( !l_b_contactError ) {
    // Set connect information
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons( port );
    if( !inet_aton( host, &socket_addr.sin_addr )) {
      cerr << "couldn't convert " << host << " to inet address\n";
      l_b_contactError = true;
    }
  }
  if ( !l_b_contactError && ::connect( sockFD,
				       (struct sockaddr *) &socket_addr,
				       sizeof(struct sockaddr_in) ) < 0 ) {
    cerr << "Unable to connect to server port " << socket_addr.sin_port << '\n';
    l_b_contactError = true;
  }

  // If we encountered no error in the connection, everzthing went well.
  // we can set the ServerContacted flag then

  if (!l_b_contactError) {
    int l_l_expected = 8;
    int l_l_write = write(sockFD, "auth 42\n", l_l_expected);
    if (l_l_write != l_l_expected)
      cerr << "Failed sending AUTH\n";
    else
      ServerContacted = true;
  }
  if(ServerContacted) {
    printf("Arts server CONTACTED\n");
  }
  else {
      printf("Arts server NOT FOUND\n");
    }
}

KAudio::~KAudio()
{
  if(ServerContacted)
    true;
}

bool KAudio::play()
{
  if(!ServerContacted) return false;
  printf("Arts server PLAY REQUEST %s (clientID=%d)\n",wavName.c_str(),clientID);

  //execID = AudioManager->playWav(clientID,wavName.c_str());
  if(autosync) sync();
  return true;
}

bool KAudio::play(const char *filename)
{
  setFilename(filename);
  return play();
}

bool KAudio::play(QString& filename)
{
  setFilename(filename);
  return play();
}

bool KAudio::setFilename(const char *filename)
{
  int l_l_expected = strlen(filename) + 7;
  char *l_s_WAVname = new char[l_l_expected];
  strcpy(l_s_WAVname, "play ");
  strcpy(l_s_WAVname+5, filename);
  l_s_WAVname[l_l_expected - 2] = '\n';
  l_s_WAVname[l_l_expected - 1] = 0;

#if 1
  for (int i= 0; i< l_l_expected; i++)
    cerr << "l_s_WAVname[" << i << "]= " << l_s_WAVname[i] << " \n";
#endif

  // Write the name (but leave out trailing 0)
  int l_l_write = write(sockFD, l_s_WAVname, l_l_expected-1);
  if (l_l_write != l_l_expected-1)
    cerr << "Failed sending PLAY\n";
  else
    cerr << "Asking ARTS to play " << l_s_WAVname ;

  char l_s_artsAnswer[256];



  return true;
}

bool KAudio::setFilename(QString& filename)
{
  return setFilename((const char *)filename);

}

/**
  * If true is given, every play call is synced directly.
  */
void KAudio::setAutosync(bool autosync)
{
  this->autosync = autosync;
}

/** If you want to recieve a Qt signal when your media is finished, you must
  *  call setSignals(true) before you play your media.
  */
void KAudio::setSignals(bool sigs=true)
{
}

/**
  * Stop current media
  */
bool KAudio::stop()
{
  // Try to keep up semantics
  if(!ServerContacted) return false;
  return true;
}

/** Sync media. This effectively blocks the calling process until the
  * media is played fully
  */
void KAudio::sync()
{
}

/** Query Server status. 0 means OK. You MUST check server status after
  * creating a KAudio object.
  */
int  KAudio::serverStatus()
{
  return !ServerContacted;
}

#include "kaudio.moc"
