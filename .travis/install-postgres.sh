#! /bin/sh

set -x
set -e

test -n "$PG_VERSION"

echo exit 101 | sudo tee /usr/sbin/policy-rc.d
sudo chmod +x /usr/sbin/policy-rc.d
sudo apt-get -qq update
sudo apt-get install -y \
    postgresql-"$PG_VERSION" \
    postgresql-client-"$PG_VERSION" \
    postgresql-server-dev-"$PG_VERSION"
