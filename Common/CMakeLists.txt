# Общая библиотека: модели, протокол, утилиты, используемые сервером и клиентом

add_library(MessengerCommon STATIC
    # Файлы добавим в Пакете 2
    placeholder.cpp
)

target_include_directories(MessengerCommon PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MessengerCommon PUBLIC
    Qt6::Core
    Qt6::Network
)