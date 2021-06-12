cmake_minimum_required(VERSION 3.16)

add_library(Panda "")
target_sources(Panda
	PRIVATE
		DummyGameObject.h
		Map.h
		PandaModule.h
		Perlin.h
		Precompile.h

		DummyGameObject.cpp
		Map.cpp
		PandaModule.cpp
		Perlin.cpp
)

target_precompile_headers(Panda PRIVATE Precompile.h)
target_compile_features(Panda PRIVATE cxx_std_20)

target_include_directories(Panda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Panda Base)
target_link_libraries(Panda GameWork)
target_link_libraries(Panda Render)
