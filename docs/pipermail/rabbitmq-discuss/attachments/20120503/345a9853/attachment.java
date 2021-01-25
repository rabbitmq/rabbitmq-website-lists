import com.rabbitmq.client.*;
public class Test {
     public static void main(String[] margs) throws Exception {
         ConnectionFactory cfconn = new ConnectionFactory();
         Connection conn = cfconn.newConnection();
         Channel channel = conn.createChannel();
         String REPLY_QUEUE = "REPLY_QUEUE";
         boolean durable = false;
         boolean autoDelete = true;
         boolean exclusive = false;
         channel.exchangeDeclare("X", "direct");
         channel.queueDeclare(REPLY_QUEUE, durable, exclusive, autoDelete, null);
         Consumer consumer = new QueueingConsumer(channel);
         channel.addReturnListener(new ReturnListener() {
             public void handleReturn(int replyCode, String replyText,
                                      String exchange, String routingKey, AMQP.BasicProperties properties,
                                      byte[] body) throws java.io.IOException {
                 System.out.println("[ERROR:Service :"+routingKey+" seems down, returning with code="+replyCode+" message="+replyText+"]");
             }
         });
         boolean mandatory=true;
         boolean immediate=true;
         channel.queueBind(REPLY_QUEUE, "X", REPLY_QUEUE);
         channel.basicPublish("X", REPLY_QUEUE, mandatory, immediate, null, new byte[]{});
         channel.queueUnbind(REPLY_QUEUE, "X", REPLY_QUEUE);
         channel.basicConsume(REPLY_QUEUE, true, consumer);
         channel.basicPublish("X", REPLY_QUEUE, mandatory, immediate, null, new byte[]{});
         Thread.sleep(500);
         conn.close();
    }
}
