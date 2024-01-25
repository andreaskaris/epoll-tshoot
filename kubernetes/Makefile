SYSTEMTAP_IMAGE ?= quay.io/akaris/systemtap:ocp-4.12.33-rt
NODE_SELECTOR ?= worker
PGREP ?= "server_epoll 0.0.0.0 2222|client 172.30.44.140 2222"

.PHONY: deploy-server
deploy-server:
	oc new-project epoll || oc project epoll
	oc apply -f epoll-server.yaml
	oc apply -f epoll-server.yaml

# The fedora images don't like my cluster's DNS setting, so
# do this weird sed replace with the actual service IP.
.PHONY: deploy-client
deploy-client:
	oc new-project epoll || oc project epoll
	cat epoll-client.yaml | \
		sed "s/epoll-server/$(shell oc get svc epoll-server -o jsonpath='{.spec.clusterIP}')/g" | \
		oc apply -f -

.PHONY: deploy
deploy: deploy-server deploy-client

.PHONY: clean
clean:
	oc delete -f epoll-client.yaml || true
	oc delete -f epoll-server.yaml || true
	oc project default
	oc delete project epoll || true

.PHONY: deploy-systemtap
deploy-systemtap:
	./epoll-systemtap.sh "$(SYSTEMTAP_IMAGE)" "$(NODE_SELECTOR)" "$(PGREP)"

.PHONY: clean-systemtap
clean-systemtap:
	oc delete -f _output/ds.yaml || true
	oc delete -f _output/cm.yaml || true
	oc project default
	oc delete project systemtap || true
