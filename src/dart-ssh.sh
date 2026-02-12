#! /bin/bash

ssh -i keys/id_rsa -o "StrictHostKeyChecking no" -o "UserKnownHostsFile=/dev/null" zre@192.168.0.1