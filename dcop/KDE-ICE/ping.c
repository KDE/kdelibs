/* $Xorg: ping.c,v 1.3 2000/08/17 19:44:15 cpqbld Exp $ */
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

Status
IcePing (iceConn, pingReplyProc, clientData)

IceConn		 iceConn;
IcePingReplyProc pingReplyProc;
IcePointer	 clientData;

{
    _IcePingWait *newping = (_IcePingWait *) malloc (sizeof (_IcePingWait));
    _IcePingWait *ptr = iceConn->ping_waits;

    if (newping == NULL)
	return (0);

    newping->ping_reply_proc = pingReplyProc;
    newping->client_data = clientData;
    newping->next = NULL;

    while (ptr && ptr->next)
	ptr = ptr->next;

    if (ptr == NULL)
	iceConn->ping_waits = newping;
    else
	ptr->next = newping;

    IceSimpleMessage (iceConn, 0, ICE_Ping);
    IceFlush (iceConn);

    return (1);
}
