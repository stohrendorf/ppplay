#!/bin/bash

COMMIT=${1:-HEAD}
VERSION=`git describe $COMMIT`
git archive --prefix="peepeeplayer-$VERSION/" --format=tar -v $COMMIT | bzip2 --best -z > "peepeeplayer-$VERSION.tar.bz2"
git archive --prefix="peepeeplayer-$VERSION/" --format=tar.gz -v $COMMIT > "peepeeplayer-$VERSION.tar.gz"
git archive --prefix="peepeeplayer-$VERSION/" --format=zip -v $COMMIT > "peepeeplayer-$VERSION.zip"
