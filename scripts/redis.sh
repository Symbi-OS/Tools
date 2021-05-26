# Temporarily disable selinux
# echo 0 > /selinux/enforce
# Temporarily open port from firewall
# firewall-cmd --add-port=6379/tcp
./redis/src/redis-server --protected-mode no
