import com.rabbitmq.client.Connection;
import com.rabbitmq.client.Address;
import com.rabbitmq.client.QueueingConsumer;
import com.rabbitmq.client.ShutdownListener;
import com.rabbitmq.client.ReturnListener;
import com.rabbitmq.client.Channel ;
import com.rabbitmq.client.ConnectionParameters;
import com.rabbitmq.client.ConnectionFactory;


public class Test {
    public static void main(String[] args) throws Exception {
        final ConnectionParameters params = new ConnectionParameters();
        params.setUsername("guest");
        params.setPassword("guest");
        final ConnectionFactory fact = new ConnectionFactory(params);
        final Connection conn = fact.newConnection("localhost", 5672);
        final Channel channel = conn.createChannel();

        channel.exchangeDeclare("my-exchange", "direct", true);
        channel.queueDeclare("my-queue", true);
        channel.queueBind("my-queue", "my-exchange", "my-key");

        final Address[] hosts = conn.getKnownHosts();
        for (int i = 0; i < hosts.length; ++i)
            System.out.println(hosts[i]);
    }
}
