#!/usr/bin/python
# -*- coding: utf-8 -*-
import time
import sys
import socket

from librabbitmq import Connection, Message, ChannelError

conn = Connection(host="my.host.broker", userid="guest", password="guest", virtual_host="/")
channel = conn.channel()
channel.exchange_declare('APA_test_exchange', type='direct',)
channel.queue_declare('APA_local_queue', auto_delete = False)
channel.queue_bind('APA_local_queue', 'APA_test_exchange', 'APA_key1')

print "Conn. connected ", conn.connected

def process_message(message):
        try:
            message.ack()
            if message:
                print "Message: ", message.body
        except Exception, exc:
            print "process_message=>Unexpected Error: %s" % str(exc)

channel.basic_consume('APA_local_queue', callback=process_message)    
while True:
    try:
        channel.basic_consume('APA_local_queue', callback=process_message)
        conn.drain_events()
    except Exception, fail:
        print "FAILS: ", str(fail)

