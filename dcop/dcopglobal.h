
#define DCOPVendorString "KDE"
#define DCOPReleaseString "1.0"
#define DCOPVersionMajor 1
#define DCOPVersionMinor 0

#define DCOPRegisterClient 1
#define DCOPSend 2
#define DCOPCall 3
#define DCOPReply 4

int DCOPAuthCount = 1;
char *DCOPAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
extern "C" {
extern IcePoAuthStatus _IcePoMagicCookie1Proc (_IceConn *, void **, int, int, int, void *, int *, void **, char **);
extern IcePaAuthStatus _IcePaMagicCookie1Proc (_IceConn *, void **, int, int, void *, int *, void **, char **);
}
IcePoAuthProc DCOPClientAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc DCOPServerAuthProcs[] = {_IcePaMagicCookie1Proc};

struct DCOPMsg {
  CARD8 majorOpcode;
  CARD8 minorOpcode;
  CARD8 data[2];
  CARD32 length B32;
};
