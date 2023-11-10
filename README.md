# rabbitmq-website-lists

RabbitMQ historical mailing lists: https://lists.rabbitmq.com/

To deploy the pages:

```shell
cd /tmp

aws s3 cp s3://rabbitmq-website/lists/lists.rabbitmq.com.tar.xz lists.rabbitmq.com.tar.xz

xzcat archive-lists/lists.rabbitmq.com.tar.xz | tar -xf -

rm lists.rabbitmq.com/index.html~
rm lists.rabbitmq.com/pipermail/*/pipermail.pck
rm -rf lists.rabbitmq.com/pipermail/*/database

git clone git@github.com:rabbitmq/rabbitmq-website-lists.git

cd rabbitmq-website-lists
mkdir -p docs
mv docs/CNAME ./CNAME
rm -rf docs/*
mv ./CNAME docs/CNAME

cp -r /tmp/lists.rabbitmq.com/* docs/
touch docs/.nojekyll

git add docs
git commit -m "Deploy RabbitMQ historical mailing lists"
git push origin
```
