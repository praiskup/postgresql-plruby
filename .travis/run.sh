#! /bin/sh

set -x
set -e

ruby extconf.rb --vendor --with-safe-level=1
make
sudo make install

export PGPORT=54321
export PGHOST=/tmp
export PGDATA=`pwd`/datadir

initdb "$PGDATA"
pg_ctl -D "$PGDATA" \
       -l logfile \
       -o "-k $PGHOST -p $PGPORT" \
       -w \
       start

make installcheck
