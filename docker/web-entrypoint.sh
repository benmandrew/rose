#!/bin/sh
set -eu

mkdir -p /usr/src/app/shared
mkdir -p /usr/src/app/web
ln -sf /usr/src/app/shared/graph.json /usr/src/app/web/graph.json

exec npx live-server --port=8080 --no-browser /usr/src/app/web
