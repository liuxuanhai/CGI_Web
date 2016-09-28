/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 20001 -c 256 -P /tmp/memcached_s_manager_session.pid
/usr/local/bin/memcached -d -m 10 -u root -l 127.0.0.1 -p 20002 -c 256 -P /tmp/memcached_s_manager_vc.pid
