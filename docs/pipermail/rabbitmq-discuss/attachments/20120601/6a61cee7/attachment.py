
import pika
from pika.adapters import BlockingConnection
import json
import sys

pika.log.setup(color=True)

parameters = pika.ConnectionParameters('rabbitmq_host')

connection = BlockingConnection(parameters)
channel = connection.channel()

a = "AAAA"
s = ",".join([a for i in xrange(1000 * 1000 * 10 * 5 * 3)])
print "Will publish: %d" % (len(s)/1024/1024)

channel.basic_publish(exchange='test',
  		      routing_key="test",
		      body=s,
		      properties=pika.BasicProperties(
				content_type="text/text",
				delivery_mode=1))
