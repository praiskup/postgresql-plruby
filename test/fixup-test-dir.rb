#!/usr/bin/ruby
require 'rbconfig'

suffix = ARGV[0].to_s

dir = File.expand_path('../..', Dir.pwd)
extension = "#{dir}/src/plruby#{suffix}.#{RbConfig::CONFIG["DLEXT"]}"
if ENV["PLRUBYDIR"] != nil
    extension = "#{ENV["PLRUBYDIR"]}/plruby#{suffix}.#{RbConfig::CONFIG["DLEXT"]}"
end

begin
   Dir.glob('*.in') do |file|
      f = File.new(file.gsub(".in", ""), "w")
      IO.foreach(file) do |x| 
         x.gsub!(/language\s+'plruby'/i, "language 'plruby#{suffix}'")
         f.print x
      end
      f.close
   end

   f = File.new("test_mklang.sql", "w")
   f.print <<EOF
 
   create function plruby#{suffix}_call_handler() returns language_handler
    as '#{extension}'
   language c;
 
   create trusted procedural language 'plruby#{suffix}'
        handler plruby#{suffix}_call_handler;
EOF
   f.close
rescue
   raise "Why I can't write #$!"
end
