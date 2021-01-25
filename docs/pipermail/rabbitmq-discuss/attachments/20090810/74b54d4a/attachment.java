import java.io.IOException;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.ConnectionParameters;
import com.rabbitmq.client.QueueingConsumer;
import com.rabbitmq.client.ShutdownSignalException;
import com.rabbitmq.client.QueueingConsumer.Delivery;

public class QueueConsumer
{
  public static void main(String[] args) throws IOException
  {
    String userName = args[0];
    String password = args[1];
    String virtualHost = "/test";
    String hostName = "localhost";
    int portNumber = 5672;
    String exchangeName = "testExchange";
    String queueName = "randomQueue";
    String routingKey = "key";
    ConnectionParameters params = new ConnectionParameters();
    params.setUsername(userName);
    params.setPassword(password);
    params.setVirtualHost(virtualHost);
    params.setRequestedHeartbeat(0);
    ConnectionFactory factory = new ConnectionFactory(params);
    Connection conn = null;
    Channel channel = null;
    int count = 3;
    try
    {
      while (count >= 3)
      {
        conn = factory.newConnection(hostName, portNumber);
        channel = conn.createChannel();
        channel.exchangeDeclare(exchangeName, "topic", false, false, false, null);

        channel.queueDeclare(queueName, false, false, false, false, null);
        channel.queueBind(queueName, exchangeName, routingKey);
        QueueingConsumer consumer = new QueueingConsumer(channel);
        System.out.println("Consumer Tag = " + channel.basicConsume(queueName, false, consumer));

        Delivery response = consumer.nextDelivery();
        if (response != null)
        {
          System.out.println("id = " + response.getProperties().messageId + "\tbody = "
              + new String(response.getBody()));
          System.out.println(response.getEnvelope());
          System.out.println(response.getProperties());
          channel.basicAck(response.getEnvelope().getDeliveryTag(), true);
          count--;
        }

      }
      channel.close();
      conn.close();
    } catch (IOException e)
    {
      e.printStackTrace();
    } catch (ShutdownSignalException e)
    {
      e.printStackTrace();
    } catch (InterruptedException e)
    {
      e.printStackTrace();
    }

  }
}