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
   int _1 = dpi._1, _2 = dpi._2, _2_2 = lround(_2/2.0);
   int _9 = dpi._9, _9_2 = (_9-1)/2;
   int _7 = dpi._7, _3 = dpi._3, _4 = dpi._4;
   tmp = QPixmap(_9,_9);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         tmp.fill(Qt::transparent);
         p.begin(&tmp);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,((j?5:1)+i)*6));
         p.drawRoundRect(0,0,_9,_9,90,90);
         if (!j) {
            p.setBrush(QColor(0,0,0,(1+i)*10));
            p.drawRoundRect(_1,_1,_9-2*_1,_9-2*_1,75,75);
            p.setBrush(QColor(0,0,0,(1+i)*14));
            p.drawRoundRect(_2,_2,_9-2*_2,_9-2*_2,60,60);
         }
         p.end();
         shadows.button[j][i] = Tile::Set(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2);
      }
   }
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,_9,_9,90,90);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect(_1,_1,_9-2*_1,_9-2*_1,80,80);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(_2,_2,_9-2*_2,_9-2*_2,70,70);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(_3,_3,_9-2*_3,_9-2*_3,60,60);
   p.end();
   lights.button =
      Tile::Mask(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2, _3,_3,-_3,-_3, 75,75);
   
   // mask
   tmp = QPixmap(_9,_9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,_9,_9,60,60);
   p.end();
   UPDATE_COLORS(tmp);
   masks.button = Tile::Mask(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2,0,0,0,0,60,60);
   
   // -> sunken
   QLinearGradient lg; QGradientStops stops;
   QImage tmpImg(_9,_9, QImage::Format_ARGB32);
   
   for (int i = 0; i < 2; ++i) {
      int add = i*30;
      tmpImg.fill(Qt::transparent);
   
      p.begin(&tmpImg);
      p.setPen(Qt::NoPen);
      p.setRenderHint(QPainter::Antialiasing);
      p.setBrush(QColor(0,0,0,55+add)); p.drawRoundRect(0,0,_9,_7,80,80);
      p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
      add = 30 - add;
      p.setBrush(QColor(0,0,0,120+add)); p.drawRoundRect(0,_1,_9,dpi._6,75,75);
      p.setBrush(QColor(0,0,0,140+add)); p.drawRoundRect(0,_2,_9,dpi._5,80,80);
      p.setBrush(QColor(0,0,0,160+add)); p.drawRoundRect(_1,_3,_7,_4,85,85);
      p.setBrush(QColor(0,0,0,180+add)); p.drawRoundRect(_2,_4,dpi._5,_3,90,90);
      p.setCompositionMode( QPainter::CompositionMode_SourceOver );
      lg = QLinearGradient(0,0,0,_9);
      stops << QGradientStop( 0, QColor(255,255,255, 90) )
         << QGradientStop( 0.5, QColor(255,255,255, 170) )
         << QGradientStop( 1, QColor(255,255,255, 90) );
      lg.setStops(stops);
      p.fillRect(_3,_9-_2,_3,_1, lg);
      stops.clear();
      stops << QGradientStop( 0, QColor(255,255,255, 30) )
         << QGradientStop( 0.5, QColor(255,255,255, 100) )
         << QGradientStop( 1, QColor(255,255,255, 30) );
      lg.setStops(stops);
      p.fillRect(_3,_9-_1,_3,_1, lg);
      stops.clear();
      p.end();
   
      shadows.lineEdit[i] = Tile::Set(QPixmap::fromImage(tmpImg),_9_2,_9_2,_9-2*_9_2,_9-2*_9_2);
   }
   
   // relief
   tmp = QPixmap(_9,_9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   QPen pen = p.pen(); pen.setWidth(_1); p.setPen(pen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::NoBrush);
   p.setPen(QColor(255,255,255,40));
   p.drawRoundRect(0,0,_9-_2,_9-_2,60,60);
   p.setPen(QColor(255,255,255,60));
   p.drawRoundRect(_2,_2,_9-_2,_9-_2,60,60);
   p.setPen(QColor(0,0,0,50));
   p.drawRoundRect(_1,_1,_9-dpi._3,_9-dpi._3,60,60);
   p.end();
   shadows.relief = Tile::Set(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2);

   
   // outlines
   tmp = QPixmap(_9,_9);
   for (int i = 0; i < 2; ++i) {
      tmp.fill(Qt::transparent);
      p.begin(&tmp);
      p.setRenderHint(QPainter::Antialiasing);
      p.setPen(QColor(255,255,255,100+i*60));
      p.setBrush(Qt::NoBrush);
      p.drawRoundRect(0,0,_9,2*_9,75,38);
      p.end();
      frames.button[i] = Tile::Set(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2);
   }
//    frames.button[0] = Tile::Nuno(100);
//    frames.button[1] = Tile::Nuno(160);
   
   // toplight
   int _49 = SCALE(49);
   int _49_2 = (_49-1)/2;
   tmp = QPixmap(_49,_49);
   tmp.fill(Qt::transparent);
   QRadialGradient rg( tmp.rect().center(), _49_2 );
   rg.setColorAt ( 0, QColor(255,255,255,160) );
   rg.setColorAt ( 1, QColor(255,255,255,0) );
   p.begin(&tmp);
   p.fillRect(0,0,_49,_49,rg);
   p.end();
   tmp = tmp.scaled( _49, dpi._5, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
   tmp = tmp.copy(0,_2,_49,dpi._3);
   lights.top = Tile::Line(tmp,Qt::Horizontal,_49_2,-_49_2);
   
   // ================================================================
   
   // RADIOUTTON =====================================
   int rw = dpi.ExclusiveIndicator;
   int rh = dpi.ExclusiveIndicator;
   // shadow
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         shadows.radio[j][i] = QPixmap(rw-_1*2*j, rh-_1*2*j);
         shadows.radio[j][i].fill(Qt::transparent);
         p.begin(&shadows.radio[j][i]);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,(1+i+2*j)*9));
         p.drawEllipse(shadows.radio[j][i].rect());
         if (!j) {
            p.setBrush(QColor(0,0,0,(i+1)*20));
            p.drawEllipse(_2_2,_2_2,rw-_2,rh-_2);
         }
         p.end();
      }
   }
   
   // mask
   rw -= _4; rh -= _4;
   masks.radio = QPixmap(rw, rh);
   masks.radio.fill(Qt::transparent);
   p.begin(&masks.radio);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawEllipse(0,0,rw,rh);
   p.end();
   UPDATE_COLORS(masks.radio);
   
   rw -= _4; rh -= _4;
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
   rw -= _4; rh -= _4;
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
   masks.notch = QPixmap(dpi._6, dpi._6);
   masks.notch.fill(Qt::transparent);
   p.begin(&masks.notch);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(Qt::black);
   p.drawEllipse(0,0,dpi._6,dpi._6);
   p.end();
   UPDATE_COLORS(masks.notch);
   // ================================================================
   
   // RECTANGULAR =====================================
   
   // raised
   
   // sunken
   int _6 = dpi._6;
   tmp = QPixmap(_9,_9);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.fillRect(_1,0,_9-_2,_1, QColor(0,0,0,10));
   p.fillRect(_2,_1,_9-_4,_1, QColor(0,0,0,20));
   p.fillRect(_2,_2,_9-_4,_1, QColor(0,0,0,40));
   p.fillRect(_3,_3,_9-_6,_1, QColor(0,0,0,80));
   
   p.fillRect(_1,_9-_1,_9-_2,_1, QColor(255,255,255,10));
   p.fillRect(_2,_9-_2,_9-_4,_1, QColor(255,255,255,20));
   p.fillRect(_2,_9-_3,_9-_4,_1, QColor(255,255,255,40));
   p.fillRect(_3,_9-_4,_9-_6,_1, QColor(255,255,255,80));
   
   p.fillRect(0,_1,_1,_9-_2, QColor(128,128,128,10));
   p.fillRect(_1,_2,_1,_9-_4, QColor(128,128,128,20));
   p.fillRect(_2,_2,_1,_9-_4, QColor(128,128,128,40));
   p.fillRect(_3,_3,_1,_9-_6, QColor(128,128,128,80));
   
   p.fillRect(_9-_1,_1,_1,_9-_2, QColor(128,128,128,10));
   p.fillRect(_9-_2,_2,_1,_9-_4, QColor(128,128,128,20));
   p.fillRect(_9-_3,_2,_1,_9-_4, QColor(128,128,128,40));
   p.fillRect(_9-_4,_3,_1,_9-_6, QColor(128,128,128,80));
   
   p.end();
   shadows.sunken = Tile::Set(tmp,_9_2,_9_2,_9-2*_9_2,_9-2*_9_2);
   
   // ================================================================
   
   // TABBAR =====================================
   
   // mask
   int _13 = SCALE(13);
   tmp = QPixmap(_13,_13);
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,255));
   p.drawRoundRect(0,0,_13,_13,99,99);
   p.end();
   int _13_2 = (_13-1)/2;
   UPDATE_COLORS(tmp);
   masks.tab = Tile::Mask(tmp,_13_2,_13_2,_13-2*_13_2,_13-2*_13_2,0,0,0,0,99,99);
   
   // light
   tmp.fill(Qt::transparent);
   p.begin(&tmp);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,20));
   p.drawRoundRect(0,0,_13,_13,99,99);
   p.setBrush(QColor(0,0,0,44));
   p.drawRoundRect(_1,_1,_13-2*_1,_13-2*_1,95,95);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(_2,_2,_13-2*_2,_13-2*_2,93,93);
   p.setBrush(QColor(0,0,0,64));
   p.drawRoundRect(_3,_3,_13-2*_3,_13-2*_3,91,91);
   p.end();
   lights.tab =
      Tile::Mask(tmp,_13_2,_13_2,_13-2*_13_2,_13-2*_13_2, _3,_3,-_3,-_3, 91,91);
   
   // shadow
   int _17 = SCALE(17), _17_2 = (_17-1)/2;
   tmpImg = QImage(_17,_17, QImage::Format_ARGB32);
   for (int i = 0; i < 2; ++i) { // opaque?
      for (int j = 0; j < 2; ++j) { // sunken?
         int add = 5*(i+2*j);
         tmpImg.fill(Qt::transparent);
         p.begin(&tmpImg);
         p.setPen(Qt::NoPen);
         p.setRenderHint(QPainter::Antialiasing);
         p.setBrush(QColor(0,0,0,10+add));
         p.drawRoundRect(0,0,_17,_17,90,90);
         p.setBrush(QColor(0,0,0,13+add));
         p.drawRoundRect(_1,_1,_17-_2,_17-_2,93,93);
         p.setBrush(QColor(0,0,0,18+add));
         p.drawRoundRect(_2,_2,_17-_4,_17-_4,96,96);
         p.setBrush(QColor(0,0,0,23+add));
         p.drawRoundRect(dpi._3,dpi._3,_17-dpi._6,_17-dpi._6,99,99);
         p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
         p.setBrush(QColor(0,0,0,255));
         p.drawRoundRect(_2,_1,_17-_4,_17-dpi._5,99,99);
         p.setCompositionMode( QPainter::CompositionMode_SourceOver );
         p.setPen(QColor(255,255,255,170));
         p.setBrush(Qt::NoBrush);
         p.drawRoundRect(_2,_1,_17-_4,_17-dpi._5,99,99);
         p.end();
         shadows.tab[i][j] =
                  Tile::Set(QPixmap::fromImage(tmpImg),_17_2,_17_2,_17-2*_17_2,_17-2*_17_2);
      }
   }
   
   
   // sunken
   int _15 = SCALE(15), _11 = SCALE(11);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,85)); p.drawRoundRect(0,0,_17,_17-_2,80,80);
   p.setCompositionMode( QPainter::CompositionMode_DestinationOut );
   p.setBrush(QColor(0,0,0,120)); p.drawRoundRect(0,_1,_17,_17-_3,75,75);
   p.setBrush(QColor(0,0,0,140)); p.drawRoundRect(0,_2,_17,dpi._13,80,80);
   p.setBrush(QColor(0,0,0,160)); p.drawRoundRect(_1,_3,_15,dpi._12,85,85);
   p.setBrush(QColor(0,0,0,180)); p.drawRoundRect(_2,dpi._4,dpi._13,_11,90,90);
   p.setCompositionMode( QPainter::CompositionMode_SourceOver );
   lg = QLinearGradient(0,0,0,_17);
   stops << QGradientStop( 0, QColor(255,255,255, 90) )
      << QGradientStop( 0.5, QColor(255,255,255, 190) )
      << QGradientStop( 1, QColor(255,255,255, 90) );
   lg.setStops(stops);
   p.fillRect(_3,_17-_2,dpi._12,_1, lg);
   stops.clear();
   stops << QGradientStop( 0, QColor(255,255,255, 30) )
      << QGradientStop( 0.5, QColor(255,255,255, 100) )
      << QGradientStop( 1, QColor(255,255,255, 30) );
   lg.setStops(stops);
   p.fillRect(_3,_17-_1,dpi._12,_1, lg);
   stops.clear();
   p.end();
   shadows.tabSunken = Tile::Set(QPixmap::fromImage(tmpImg),_17_2,_17_2,_17-2*_17_2,_17-2*_17_2);
   
   // GROUPBOX =====================================
   
   // shadow
   tmpImg = QImage(_49,_49, QImage::Format_ARGB32);
   tmpImg.fill(Qt::transparent);
   p.begin(&tmpImg);
   p.setPen(Qt::NoPen);
   p.setRenderHint(QPainter::Antialiasing);
   p.setBrush(QColor(0,0,0,5)); p.drawRoundRect(0,0,_49,2*_49,14,7);
   p.setBrush(QColor(0,0,0,9)); p.drawRoundRect(_1,_1,_49-_2,2*_49,13,7);
   p.setBrush(QColor(0,0,0,11)); p.drawRoundRect(_2,_2,_49-dpi._4,2*_49,12,6);
   p.setBrush(QColor(0,0,0,13)); p.drawRoundRect(dpi._3,dpi._3,_49-dpi._6,2*_49,48,24);
   p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   p.setBrush(QColor(0,0,0,0)); p.drawRoundRect(dpi._4,dpi._2,_49-dpi._8,2*_49,11,6);
//    p.setCompositionMode( QPainter::CompositionMode_SourceOver );
//    p.setPen(QColor(255,255,255,200)); p.setBrush(Qt::NoBrush);
//    p.drawRoundRect(dpi._4,dpi._2,_49-dpi._8,2*_49,11,6);
   p.setRenderHint(QPainter::Antialiasing, false);
//    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
   int _33 = SCALE(33);
   for (int i = 1; i < _33; ++i) {
      p.setPen(QColor(0,0,0,CLAMP(i*lround(255.0/dpi._32),0,255)));
      p.drawLine(0, _49-i, _49, _49-i);
   }
   p.end();
   int _12 = dpi._12;
   shadows.group = Tile::Set(QPixmap::fromImage(tmpImg),_12,_12,_49-2*_12,_1);
   
   // mask --> uses buttons
//    int _25 = SCALE(25);
//    tmp = QPixmap(_25,_25);
//    tmp.fill(Qt::transparent);
//    p.begin(&tmp);
//    p.setPen(Qt::NoPen);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setBrush(QColor(0,0,0,255));
//    p.drawRoundRect(0,0,_25,_25,22,22);
//    p.end();
//    _12 = (_25-1)/2;
//    masks.group = Tile::Mask(tmp,_12,_12,_25-2*_12,_25-2*_12,0,0,0,0,22,22);
   
   // shadow line
   int w,h,c1,c2;
   for (int i = 0; i < 2; ++i) { // orientarion
      if (i) {
         w = _2; h = _49;
         lg = QLinearGradient(0,0,0,_49);
      }
      else {
         w = _49; h = _2;
         lg = QLinearGradient(0,0,_49,0);
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
            p.fillRect(0,0,_1,_49,lg);
         }
         else {
            p.fillRect(0,0,_49,_1,lg);
         }
         stops.clear();
         stops << QGradientStop( 0, QColor(c2,c2,c2,0) )
            << QGradientStop( 0.5, QColor(c2,c2,c2,74) )
            << QGradientStop( 1, QColor(c2,c2,c2,0) );
         lg.setStops(stops);
         if (i) {
            p.fillRect(_1,0,_2-_1,_49,lg);
         }
         else {
            p.fillRect(0,_1,_49,_2-_1,lg);
         }
         stops.clear();
         p.end();
         shadows.line[i][j] =
            Tile::Line(tmp, i ? Qt::Vertical : Qt::Horizontal, _49_2, -_49_2);
      }
   }
   
   // slider handles =================================================
   QPoint triangle[3] = { QPoint(0, 100), QPoint(-100, -100), QPoint(100, -100) };
   int size;
   for (int i = 0; i < 4; ++i) { // direction
      size = dpi.SliderControl;
      for (int j = 0; j < 2; ++j) { // sunken?
         if (j) size -= _2;
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
      size = dpi.SliderControl - _4;
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
//    QRegion circle = QRegion(0,0,2*_9,2*_9, QRegion::Ellipse);
//    QRegion rect = QRegion(0,0,_9,_9);
//    masks.corner[0] = rect - (circle & rect);
//    circle.translate(-_9, 0);
//    masks.corner[1] = rect - (circle & rect);
//    circle.translate(0, -_9);
//    masks.corner[3] = rect - (circle & rect);
//    circle.translate(_9, 0);
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
   int _14 = SCALE(14);// _15 = SCALE(15), _16 = dpi._16;
   _INITPIX_(masks.winClose, _14,_14);
   p.setBrush(Qt::black);
   _CLOSE_ARROW_(masks.winClose, 0);
   p.end();
   UPDATE_COLORS(masks.winClose);
   _INITPIX_(masks.winMin,_14,_14);
   p.setBrush(Qt::black);
   _MIN_ARROW_(masks.winMin, 0);
   p.end();
   UPDATE_COLORS(masks.winMin);
   _INITPIX_(masks.winMax,_14,_14);
   p.setBrush(Qt::black);
   _MAX_ARROW_(masks.winMax, 0);
   p.end();
   UPDATE_COLORS(masks.winMax);
   /*
   shadows.winClose[0] = QPixmap(_16, _16);
   shadows.winMin[0] = QPixmap(_16, _16);
   shadows.winMax[0] = QPixmap(_16, _16);
   shadows.winClose[0] = QPixmap(_15, _15);
   shadows.winMin[0] = QPixmap(_15, _15);
   shadows.winMax[0] = QPixmap(_15, _15);
   */
   // ================================================================
}
#undef fillRect
