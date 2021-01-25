
var amqp = require('amqp');

var connection = amqp.createConnection({ host: 'rabbit_host' });


connection.on('ready', function () {
 var big="";
  for(var i=0;i<10000 * 3500;i++){
    big+="AAAAAAA";
  }
  console.log("Size: ", Buffer.byteLength(big)/1024/1024);
  console.log("Publish 1 ");
  connection.publish('test', { a : big}, function(err){
    console.log("Error publishing");
  });
  console.log("Publish 2");
});
