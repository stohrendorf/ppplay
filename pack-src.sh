#!/bin/bash

COMMIT=${1:-HEAD}
VERSION=`git describe $COMMIT`
git archive --prefix="ppplay-$VERSION/" --format=tar -v $COMMIT | bzip2 --best -z > "ppplay-$VERSION.tar.bz2"
git archive --prefix="ppplay-$VERSION/" --format=tar.gz -v $COMMIT > "ppplay-$VERSION.tar.gz"
git archive --prefix="ppplay-$VERSION/" --format=zip -v $COMMIT > "ppplay-$VERSION.zip"
