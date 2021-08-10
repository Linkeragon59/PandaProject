cmake_minimum_required(VERSION 3.16)

add_library(SoLoud "")
target_sources(SoLoud
	PRIVATE
		20200207/include/soloud.h
		20200207/include/soloud_audiosource.h
		20200207/include/soloud_bassboostfilter.h
		20200207/include/soloud_biquadresonantfilter.h
		20200207/include/soloud_bus.h
		20200207/include/soloud_c.h
		20200207/include/soloud_dcremovalfilter.h
		20200207/include/soloud_echofilter.h
		20200207/include/soloud_error.h
		20200207/include/soloud_fader.h
		20200207/include/soloud_fft.h
		20200207/include/soloud_fftfilter.h
		20200207/include/soloud_file.h
		20200207/include/soloud_file_hack_off.h
		20200207/include/soloud_file_hack_on.h
		20200207/include/soloud_filter.h
		20200207/include/soloud_flangerfilter.h
		20200207/include/soloud_freeverbfilter.h
		20200207/include/soloud_internal.h
		20200207/include/soloud_lofifilter.h
		20200207/include/soloud_misc.h
		20200207/include/soloud_monotone.h
		20200207/include/soloud_noise.h
		20200207/include/soloud_openmpt.h
		20200207/include/soloud_queue.h
		20200207/include/soloud_robotizefilter.h
		20200207/include/soloud_sfxr.h
		20200207/include/soloud_speech.h
		20200207/include/soloud_tedsid.h
		20200207/include/soloud_thread.h
		20200207/include/soloud_vic.h
		20200207/include/soloud_vizsn.h
		20200207/include/soloud_wav.h
		20200207/include/soloud_waveshaperfilter.h
		20200207/include/soloud_wavstream.h
		
		20200207/src/audiosource/wav/dr_flac.h
		20200207/src/audiosource/wav/dr_impl.cpp
		20200207/src/audiosource/wav/dr_mp3.h
		20200207/src/audiosource/wav/dr_wav.h
		20200207/src/audiosource/wav/soloud_wav.cpp
		20200207/src/audiosource/wav/soloud_wavstream.cpp
		20200207/src/audiosource/wav/stb_vorbis.c
		20200207/src/audiosource/wav/stb_vorbis.h
		
		# Common
		20200207/src/backend/miniaudio/miniaudio.h
		20200207/src/backend/miniaudio/soloud_miniaudio.cpp	# WITH_MINIAUDIO
		20200207/src/backend/nosound/soloud_nosound.cpp		# WITH_NOSOUND
		20200207/src/backend/null/soloud_null.cpp			# WITH_NULL
		# Windows
		20200207/src/backend/wasapi/soloud_wasapi.cpp		# WITH_WASAPI
		20200207/src/backend/winmm/soloud_winmm.cpp			# WITH_WINMM
		20200207/src/backend/xaudio2/soloud_xaudio2.cpp		# WITH_XAUDIO2
		# Linux
		20200207/src/backend/alsa/soloud_alsa.cpp			# WITH_ALSA
		20200207/src/backend/jack/soloud_jack.cpp			# WITH_JACK
		20200207/src/backend/oss/soloud_oss.cpp				# WITH_OSS
		
		20200207/src/core/soloud.cpp
		20200207/src/core/soloud_audiosource.cpp
		20200207/src/core/soloud_bus.cpp
		20200207/src/core/soloud_core_3d.cpp
		20200207/src/core/soloud_core_basicops.cpp
		20200207/src/core/soloud_core_faderops.cpp
		20200207/src/core/soloud_core_filterops.cpp
		20200207/src/core/soloud_core_getters.cpp
		20200207/src/core/soloud_core_setters.cpp
		20200207/src/core/soloud_core_voicegroup.cpp
		20200207/src/core/soloud_core_voiceops.cpp
		20200207/src/core/soloud_fader.cpp
		20200207/src/core/soloud_fft.cpp
		20200207/src/core/soloud_fft_lut.cpp
		20200207/src/core/soloud_file.cpp
		20200207/src/core/soloud_filter.cpp
		20200207/src/core/soloud_misc.cpp
		20200207/src/core/soloud_queue.cpp
		20200207/src/core/soloud_thread.cpp
)

target_compile_features(SoLoud PRIVATE cxx_std_20)
if (MSVC)
	target_compile_options(SoLoud PRIVATE /wd"4100" /wd"4244" /wd"4245" /wd"4456" /wd"4457" /wd"4701" /wd"4996")
	target_compile_definitions(SoLoud PRIVATE WITH_MINIAUDIO)
else()
endif()

target_include_directories(SoLoud PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/20200207/include)
