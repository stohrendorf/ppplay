add_library(ppplay_input_hsc STATIC
	    plugin.h
	hscmodule.cpp
	hscmodule.h
)
target_link_libraries(ppplay_input_hsc PUBLIC ppplay_module_base ppplay_core ppplay_opl)
