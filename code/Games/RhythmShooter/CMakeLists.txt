cmake_minimum_required(VERSION 3.16)

add_executable(RhythmShooter "")
target_sources(RhythmShooter
	PRIVATE
		RhythmShooterModule.h
		Precompile.h

		RhythmShooterModule.cpp
		main.cpp
)

target_precompile_headers(RhythmShooter PRIVATE Precompile.h)
target_compile_features(RhythmShooter PRIVATE cxx_std_20)

target_include_directories(RhythmShooter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RhythmShooter Base)
target_link_libraries(RhythmShooter GameWork)
target_link_libraries(RhythmShooter Render)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	set_property(TARGET RhythmShooter PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
endif ()
