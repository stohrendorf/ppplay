set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include(GenerateExportHeader)
add_compiler_export_flags()

add_library(ppplay_opl SHARED
	abstractchannel.cpp
	abstractrhythmchannel.cpp
	bassdrumchannel.cpp
	channel2op.cpp
	highhatoperator.cpp
	highhatsnaredrumchannel.cpp
	operator.cpp
	opl3.cpp
	phasegenerator.cpp
	snaredrumoperator.cpp
	tomtomtopcymbalchannel.cpp
	topcymbaloperator.cpp
)

generate_export_header(ppplay_opl)

install(TARGETS ppplay_opl DESTINATION lib EXPORT ppplayExports)
install(FILES
	opl3.h
	${CMAKE_CURRENT_BINARY_DIR}/ppplay_opl_export.h
	DESTINATION include/ppplay/opl
)
