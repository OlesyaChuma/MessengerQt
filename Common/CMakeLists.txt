# Общая библиотека: модели, протокол, фрейм-кодек, хешер паролей.

add_library(MessengerCommon STATIC
    Models.h
    Protocol.h
    Protocol.cpp
    FrameCodec.h
    FrameCodec.cpp
    PasswordHasher.h
    PasswordHasher.cpp
)

target_include_directories(MessengerCommon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MessengerCommon PUBLIC
    Qt6::Core
    Qt6::Network
)
