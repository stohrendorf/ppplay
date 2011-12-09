#!/bin/bash

COMMIT=${1:-HEAD}
VERSION=`git describe $COMMIT`
git archive --prefix="peepeeplayer-$VERSION/" --format=tar -v $COMMIT | bzip2 --best -z > "peepeeplayer-$VERSION.tar.bz2"

