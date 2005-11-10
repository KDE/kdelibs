#!/usr/bin/env ruby

puts "What's the name of the new class?"
classname = gets.strip

if File.exists?(classname.downcase + '.h') or File.exists?(classname.downcase + '.cpp') or File.exists?("ifaces/#{classname.downcase}.h") or File.exists?("ifaces/#{classname.downcase}.cpp")
  puts "The class seems to exist already!"
  puts "aborting"
  exit -1
end

system("svn copy template.h #{classname.downcase}.h")
system("svn copy template.cpp #{classname.downcase}.cpp")
template_h   = File.new("template.h", 'r');
template_cpp = File.new("template.cpp", 'r');
hfile   = File.new(classname.downcase + ".h", 'w');
cppfile = File.new(classname.downcase + ".cpp", 'w');

hfile.write(template_h.read.gsub(/Template/, classname).gsub(/TEMPLATE/, classname.upcase).gsub(/template/, classname.downcase))
cppfile.write(template_cpp.read.gsub(/Template/, classname).gsub(/TEMPLATE/, classname.upcase).gsub(/template/, classname.downcase))

Dir.chdir("ifaces")

system("svn copy template.h #{classname.downcase}.h")
system("svn copy template.cpp #{classname.downcase}.cpp")
template_h   = File.new("template.h", 'r');
template_cpp = File.new("template.cpp", 'r');
hfile   = File.new(classname.downcase + ".h", 'w');
cppfile = File.new(classname.downcase + ".cpp", 'w');

hfile.write(template_h.read.gsub(/Template/, classname).gsub(/TEMPLATE/, classname.upcase).gsub(/template/, classname.downcase))
cppfile.write(template_cpp.read.gsub(/Template/, classname).gsub(/TEMPLATE/, classname.upcase).gsub(/template/, classname.downcase))
