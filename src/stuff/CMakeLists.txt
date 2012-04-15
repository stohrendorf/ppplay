#add_library(fft STATIC
#	fft.cpp)

add_library(utils STATIC
	moduleregistry.cpp
	numberutils.cpp
	stringutils.cpp
)

add_library(timer STATIC
	itimer.cpp
	sdltimer.cpp
)
