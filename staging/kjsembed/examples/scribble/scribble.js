#!/usr/bin/env kjscmd

function scribble() {

  frame = new QWidget("QFrame", this);
  frame.startX = 0;
  frame.startY = 0;
  frame.endX = 0;
  frame.endY = 0; 

  frame.onMouseButtonPressEvent = function ( ev ) {
    println('Start');
    this.startX = ev.x;
    this.startY = ev.y;
  }

  frame.onMouseButtonReleaseEvent = function ( ev ) {
    println('End');
    this.endX = ev.x;
    this.endY = ev.y;
    this.update();
  }

  frame.onPaintEvent = function ( ev ) {
    println('Paint');
    var p = new QPainter();
    p.begin( this );
    p.drawLine( this.startX, this.startY, this.endX, this.endY );
    p.end();  
  }

  return frame;

}

frame = scribble();
frame.show();
exec();
