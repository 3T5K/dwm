#!/bin/sh

for match in */doc.md; do
    dir=${match%/doc.md}
    pushd "$dir" > /dev/null
        grep '^git diff.*>.*\.diff$' doc.md \
            | sh \
            || exit 1
    popd > /dev/null
done
