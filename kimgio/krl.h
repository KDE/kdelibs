/*
* KRL.H -- Prototypes for QImageIO read/write handlers for
*	the krl graphic format.
*
*	Copyright (c) May 1999, Antonio Larrosa Jimenez.
*	Distributed under the LGPL.
*
*/
#ifndef	_KRL_H
#define _KRL_H

class QImageIO;

void kimgio_krl_read( QImageIO *io );
void kimgio_krl_write(QImageIO *io );

#endif
