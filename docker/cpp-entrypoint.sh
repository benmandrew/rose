#!/bin/sh
set -eu

mkdir -p /app/shared
/app/rose /app/shared/graph.json
exec tail -f /dev/null
