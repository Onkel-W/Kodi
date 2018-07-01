set(SOURCES GameClientStreamAudio.cpp
            GameClientStreamInput.cpp
            GameClientStreamMemory.cpp
            GameClientStreams.cpp
            GameClientStreamSwFramebuffer.cpp
            GameClientStreamVideo.cpp
)

set(HEADERS GameClientStreamAudio.h
            GameClientStreamInput.h
            GameClientStreamMemory.h
            GameClientStreams.h
            GameClientStreamSwFramebuffer.h
            GameClientStreamVideo.h
            IGameClientStream.h
)

core_add_library(game_addon_streams)
