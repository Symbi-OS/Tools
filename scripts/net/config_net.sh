ip addr add 192.168.122.47/24 dev enp1s0
ip link set up enp1s0
systemctl start sshd
