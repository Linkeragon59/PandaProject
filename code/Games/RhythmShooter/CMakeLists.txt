cmake_minimum_required(VERSION 3.16)

add_library(RhythmShooter "")
target_sources(RhythmShooter
	PRIVATE
		RhythmShooterModule.h
		Precompile.h
		
		RhythmShooterModule.cpp
)

target_precompile_headers(RhythmShooter PRIVATE Precompile.h)
target_compile_features(RhythmShooter PRIVATE cxx_std_20)

target_include_directories(RhythmShooter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RhythmShooter Base)
target_link_libraries(RhythmShooter GameWork)
target_link_libraries(RhythmShooter Render)
