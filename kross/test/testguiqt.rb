#!/usr/bin/env krosstest

#require 'Qt'
require 'korundum4'
require 'TestObject1'

def test()
    label = KDE::SqueezedTextLabel.new()
    #label = Qt::Label.new()

    label.setObjectName("MyQtLabelObjectName")
    obj = TestObject1.func_qwidget_qwidget(label)
    puts "####### testguiqt.rb test() label=" + label.to_s + " object=" + obj.to_s
end

test()
