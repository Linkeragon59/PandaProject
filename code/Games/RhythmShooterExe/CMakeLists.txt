cmake_minimum_required(VERSION 3.16)

add_executable(RhythmShooterExe main.cpp)

target_compile_features(RhythmShooterExe PRIVATE cxx_std_20)

target_link_libraries(RhythmShooterExe Base)
target_link_libraries(RhythmShooterExe GameWork)
target_link_libraries(RhythmShooterExe RhythmShooter)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	set_property(TARGET RhythmShooterExe PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
endif ()
