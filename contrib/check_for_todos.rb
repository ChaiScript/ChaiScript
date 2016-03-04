#!/usr/bin/env ruby

require 'json'

`grep -rPIHni 'todo' src/* include/* samples/*`.lines { |line|
  if /(?<filename>.+(hpp|cpp|chai)):(?<linenumber>[0-9]+):(?<restofline>.+)/ =~ line 
    puts(JSON.dump({:line => linenumber, :filename => filename, :tool => "todo_checker", :message => "todo: #{restofline.strip}", :messagetype => "info"}))
  end
}


