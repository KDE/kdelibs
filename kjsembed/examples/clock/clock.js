

function updateTime() {
    now = new Date();

    var hours = now.getHours();
    hours = hours < 10 ? '0' + hours : hours;
    var mins = now.getMinutes();
    mins = mins < 10 ? '0' + mins : mins;

    lcd.display( hours + ':' + mins );
}

lcd = new QLCDNumber(this);
timer = new QTimer( this );

connect( timer, 'timeout()', this, 'updateTime()' );
timer.start(500,false); // Workaround: specify false

lcd.show();
exec();
