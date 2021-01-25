#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import sys
import os

from librabbitmq import Connection, Message

conn = Connection(host="my.host.broker", userid="guest", password="guest", virtual_host="/")
channel = conn.channel()
channel.exchange_declare('APA_test_exchange', type='direct')

burst_size = int(sys.argv[1])
sleep_time = float(sys.argv[2])
try:
    inter_publish_sleep_time = float(sys.argv[3])
except Exception:
    inter_publish_sleep_time = None

burst_num = 0
while True:
    try:
        for num in xrange(burst_size):
            body = "%d Burst %d Mensaje %d" % (os.getpid(), burst_num, num)
            m = Message(channel=channel, properties={}, delivery_info=1, body=body)
            res = channel.basic_publish(m, 'APA_test_exchange', 'APA_key1')
            if inter_publish_sleep_time:
                time.sleep(inter_publish_sleep_time)
            burst_num += 1
        print "Sleep before generating new burst..."
        time.sleep(sleep_time)
    except Exception, fail:
        print "FAILS: ", str(fail)




