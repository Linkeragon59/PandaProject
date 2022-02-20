cmake_minimum_required(VERSION 3.16)

add_executable(Panda)

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
		main.cpp
)

target_precompile_headers(Panda PRIVATE Precompile.h)
target_compile_features(Panda PRIVATE cxx_std_23)

target_include_directories(Panda PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Panda PRIVATE GameCore)
target_link_libraries(Panda PRIVATE Render)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	set_property(TARGET Panda PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
endif ()
