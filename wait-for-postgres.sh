#!/bin/bash
# wait-for-postgres.sh

set -e
  
host="$1"
shift

export PGUSER=$(grep 'storage.type.postgresql.username' $OWPROV_CONFIG/owprov.properties | awk -F '= ' '{print $2}')
export PGPASSWORD=$(grep 'storage.type.postgresql.password' $OWPROV_CONFIG/owprov.properties | awk -F '= ' '{print $2}')
  
until psql -h "$host" -c '\q'; do
  >&2 echo "Postgres is unavailable - sleeping"
  sleep 1
done
  
>&2 echo "Postgres is up - executing command"

if [ "$1" = '/openwifi/owprov' -a "$(id -u)" = '0' ]; then
    if [ "$RUN_CHOWN" = 'true' ]; then
      chown -R "$OWPROV_USER": "$OWPROV_ROOT" "$OWPROV_CONFIG"
    fi
    exec gosu "$OWPROV_USER" "$@"
fi

exec "$@"
