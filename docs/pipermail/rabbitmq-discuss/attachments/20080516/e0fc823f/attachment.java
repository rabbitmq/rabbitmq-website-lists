package com.rabbitmq.test;

import java.io.IOException;

import com.rabbitmq.client.Address;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.ConnectionParameters;
import com.rabbitmq.client.MessageProperties;



public class ConcurrentRabbitMQProducerTest implements Runnable {

	private static final int TRANSIENT = 0;
	private static final int PERSISTENT = 1;
	private static final int TRANSACTIONAL = 2;
	
	public static final int POLL_MS = 5;
	
	private Connection _conn;
	private int _messageCount;
	private int _messageHandling;  
	private int _ticket;
	
	public boolean shouldPersist() {
		if (_messageHandling == PERSISTENT) {
			return true;
		}
		return false;
	}

	public static void main(String[] args) {
		
		if (args.length < 4) {
			System.out.println("to less arguments");
			System.out.println("1. HOST 2. PORT 3. Number of messages 4. message handling (0 transient, 1 persistent)");
			return;
		}
		
		String host = args[0];
		int port = Integer.parseInt(args[1]);
		int messageCount = Integer.parseInt(args[2]);
		int messageHandling = Integer.parseInt(args[3]);
		
		ConcurrentRabbitMQProducerTest producer = new ConcurrentRabbitMQProducerTest(host,port,messageCount,messageHandling);
		
		Thread t = new Thread(producer);
		t.start();
			
	}
	
	public ConcurrentRabbitMQProducerTest(String host, int port, int messageCount, int messageHandling) {
		
		try {
			_conn = new ConnectionFactory().newConnection(host, port);
		} catch (Exception ex) {
			System.out.println("illegal argument");
			ex.printStackTrace();
		}

		_messageCount = messageCount;
		_messageHandling = messageHandling;
	}
	
	public void run() {
		
        try {
        	System.out.println("Starting Producer Thread");
            runIt();
        } catch (IOException ex) {
            System.err.println("hit IOException in Producer: trace follows");
            ex.printStackTrace();
            throw new RuntimeException(ex);
        }	
	}
	
	public void runIt() throws IOException {
		
		Channel channel = _conn.createChannel();
		_ticket = channel.accessRequest("/data");
		
		String routingKey = "test";
        String queueName = "test queue1";
        String queueName2 = "test queue2";
        String exchangeName = "exchange";
        
        channel.queueDeclare(_ticket, queueName, shouldPersist());
        channel.queueDeclare(_ticket, queueName2, shouldPersist());
        
        channel.exchangeDeclare(_ticket, exchangeName, "topic", true);
        
        channel.queueBind(_ticket, queueName, exchangeName, "test.#");
        channel.queueBind(_ticket, queueName2, exchangeName, "test.#");
        System.out.println("start");
        byte[] message = new byte[256];
        for (int i=0; i < _messageCount; ++i) {
        	//System.out.println("test" + i);
        	channel.basicPublish(_ticket, exchangeName, routingKey, shouldPersist() ? MessageProperties.MINIMAL_PERSISTENT_BASIC : MessageProperties.MINIMAL_BASIC,
                    message);
        	
        }
        
        System.out.println(_messageCount + "messages sent");
        
        channel.close(200, "Closing ch1 with no error");
        System.out.println("Closing channel.");
        _conn.close(200, "Goodbye.");
        System.out.println("Closing connection.");
        
        
	}
	
}
