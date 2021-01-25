#!/usr/bin/perl

use strict;
use warnings;
use IO::Socket::INET;

my $host = 'localhost';
my $user = 'guest';
my $pass = 'guest';

my $sock = IO::Socket::INET->new(
    Proto     => 'tcp',
    PeerAddr  => $host,
    PeerPort  => 61613,
);

die "Can't connect to RabbitMQ: $!" unless $sock;

$sock->autoflush();

$sock->write(
    "CONNECT\n"        .
    "login:$user\n"    .
    "passcode:$pass\n" .
    "\n"               .
    "\x00"
);

sleep 1;

my $queue = '/queue/stomp_bug';
my $body = 'X' x 1000;
my $len = length $body;

my $msg = "SEND\n"                .
          "destination:$queue\n"  .
          "content-length:$len\n" .
          "\n"                    .
          $body                   .
          "\x00"                  ;

for (1..5) {

    # Send 100 messages of 1 KB at once
    $sock->write( $msg x 100 );

    # This solves the problem, but it is unacceptable
    # $sock->write( "\n" x 5000 );

    sleep 10;
}

$sock->close;

# Using the management console, inspect the Ready messages of queue /stomp_bug
# The expected progression is:  100 ... 200 ... 300 ... 400 ... 500
# But instead, we usually get:   99 ... 199 ... 299 ... 399 ... 499        
