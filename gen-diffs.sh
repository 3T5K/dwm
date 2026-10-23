#!/bin/sh

for match in */doc.md; do
    dir=${match%/doc.md}
    pushd "$dir" > /dev/null
        echo "generating: $dir"
        grep '^git diff.*>.*\.diff$' doc.md \
            | sed 's/$/ || exit 1/' | sh || exit 1
    popd > /dev/null
done
