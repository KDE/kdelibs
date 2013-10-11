#!/usr/bin/env kjscmd


function Calculator(ui)
{
  // Setup entry functions
  var display = ui.findChild('display');
  this.display = display;

  this.one = function() { display.intValue = display.intValue*10+1; }
  this.two = function() { display.intValue = display.intValue*10+2; }
  this.three = function() { display.intValue = display.intValue*10+3; }
  this.four = function() { display.intValue = display.intValue*10+4; }
  this.five = function() { display.intValue = display.intValue*10+5; }
  this.six = function() { display.intValue = display.intValue*10+6; }
  this.seven = function() { display.intValue = display.intValue*10+7; }
  this.eight = function() { display.intValue = display.intValue*10+8; }
  this.nine = function() { display.intValue = display.intValue*10+9; }
  this.zero = function() { display.intValue = display.intValue*10+0; }

  ui.connect( ui.findChild('one'), 'clicked()', this, 'one()' );
  ui.connect( ui.findChild('two'), 'clicked()', this, 'two()' );
  ui.connect( ui.findChild('three'), 'clicked()', this, 'three()' );
  ui.connect( ui.findChild('four'), 'clicked()', this, 'four()' );
  ui.connect( ui.findChild('five'), 'clicked()', this, 'five()' );
  ui.connect( ui.findChild('six'), 'clicked()', this, 'six()' );
  ui.connect( ui.findChild('seven'), 'clicked()', this, 'seven()' );
  ui.connect( ui.findChild('eight'), 'clicked()', this, 'eight()' );
  ui.connect( ui.findChild('nine'), 'clicked()', this, 'nine()' );
  ui.connect( ui.findChild('zero'), 'clicked()', this, 'zero()' );

  this.val = 0;
  this.display.intValue = 0;
  this.lastop = function() {}

  this.plus = function()
              {
                 this.val = display.intValue+this.val;
                 display.intValue = 0;
                 this.lastop=this.plus
              }

  this.minus = function()
               {
                  this.val = display.intValue-this.val;
                  display.intValue = 0;
                  this.lastop=this.minus;
               }


  ui.connect( ui.findChild('plus'), 'clicked()', this, 'plus()' );
  ui.connect( ui.findChild('minus'), 'clicked()', this, 'minus()' );

  this.equals = function() { this.lastop(); display.intValue = this.val; }
  this.clear = function() { this.lastop=function(){}; display.intValue = 0; this.val = 0; }

  ui.connect( ui.findChild('equals'), 'clicked()', this, 'equals()' );
  ui.connect( ui.findChild('clear'), 'clicked()', this, 'clear()' );
}

var loader = new QUiLoader();
var ui = loader.load('calc.ui', this);
var calc = new Calculator(ui);

ui.show();
exec();

