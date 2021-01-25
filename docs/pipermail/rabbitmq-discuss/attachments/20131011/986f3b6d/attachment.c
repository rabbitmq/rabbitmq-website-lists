/* vim:set ft=c ts=2 sw=2 sts=2 et cindent: */
/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alan Antonuk are Copyright (c) 2012-2013
 * Alan Antonuk. All Rights Reserved.
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
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "utils.h"

static char acCACertFileName [ 256 ];
static char acCALocalhostCertFileName [ 256 ];
static char acCAKeyFileName [ 256 ];
#define HU_SSL_PATH						"/etc/rabbitmq/ssl/"
#define HU_SSL_CA_CERT_FILE_NAME		"ca_cert.pem"
#define HU_SSL_LH_CA_CERT_FILE_NAME		"localhost_ca_cert.pem"
#define HU_SSL_CA_KEY_FILE_NAME			"localhost_ca_key.pem"

int main(int argc, char const *const *argv)
{
  char const *hostname;
  int port, status;
  char const *exchange;
  char const *routingkey;
  char const *messagebody;
  amqp_socket_t *socket = NULL;
  amqp_connection_state_t conn= NULL;

  if (argc < 6) {
    fprintf(stderr, "Usage: amqp_sendstring host port exchange routingkey messagebody\n");
    return 1;
  }

  hostname = argv[1];
  port = atoi(argv[2]);
  exchange = argv[3];
  routingkey = argv[4];
  messagebody = argv[5];

#if 0
  conn = amqp_new_connection();

  socket = amqp_tcp_socket_new(conn);
  if (!socket) {
    die("creating TCP socket");
  }

  status = amqp_socket_open(socket, hostname, port);
  if (status) {
    die("opening TCP socket");
  }

  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
                    "Logging in");
#endif

  conn = amqp_new_connection();
  socket = amqp_ssl_socket_new(conn);

  	strcpy ( acCACertFileName,HU_SSL_PATH );
  	strcat ( acCACertFileName,HU_SSL_CA_CERT_FILE_NAME );

  	strcpy ( acCALocalhostCertFileName,HU_SSL_PATH );
  	strcat ( acCALocalhostCertFileName,HU_SSL_LH_CA_CERT_FILE_NAME );

  	strcpy ( acCAKeyFileName,HU_SSL_PATH );
  	strcat ( acCAKeyFileName,HU_SSL_CA_KEY_FILE_NAME );

  	status = amqp_ssl_socket_set_cacert(socket, acCACertFileName);
  	if (status)
  	{
  		printf("setting CA certificate\n");
  	}

  	status = amqp_ssl_socket_set_key(socket, acCALocalhostCertFileName, acCAKeyFileName);
  	if (status)
  	{
  		printf("setting client cert\n");
  	}

  	status= amqp_socket_open(socket,hostname, port);
  	if (status)
  	{
  		printf("opening SSL/TLS connection\n");
  	}
  	amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");

    amqp_channel_open(conn, 1);
    amqp_get_rpc_reply(conn);

  while(1)
  {
    amqp_basic_properties_t props;
    struct timeval t;
    int ret;

	t.tv_sec = 3;
	t.tv_usec = 0;
	select(0, NULL, NULL, NULL, &t);

	props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
	props.content_type = amqp_cstring_bytes("text/plain");
	props.delivery_mode = 2; /* persistent delivery mode */
	printf("conn %u\n",conn);
	ret=amqp_basic_publish(conn,
							1,
							amqp_cstring_bytes(exchange),
							amqp_cstring_bytes(routingkey),
							1,
							0,
							&props,
							amqp_cstring_bytes(messagebody));
	printf("ret=%d\n",ret);

  }

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}
