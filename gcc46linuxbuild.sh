echo "CMake configuration script for GCC 4.6 usage on a linux host"
echo "Copyright (C) 2011 by Steffen Ohrendorf <steffen.ohrendorf@gmx.de>"

if ! test -e `which gcc-4.6`
then
	echo "Cannot find the 'gcc-4.6' executable in \$PATH."
	echo "If your version of gcc is >= 4.6, use the preferred build procedure."
	echo "gcc -dumpversion output:"
	gcc -dumpversion
fi

if ! test -d "build"
then
	mkdir build
	if [ "$?" != "0" ]
	then
		echo "Cannot create build directory!"
		exit 1
	fi
fi

cd build
if [ "$?" != "0" ]
then
	echo "Cannot cd to directory 'build'"
	exit 1
fi

echo "Starting cmake configuration..."

cmake -DCMAKE_CXX_COMPILER=`which gcc-4.6` -DCMAKE_C_COMPILER=`which gcc-4.6` -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

echo "Configured, now cd to 'build' and type 'make' to build PeePeePlayer."

