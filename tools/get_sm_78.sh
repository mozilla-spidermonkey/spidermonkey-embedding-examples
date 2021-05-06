#!/bin/bash
# get commit and appropriate mozjs tar
repo=mozilla-esr78
jobs=( $(curl "https://treeherder.mozilla.org/api/project/$repo/push/?full=true&count=10" | jq '.results[].id') )
for i in "${jobs[@]}"
do
    task_id=$(curl "https://treeherder.mozilla.org/api/jobs/?push_id=$i" | jq -r '.results[] | select(.[] == "spidermonkey-sm-package-linux64/opt") | .[14]')
    echo "Task id $task_id"
    if [ ! -z "${task_id}" ]; then
        tar_file=$(curl "https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/$task_id/runs/0/artifacts" | jq -r '.artifacts[] | select(.name | contains("tar.bz2")) | .name')
        echo "Tar at https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/$task_id/runs/0/artifacts/$tar_file"
        curl -L --output mozjs.tar.bz2 "https://firefox-ci-tc.services.mozilla.com/api/queue/v1/task/$task_id/runs/0/artifacts/$tar_file"
        break
    fi
done
