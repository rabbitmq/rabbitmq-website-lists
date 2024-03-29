/* vim:set ft=c ts=2 sw=2 sts=2 et cindent: */
/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alan Antonuk are Copyright (c) 2012-2013
 * Alan Antonuk. All Rights Reserved.
 *
 * Portions created by Mike Steinert are Copyright (c) 2012-2013
 * Mike Steinert. All Rights Reserved.
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_ssl_socket.h>
#include <amqp_framing.h>

#include <assert.h>

int main(int argc, char const *const *argv)
{
  char const *hostname;
  int port, status;
  char const *exchange;
  char const *bindingkey;
  amqp_socket_t *socket;
  amqp_connection_state_t conn;

  amqp_bytes_t queuename;

  if (argc < 5) {
    fprintf(stderr, "Usage: amqps_listen host port exchange bindingkey "
            "[cacert.pem [key.pem cert.pem]]\n");
    return 1;
  }

  hostname = argv[1];
  port = atoi(argv[2]);
  exchange = argv[3];
  bindingkey = argv[4];

  conn = amqp_new_connection();
  int basic_consume;
  int queue_bind;
  socket = amqp_ssl_socket_new(conn);
  if (!socket) {
    //die("creating SSL/TLS socket");
  }

  if (argc > 5) {
    status = amqp_ssl_socket_set_cacert(socket, argv[5]);
    if (status) {
      //die("setting CA certificate");
    }
  }

  if (argc > 7) {
    status = amqp_ssl_socket_set_key(socket, argv[7], argv[6]);
    if (status) {
      //die("setting client cert");
    }
  }

  status = amqp_socket_open(socket, hostname, port);
  if (status) {
    //die("opening SSL/TLS connection");
  }

//  amqp_set_socket(conn, socket);
  amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  amqp_channel_open(conn, 1);
  amqp_get_rpc_reply(conn);

  {
    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 1, 1,
                                 amqp_empty_table);
    amqp_get_rpc_reply(conn);
    queuename = amqp_bytes_malloc_dup(r->queue);
    if (queuename.bytes == NULL) {
      fprintf(stderr, "Out of memory while copying queue name");
      return 1;
    }
  }

  queue_bind = amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey),
                  amqp_empty_table);
  printf("\n\n amqp_queue_bind %u\n",queue_bind);
//  if(!queue_bind)
//  	{
//	  printf("\n\namqp_queue_bind %u\n",(unsigned int)queue_bind);
////  		return HU_CALL_FAILED;
//  	}


  basic_consume = amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  printf("\n\n basic_consume %d\n",( unsigned int)basic_consume);
  if(!basic_consume)
  {
	  printf("\n\n basic_consume %u\n",( unsigned int)basic_consume);

  }
  {
    amqp_frame_t frame;
    int result;

    amqp_basic_deliver_t *d;
    amqp_basic_properties_t *p;
    size_t body_target;
    size_t body_received;
    struct timeval t;
    t.tv_sec=10;
    t.tv_usec=10;

    while (1) {
      amqp_maybe_release_buffers(conn);
      result = amqp_simple_wait_frame_noblock(conn, &frame, &t);
      //result = amqp_simple_wait_frame(conn, &frame);
      printf("Result %d\n", result);
      if (result == AMQP_STATUS_CONNECTION_CLOSED)
	    {
		    printf("AT AMQP_STATUS_CONNECTION_CLOSED \n");
	    }
	    if( result == AMQP_STATUS_SSL_ERROR || result == AMQP_STATUS_SSL_CONNECTION_FAILED)
	    {
		    printf("AT AMQP_STATUS_SSL_ERROR \n");
	    }
      if (result == AMQP_STATUS_TIMEOUT)
	    {
		    printf("AT AMQP_STATUS_TIMEOUT \n");
		    sleep(1);
		    continue;
	    }
      printf("Frame type %d, channel %d\n", frame.frame_type, frame.channel);
      if (frame.frame_type != AMQP_FRAME_METHOD) {
        continue;
      }

      printf("Method %s\n", amqp_method_name(frame.payload.method.id));
      if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD) {
        continue;
      }

      d = (amqp_basic_deliver_t *) frame.payload.method.decoded;
      printf("Delivery %u, exchange %.*s routingkey %.*s\n",
             (unsigned) d->delivery_tag,
             (int) d->exchange.len, (char *) d->exchange.bytes,
             (int) d->routing_key.len, (char *) d->routing_key.bytes);

      result = amqp_simple_wait_frame_noblock(conn, &frame, &t);
      if (result < 0) {
        break;
      }

      if (frame.frame_type != AMQP_FRAME_HEADER) {
        fprintf(stderr, "Expected header!");
        abort();
      }
      p = (amqp_basic_properties_t *) frame.payload.properties.decoded;
      if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {
        printf("Content-type: %.*s\n",
               (int) p->content_type.len, (char *) p->content_type.bytes);
      }
      printf("----\n");

      body_target = frame.payload.properties.body_size;
      body_received = 0;

      while (body_received < body_target) {
        result = amqp_simple_wait_frame_noblock(conn, &frame, &t);
        if (result < 0) {
          break;
        }

        if (frame.frame_type != AMQP_FRAME_BODY) {
          fprintf(stderr, "Expected body!");
          abort();
        }

        body_received += frame.payload.body_fragment.len;
        assert(body_received <= body_target);

        //amqp_dump(frame.payload.body_fragment.bytes,
          //        frame.payload.body_fragment.len);
      }

      if (body_received != body_target) {
        /* Can only happen when amqp_simple_wait_frame returns <= 0 */
        /* We break here to close the connection */
        break;
      }
    }
  }

  amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(conn);

  return 0;
}
