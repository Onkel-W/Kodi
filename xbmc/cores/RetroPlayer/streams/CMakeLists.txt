set(SOURCES RetroPlayerAudio.cpp
            RetroPlayerInput.cpp
            RetroPlayerMemory.cpp
            RetroPlayerVideo.cpp
            RPStreamManager.cpp
)

set(HEADERS IRetroPlayerStream.h
            IStreamManager.h
            RetroPlayerAudio.h
            RetroPlayerInput.h
            RetroPlayerMemory.h
            RetroPlayerStreamTypes.h
            RetroPlayerVideo.h
            RPStreamManager.h
)

core_add_library(retroplayer_streams)
