// -*- java -*-
// Bootstrap the JS

var clocklabel = null;
var now;
var timer;

function updateTime() {
    if ( clocklabel == null ) // Hack to avoid timer weirdness
	return;
    now = new Date();

    var hours = now.getHours();
    hours = hours < 10 ? '0' + hours : hours;
    var mins = now.getMinutes();
    mins = mins < 10 ? '0' + mins : mins;

//    clocklabel.display( hours + ':' + mins );
    clocklabel.text= hours + ':' + mins;
}


function clock() {
    try {
	if ( clocklabel == null ) {
	    clocklabel = new Widget( 'QLabel', workspace );
//	    clocklabel = new QLCDNumber( workspace );
	    updateTime();
	    clocklabel.show();
	}
	else {
	    updateTime();
	}

	timer = new QTimer( workspace );
	connect( timer, 'timeout()', this, 'updateTime()' );
	timer.start(500,false); // Workaround: specify false

    }
    catch( err ) {
	println( 'clockAction failed: ' + err );
    }
}

function fullAction(yesno) {
    try {
	println( 'argument is ' + yesno );

	if ( yesno ) {
	    window.showFullScreen();
	}
	else {
	    window.showNormal();
	}
    }
    catch( err ) {
	println( 'fullAction failed: ' + err );
    }
}

function templateAction() {
    try {
	println( 'We love KJSEmbed!!!' );

	var form = new Form( 'setup_template.ui', 0 );
	var result = form.exec();
	if ( result == 0 )
	    return;

	var title = form.titleEdit.text;
	var doCreate = form.createBox.checked;

	println( 'Title:' + title );
	println( 'Create:' + doCreate );

	println( workspace );
	var w = workspace.activeWindow();
	println( 'xxxxxxxxxxxxxxxxx' );
	if ( w == null )
	    return;
	println( '5434584' );

	var now = new Date();
	w.append( '<h1>' + title + '<h1>' + '<i>Enter your text here</i>' );
	println( '4593485' );
	if ( doCreate )
	    w.append( '<hr>Created: ' + now.toString() );
    }
    catch( err ) {
	println( 'templateAction failed: ' + err );
    }
}

try {
    // Template
    var act = new QAction();
    act.text = "Insert Template";
    act.icon = new QIcon( 'images/js.png' );

    editToolBar.addAction( act );

    connect( act, 'triggered(bool)', this, 'templateAction()' );

    // Full screen
    var act2 = new QAction();
    act2.text = "Full Screen";
    act2.checkable = true;
    act2.icon = new QIcon( 'images/window_fullscreen.png' );

    editToolBar.addAction( act2 );

    connect( act2, 'triggered(bool)', this, 'fullAction(bool)' );

    // Clock
    var act3 = new QAction();
    act3.text = "Clock";
    act3.icon = new QIcon( 'images/today.png' );

    editToolBar.addAction( act3 );
    connect( act3, 'triggered(bool)', this, 'clock()' );

    println( 'Bootstrap script finished OK!!' );

}
catch( e ) {
  println( 'Bootstrap failed: ' + e );
}

