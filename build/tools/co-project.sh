#!/bin/sh

host="bzr+ssh://repo.1-loop.net"
#
repo="home/repo/bzr/noaaport"
tag=trunk
#
site=${host}/${repo}

[ $# ne 1 ] && { echo "Needs project name."; exit 0; }
p=$1

bzr branch $site/$project/$tag $p
