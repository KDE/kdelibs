#include "kpixmapeffect.h"
#include <qimage.h>
#include <qpainter.h>
#include <dither.h>
#include <qcolor.h>

#include <math.h>


void KPixmapEffect::gradient(KPixmap &pixmap, const QColor &ca,
                                     const QColor &cb, GradientType eff,
                                     int ncols)
{
    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image;

    register unsigned int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient ){
        QPixmap pmCrop;
        QColor cRow;

        uint *p;
        uint rgbRow;
	float rat;

        pmCrop.resize( 30, pixmap.height() );
        QImage image( 30, pixmap.height(), 32 );

        for ( y = 0; y < pixmap.height(); y++ ) {
            p = (uint *) image.scanLine(y);
            rat = (float)y / pixmap.height();

            cRow.setRgb( rca + (int) ( rDiff * rat ),
                         gca + (int) ( gDiff * rat ),
                         bca + (int) ( bDiff * rat ) );

            rgbRow = cRow.rgb();

            for( x = 0; x < 30; x++ ) {
                *p = rgbRow;
                p++;
            }
        }

	if(pmCrop.depth() < 16 ) {
	  if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	  QColor *dPal = new QColor[ncols];
	  for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	  }
	  kFSDither dither(dPal, ncols);
	  image = dither.dither(image);
	  pmCrop.convertFromImage(image);
	  delete [] dPal;
	}
	else
	  pmCrop.convertFromImage( image );
	
        // Copy the cropped pixmap into the KPixmap.
        // Extract only a central column from the cropped pixmap
        // to avoid edge effects.

        int sSize = 20;
        int sOffset = 5;

	int s = pixmap.width() / sSize + 1;

        QPainter paint;
        paint.begin( &pixmap );

	for( int i=0; i<s; i++ )
	  paint.drawPixmap( sSize*i, 0, 
			    pmCrop, sOffset, 0 , sSize, pixmap.height() );
        paint.end();

    }
    
    else {
	float rfd, gfd, bfd;
	float rd = rca, gd = gca, bd = bca;
	
	QImage image(pixmap.width(), pixmap.height(), 32);
	unsigned char xtable[pixmap.width()][3], ytable[pixmap.height()][3];

	unsigned int *scanline;

	if(eff == HorizontalGradient){

	  rfd = (float)rDiff / pixmap.width();
	  gfd = (float)gDiff / pixmap.width();
	  bfd = (float)bDiff / pixmap.width();
	  
	  unsigned int *src = (unsigned int *)image.scanLine(0);
	  for(x = 0; x < pixmap.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd )
            src[x] = qRgb((int)rd, (int)gd, (int)bd);
	  
	  // Believe it or not, manually copying in a for loop is faster
	  // than calling memcpy for each scanline (on the order of ms...).
	  // I think this is due to the function call overhead (mosfet).
	  
	  for(y = 1; y < pixmap.height(); ++y)
	    {
	      scanline = (unsigned int *)image.scanLine(y);
	      for(x=0; x < pixmap.width(); ++x)
		scanline[x] = src[x];
	    }
	}

	else if ( eff == DiagonalGradient || eff == CrossDiagonalGradient) 
	  {
	    // Diagonal dgradient code inspired by BlackBox (mosfet)
	    // BlackBox dgradient is (C) Brad Hughes, <bhughes@tcac.net> and
	    // Mike Cole <mike@mydot.com>.
	    
	    unsigned int w = pixmap.width() * 2, h = pixmap.height() * 2;
	    
	    rfd = (float)rDiff/w;
	    gfd = (float)gDiff/w;
	    bfd = (float)bDiff/w;
	    
	    int dir;
	    for (x = 0; x < pixmap.width(); x++, rd+=rfd, gd+=gfd, bd+=bfd) {
	      dir = eff == DiagonalGradient? x : pixmap.width() - x - 1;
	      xtable[dir][0] = (unsigned char) rd;
	      xtable[dir][1] = (unsigned char) gd;
	      xtable[dir][2] = (unsigned char) bd;
	    }
	    rfd = (float)rDiff/h;
	    gfd = (float)gDiff/h;
	    bfd = (float)bDiff/h;
	    
	    rd = gd = bd = 0;
	    for (y = 0; y < pixmap.height(); y++, rd+=rfd, gd+=gfd, bd+=bfd) {
	      ytable[y][0] = (unsigned char) rd;
	      ytable[y][1] = (unsigned char) gd;
	      ytable[y][2] = (unsigned char) bd;
	    }
	    
	    for (y = 0; y < pixmap.height(); y++) {
	      unsigned int *scanline = (unsigned int *)image.scanLine(y);
	      for (x = 0; x < pixmap.width(); x++) {
                scanline[x] = qRgb(xtable[x][0] + ytable[y][0],
                                   xtable[x][1] + ytable[y][1],
                                   xtable[x][2] + ytable[y][2]);
	      }
	    }
	  }

	else if (eff == RectangleGradient || 
		 eff == PyramidGradient ||
		 eff == PipeCrossGradient ||
		 eff == EllipticGradient)
	  {
	    int rSign = rDiff>0? 1: -1;
	    int gSign = gDiff>0? 1: -1;
	    int bSign = bDiff>0? 1: -1;
    
	    rfd = (float)rDiff / pixmap.width();
	    gfd = (float)gDiff / pixmap.width();
	    bfd = (float)bDiff / pixmap.width();
    
	    rd = (float)rDiff/2;
	    gd = (float)gDiff/2;
	    bd = (float)bDiff/2;
    
	    for (x = 0; x < pixmap.width(); x++, rd-=rfd, gd-=gfd, bd-=bfd) 
	      {
		xtable[x][0] = (unsigned char) abs((int)rd);
		xtable[x][1] = (unsigned char) abs((int)gd);
		xtable[x][2] = (unsigned char) abs((int)bd);
	      }
	    
	    rfd = (float)rDiff/pixmap.height();
	    gfd = (float)gDiff/pixmap.height();
	    bfd = (float)bDiff/pixmap.height();
    
	    rd = (float)rDiff/2;
	    gd = (float)gDiff/2;
	    bd = (float)bDiff/2;
	    
	    for (y = 0; y < pixmap.height(); y++, rd-=rfd, gd-=gfd, bd-=bfd) 
	      {
		ytable[y][0] = (unsigned char) abs((int)rd);
		ytable[y][1] = (unsigned char) abs((int)gd);
		ytable[y][2] = (unsigned char) abs((int)bd);
	      }
	    
	    for (y = 0; y < pixmap.height(); y++) {
	      unsigned int *scanline = (unsigned int *)image.scanLine(y);
	      for (x = 0; x < pixmap.width(); x++) {
		if (eff == PyramidGradient)
		  {
		    scanline[x] = qRgb(rcb-rSign*(xtable[x][0]+ytable[y][0]),
				       gcb-gSign*(xtable[x][1]+ytable[y][1]),
				       bcb-bSign*(xtable[x][2]+ytable[y][2]));
		  }
		if (eff == RectangleGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				       QMAX(xtable[x][0], ytable[y][0]) * 2,
				       gcb - gSign *
				       QMAX(xtable[x][1], ytable[y][1]) * 2,
				       bcb - bSign *
				       QMAX(xtable[x][2], ytable[y][2]) * 2);
		  }
		if (eff == PipeCrossGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				       QMIN(xtable[x][0], ytable[y][0]) * 2,
				       gcb - gSign *
				       QMIN(xtable[x][1], ytable[y][1]) * 2,
				       bcb - bSign *
				       QMIN(xtable[x][2], ytable[y][2]) * 2);
		  }
		if (eff == EllipticGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				      (int)sqrt((xtable[x][0]*xtable[x][0] +
						 ytable[y][0]*ytable[y][0])*2),
				       gcb - gSign *
				      (int)sqrt((xtable[x][1]*xtable[x][1] +
						 ytable[y][1]*ytable[y][1])*2),
				       bcb - bSign *
				     (int)sqrt((xtable[x][2]*xtable[x][2] +
						ytable[y][2]*ytable[y][2])*2));
		  }
	      }
	    }
	  }
	
	if(pixmap.depth() < 16 ) {
	  if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	  QColor *dPal = new QColor[ncols];
	  for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	  }
	  kFSDither dither(dPal, ncols);
	  image = dither.dither(image);
	  pixmap.convertFromImage(image);
	  delete [] dPal;
	}
	else
	  pixmap.convertFromImage(image);
    }
}
    

//CT this is merely the same code as in the above method, but it's supposedly
//   way less performant since it introduces a lot of supplementary tests
//   and simple math operations for the calculus of the balance.
//   Yes, I could have merged them, but then the excellent performance of
//   the balanced code would suffer with no other gain than a mere
//   source code and byte code size economy.

void KPixmapEffect::unbalancedGradient(KPixmap &pixmap, const QColor &ca,
                                     const QColor &cb, GradientType eff,
                                     int factor, int ncols)
{

    if (!factor) factor = 1;

    double balance = abs(factor)/1000.;
    double rat;

    int rDiff, gDiff, bDiff;
    int rca, gca, bca, rcb, gcb, bcb;

    QImage image;

    register unsigned int x, y;

    rDiff = (rcb = cb.red())   - (rca = ca.red());
    gDiff = (gcb = cb.green()) - (gca = ca.green());
    bDiff = (bcb = cb.blue())  - (bca = ca.blue());

    if( eff == VerticalGradient ){
        QPixmap pmCrop;
        QColor cRow;

        uint *p;
        uint rgbRow;

        pmCrop.resize( 30, pixmap.height() );
        QImage image( 30, pixmap.height(), 32 );

        for ( y = 0; y < pixmap.height(); y++ ) {
            p = (uint *) image.scanLine(y);

            rat =  exp( ((double)y - pixmap.height() ) * balance );

            cRow.setRgb( rca + (int) ( rDiff * rat ),
                         gca + (int) ( gDiff * rat ),
                         bca + (int) ( bDiff * rat ) );

            rgbRow = cRow.rgb();

            for( x = 0; x < 30; x++ ) {
                *p = rgbRow;
                p++;
            }
        }

	if(pmCrop.depth() < 16 ) {
	  if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	  QColor *dPal = new QColor[ncols];
	  for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	  }
	  kFSDither dither(dPal, ncols);
	  image = dither.dither(image);
	  pmCrop.convertFromImage(image);
	  delete [] dPal;
	}
	else
	  pmCrop.convertFromImage( image );
	
        // Copy the cropped pixmap into the KPixmap.
        // Extract only a central column from the cropped pixmap
        // to avoid edge effects.

        int sSize = 20;
        int sOffset = 5;

	int s = pixmap.width() / sSize + 1;

        QPainter paint;
        paint.begin( &pixmap );

	for( int i=0; i<s; i++ )
	  paint.drawPixmap( sSize*i, 0, 
			    pmCrop, sOffset, 0 , sSize, pixmap.height() );
        paint.end();

    }
    
    else {
	float rd, gd, bd;
	
	QImage image(pixmap.width(), pixmap.height(), 32);
	unsigned char xtable[pixmap.width()][3], ytable[pixmap.height()][3];

	unsigned int *scanline;

	if(eff == HorizontalGradient){

	  unsigned int *src = (unsigned int *)image.scanLine(0);
	  for(x = 0; x < pixmap.width(); x++ ) 
	    {
	      rat =  exp( ((double)x - pixmap.width() ) * balance );

	      rd = rca + (int) ( rDiff * rat );
	      gd = gca + (int) ( gDiff * rat );
	      bd = bca + (int) ( bDiff * rat );

	      src[x] = qRgb((int)rd, (int)gd, (int)bd);
	    }
	  
	  // Believe it or not, manually copying in a for loop is faster
	  // than calling memcpy for each scanline (on the order of ms...).
	  // I think this is due to the function call overhead (mosfet).
	  
	  for(y = 1; y < pixmap.height(); ++y)
	    {
	      scanline = (unsigned int *)image.scanLine(y);
	      for(x=0; x < pixmap.width(); ++x)
		scanline[x] = src[x];
	    }
	}

	else if ( eff == DiagonalGradient || eff == CrossDiagonalGradient) 
	  {
	    int dir;
	    for (x = 0; x < pixmap.width(); x++) {
	      dir = eff == DiagonalGradient? x : pixmap.width() - x - 1;

	      rat =  exp( ((double)x - pixmap.width() ) * balance );

	      xtable[dir][0] = (unsigned char) (rca + (int) ( rDiff/2 * rat ));
	      xtable[dir][1] = (unsigned char) (gca + (int) ( gDiff/2 * rat ));
	      xtable[dir][2] = (unsigned char) (bca + (int) ( bDiff/2 * rat ));
	    }

	    for (y = 0; y < pixmap.height(); y++) {

	      rat =  exp( ((double)y - pixmap.height() ) * balance );

	      ytable[y][0] = (unsigned char) ( rDiff/2 * rat );
	      ytable[y][1] = (unsigned char) ( gDiff/2 * rat );
	      ytable[y][2] = (unsigned char) ( bDiff/2 * rat );
	    }
	    
	    for (y = 0; y < pixmap.height(); y++) {
	      unsigned int *scanline = (unsigned int *)image.scanLine(y);
	      for (x = 0; x < pixmap.width(); x++) {
                scanline[x] = qRgb(xtable[x][0] + ytable[y][0],
                                   xtable[x][1] + ytable[y][1],
                                   xtable[x][2] + ytable[y][2]);
	      }
	    }
	  }

	else if (eff == RectangleGradient || 
		 eff == PyramidGradient ||
		 eff == PipeCrossGradient ||
		 eff == EllipticGradient)
	  {
	    int rSign = rDiff>0? 1: -1;
	    int gSign = gDiff>0? 1: -1;
	    int bSign = bDiff>0? 1: -1;

	    for (x = 0; x < pixmap.width(); x++) 
	      {
		rat =  exp( ((double)x - pixmap.width() ) * balance );

		xtable[x][0] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
		xtable[x][1] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
		xtable[x][2] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
	      }
	    
	    for (y = 0; y < pixmap.height(); y++) 
	      {
		rat =  exp( ((double)y - pixmap.height() ) * balance );

		ytable[y][0] = (unsigned char) abs((int)(rDiff*(0.5-rat)));
		ytable[y][1] = (unsigned char) abs((int)(gDiff*(0.5-rat)));
		ytable[y][2] = (unsigned char) abs((int)(bDiff*(0.5-rat)));
	      }
	    
	    for (y = 0; y < pixmap.height(); y++) {
	      unsigned int *scanline = (unsigned int *)image.scanLine(y);
	      for (x = 0; x < pixmap.width(); x++) {
		if (eff == PyramidGradient)
		  {
		    scanline[x] = qRgb(rcb-rSign*(xtable[x][0]+ytable[y][0]),
				       gcb-gSign*(xtable[x][1]+ytable[y][1]),
				       bcb-bSign*(xtable[x][2]+ytable[y][2]));
		  }
		if (eff == RectangleGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				       QMAX(xtable[x][0], ytable[y][0]) * 2,
				       gcb - gSign *
				       QMAX(xtable[x][1], ytable[y][1]) * 2,
				       bcb - bSign *
				       QMAX(xtable[x][2], ytable[y][2]) * 2);
		  }
		if (eff == PipeCrossGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				       QMIN(xtable[x][0], ytable[y][0]) * 2,
				       gcb - gSign *
				       QMIN(xtable[x][1], ytable[y][1]) * 2,
				       bcb - bSign *
				       QMIN(xtable[x][2], ytable[y][2]) * 2);
		  }
		if (eff == EllipticGradient)
		  {
		    scanline[x] = qRgb(rcb - rSign * 
				      (int)sqrt((xtable[x][0]*xtable[x][0] +
						 ytable[y][0]*ytable[y][0])*2),
				       gcb - gSign *
				      (int)sqrt((xtable[x][1]*xtable[x][1] +
						 ytable[y][1]*ytable[y][1])*2),
				       bcb - bSign *
				     (int)sqrt((xtable[x][2]*xtable[x][2] +
						ytable[y][2]*ytable[y][2])*2));
		  }
	      }
	    }
	  }
	
	if(pixmap.depth() < 16 ) {
	  if ( ncols < 2 || ncols > 256 )
	    ncols = 3;
	  QColor *dPal = new QColor[ncols];
	  for (int i=0; i<ncols; i++) {
	    dPal[i].setRgb ( rca + rDiff * i / ( ncols - 1 ),
			     gca + gDiff * i / ( ncols - 1 ),
			     bca + bDiff * i / ( ncols - 1 ) );
	  }
	  kFSDither dither(dPal, ncols);
	  image = dither.dither(image);
	  pixmap.convertFromImage(image);
	  delete [] dPal;
	}
	else
	  pixmap.convertFromImage(image);
    }
}
    

/* This builds a 256 byte unsigned char lookup table with all
 * the possible percent values prior to applying the effect, then uses
 * integer math for the pixels. For any image larger than 9x9 this will be
 * less expensive than doing a float operation on the 3 color components of
 * each pixel. (mosfet)
 */
void KPixmapEffect::intensity(QImage &image, float percent, bool brighten)
{
  int i, tmp, r, g, b;
  int segColors = image.depth() > 8 ? 256 : image.numColors();
  unsigned char segTbl[segColors];
  int pixels = image.depth() > 8 ? image.width()*image.height() :
    image.numColors();
  unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
    (unsigned int *)image.colorTable();

  if(brighten){ // keep overflow check out of loops
    for(i=0; i < segColors; ++i){
      tmp = (int)(i*percent);
      if(tmp > 255)
	tmp = 255;
      segTbl[i] = tmp;
    }
  }
  else{
    for(i=0; i < segColors; ++i){
      tmp = (int)(i*percent);
      if(tmp < 0)
	tmp = 0;
      segTbl[i] = tmp;
    }
  }

  if(brighten){ // same here
    for(i=0; i < pixels; ++i){
      r = qRed(data[i]);
      g = qGreen(data[i]);
      b = qBlue(data[i]);
      r = r + segTbl[r] > 255 ? 255 : r + segTbl[r];
      g = g + segTbl[g] > 255 ? 255 : g + segTbl[g];
      b = b + segTbl[b] > 255 ? 255 : b + segTbl[b];
      data[i] = qRgb(r, g, b);
    }
  }
  else{
    for(i=0; i < pixels; ++i){
      r = qRed(data[i]);
      g = qGreen(data[i]);
      b = qBlue(data[i]);
      r = r - segTbl[r] < 0 ? 0 : r - segTbl[r];
      g = g - segTbl[g] < 0 ? 0 : g - segTbl[g];
      b = b - segTbl[b] < 0 ? 0 : b - segTbl[b];
      data[i] = qRgb(r, g, b);
    }
  }
}

void KPixmapEffect::channelIntensity(QImage &image, float percent,
				     RGBComponent channel,
				     bool brighten)
{
  int i, tmp, c;
  int segColors = image.depth() > 8 ? 256 : image.numColors();
  unsigned char segTbl[segColors];
  int pixels = image.depth() > 8 ? image.width()*image.height() :
    image.numColors();
  unsigned int *data = image.depth() > 8 ? (unsigned int *)image.bits() :
    (unsigned int *)image.colorTable();

  if(brighten){ // keep overflow check out of loops
    for(i=0; i < segColors; ++i){
      tmp = (int)(i*percent);
      if(tmp > 255)
	tmp = 255;
      segTbl[i] = tmp;
    }
  }
  else{
    for(i=0; i < segColors; ++i){
      tmp = (int)(i*percent);
      if(tmp < 0)
	tmp = 0;
      segTbl[i] = tmp;
    }
  }

  if(brighten){ // same here
    if(channel == Red){ // and here ;-)
      for(i=0; i < pixels; ++i){
	c = qRed(data[i]);
	c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
	data[i] = qRgb(c, qGreen(data[i]), qBlue(data[i]));
      }
    }
    if(channel == Green){
      for(i=0; i < pixels; ++i){
	c = qGreen(data[i]);
	c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
	data[i] = qRgb(qRed(data[i]), c, qBlue(data[i]));
      }
    }
    else{
      for(i=0; i < pixels; ++i){
	c = qBlue(data[i]);
	c = c + segTbl[c] > 255 ? 255 : c + segTbl[c];
	data[i] = qRgb(qRed(data[i]), qGreen(data[i]), c);
      }
    }

  }
  else{
    if(channel == Red){
      for(i=0; i < pixels; ++i){
	c = qRed(data[i]);
	c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
	data[i] = qRgb(c, qGreen(data[i]), qBlue(data[i]));
      }
    }
    if(channel == Green){
      for(i=0; i < pixels; ++i){
	c = qGreen(data[i]);
	c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
	data[i] = qRgb(qRed(data[i]), c, qBlue(data[i]));
      }
    }
    else{
      for(i=0; i < pixels; ++i){
	c = qBlue(data[i]);
	c = c - segTbl[c] < 0 ? 0 : c - segTbl[c];
	data[i] = qRgb(qRed(data[i]), qGreen(data[i]), c);
      }
    }
  }
}

