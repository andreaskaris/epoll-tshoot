#!/bin/bash
set -eux

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

IMAGE="$1"
NODE_SELECTOR="$2"
PGREP="$3"
PROJECT="systemtap"

privileged ()
{
    oc label ns $1 security.openshift.io/scc.podSecurityLabelSync="false" --overwrite=true;
    oc label ns $1 pod-security.kubernetes.io/enforce=privileged --overwrite=true;
    oc label ns $1 pod-security.kubernetes.io/warn=privileged --overwrite=true;
    oc label ns $1 pod-security.kubernetes.io/audit=privileged --overwrite=true
}

oc new-project "${PROJECT}" || oc project "${PROJECT}"
oc adm policy add-scc-to-user privileged -z default
privileged "${PROJECT}"
oc create configmap systemtap --from-file="${DIR}/../systemtap/epoll_syscall.stap" --from-file="${DIR}/../systemtap/entrypoint.sh" -o yaml --dry-run > \
    "${DIR}/_output/cm.yaml"
cat <<EOF> "${DIR}/_output/ds.yaml"
apiVersion: apps/v1
kind: DaemonSet
metadata:
  name: systemtap
  labels:
    app: systemd
spec:
  selector:
    matchLabels:
      name: systemtap
  template:
    metadata:
      labels:
        name: systemtap
    spec:
      dnsPolicy: ClusterFirst
      enableServiceLinks: true
      hostIPC: true
      hostPID: true
      hostNetwork: true
      tolerations:
      - key: node-role.kubernetes.io/control-plane
        operator: Exists
        effect: NoSchedule
      - key: node-role.kubernetes.io/master
        operator: Exists
        effect: NoSchedule
      nodeSelector:
        node-role.kubernetes.io/${NODE_SELECTOR}: ""
      containers:
      - name: systemtap
        image: ${IMAGE}
        volumeMounts:
        - name: host
          mountPath: /host
        - name: systemtap
          mountPath: /systemtap
        command:
        - /bin/bash
        - /systemtap/entrypoint.sh
        - "${PGREP}"
        securityContext:
          privileged: true
          runAsUser: 0
        stdin: true
        stdinOnce: true
        terminationMessagePath: /dev/termination-log
        terminationMessagePolicy: File
        tty: true
      terminationGracePeriodSeconds: 30
      volumes:
      - name: host
        hostPath:
          path: /
      - name: sys
        hostPath:
          path: /sys
      - name: systemtap
        configMap:
          name: systemtap
EOF

oc apply -f "${DIR}/_output/cm.yaml"
oc apply -f "${DIR}/_output/ds.yaml"
