#!/bin/sh

for RABBIT_QUEUE_NAME in $1 ; do
SHOVELS="$SHOVELS `
sed 's/${RABBIT_QUEUE_NAME}/'$RABBIT_QUEUE_NAME'/g
     ;s/${RABBIT_POD_ADMIN}/'$2'/g
     ;s/${RABBIT_POD_PASSWORD}/'$3'/g
     ;s/${HOSTNAME}/'$4'/g
     ;s/${RABBIT_ENTERPRISE_ADMIN}/'$5'/g
     ;s/${RABBIT_ENTERPRISE_ADMIN_PASSWORD}/'$6'/g
     ;s/${RABBIT_ENTERPRISE_HOST}/'$7'/g
     ;s/${RABBIT_ENTERPRISE_V_HOST}/'$8'/g
' single_shovel.template.erl 
`,
"
done 
SHOVELS=${SHOVELS:0:${#SHOVELS}-2}
echo "[{rabbitmq_shovel,
	[{shovels, [
	$SHOVELS
]}]}
]."