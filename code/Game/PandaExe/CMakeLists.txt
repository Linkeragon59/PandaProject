cmake_minimum_required(VERSION 3.6.2)

add_executable(PandaExe main.cpp)

target_compile_features(PandaExe PRIVATE cxx_std_20)

target_link_libraries(PandaExe Base)
target_link_libraries(PandaExe GameWork)
target_link_libraries(PandaExe Render)
target_link_libraries(PandaExe Panda)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	set_property(TARGET PandaExe PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
endif ()
