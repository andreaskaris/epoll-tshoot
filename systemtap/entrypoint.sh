#!/bin/bash
set -eux

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
LOG_DIR=/host/var/log/systemtap

PGREP="${1}"
if [ "${PGREP}" == "" ]; then
    echo "Empty pgrep expression. Exiting." >&2
    exit 1
fi

if [ ! -f "${LOG_DIR}" ]; then
    mkdir -p "${LOG_DIR}"
fi

pids=$(pgrep -f "${PGREP}")
if [ $? != 0 ]; then
    echo "Could not find process: '$var'. Skipping ..."
    continue
fi
for pid in ${pids}; do
    echo "" >> "${LOG_DIR}/${pid}.log"
    echo "[$(date)] Monitoring pid ${pid}." | tee -a "${LOG_DIR}/${pid}.log"
    stap -x ${pid} ${DIR}/epoll_syscall.stap >> "${LOG_DIR}/${pid}.log" 2>&1 &
done

echo "Sleeping ..."
/usr/bin/sleep infinity
