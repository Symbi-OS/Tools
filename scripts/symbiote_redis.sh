# Temporarily disable selinux
# echo 0 > /selinux/enforce
# Temporarily open port from firewall
# firewall-cmd --add-port=6379/tcp
sync
LD_PRELOAD=~/Symbios/Apps/lib_constructors/libelevate.so ./redis/src/redis-server --protected-mode no --save '' --appendonly no

