require 'bunny'

conn = Bunny.new
conn.start

ch = conn.channel
ch.confirm_select

x = conn.direct('hello-exchange', :durable => true)

msg = ARGV[0]
x.publish(msg, :routing_key => 'hola')

ch.wait_for_confirms

sleep 10