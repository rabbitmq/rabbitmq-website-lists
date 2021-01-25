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

# Buffer of raw messages
raw_burst = ''

for n in xrange(1000):

    # Add a message of aprox 1 KB
    body = '';
    for i in xrange(100):
       body += "msg %05d/" % (n)

    body += 'END'
    body_len = str(len(body));
    queue = '/queue/stomp_bug'

    raw_msg = "SEND\n"                        + \
              "destination:"+queue+"\n"       + \
              "content-length:"+body_len+"\n" + \
              "\n"                            + \
              body                            + \
              "\x00"

    raw_burst += raw_msg

# Send all messages in a shot
print "Send 1000 messages of 1 KB"
write_to_sock(raw_burst)

time.sleep(5)

sock.close
