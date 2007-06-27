#define fillRect(_X_,_Y_,_W_,_H_,_B_) setPen(Qt::NoPen); p.setBrush(_B_); p.drawRect(_X_,_Y_,_W_,_H_)

// simply sets the pixmaps alpha value to all rgb (i.e. grey) channels
// TODO: maybe adjust rgb to psychovisual values? (qGrey() inversion)
static QPixmap rgbFromAlpha(const QPixmap &pix) {
   QImage img = pix.toImage();
   unsigned int *data = ( unsigned int * ) img.bits();
   int total = img.width() * img.height(), alpha;
   for ( int i = 0 ; i < total ; ++i ) {
      alpha = qAlpha(data[i]);
      data[i] = qRgba( alpha, alpha, alpha, 255 );
   }
   return QPixmap::fromImage(img);
}

#ifdef QT_NO_XRENDER
#define UPDATE_COLORS(_PIX_) _PIX_ = rgbFromAlpha(_PIX_);
#else
#define UPDATE_COLORS(_PIX_) //
#endif

void OxygenStyle::generatePixmaps()
{
   QPixmap tmp; QPainter p;
   
   // PUSHBUTTON =====================================
   // shadow
   int $1 = dpi.$1, $2 = dpi.$2, $2_2 = lround($2/2.0);
   int $9 = dpi.$9, $9_2 = ($9-1)/2;
   int $7 = dpi.$7, $3 = dpi.$3, $4 = dpi.$4;
   tmp = QPixmap($9,$9);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         tmp.fill(Qt::transparent);
         p.begin(&tmp);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,((j?5:1)+i)*6));
         p.drawRoundRect(0,0,$9,$9,90,90);
         if (!j) {
            p.setBrush(QColor(0,0,0,(1+i)*10));
            p.drawRoundRect($1,$1,$9-2*$1,$9-2*$1,75,75);
            p.setBrush(QColor(0,0,0,(1+i)*14));
            p.drawRoundRect($2,$2,$9-2*$2,$9-2*$2,60,60);
         }
         p.end();
         shadows.button[j][i] = Tile::Set(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2);
      }
   }
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,$9,$9,90,90);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect($1,$1,$9-2*$1,$9-2*$1,80,80);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect($2,$2,$9-2*$2,$9-2*$2,70,70);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect($3,$3,$9-2*$3,$9-2*$3,60,60);
   p.end();
   lights.button =
      Tile::Mask(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2, $3,$3,-$3,-$3, 75,75);
   
   // mask
   tmp = QPixmap($9,$9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,$9,$9,60,60);
   p.end();
   UPDATE_COLORS(tmp);
   masks.button = Tile::Mask(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2,0,0,0,0,60,60);
   
   // -> sunken
   QLinearGradient lg; QGradientStops stops;
   QImage tmpImg($9,$9, QImage::Format_ARGB32);
   
   for (int i = 0; i < 2; ++i) {
      int add = i*30;
      tmpImg.fill(Qt::transparent);
   
      p.begin(&tmpImg);
      p.setPen(Qt::NoPen);
      p.setRenderHint(QPainter::Antialiasing);
      p.setBrush(QColor(0,0,0,55+add)); p.drawRoundRect(0,0,$9,$7,80,80);
      p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
      add = 30 - add;
      p.setBrush(QColor(0,0,0,120+add)); p.drawRoundRect(0,$1,$9,dpi.$6,75,75);
      p.setBrush(QColor(0,0,0,140+add)); p.drawRoundRect(0,$2,$9,dpi.$5,80,80);
      p.setBrush(QColor(0,0,0,160+add)); p.drawRoundRect($1,$3,$7,$4,85,85);
      p.setBrush(QColor(0,0,0,180+add)); p.drawRoundRect($2,$4,dpi.$5,$3,90,90);
      p.setCompositionMode( QPainter::CompositionMode_SourceOver );
      lg = QLinearGradient(0,0,0,$9);
      stops << QGradientStop( 0, QColor(255,255,255, 90) )
         << QGradientStop( 0.5, QColor(255,255,255, 170) )
         << QGradientStop( 1, QColor(255,255,255, 90) );
      lg.setStops(stops);
      p.fillRect($3,$9-$2,$3,$1, lg);
      stops.clear();
      stops << QGradientStop( 0, QColor(255,255,255, 30) )
         << QGradientStop( 0.5, QColor(255,255,255, 100) )
         << QGradientStop( 1, QColor(255,255,255, 30) );
      lg.setStops(stops);
      p.fillRect($3,$9-$1,$3,$1, lg);
      stops.clear();
      p.end();
   
      shadows.lineEdit[i] = Tile::Set(QPixmap::fromImage(tmpImg),$9_2,$9_2,$9-2*$9_2,$9-2*$9_2);
   }
   
   // relief
   tmp = QPixmap($9,$9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   QPen pen = p.pen(); pen.setWidth($1); p.setPen(pen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::NoBrush);
   p.setPen(QColor(255,255,255,40));
   p.drawRoundRect(0,0,$9-$2,$9-$2,60,60);
   p.setPen(QColor(255,255,255,60));
   p.drawRoundRect($2,$2,$9-$2,$9-$2,60,60);
   p.setPen(QColor(0,0,0,50));
   p.drawRoundRect($1,$1,$9-dpi.$3,$9-dpi.$3,60,60);
   p.end();
   shadows.relief = Tile::Set(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2);

   
   // outlines
   tmp = QPixmap($9,$9);
   for (int i = 0; i < 2; ++i) {
      tmp.fill(Qt::transparent);
      p.begin(&tmp);
      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(QColor(255,255,255,100+i*60));
      p.setBrush(Qt::NoBrush);
      p.drawRoundRect(0,0,$9,2*$9,75,38);
      p.end();
      frames.button[i] = Tile::Set(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2);
   }
//    frames.button[0] = Tile::Nuno(100);
//    frames.button[1] = Tile::Nuno(160);
   
   // toplight
   int $49 = SCALE(49);
   int $49_2 = ($49-1)/2;
   tmp = QPixmap($49,$49);
   tmp.fill(Qt::transparent);
   QRadialGradient rg( tmp.rect().center(), $49_2 );
   rg.setColorAt ( 0, QColor(255,255,255,160) );
   rg.setColorAt ( 1, QColor(255,255,255,0) );
   p.begin(&tmp);
   p.fillRect(0,0,$49,$49,rg);
   p.end();
   tmp = tmp.scaled( $49, dpi.$5, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
   tmp = tmp.copy(0,$2,$49,dpi.$3);
   lights.top = Tile::Line(tmp,Qt::Horizontal,$49_2,-$49_2);
   
   // ================================================================
   
   // RADIOUTTON =====================================
   int rw = dpi.ExclusiveIndicator;
   int rh = dpi.ExclusiveIndicator;
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         shadows.radio[j][i] = QPixmap(rw-$1*2*j, rh-$1*2*j);
         shadows.radio[j][i].fill(Qt::transparent);
         p.begin(&shadows.radio[j][i]);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,(1+i+2*j)*9));
         p.drawEllipse(shadows.radio[j][i].rect());
         if (!j) {
            p.setBrush(QColor(0,0,0,(i+1)*20));
            p.drawEllipse($2_2,$2_2,rw-$2,rh-$2);
         }
         p.end();
      }
   }
   
   // mask
   rw -= $4; rh -= $4;
   masks.radio = QPixmap(rw, rh);
   masks.radio.fill(Qt::transparent);
   p.begin(&masks.radio);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawEllipse(0,0,rw,rh);
   p.end();
   UPDATE_COLORS(masks.radio);
   
   rw -= $4; rh -= $4;
   masks.radioGroove = QPixmap(rw, rh);
   masks.radioGroove.fill(Qt::transparent);
   p.begin(&masks.radioGroove);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawEllipse(0,0,rw,rh);
   p.end();
   UPDATE_COLORS(masks.radioGroove);
   
   // mask fill
   rw -= $4; rh -= $4;
   masks.radioIndicator = QPixmap(rw, rh);
   masks.radioIndicator.fill(Qt::transparent);
   p.begin(&masks.radioIndicator);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawEllipse(0,0,rw,rh);
   p.end();
   UPDATE_COLORS(masks.radioIndicator);
   
   // ================================================================
   
   // NOTCH =====================================
   masks.notch = QPixmap(dpi.$6, dpi.$6);
   masks.notch.fill(Qt::transparent);
   p.begin(&masks.notch);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::black);
   p.drawEllipse(0,0,dpi.$6,dpi.$6);
   p.end();
   UPDATE_COLORS(masks.notch);
   // ================================================================
   
   // RECTANGULAR =====================================
   
   // raised
   
   // sunken
   int $6 = dpi.$6;
   tmp = QPixmap($9,$9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.fillRect($1,0,$9-$2,$1, QColor(0,0,0,10));
   p.fillRect($2,$1,$9-$4,$1, QColor(0,0,0,20));
   p.fillRect($2,$2,$9-$4,$1, QColor(0,0,0,40));
   p.fillRect($3,$3,$9-$6,$1, QColor(0,0,0,80));
   
   p.fillRect($1,$9-$1,$9-$2,$1, QColor(255,255,255,10));
   p.fillRect($2,$9-$2,$9-$4,$1, QColor(255,255,255,20));
   p.fillRect($2,$9-$3,$9-$4,$1, QColor(255,255,255,40));
   p.fillRect($3,$9-$4,$9-$6,$1, QColor(255,255,255,80));
   
   p.fillRect(0,$1,$1,$9-$2, QColor(128,128,128,10));
   p.fillRect($1,$2,$1,$9-$4, QColor(128,128,128,20));
   p.fillRect($2,$2,$1,$9-$4, QColor(128,128,128,40));
   p.fillRect($3,$3,$1,$9-$6, QColor(128,128,128,80));
   
   p.fillRect($9-$1,$1,$1,$9-$2, QColor(128,128,128,10));
   p.fillRect($9-$2,$2,$1,$9-$4, QColor(128,128,128,20));
   p.fillRect($9-$3,$2,$1,$9-$4, QColor(128,128,128,40));
   p.fillRect($9-$4,$3,$1,$9-$6, QColor(128,128,128,80));
   
   p.end();
   shadows.sunken = Tile::Set(tmp,$9_2,$9_2,$9-2*$9_2,$9-2*$9_2);
   
   // ================================================================
   
   // TABBAR =====================================
   
   // mask
   int $13 = SCALE(13);
   tmp = QPixmap($13,$13);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,$13,$13,99,99);
   p.end();
   int $13_2 = ($13-1)/2;
   UPDATE_COLORS(tmp);
   masks.tab = Tile::Mask(tmp,$13_2,$13_2,$13-2*$13_2,$13-2*$13_2,0,0,0,0,99,99);
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,$13,$13,99,99);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect($1,$1,$13-2*$1,$13-2*$1,95,95);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect($2,$2,$13-2*$2,$13-2*$2,93,93);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect($3,$3,$13-2*$3,$13-2*$3,91,91);
   p.end();
   lights.tab =
      Tile::Mask(tmp,$13_2,$13_2,$13-2*$13_2,$13-2*$13_2, $3,$3,-$3,-$3, 91,91);
   
   // shadow
   int $17 = SCALE(17), $17_2 = ($17-1)/2;
   tmpImg = QImage($17,$17, QImage::Format_ARGB32);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         int add = 5*(i+2*j);
         tmpImg.fill(Qt::transparent);
         p.begin(&tmpImg);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,10+add));
         p.drawRoundRect(0,0,$17,$17,90,90);
         p.setBrush(QColor(0,0,0,13+add));
         p.drawRoundRect($1,$1,$17-$2,$17-$2,93,93);
         p.setBrush(QColor(0,0,0,18+add));
         p.drawRoundRect($2,$2,$17-$4,$17-$4,96,96);
         p.setBrush(QColor(0,0,0,23+add));
         p.drawRoundRect(dpi.$3,dpi.$3,$17-dpi.$6,$17-dpi.$6,99,99);
         p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
         p.setBrush(QColor(0,0,0,255));
         p.drawRoundRect($2,$1,$17-$4,$17-dpi.$5,99,99);
         p.setCompositionMode( QPainter::CompositionMode_SourceOver );
         p.setPen(QColor(255,255,255,170));
         p.setBrush(Qt::NoBrush);
         p.drawRoundRect($2,$1,$17-$4,$17-dpi.$5,99,99);
         p.end();
         shadows.tab[i][j] =
                  Tile::Set(QPixmap::fromImage(tmpImg),$17_2,$17_2,$17-2*$17_2,$17-2*$17_2);
      }
   }
   
   
   // sunken
   int $15 = SCALE(15), $11 = SCALE(11);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,85)); p.drawRoundRect(0,0,$17,$17-$2,80,80);
   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   p.setBrush(QColor(0,0,0,120)); p.drawRoundRect(0,$1,$17,$17-$3,75,75);
   p.setBrush(QColor(0,0,0,140)); p.drawRoundRect(0,$2,$17,dpi.$13,80,80);
   p.setBrush(QColor(0,0,0,160)); p.drawRoundRect($1,$3,$15,dpi.$12,85,85);
   p.setBrush(QColor(0,0,0,180)); p.drawRoundRect($2,dpi.$4,dpi.$13,$11,90,90);
   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   lg = QLinearGradient(0,0,0,$17);
   stops << QGradientStop( 0, QColor(255,255,255, 90) )
      << QGradientStop( 0.5, QColor(255,255,255, 190) )
      << QGradientStop( 1, QColor(255,255,255, 90) );
   lg.setStops(stops);
   p.fillRect($3,$17-$2,dpi.$12,$1, lg);
   stops.clear();
   stops << QGradientStop( 0, QColor(255,255,255, 30) )
      << QGradientStop( 0.5, QColor(255,255,255, 100) )
      << QGradientStop( 1, QColor(255,255,255, 30) );
   lg.setStops(stops);
   p.fillRect($3,$17-$1,dpi.$12,$1, lg);
   stops.clear();
   p.end();
   shadows.tabSunken = Tile::Set(QPixmap::fromImage(tmpImg),$17_2,$17_2,$17-2*$17_2,$17-2*$17_2);
   
   // GROUPBOX =====================================
   
   // shadow
   tmpImg = QImage($49,$49, QImage::Format_ARGB32);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,5)); p.drawRoundRect(0,0,$49,2*$49,14,7);
   p.setBrush(QColor(0,0,0,9)); p.drawRoundRect($1,$1,$49-$2,2*$49,13,7);
   p.setBrush(QColor(0,0,0,11)); p.drawRoundRect($2,$2,$49-dpi.$4,2*$49,12,6);
   p.setBrush(QColor(0,0,0,13)); p.drawRoundRect(dpi.$3,dpi.$3,$49-dpi.$6,2*$49,48,24);
   p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   p.setBrush(QColor(0,0,0,0)); p.drawRoundRect(dpi.$4,dpi.$2,$49-dpi.$8,2*$49,11,6);
//    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
//    p.setPen(QColor(255,255,255,200)); p.setBrush(Qt::NoBrush);
//    p.drawRoundRect(dpi.$4,dpi.$2,$49-dpi.$8,2*$49,11,6);
   p.setRenderHint(QPainter::Antialiasing, false);
//    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   int $33 = SCALE(33);
   for (int i = 1; i < $33; ++i) {
      p.setPen(QColor(0,0,0,CLAMP(i*lround(255.0/dpi.$32),0,255)));
      p.drawLine(0, $49-i, $49, $49-i);
   }
   p.end();
   int $12 = dpi.$12;
   shadows.group = Tile::Set(QPixmap::fromImage(tmpImg),$12,$12,$49-2*$12,$1);
   
   // mask --> uses buttons
//    int $25 = SCALE(25);
//    tmp = QPixmap($25,$25);
//    tmp.fill(Qt::transparent);
//    p.begin(&tmp);
//    p.setPen(Qt::NoPen);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setBrush(QColor(0,0,0,255));
//    p.drawRoundRect(0,0,$25,$25,22,22);
//    p.end();
//    $12 = ($25-1)/2;
//    masks.group = Tile::Mask(tmp,$12,$12,$25-2*$12,$25-2*$12,0,0,0,0,22,22);
   
   // shadow line
   int w,h,c1,c2;
   for (int i = 0; i < 2; ++i) { // orientarion
      if (i) {
         w = $2; h = $49;
         lg = QLinearGradient(0,0,0,$49);
      }
      else {
         w = $49; h = $2;
         lg = QLinearGradient(0,0,$49,0);
      }
      tmp = QPixmap(w,h);
      for (int j = 0; j < 3; ++j) { // direction
         c1 = (j > 0) ? 255 : 111; c2 = (j > 0) ? 111 : 255;
         tmp.fill(Qt::transparent);
         p.begin(&tmp);
         stops << QGradientStop( 0, QColor(c1,c1,c1,0) )
            << QGradientStop( 0.5, QColor(c1,c1,c1,71) )
            << QGradientStop( 1, QColor(c1,c1,c1,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect(0,0,$1,$49,lg);
         }
         else {
            p.fillRect(0,0,$49,$1,lg);
         }
         stops.clear();
         stops << QGradientStop( 0, QColor(c2,c2,c2,0) )
            << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
            << QGradientStop( 1, QColor(c2,c2,c2,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect($1,0,$2-$1,$49,lg);
         }
         else {
            p.fillRect(0,$1,$49,$2-$1,lg);
         }
         stops.clear();
         p.end();
         shadows.line[i][j] =
            Tile::Line(tmp, i ? Qt::Vertical : Qt::Horizontal, $49_2, -$49_2);
      }
   }
   
   // slider handles =================================================
   QPoint triangle[3] = { QPoint(0, 100), QPoint(-100, -100), QPoint(100, -100) };
   int size;
   for (int i = 0; i < 4; ++i) { // direction
      size = dpi.SliderControl;
      for (int j = 0; j < 2; ++j) { // sunken?
         if (j) size -= $2;
         for (int k = 0; k < 2; ++k) { // opaque?
            shadows.slider[i][j][k] = QPixmap(size, size);
            shadows.slider[i][j][k].fill(Qt::transparent);
            p.begin(&shadows.slider[i][j][k]);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0,0,0, (1+j+k)*18));
            p.translate(size/2, size/2);
            p.scale(size/200.0, size/200.0);
            p.rotate(-i*90.0);
            p.drawPolygon(triangle, 3);
            if (!j) {
               p.scale(0.78, 0.78);
               p.setBrush(QColor(0,0,0, (k+1)*12));
               p.drawPolygon(triangle, 3);
            }
            p.end();
         }
      }
      size = dpi.SliderControl - $4;
      masks.slider[i] = QPixmap(size, size);
      masks.slider[i].fill(Qt::transparent);
      p.begin(&masks.slider[i]);
      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(Qt::NoPen);
      p.setBrush(Qt::black);
      p.translate(size/2, size/2); p.scale(size/200.0, size/200.0);
      p.rotate(-i*90.0);
      p.drawPolygon(triangle, 3);
      p.end();
      lights.slider[i] = QPixmap(size, size);
      lights.slider[i].fill(Qt::transparent);
      p.begin(&lights.slider[i]);
      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(Qt::white);
      p.setBrush(Qt::NoBrush);
      p.translate(size/2, size/2); p.scale(size/200.0, size/200.0);
      p.rotate(-i*90.0);
      p.drawPolygon(triangle, 3);
      p.end();
      UPDATE_COLORS(masks.slider[i]);
   }
   // ================================================================
   // ================================================================
   // Popup corners - not really pxmaps, though ;) ===================
   // they at least break beryl's popup shadows...
   // see oxygen.cpp#OxygenStyle::eventfilter as well
//    QRegion circle = QRegion(0,0,2*$9,2*$9, QRegion::Ellipse);
//    QRegion rect = QRegion(0,0,$9,$9);
//    masks.corner[0] = rect - (circle & rect);
//    circle.translate(-$9, 0);
//    masks.corner[1] = rect - (circle & rect);
//    circle.translate(0, -$9);
//    masks.corner[3] = rect - (circle & rect);
//    circle.translate($9, 0);
//    masks.corner[2] = rect - (circle & rect);
   // ================================================================
#define _INITPIX_(_PIX_,_W_,_H_)\
   _PIX_ = QPixmap(_W_, _H_);\
   _PIX_.fill(Qt::transparent);\
   p.begin(&_PIX_);\
   p.setPen(Qt::NoPen);\
   p.setRenderHint(QPainter::Antialiasing)
      
#define _CLOSE_ARROW_(_PIX_, _OFF_)\
   triangle[0] = QPoint(_OFF_, _PIX_.rect().center().y()+_OFF_);\
   triangle[1] = _PIX_.rect().topRight()+QPoint(-_OFF_,_OFF_);\
   triangle[2] = _PIX_.rect().bottomRight()-QPoint(_OFF_,_OFF_);\
   p.drawPolygon(triangle, 3)
      
#define _MIN_ARROW_(_PIX_, _OFF_)\
   triangle[0] = _PIX_.rect().bottomLeft()+QPoint(_OFF_,-_OFF_);\
   triangle[1] = _PIX_.rect().topLeft()+QPoint(_OFF_,_OFF_);\
   triangle[2] = _PIX_.rect().bottomRight()-QPoint(_OFF_,_OFF_);\
   p.drawPolygon(triangle, 3)
      
#define _MAX_ARROW_(_PIX_, _OFF_)\
   triangle[0] = _PIX_.rect().topLeft()+QPoint(_OFF_,_OFF_);\
   triangle[1] = _PIX_.rect().topRight()+QPoint(-_OFF_,_OFF_);\
   triangle[2] = _PIX_.rect().bottomRight()-QPoint(_OFF_,_OFF_);\
   p.drawPolygon(triangle, 3)
   
   // Window Buttons ===================================
//    QPoint triangle[3];
   int $14 = SCALE(14);// $15 = SCALE(15), $16 = dpi.$16;
   _INITPIX_(masks.winClose, $14,$14);
   p.setBrush(Qt::black);
   _CLOSE_ARROW_(masks.winClose, 0);
   p.end();
   UPDATE_COLORS(masks.winClose);
   _INITPIX_(masks.winMin,$14,$14);
   p.setBrush(Qt::black);
   _MIN_ARROW_(masks.winMin, 0);
   p.end();
   UPDATE_COLORS(masks.winMin);
   _INITPIX_(masks.winMax,$14,$14);
   p.setBrush(Qt::black);
   _MAX_ARROW_(masks.winMax, 0);
   p.end();
   UPDATE_COLORS(masks.winMax);
   /*
   shadows.winClose[0] = QPixmap($16, $16);
   shadows.winMin[0] = QPixmap($16, $16);
   shadows.winMax[0] = QPixmap($16, $16);
   shadows.winClose[0] = QPixmap($15, $15);
   shadows.winMin[0] = QPixmap($15, $15);
   shadows.winMax[0] = QPixmap($15, $15);
   */
   // ================================================================
}
#undef fillRect
