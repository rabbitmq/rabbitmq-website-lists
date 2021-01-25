import pika
import uuid
import sys

class RpcServer(object):
    def __init__(self):
        self.connection = pika.BlockingConnection(
            pika.ConnectionParameters(
                host='localhost'
            )
        )
        self.channel = self.connection.channel()
        # RPC serverlara mesaj gondermek icin exchange tanimlamasi
        self.channel.exchange_declare(exchange='broadcast_to_all', type='fanout')
        # Geri donuslerin alinacagi gecici kuyruk
        self.channel.queue_declare(queue='hello', durable=True)
        self.callback_queue = 'hello'
        # Geri donus kuyrugu dinle
        self.channel.basic_consume(self.on_response, no_ack=True,
                                   queue=self.callback_queue)

    def on_response(self, ch, method, props, body):
        if self.corr_id == props.correlation_id:
            self.response = body

    def call(self):
         self.response = None
         self.corr_id = str(uuid.uuid4())
         # Mesaji broadcast yayinla
         mesaj = ' '.join(sys.argv[1:]) or "3"
         self.channel.basic_publish(exchange='broadcast_to_all',
                                    routing_key='',
                                    properties=pika.BasicProperties(
                                         reply_to = self.callback_queue,
                                         correlation_id = self.corr_id,
                                         ),
                                   body=str(mesaj))
         # Mesaja cevap gelene kadar bekle.
         while self.response is None:
             self.connection.process_data_events()
         return str(self.response)

test = RpcServer()
print "[x] Start"
response = test.call()
print "[.] Got %r" % (response,)