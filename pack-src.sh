#!/bin/bash


if [ "$1" == "" ]
then
	COMMIT=HEAD
else
	COMMIT=$1
fi

VERSION=`git describe $COMMIT`
git archive --prefix="peepeeplayer-$VERSION/" --format=tar -v $COMMIT | bzip2 --best -z > "peepeeplayer-$VERSION.tar.bz2"

