#include <stdlib.h>
#include <stdio.h>

#include <kapplication.h>
#include <qstring.h>

#include "kwalletbackend.h"

int main(int argc, char **argv) {
   KApplication a(argc, argv, "kwalletbackendtest");

   KWalletBackend be("ktestwallet");
   printf("KWalletBackend constructed\n");

   QByteArray apass, bpass, cpass;

   apass.duplicate("apassword", 9);
   bpass.duplicate("bpassword", 9);
   cpass.duplicate("cpassword", 9);

   printf("Passwords initialised.\n");
   int rc = be.lock(apass);

   printf("be.lock(apass) returned %d\n", rc);

   rc = be.unlock(bpass);

   printf("be.unlock(bpass) returned %d\n", rc);

   rc = be.unlock(apass);

   printf("be.unlock(apass) returned %d\n", rc);

   return 0;
}


