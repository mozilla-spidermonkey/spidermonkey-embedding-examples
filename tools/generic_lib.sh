#!/bin/bash
# Make this repo generic instead of version specific.
# run script with $1 for meson.build location
sed -i --regexp-extended "s/dependency\(.mozjs.*$/dependency('mozjs')/gm" "$1"
