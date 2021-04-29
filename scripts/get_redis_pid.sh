ps aux | grep redis-ser | grep -v grep | tr -s ' ' | cut -d ' ' -f 2
