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

   printf("be.lock(apass) returned %d  (should be -255)\n", rc);

   rc = be.unlock(bpass);

   printf("be.unlock(bpass) returned %d  (should be 0 or 1)\n", rc);

   rc = be.lock(bpass);

   printf("be.lock(bpass) returned %d  (should be 0)\n", rc);

   rc = be.unlock(apass);

   printf("be.unlock(apass) returned %d  (should be negative)\n", rc);

   rc = be.unlock(bpass);

   printf("be.unlock(bpass) returned %d  (should be 0)\n", rc);

   return 0;
}


