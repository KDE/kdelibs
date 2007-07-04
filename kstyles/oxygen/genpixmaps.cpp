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
   int f1 = dpi.f1, f2 = dpi.f2, f2_2 = lround(f2/2.0);
   int f9 = dpi.f9, f9_2 = (f9-1)/2;
   int f7 = dpi.f7, f3 = dpi.f3, f4 = dpi.f4;
   tmp = QPixmap(f9,f9);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         tmp.fill(Qt::transparent);
         p.begin(&tmp);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,((j?5:1)+i)*6));
         p.drawRoundRect(0,0,f9,f9,90,90);
         if (!j) {
            p.setBrush(QColor(0,0,0,(1+i)*10));
            p.drawRoundRect(f1,f1,f9-2*f1,f9-2*f1,75,75);
            p.setBrush(QColor(0,0,0,(1+i)*14));
            p.drawRoundRect(f2,f2,f9-2*f2,f9-2*f2,60,60);
         }
         p.end();
         shadows.button[j][i] = Tile::Set(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
      }
   }
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,f9,f9,90,90);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect(f1,f1,f9-2*f1,f9-2*f1,80,80);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(f2,f2,f9-2*f2,f9-2*f2,70,70);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(f3,f3,f9-2*f3,f9-2*f3,60,60);
   p.end();
   lights.button =
      Tile::Mask(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2, f3,f3,-f3,-f3, 75,75);
   
   // mask
   tmp = QPixmap(f9,f9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,f9,f9,60,60);
   p.end();
   UPDATE_COLORS(tmp);
   masks.button = Tile::Mask(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2,0,0,0,0,60,60);
   
   // -> sunken
   QLinearGradient lg; QGradientStops stops;
   QImage tmpImg(f9,f9, QImage::Format_ARGB32);
   
   for (int i = 0; i < 2; ++i) {
      int add = i*30;
      tmpImg.fill(Qt::transparent);
   
      p.begin(&tmpImg);
      p.setPen(Qt::NoPen);
      p.setRenderHint(QPainter::Antialiasing);
      p.setBrush(QColor(0,0,0,55+add)); p.drawRoundRect(0,0,f9,f7,80,80);
      p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
      add = 30 - add;
      p.setBrush(QColor(0,0,0,120+add)); p.drawRoundRect(0,f1,f9,dpi.f6,75,75);
      p.setBrush(QColor(0,0,0,140+add)); p.drawRoundRect(0,f2,f9,dpi.f5,80,80);
      p.setBrush(QColor(0,0,0,160+add)); p.drawRoundRect(f1,f3,f7,f4,85,85);
      p.setBrush(QColor(0,0,0,180+add)); p.drawRoundRect(f2,f4,dpi.f5,f3,90,90);
      p.setCompositionMode( QPainter::CompositionMode_SourceOver );
      lg = QLinearGradient(0,0,0,f9);
      stops << QGradientStop( 0, QColor(255,255,255, 90) )
         << QGradientStop( 0.5, QColor(255,255,255, 170) )
         << QGradientStop( 1, QColor(255,255,255, 90) );
      lg.setStops(stops);
      p.fillRect(f3,f9-f2,f3,f1, lg);
      stops.clear();
      stops << QGradientStop( 0, QColor(255,255,255, 30) )
         << QGradientStop( 0.5, QColor(255,255,255, 100) )
         << QGradientStop( 1, QColor(255,255,255, 30) );
      lg.setStops(stops);
      p.fillRect(f3,f9-f1,f3,f1, lg);
      stops.clear();
      p.end();
   
      shadows.lineEdit[i] = Tile::Set(QPixmap::fromImage(tmpImg),f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   }
   
   // relief
   tmp = QPixmap(f9,f9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   QPen pen = p.pen(); pen.setWidth(f1); p.setPen(pen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::NoBrush);
   p.setPen(QColor(255,255,255,40));
   p.drawRoundRect(0,0,f9-f2,f9-f2,60,60);
   p.setPen(QColor(255,255,255,60));
   p.drawRoundRect(f2,f2,f9-f2,f9-f2,60,60);
   p.setPen(QColor(0,0,0,50));
   p.drawRoundRect(f1,f1,f9-dpi.f3,f9-dpi.f3,60,60);
   p.end();
   shadows.relief = Tile::Set(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);

   
   // outlines
   tmp = QPixmap(f9,f9);
   for (int i = 0; i < 2; ++i) {
      tmp.fill(Qt::transparent);
      p.begin(&tmp);
      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(QColor(255,255,255,100+i*60));
      p.setBrush(Qt::NoBrush);
      p.drawRoundRect(0,0,f9,2*f9,75,38);
      p.end();
      frames.button[i] = Tile::Set(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   }
//    frames.button[0] = Tile::Nuno(100);
//    frames.button[1] = Tile::Nuno(160);
   
   // toplight
   int f49 = SCALE(49);
   int f49_2 = (f49-1)/2;
   tmp = QPixmap(f49,f49);
   tmp.fill(Qt::transparent);
   QRadialGradient rg( tmp.rect().center(), f49_2 );
   rg.setColorAt ( 0, QColor(255,255,255,160) );
   rg.setColorAt ( 1, QColor(255,255,255,0) );
   p.begin(&tmp);
   p.fillRect(0,0,f49,f49,rg);
   p.end();
   tmp = tmp.scaled( f49, dpi.f5, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
   tmp = tmp.copy(0,f2,f49,dpi.f3);
   lights.top = Tile::Line(tmp,Qt::Horizontal,f49_2,-f49_2);
   
   // ================================================================
   
   // RADIOUTTON =====================================
   int rw = dpi.ExclusiveIndicator;
   int rh = dpi.ExclusiveIndicator;
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         shadows.radio[j][i] = QPixmap(rw-f1*2*j, rh-f1*2*j);
         shadows.radio[j][i].fill(Qt::transparent);
         p.begin(&shadows.radio[j][i]);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,(1+i+2*j)*9));
         p.drawEllipse(shadows.radio[j][i].rect());
         if (!j) {
            p.setBrush(QColor(0,0,0,(i+1)*20));
            p.drawEllipse(f2_2,f2_2,rw-f2,rh-f2);
         }
         p.end();
      }
   }
   
   // mask
   rw -= f4; rh -= f4;
   masks.radio = QPixmap(rw, rh);
   masks.radio.fill(Qt::transparent);
   p.begin(&masks.radio);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawEllipse(0,0,rw,rh);
   p.end();
   UPDATE_COLORS(masks.radio);
   
   rw -= f4; rh -= f4;
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
   rw -= f4; rh -= f4;
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
   masks.notch = QPixmap(dpi.f6, dpi.f6);
   masks.notch.fill(Qt::transparent);
   p.begin(&masks.notch);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::black);
   p.drawEllipse(0,0,dpi.f6,dpi.f6);
   p.end();
   UPDATE_COLORS(masks.notch);
   // ================================================================
   
   // RECTANGULAR =====================================
   
   // raised
   
   // sunken
   int f6 = dpi.f6;
   tmp = QPixmap(f9,f9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.fillRect(f1,0,f9-f2,f1, QColor(0,0,0,10));
   p.fillRect(f2,f1,f9-f4,f1, QColor(0,0,0,20));
   p.fillRect(f2,f2,f9-f4,f1, QColor(0,0,0,40));
   p.fillRect(f3,f3,f9-f6,f1, QColor(0,0,0,80));
   
   p.fillRect(f1,f9-f1,f9-f2,f1, QColor(255,255,255,10));
   p.fillRect(f2,f9-f2,f9-f4,f1, QColor(255,255,255,20));
   p.fillRect(f2,f9-f3,f9-f4,f1, QColor(255,255,255,40));
   p.fillRect(f3,f9-f4,f9-f6,f1, QColor(255,255,255,80));
   
   p.fillRect(0,f1,f1,f9-f2, QColor(128,128,128,10));
   p.fillRect(f1,f2,f1,f9-f4, QColor(128,128,128,20));
   p.fillRect(f2,f2,f1,f9-f4, QColor(128,128,128,40));
   p.fillRect(f3,f3,f1,f9-f6, QColor(128,128,128,80));
   
   p.fillRect(f9-f1,f1,f1,f9-f2, QColor(128,128,128,10));
   p.fillRect(f9-f2,f2,f1,f9-f4, QColor(128,128,128,20));
   p.fillRect(f9-f3,f2,f1,f9-f4, QColor(128,128,128,40));
   p.fillRect(f9-f4,f3,f1,f9-f6, QColor(128,128,128,80));
   
   p.end();
   shadows.sunken = Tile::Set(tmp,f9_2,f9_2,f9-2*f9_2,f9-2*f9_2);
   
   // ================================================================
   
   // TABBAR =====================================
   
   // mask
   int f13 = SCALE(13);
   tmp = QPixmap(f13,f13);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,f13,f13,99,99);
   p.end();
   int f13_2 = (f13-1)/2;
   UPDATE_COLORS(tmp);
   masks.tab = Tile::Mask(tmp,f13_2,f13_2,f13-2*f13_2,f13-2*f13_2,0,0,0,0,99,99);
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,f13,f13,99,99);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect(f1,f1,f13-2*f1,f13-2*f1,95,95);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(f2,f2,f13-2*f2,f13-2*f2,93,93);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(f3,f3,f13-2*f3,f13-2*f3,91,91);
   p.end();
   lights.tab =
      Tile::Mask(tmp,f13_2,f13_2,f13-2*f13_2,f13-2*f13_2, f3,f3,-f3,-f3, 91,91);
   
   // shadow
   int f17 = SCALE(17), f17_2 = (f17-1)/2;
   tmpImg = QImage(f17,f17, QImage::Format_ARGB32);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         int add = 5*(i+2*j);
         tmpImg.fill(Qt::transparent);
         p.begin(&tmpImg);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,10+add));
         p.drawRoundRect(0,0,f17,f17,90,90);
         p.setBrush(QColor(0,0,0,13+add));
         p.drawRoundRect(f1,f1,f17-f2,f17-f2,93,93);
         p.setBrush(QColor(0,0,0,18+add));
         p.drawRoundRect(f2,f2,f17-f4,f17-f4,96,96);
         p.setBrush(QColor(0,0,0,23+add));
         p.drawRoundRect(dpi.f3,dpi.f3,f17-dpi.f6,f17-dpi.f6,99,99);
         p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
         p.setBrush(QColor(0,0,0,255));
         p.drawRoundRect(f2,f1,f17-f4,f17-dpi.f5,99,99);
         p.setCompositionMode( QPainter::CompositionMode_SourceOver );
         p.setPen(QColor(255,255,255,170));
         p.setBrush(Qt::NoBrush);
         p.drawRoundRect(f2,f1,f17-f4,f17-dpi.f5,99,99);
         p.end();
         shadows.tab[i][j] =
                  Tile::Set(QPixmap::fromImage(tmpImg),f17_2,f17_2,f17-2*f17_2,f17-2*f17_2);
      }
   }
   
   
   // sunken
   int f15 = SCALE(15), f11 = SCALE(11);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,85)); p.drawRoundRect(0,0,f17,f17-f2,80,80);
   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   p.setBrush(QColor(0,0,0,120)); p.drawRoundRect(0,f1,f17,f17-f3,75,75);
   p.setBrush(QColor(0,0,0,140)); p.drawRoundRect(0,f2,f17,dpi.f13,80,80);
   p.setBrush(QColor(0,0,0,160)); p.drawRoundRect(f1,f3,f15,dpi.f12,85,85);
   p.setBrush(QColor(0,0,0,180)); p.drawRoundRect(f2,dpi.f4,dpi.f13,f11,90,90);
   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   lg = QLinearGradient(0,0,0,f17);
   stops << QGradientStop( 0, QColor(255,255,255, 90) )
      << QGradientStop( 0.5, QColor(255,255,255, 190) )
      << QGradientStop( 1, QColor(255,255,255, 90) );
   lg.setStops(stops);
   p.fillRect(f3,f17-f2,dpi.f12,f1, lg);
   stops.clear();
   stops << QGradientStop( 0, QColor(255,255,255, 30) )
      << QGradientStop( 0.5, QColor(255,255,255, 100) )
      << QGradientStop( 1, QColor(255,255,255, 30) );
   lg.setStops(stops);
   p.fillRect(f3,f17-f1,dpi.f12,f1, lg);
   stops.clear();
   p.end();
   shadows.tabSunken = Tile::Set(QPixmap::fromImage(tmpImg),f17_2,f17_2,f17-2*f17_2,f17-2*f17_2);
   
   // GROUPBOX =====================================
   
   // shadow
   tmpImg = QImage(f49,f49, QImage::Format_ARGB32);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,5)); p.drawRoundRect(0,0,f49,2*f49,14,7);
   p.setBrush(QColor(0,0,0,9)); p.drawRoundRect(f1,f1,f49-f2,2*f49,13,7);
   p.setBrush(QColor(0,0,0,11)); p.drawRoundRect(f2,f2,f49-dpi.f4,2*f49,12,6);
   p.setBrush(QColor(0,0,0,13)); p.drawRoundRect(dpi.f3,dpi.f3,f49-dpi.f6,2*f49,48,24);
   p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   p.setBrush(QColor(0,0,0,0)); p.drawRoundRect(dpi.f4,dpi.f2,f49-dpi.f8,2*f49,11,6);
//    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
//    p.setPen(QColor(255,255,255,200)); p.setBrush(Qt::NoBrush);
//    p.drawRoundRect(dpi.f4,dpi.f2,f49-dpi.f8,2*f49,11,6);
   p.setRenderHint(QPainter::Antialiasing, false);
//    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   int f33 = SCALE(33);
   for (int i = 1; i < f33; ++i) {
      p.setPen(QColor(0,0,0,CLAMP(i*lround(255.0/dpi.f32),0,255)));
      p.drawLine(0, f49-i, f49, f49-i);
   }
   p.end();
   int f12 = dpi.f12;
   shadows.group = Tile::Set(QPixmap::fromImage(tmpImg),f12,f12,f49-2*f12,f1);
   
   // mask --> uses buttons
//    int f25 = SCALE(25);
//    tmp = QPixmap(f25,f25);
//    tmp.fill(Qt::transparent);
//    p.begin(&tmp);
//    p.setPen(Qt::NoPen);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setBrush(QColor(0,0,0,255));
//    p.drawRoundRect(0,0,f25,f25,22,22);
//    p.end();
//    f12 = (f25-1)/2;
//    masks.group = Tile::Mask(tmp,f12,f12,f25-2*f12,f25-2*f12,0,0,0,0,22,22);
   
   // shadow line
   int w,h,c1,c2;
   for (int i = 0; i < 2; ++i) { // orientarion
      if (i) {
         w = f2; h = f49;
         lg = QLinearGradient(0,0,0,f49);
      }
      else {
         w = f49; h = f2;
         lg = QLinearGradient(0,0,f49,0);
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
            p.fillRect(0,0,f1,f49,lg);
         }
         else {
            p.fillRect(0,0,f49,f1,lg);
         }
         stops.clear();
         stops << QGradientStop( 0, QColor(c2,c2,c2,0) )
            << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
            << QGradientStop( 1, QColor(c2,c2,c2,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect(f1,0,f2-f1,f49,lg);
         }
         else {
            p.fillRect(0,f1,f49,f2-f1,lg);
         }
         stops.clear();
         p.end();
         shadows.line[i][j] =
            Tile::Line(tmp, i ? Qt::Vertical : Qt::Horizontal, f49_2, -f49_2);
      }
   }
   
   // slider handles =================================================
   QPoint triangle[3] = { QPoint(0, 100), QPoint(-100, -100), QPoint(100, -100) };
   int size;
   for (int i = 0; i < 4; ++i) { // direction
      size = dpi.SliderControl;
      for (int j = 0; j < 2; ++j) { // sunken?
         if (j) size -= f2;
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
      size = dpi.SliderControl - f4;
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
//    QRegion circle = QRegion(0,0,2*f9,2*f9, QRegion::Ellipse);
//    QRegion rect = QRegion(0,0,f9,f9);
//    masks.corner[0] = rect - (circle & rect);
//    circle.translate(-f9, 0);
//    masks.corner[1] = rect - (circle & rect);
//    circle.translate(0, -f9);
//    masks.corner[3] = rect - (circle & rect);
//    circle.translate(f9, 0);
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
   int f14 = SCALE(14);// f15 = SCALE(15), f16 = dpi.f16;
   _INITPIX_(masks.winClose, f14,f14);
   p.setBrush(Qt::black);
   _CLOSE_ARROW_(masks.winClose, 0);
   p.end();
   UPDATE_COLORS(masks.winClose);
   _INITPIX_(masks.winMin,f14,f14);
   p.setBrush(Qt::black);
   _MIN_ARROW_(masks.winMin, 0);
   p.end();
   UPDATE_COLORS(masks.winMin);
   _INITPIX_(masks.winMax,f14,f14);
   p.setBrush(Qt::black);
   _MAX_ARROW_(masks.winMax, 0);
   p.end();
   UPDATE_COLORS(masks.winMax);
   /*
   shadows.winClose[0] = QPixmap(f16, f16);
   shadows.winMin[0] = QPixmap(f16, f16);
   shadows.winMax[0] = QPixmap(f16, f16);
   shadows.winClose[0] = QPixmap(f15, f15);
   shadows.winMin[0] = QPixmap(f15, f15);
   shadows.winMax[0] = QPixmap(f15, f15);
   */
   // ================================================================
}
#undef fillRect
