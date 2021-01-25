import socket
import time

host  = 'localhost'
user  = 'guest'
passw = 'guest'

sock = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
sock.connect((host, 61613))

def write_to_sock(msg):
    totalsent = 0
    while totalsent < len(msg):
        sent = sock.send(msg[totalsent:])
        if sent == 0:
            raise RuntimeError("socket connection broken")
        totalsent = totalsent + sent

raw_connect = "CONNECT\n"            + \
              "login:"+user+"\n"     + \
              "passcode:"+passw+"\n" + \
              "\n"                   + \
              "\x00"             

write_to_sock(raw_connect)

queue = '/queue/stomp_bug'

# Buffer of raw messages
raw_burst = ''

msg_count = 1001

for n in xrange(msg_count):

    # Add a message of aprox 1 KB
    body = '';
    for i in xrange(100):
       body += "msg %05d/" % (n)

    body += 'END'
    body_len = str(len(body));

    # Last message is sent to a different queue
    if n == msg_count-1:
        queue = '/queue/stomp_bug_last'

    raw_msg = "SEND\n"                        + \
              "destination:"+queue+"\n"       + \
              "content-length:"+body_len+"\n" + \
              "\n"                            + \
              body                            + \
              "\x00"

    raw_burst += raw_msg

print "Send "+str(msg_count)+" messages of 1 KB"

# Send all messages in a shot
write_to_sock(raw_burst)

# /queue/stomp_bug       will get 1000 messages
# /queue/stomp_bug_last  will get 1 message, the last one

time.sleep(5)

sock.close
