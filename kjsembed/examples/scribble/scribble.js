
function scribble() {

  frame = new Widget("QFrame", this);

  frame.onMouseButtonPressEvent = function ( ev ) {
    this.startX = ev.x;
    this.startY = ev.y;
    println('Start');
  }

  frame.onMouseButtonReleaseEvent = function ( ev ) {
    this.endX = ev.x;
    this.endY = ev.y;
    println('End');
    this.update();
  }

  frame.onPaintEvent = function ( ev ) {
    println('Paint');
    var p = new QPainter();
    p.begin( this );
    p.drawLine( startX, startY, endX, endY );
    p.end();
    println('Yo');
  }

  return frame;
}

frame = scribble();
frame.show();
exec();
