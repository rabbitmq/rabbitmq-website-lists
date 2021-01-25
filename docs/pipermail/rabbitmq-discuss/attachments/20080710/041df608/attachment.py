#!/usr/bin/env python
# amqpevent.py
# Copyright (C) 2008 Justin Azoff JAzoff@uamail.albany.edu
#
# This module is released under the MIT License:
# http://www.opensource.org/licenses/mit-license.php
import sys

from amqplib.client_0_8 import Connection, Message
import time

class Events:
    def __init__(self, event_type):
        self.event_type = event_type
        self.conn = Connection(host='xxx.xx.x.x',userid='guest',password='guest')
        self.ch = self.conn.channel()
        tkt = self.ch.access_request('/data', active=True, write=True, read=True)

        self.timeouts={}
        self._declared = {}

    def create_exchange(self):
        if self.event_type in self._declared:
            return
        self.ch.exchange_declare(self.event_type, 'fanout', auto_delete=False, durable=True)
        self._declared[self.event_type] = True

    def register_handler(self, handler_name, timeout=60*60, reassign_timeout=60, auto_delete=False):
        self.create_exchange()
        q = "%s_%s" % (self.event_type, handler_name)
        self.ch.queue_declare(q, durable=True,auto_delete=auto_delete)
        self.ch.queue_bind(q, self.event_type)

        self.timeouts[handler_name]=timeout

    def add_event(self, data):
        self.create_exchange()
        msg = Message(data, content_type='text/plain',delivery_mode=2,
            application_headers={'added':int(time.time())})

        self.ch.basic_publish(msg, self.event_type)

    def ack_event(self, msg):
        self.ch.basic_ack(msg.delivery_tag)

    def is_timed_out(self, msg, handler_name):
        added = msg.properties.get('application_headers',{}).get('added')
        if not added: return False
        diff = time.time() - added
        return diff > self.timeouts[handler_name]

    def get_event(self, handler_name, auto_ack=False):
        if handler_name not in self.timeouts:
            self.register_handler(handler_name)

        q = "%s_%s" % (self.event_type, handler_name)
        while 1:
            msg = self.ch.basic_get(q)
            if msg and self.is_timed_out(msg, handler_name):
                self.ack_event(msg)
                continue
            if auto_ack:
                self.ack_event(msg)
            return msg

    def register_callback(self, handler_name, callback,auto_ack=False):
        if handler_name not in self.timeouts:
            self.register_handler(handler_name)

        q = "%s_%s" % (self.event_type, handler_name)
        def cb(msg):
            if msg and self.is_timed_out(msg, handler_name):
                self.ack_event(msg)
            else:
                callback(self, msg)
                if auto_ack:
                    self.ack_event(msg)

        self.ch.basic_consume(q, callback=cb)
        
    def wait_for_callbacks(self):
        while self.ch.callbacks:
            self.ch.wait()


    def show(self,msg):
        if not msg: return
        print msg.body
        for key, val in msg.properties.items():
            print '%s: %s' % (key, str(val))
        for key, val in msg.delivery_info.items():
            print '> %s: %s' % (key, str(val))
       
        print ''

    def close(self):
        self.ch.close()

