#!/bin/bash
# Make SpiderMonkey and this repo generic instead of version specific.
# script should be run in mozjs dir with $1 for mason.build location
sed -i 's/mozjs-$MOZILLA_SYMBOLVERSION/mozjs/g' old-configure.in
sed -i --regexp-extended "s/dependency\(.mozjs.*$/dependency('mozjs')/gm" "$1"
