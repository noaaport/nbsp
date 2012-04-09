#!/bin/sh

cp dist.tgz /var/news

cd /var/news
tar -xzf dist.tgz 
rm dist.tgz

cd db
touch history
makedbz -i
for ext in dir hash index
do
    mv history.n.$ext history.$ext
done
