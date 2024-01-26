#!/bin/bash
set -eux

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
LOG_DIR=/host/var/log/systemtap
STAP_LOG_MAX_SIZE=1000
LOG_ROTATE_SIZE=900M
LOG_ROTATE_OLDER_THAN=5

timeout 86400 stap /systemtap/epoll_syscall.stap -o /host/var/log/systemtap/stap.log -S ${STAP_LOG_MAX_SIZE} &

for i in {1..1460}; do
    sleep 60
    echo "Checking if logs need rotation ..."
    pushd "${LOG_DIR}"
        find . -maxdepth 1 -type f -size +${LOG_ROTATE_SIZE} -mmin +${LOG_ROTATE_OLDER_THAN} -exec gzip {} \;
    popd
done
echo "Ran for more than a day (1440 minutes plus 20 minutes cooldown), done."
