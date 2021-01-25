
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.*;
public class RabbitMQConsumer {
    public static void main(String []args) throws Exception {
ConnectionParameters cp = new ConnectionParameters();
 cp.setUsername("guest");
 cp.setPassword("guest");
 cp.setVirtualHost("/");
 ConnectionFactory factory = new ConnectionFactory(cp);
 Connection conn = factory.newConnection("localhost");
      Channel channel = conn.createChannel();

       System.err.println("channel.isOpen()" + channel.isOpen());	
      String exchangeName = "myExchange";
      String queueName = "myQueue";
      String routingKey = "testRoute";
      boolean durable = true;
      channel.exchangeDeclare(exchangeName, "direct", durable);
      channel.queueDeclare(queueName, durable);
      channel.queueBind(queueName, exchangeName, routingKey);
      boolean noAck = true;
      QueueingConsumer consumer = new QueueingConsumer(channel);
       System.err.println("channel.isOpen()" + channel.isOpen());	
      channel.basicConsume(queueName, noAck, consumer);
      boolean runInfinite = true;
      while (runInfinite) {
            QueueingConsumer.Delivery delivery;
            try {
               delivery = consumer.nextDelivery();
            } catch (InterruptedException ie) {
               continue;
            }
         System.out.println("Message received" 
+ new String(delivery.getBody()));
         channel.basicAck(delivery.getEnvelope().getDeliveryTag(), false);
      }
      channel.close();
      conn.close();
      }
}
