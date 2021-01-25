# Make sure needed plugins are enabled
rabbitmq-plugins enable rabbitmq_management
rabbitmq-plugins enable rabbitmq_federation
rabbitmq-plugins enable rabbitmq_federation_management

# Start three separate RabbitMQ nodes:  rabbita, rabbitb, and rabbitc
RABBITMQ_NODE_PORT=5673 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15673}]" RABBITMQ_NODENAME=rabbita rabbitmq-server -detached
RABBITMQ_NODE_PORT=5674 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15674}]" RABBITMQ_NODENAME=rabbitb rabbitmq-server -detached
RABBITMQ_NODE_PORT=5675 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15675}]" RABBITMQ_NODENAME=rabbitc rabbitmq-server -detached

# Start three more that will serve as the 2nd node of each cluster
RABBITMQ_NODE_PORT=5676 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15676}]" RABBITMQ_NODENAME=rabbita1 rabbitmq-server -detached
RABBITMQ_NODE_PORT=5677 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15677}]" RABBITMQ_NODENAME=rabbitb1 rabbitmq-server -detached
RABBITMQ_NODE_PORT=5678 RABBITMQ_SERVER_START_ARGS="-rabbitmq_management listener [{port,15678}]" RABBITMQ_NODENAME=rabbitc1 rabbitmq-server -detached

# Reset all the nodes (in case we used them previously)
rabbitmqctl -n rabbita stop_app
rabbitmqctl -n rabbita reset
rabbitmqctl -n rabbita start_app

rabbitmqctl -n rabbita1 stop_app
rabbitmqctl -n rabbita1 reset
rabbitmqctl -n rabbita1 start_app

rabbitmqctl -n rabbitb stop_app
rabbitmqctl -n rabbitb reset
rabbitmqctl -n rabbitb start_app

rabbitmqctl -n rabbitb1 stop_app
rabbitmqctl -n rabbitb1 reset
rabbitmqctl -n rabbitb1 start_app

rabbitmqctl -n rabbitc stop_app
rabbitmqctl -n rabbitc reset
rabbitmqctl -n rabbitc start_app

rabbitmqctl -n rabbitc1 stop_app
rabbitmqctl -n rabbitc1 reset
rabbitmqctl -n rabbitc1 start_app

# Start Clustering

# Join rabbita1 to rabbita
rabbitmqctl -n rabbita1 stop_app
rabbitmqctl -n rabbita1 join_cluster rabbita@`hostname -s`
rabbitmqctl -n rabbita1 start_app

# Join rabbitb1 to rabbitb
rabbitmqctl -n rabbitb1 stop_app
rabbitmqctl -n rabbitb1 join_cluster rabbitb@`hostname -s`
rabbitmqctl -n rabbitb1 start_app

# Join rabbitc1 to rabbitc
rabbitmqctl -n rabbitc1 stop_app
rabbitmqctl -n rabbitc1 join_cluster rabbitc@`hostname -s`
rabbitmqctl -n rabbitc1 start_app

# Create the test virtual hosts
curl -i -u guest:guest -H "content-type:application/json" -XPUT http://localhost:15673/api/vhosts/test
curl -i -u guest:guest -H "content-type:application/json" -XPUT http://localhost:15674/api/vhosts/test
curl -i -u guest:guest -H "content-type:application/json" -XPUT http://localhost:15675/api/vhosts/test

# Give guest user access to virtual hosts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"configure":".*","write":".*","read":".*"}' http://localhost:15673/api/permissions/test/guest
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"configure":".*","write":".*","read":".*"}' http://localhost:15674/api/permissions/test/guest
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"configure":".*","write":".*","read":".*"}' http://localhost:15675/api/permissions/test/guest

# Setup mirrored queue policies
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"pattern":"^m\.", "definition":{"ha-mode":"all"}}' http://localhost:15673/api/policies/test/ha-all 
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"pattern":"^m\.", "definition":{"ha-mode":"all"}}' http://localhost:15674/api/policies/test/ha-all 
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"pattern":"^m\.", "definition":{"ha-mode":"all"}}' http://localhost:15675/api/policies/test/ha-all 

# Create federation upstreams

rabbitmqctl set_parameter -n rabbita -p test federation-upstream RBTB_upstream '{"uri":"amqp://localhost:5674","expires":3600000}'
rabbitmqctl set_parameter -n rabbita -p test federation-upstream RBTC_upstream '{"uri":"amqp://localhost:5675","expires":3600000}'
rabbitmqctl set_parameter -n rabbitb -p test federation-upstream RBTA_upstream '{"uri":"amqp://localhost:5673","expires":3600000}'
rabbitmqctl set_parameter -n rabbitb -p test federation-upstream RBTC_upstream '{"uri":"amqp://localhost:5675","expires":3600000}'
rabbitmqctl set_parameter -n rabbitc -p test federation-upstream RBTA_upstream '{"uri":"amqp://localhost:5673","expires":3600000}'
rabbitmqctl set_parameter -n rabbitc -p test federation-upstream RBTB_upstream '{"uri":"amqp://localhost:5674","expires":3600000}'

# Create policies to match exchanges to federation upstreams
rabbitmqctl set_policy -n rabbita -p test federate-remotes "^RBTA\." '{"federation-upstream-set":"all"}'
rabbitmqctl set_policy -n rabbita -p test federate-global "^Global\." '{"federation-upstream-set":"all"}'
rabbitmqctl set_policy -n rabbitb -p test federate-remotes "^RBTB\." '{"federation-upstream-set":"all"}'
rabbitmqctl set_policy -n rabbitb -p test federate-global "^Global\." '{"federation-upstream-set":"all"}'
rabbitmqctl set_policy -n rabbitc -p test federate-remotes "^RBTC\." '{"federation-upstream-set":"all"}'
rabbitmqctl set_policy -n rabbitc -p test federate-global "^Global\." '{"federation-upstream-set":"all"}'

# Create federated exchanges
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/Local.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/Local.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/Global.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/Global.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/RBTA.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15673/api/exchanges/test/RBTA.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/Local.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/Local.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/Global.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/Global.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/RBTB.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15674/api/exchanges/test/RBTB.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/Local.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/Local.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/Global.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/Global.Broadcasts
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/RBTC.Requests
curl -i -u guest:guest -H "content-type:application/json" -XPUT -d'{"type":"direct","durable":true}' http://localhost:15675/api/exchanges/test/RBTC.Broadcasts