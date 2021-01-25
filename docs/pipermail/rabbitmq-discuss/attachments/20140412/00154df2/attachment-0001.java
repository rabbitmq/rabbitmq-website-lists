diff -r 7ccaa1c58053 src/com/rabbitmq/client/impl/recovery/AutorecoveringConnection.java
--- a/src/com/rabbitmq/client/impl/recovery/AutorecoveringConnection.java	Thu Apr 10 10:24:43 2014 +0100
+++ b/src/com/rabbitmq/client/impl/recovery/AutorecoveringConnection.java	Sat Apr 12 13:30:50 2014 -0700
@@ -19,6 +19,7 @@
 import java.net.ConnectException;
 import java.net.InetAddress;
 import java.util.ArrayList;
+import java.util.HashMap;
 import java.util.List;
 import java.util.Map;
 import java.util.concurrent.ConcurrentHashMap;
@@ -439,7 +440,8 @@
     }
 
     private void recoverQueues() {
-        for (Map.Entry<String, RecordedQueue> entry : this.recordedQueues.entrySet()) {
+        Map<String, RecordedQueue> copy = new HashMap<String, RecordedQueue>(this.recordedQueues);
+        for (Map.Entry<String, RecordedQueue> entry : copy.entrySet()) {
             String oldName = entry.getKey();
             RecordedQueue q = entry.getValue();
             try {
@@ -473,7 +475,8 @@
     }
 
     private void recoverConsumers() {
-        for (Map.Entry<String, RecordedConsumer> entry : this.consumers.entrySet()) {
+        Map<String, RecordedConsumer> copy = new HashMap<String, RecordedConsumer>(this.consumers);
+        for (Map.Entry<String, RecordedConsumer> entry : copy.entrySet()) {
             String tag = entry.getKey();
             RecordedConsumer consumer = entry.getValue();
 
