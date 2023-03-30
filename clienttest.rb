

require 'socket'

s = TCPSocket.new 'localhost', 8059

s.write("Hello World!")

s.each_line do |line|
    puts line
end

s.close