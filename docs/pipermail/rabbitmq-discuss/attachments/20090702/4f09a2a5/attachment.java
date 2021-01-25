import com.rabbitmq.client.AMQP;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.ConnectionParameters;
import com.rabbitmq.client.DefaultConsumer;
import com.rabbitmq.client.Envelope;

import org.json.simple.JSONValue;

class ChannelTest
{
  /** Fake consumer; just reads messages and prints them out.  A real consumer
   * would probably actual ack its messages, but this is just to test parallel
   * subscriptions, so we don't drain our queues here.
   */
  static class MyConsumer extends DefaultConsumer
  {
    public MyConsumer(Channel ch) { super(ch); }
    public void handleDelivery(
        java.lang.String consumerTag, 
        Envelope envelope, 
        AMQP.BasicProperties properties, 
        byte[] body) 
    {
      try {
        Object o = JSONValue.parse(new String(body, "utf-8"));
        System.out.println(o);
      } catch(Throwable t) {
        System.out.println("Bad Message");
      }
      try{Thread.sleep(1000);}catch(Throwable t){}
    }
  }

  /** Pointless intermediate class to see if adding another thread helps
   * matters any.  It doesn't....
   */
  static class MyTestThread extends Thread {
    Channel ch; String q;
    public MyTestThread(Channel ch, String queue) { 
      this.ch = ch; 
      this.q = queue;
    }

    public void run() {
      try {
        ch.basicConsume(this.q, false, new MyConsumer(ch));
      } catch(Throwable t) {}
    }
  }

  /** Create a connection to the local AMQP server.
   */
  public static Connection getConnection()
  {
    ConnectionParameters params = new ConnectionParameters();
    params.setUsername("user");
    params.setPassword("notsecret");
    ConnectionFactory factory = new ConnectionFactory(params);
    try {
      return factory.newConnection("localhost", 5672);
    } catch(java.io.IOException e) {
      System.out.println(e);
      System.out.println("Failed to establish AMQP connection");
      System.exit(1);
    }
    return null;
  }

  public static final void main(String[] args) throws Throwable
  {
    ;
    System.out.println("Starting consume");
    Connection conn1 = getConnection();
    Connection conn2 = getConnection();
    (new MyTestThread(conn1.createChannel(), "Q1")).start();
    (new MyTestThread(conn2.createChannel(), "Q2")).start();
    System.out.println("Started consume");
  }
}
