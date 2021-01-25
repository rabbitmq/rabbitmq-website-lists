import java.io.IOException;
import java.util.UUID;

import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.ConnectionParameters;
import com.rabbitmq.client.AMQP.BasicProperties;

public class QueueProducer
{

  public static void main(String args[]) throws IOException
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
    try
    {
      conn = factory.newConnection(hostName, portNumber);
      channel = conn.createChannel();
      channel.exchangeDeclare(exchangeName, "topic");
      channel.queueDeclare(queueName);
      channel.queueBind(queueName, exchangeName, routingKey);
      byte[] messageBodyBytes = "test text".getBytes();
      long count = 3;
      while (count != 0)
      {
        UUID id = UUID.randomUUID();
        BasicProperties prop = new BasicProperties();
        prop.messageId = id.toString();
        channel.basicPublish(exchangeName, routingKey, prop, messageBodyBytes);
        count--;
      }
      channel.close();
      conn.close();
    } catch (IOException e)
    {
      e.printStackTrace();
    }
  }

}