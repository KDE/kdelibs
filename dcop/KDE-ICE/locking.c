/* $Xorg: locking.c,v 1.3 2000/08/17 19:44:15 cpqbld Exp $ */
/******************************************************************************


Copyright 1993, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author: Ralph Mor, X Consortium
******************************************************************************/

#include <KDE-ICE/ICElib.h>
#include "KDE-ICE/ICElibint.h"


/*
 * NOT IMPLEMENTED YET
 */


Status
IceInitThreads ()

{
    return (0);
}


void
IceAppLockConn (iceConn)

IceConn iceConn;

{
    (void)iceConn;/*unused in the macro*/
    IceLockConn (iceConn);
}


void
IceAppUnlockConn (iceConn)

IceConn iceConn;

{
    (void)iceConn;/*unused in the macro*/
    IceUnlockConn (iceConn);
}
