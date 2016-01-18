#!/usr/bin/env ruby

require 'json'

`grep -rPIHn '\t' src/* include/* samples/*`.lines { |line|
  if /(?<filename>.+(hpp|cpp|chai)):(?<linenumber>[0-9]+):(?<restofline>.+)/ =~ line 
    puts(JSON.dump({:line => linenumber, :filename => filename, :tool => "tab_checker", :message => "Source Code Line Contains Tabs", :messagetype => "warning"}))
  end
}


