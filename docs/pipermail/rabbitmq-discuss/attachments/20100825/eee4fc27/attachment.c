/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and
 * limitations under the License.
 *
 * The Original Code is librabbitmq.
 *
 * The Initial Developers of the Original Code are LShift Ltd, Cohesive
 * Financial Technologies LLC, and Rabbit Technologies Ltd.  Portions
 * created before 22-Nov-2008 00:00:00 GMT by LShift Ltd, Cohesive
 * Financial Technologies LLC, or Rabbit Technologies Ltd are Copyright
 * (C) 2007-2008 LShift Ltd, Cohesive Financial Technologies LLC, and
 * Rabbit Technologies Ltd.
 *
 * Portions created by LShift Ltd are Copyright (C) 2007-2009 LShift
 * Ltd. Portions created by Cohesive Financial Technologies LLC are
 * Copyright (C) 2007-2009 Cohesive Financial Technologies
 * LLC. Portions created by Rabbit Technologies Ltd are Copyright (C)
 * 2007-2009 Rabbit Technologies Ltd.
 *
 * Portions created by Tony Garnock-Jones are Copyright (C) 2009-2010
 * LShift Ltd and Tony Garnock-Jones.
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 2 or later (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of those
 * above. If you wish to allow use of your version of this file only
 * under the terms of the GPL, and not to allow others to use your
 * version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the
 * notice and other provisions required by the GPL. If you do not
 * delete the provisions above, a recipient may use your version of
 * this file under the terms of any one of the MPL or the GPL.
 *
 * ***** END LICENSE BLOCK *****
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp.h>
#include <amqp_framing.h>

#include <unistd.h>
#include <assert.h>

#include "example_utils.h"

#define SUMMARY_EVERY_US 1000000*20

/* amqp.h should support this */
static int _amqp_basic_qos(amqp_connection_state_t state,
                           amqp_channel_t channel,
                           uint32_t prefetch_size,
                           uint16_t prefetch_count,
                           amqp_boolean_t global)
{
    amqp_rpc_reply_t rpc_reply =
        AMQP_SIMPLE_RPC(state, channel, BASIC, QOS, QOS_OK,
                        amqp_basic_qos_t, prefetch_size, prefetch_count, global);

    if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
        return -1;
    return 0;
}

static void run(amqp_connection_state_t conn, int num)
{
    long long start_time = now_microseconds();
    int received = 0;
    int previous_received = 0;
    long long previous_report_time = start_time;
    long long next_summary_time = start_time + SUMMARY_EVERY_US;

    amqp_frame_t frame;
    int result;
    size_t body_received;
    size_t body_target;

    long long now;

    if (num==0)
        return;

    while (1) {
        now = now_microseconds();
        if (now > next_summary_time) {
            int countOverInterval = received - previous_received;
            double intervalRate = countOverInterval / ((now - previous_report_time) / 1000000.0);
            printf("%d ms: Received %d - %d since last report (%d Hz)\n",
                   (int)(now - start_time) / 1000, received, countOverInterval, (int) intervalRate);

            previous_received = received;
            previous_report_time = now;
            next_summary_time += SUMMARY_EVERY_US;
        }

        amqp_maybe_release_buffers(conn);
        result = amqp_simple_wait_frame(conn, &frame);
        if (result < 0)
            return;

        if (frame.frame_type != AMQP_FRAME_METHOD)
            continue;

        if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
            continue;

        result = amqp_simple_wait_frame(conn, &frame);
        if (result < 0)
            return;
    
        if (frame.frame_type != AMQP_FRAME_HEADER) {
            fprintf(stderr, "Expected header!");
            abort();
        }

        body_target = frame.payload.properties.body_size;
        body_received = 0;

        while (body_received < body_target) {
            result = amqp_simple_wait_frame(conn, &frame);
            if (result < 0)
                return;

            if (frame.frame_type != AMQP_FRAME_BODY) {
                fprintf(stderr, "Expected body!");
                abort();
            }

            body_received += frame.payload.body_fragment.len;
            assert(body_received <= body_target);
        }

        received++;
        if (num>0 && received>=num)
            break;
    }
}

static void usage() {
    printf("Usage: amqp_consumer -k routingkey -q queue [-b broker] [-p port] [-n num]\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    char const *broker = "localhost";
    int port = 5672;
    char const *exchange = "amq.direct";
    char const *routingkey = NULL;
    char const *qname = NULL;

    int sockfd;
    amqp_connection_state_t conn;

    amqp_bytes_t queuename;
    amqp_boolean_t durable =1;
    amqp_boolean_t auto_delete =0;
    amqp_boolean_t no_ack =1;
    int prefetch_count = 50;

    /* if num==0, bind and declare queue then exit */
    /* if num>0, consume num of messages then exit */
    /* if num<0, consume messages until process is stopped */
    int num = -1;  

    int c;
  
    while((c = getopt(argc, argv, "b:p:r:q:n:")) != -1) {
        switch(c) {
        case 'b':
            broker = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'r':
            routingkey = optarg;
            break;
        case 'q':
            qname = optarg;
            break;
        case 'n':
            num = atoi(optarg);
            break;
        default:
            fprintf(stderr,"unknown option %c\n", c);
            usage();
        }
    }

    if (!routingkey || !qname) {
        usage();
    }

    conn = amqp_new_connection();

    die_on_error(sockfd = amqp_open_socket(broker, port), "Opening socket");
    amqp_set_sockfd(conn, sockfd);
    die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
                      "Logging in");
    amqp_channel_open(conn, 1);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

    {
        queuename = amqp_cstring_bytes(qname);

        amqp_queue_declare(conn, 1, queuename, 0, durable, 0, auto_delete,
                           AMQP_EMPTY_TABLE);
        die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
        /* queuename = amqp_bytes_malloc_dup(r->queue); */
        if (queuename.bytes == NULL) {
            fprintf(stderr, "Out of memory while copying queue name");
            return 1;
        }
    }

    amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(routingkey),
                    AMQP_EMPTY_TABLE);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

    /* set prefectch count */
    if (prefetch_count > 0 ) {
        int rt =_amqp_basic_qos(conn, 1, 0, prefetch_count, 0);
        if (rt !=0) {
            fprintf(stderr, "cannot set prefetch count\n");
        }
    }

    amqp_basic_consume(conn, 1, queuename, AMQP_EMPTY_BYTES, 0, no_ack, 0);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");

    run(conn,num);

    die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
    die_on_error(amqp_destroy_connection(conn), "Ending connection");

    return 0;
}
