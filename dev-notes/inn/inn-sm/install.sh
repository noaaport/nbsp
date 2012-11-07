#!/bin/sh

crontab -u news news.cron
install -m 0644 -o news -g news active /var/news/db
