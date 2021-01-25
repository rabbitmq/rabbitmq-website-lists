require 'bunny'

class HelloConsumer < Bunny::Consumer
end

conn = Bunny.new
conn.start

ch = conn.channel
x = conn.direct('hello-exchange', :durable => true)
q = ch.queue('hello-queue', :durable => true)
q.bind(x, :routing_key => 'hola')

consumer = HelloConsumer.new(ch, q, 'hello-consumer')

consumer.on_delivery do |delivery_info, properties, payload|
  if payload == 'quit'
    ch.work_pool.shutdown
  else
    puts payload
  end
end

q.subscribe_with(consumer, :block => true)