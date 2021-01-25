import pika

connection = pika.BlockingConnection(
    pika.ConnectionParameters(
        host='localhost'
    )
)
channel = connection.channel()
# broadcast kanalini dinle
channel.exchange_declare(exchange='broadcast_to_all', type='fanout')
# broadcast e gelen mesajlari bu kuyruk ile al
result = channel.queue_declare(exclusive=True)
queue_name = result.method.queue
# broadcast ile kuyruku birlestir.
channel.queue_bind(exchange='broadcast_to_all', queue=queue_name)

print '[*] Waiting for messages. To exit press CTRL+C'


def on_request(ch, method, props, body):
    n = str(body)
    if n == "1" or n == "n":
        print "[.] i got message = %s"  % (n,)
        response = "I'm 1:)"
    else:
        print "[x] It's not for me"
        return

    # mesaji gonderen adamin olusturdugu kuyraka mesaj gondererek cevabi don.
    ch.basic_publish(exchange='',
                     routing_key=props.reply_to,
                     properties=pika.BasicProperties(correlation_id = \
                                                         props.correlation_id),
                     body=str(response))
    ch.basic_ack(delivery_tag=method.delivery_tag)

channel.basic_consume(on_request,
                      queue=queue_name)

channel.start_consuming()