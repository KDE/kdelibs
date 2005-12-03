/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <qimage.h>
#include <math.h>

void
mult(float a[3][3],	/* I - First matrix */
     float b[3][3],	/* I - Second matrix */
     float c[3][3])	/* I - Destination matrix */
{
  int	x, y;		/* Looping vars */
  float	temp[3][3];	/* Temporary matrix */


 /*
  * Multiply a and b, putting the result in temp...
  */

  for (y = 0; y < 3; y ++)
    for (x = 0; x < 3; x ++)
      temp[y][x] = b[y][0] * a[0][x] +
                   b[y][1] * a[1][x] +
                   b[y][2] * a[2][x];

 /*
  * Copy temp to c (that way c can be a pointer to a or b).
  */

  memcpy(c, temp, sizeof(temp));
}

void
saturate(float mat[3][3],	/* I - Matrix to append to */
         float sat)		/* I - Desired color saturation */
{
  float	smat[3][3];		/* Saturation matrix */


  smat[0][0] = (1.0 - sat) * 0.3086 + sat;
  smat[0][1] = (1.0 - sat) * 0.3086;
  smat[0][2] = (1.0 - sat) * 0.3086;
  smat[1][0] = (1.0 - sat) * 0.6094;
  smat[1][1] = (1.0 - sat) * 0.6094 + sat;
  smat[1][2] = (1.0 - sat) * 0.6094;
  smat[2][0] = (1.0 - sat) * 0.0820;
  smat[2][1] = (1.0 - sat) * 0.0820;
  smat[2][2] = (1.0 - sat) * 0.0820 + sat;

  mult(smat, mat, mat);
}

void
xform(float mat[3][3],	/* I - Matrix */
      float x,		/* I - Input X coordinate */
      float y,		/* I - Input Y coordinate */
      float z,		/* I - Input Z coordinate */
      float *tx,	/* O - Output X coordinate */
      float *ty,	/* O - Output Y coordinate */
      float *tz)	/* O - Output Z coordinate */
{
  *tx = x * mat[0][0] + y * mat[1][0] + z * mat[2][0];
  *ty = x * mat[0][1] + y * mat[1][1] + z * mat[2][1];
  *tz = x * mat[0][2] + y * mat[1][2] + z * mat[2][2];
}

void
xrotate(float mat[3][3],	/* I - Matrix */
        float rs,		/* I - Rotation angle sine */
        float rc)		/* I - Rotation angle cosine */
{
  float rmat[3][3];		/* I - Rotation matrix */


  rmat[0][0] = 1.0;
  rmat[0][1] = 0.0;
  rmat[0][2] = 0.0;

  rmat[1][0] = 0.0;
  rmat[1][1] = rc;
  rmat[1][2] = rs;

  rmat[2][0] = 0.0;
  rmat[2][1] = -rs;
  rmat[2][2] = rc;

  mult(rmat, mat, mat);
}

void
yrotate(float mat[3][3],	/* I - Matrix */
        float rs,		/* I - Rotation angle sine */
        float rc)		/* I - Rotation angle cosine */
{
  float rmat[3][3];		/* I - Rotation matrix */


  rmat[0][0] = rc;
  rmat[0][1] = 0.0;
  rmat[0][2] = -rs;

  rmat[1][0] = 0.0;
  rmat[1][1] = 1.0;
  rmat[1][2] = 0.0;

  rmat[2][0] = rs;
  rmat[2][1] = 0.0;
  rmat[2][2] = rc;

  mult(rmat,mat,mat);
}

void
zrotate(float mat[3][3],	/* I - Matrix */
        float rs,		/* I - Rotation angle sine */
        float rc)		/* I - Rotation angle cosine */
{
  float rmat[3][3];		/* I - Rotation matrix */


  rmat[0][0] = rc;
  rmat[0][1] = rs;
  rmat[0][2] = 0.0;

  rmat[1][0] = -rs;
  rmat[1][1] = rc;
  rmat[1][2] = 0.0;

  rmat[2][0] = 0.0;
  rmat[2][1] = 0.0;
  rmat[2][2] = 1.0;

  mult(rmat,mat,mat);
}

void
zshear(float mat[3][3],	/* I - Matrix */
       float dx,	/* I - X shear */
       float dy)	/* I - Y shear */
{
  float smat[3][3];	/* Shear matrix */


  smat[0][0] = 1.0;
  smat[0][1] = 0.0;
  smat[0][2] = dx;

  smat[1][0] = 0.0;
  smat[1][1] = 1.0;
  smat[1][2] = dy;

  smat[2][0] = 0.0;
  smat[2][1] = 0.0;
  smat[2][2] = 1.0;

  mult(smat, mat, mat);
}

void
huerotate(float mat[3][3],	/* I - Matrix to append to */
          float rot)		/* I - Hue rotation in degrees */
{
  float hmat[3][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};		/* Hue matrix */
  float lx, ly, lz;		/* Luminance vector */
  float xrs, xrc;		/* X rotation sine/cosine */
  float yrs, yrc;		/* Y rotation sine/cosine */
  float zrs, zrc;		/* Z rotation sine/cosine */
  float zsx, zsy;		/* Z shear x/y */


 /*
  * Rotate the gray vector into positive Z...
  */

  xrs = M_SQRT1_2;
  xrc = M_SQRT1_2;
  xrotate(hmat,xrs,xrc);

  yrs = -1.0 / sqrt(3.0);
  yrc = -M_SQRT2 * yrs;
  yrotate(hmat,yrs,yrc);

 /*
  * Shear the space to make the luminance plane horizontal...
  */

  xform(hmat, 0.3086, 0.6094, 0.0820, &lx, &ly, &lz);
  zsx = lx / lz;
  zsy = ly / lz;
  zshear(hmat, zsx, zsy);

 /*
  * Rotate the hue...
  */

  zrs = sin(rot * M_PI / 180.0);
  zrc = cos(rot * M_PI / 180.0);

  zrotate(hmat, zrs, zrc);

 /*
  * Unshear the space to put the luminance plane back...
  */

  zshear(hmat, -zsx, -zsy);

 /*
  * Rotate the gray vector back into place...
  */

  yrotate(hmat, -yrs, yrc);
  xrotate(hmat, -xrs, xrc);

 /*
  * Append it to the current matrix...
  */

  mult(hmat, mat, mat);
}

void
bright(float mat[3][3],
       float scale)
{
  for (int i=0;i<3;i++)
    for (int j=0;j<3;j++)
      mat[i][j] *= scale;
}

//----------------------------------------------------------------------------------------------------

QImage convertImage(const QImage& image, int hue, int saturation, int brightness, int gamma)
{
	float	mat[3][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};
	int	lut[3][3][256];
	QRgb	c;
	int	r,g,b,v,r2,g2,b2;
	float	gam = 1.0/(float(gamma)/1000.0);
	QImage	img(image);

	saturate(mat,saturation*0.01);
	huerotate(mat,(float)hue);
	bright(mat,brightness*0.01);
	for (int i = 0; i < 3; i ++)
	  for (int j = 0; j < 3; j ++)
	    for (int k = 0; k < 256; k ++)
		lut[i][j][k] = (int)(mat[i][j] * k + 0.5);
	
	img.detach();
	for (int i=0;i<image.width();i++)
	   for (int j=0;j<image.height();j++)
	   {
	   	c = image.pixel(i,j);
	   	r = qRed(c);
	   	g = qGreen(c);
	   	b = qBlue(c);

	   	v = lut[0][0][r] + lut[1][0][g] + lut[2][0][b];
	   	if (gamma != 1000) v = (int)rint(pow(v,gam));
	   	if (v < 0) r2 = 0;
	   	else if (v > 255) r2 = 255;
	   	else r2 = v;

	   	v = lut[0][1][r] + lut[1][1][g] + lut[2][1][b];
	   	if (gamma != 1000) v = (int)rint(pow(v,gam));
	   	if (v < 0) g2 = 0;
	   	else if (v > 255) g2 = 255;
	   	else g2 = v;

	   	v = lut[0][2][r] + lut[1][2][g] + lut[2][2][b];
	   	if (gamma != 1000) v = (int)rint(pow(v,gam));
	   	if (v < 0) b2 = 0;
	   	else if (v > 255) b2 = 255;
	   	else b2 = v;

	   	img.setPixel(i,j,qRgb(r2,g2,b2));
	   }
	return img;
}
