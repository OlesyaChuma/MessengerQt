# Серверная часть: ядро + (на Пакете 4) GUI.
# Сейчас сервер запускается как консольное приложение с админ-меню.

qt_add_executable(ChatServer
    main.cpp
    core/Config.h
    core/Config.cpp
    core/Database.h
    core/Database.cpp
    core/AuthService.h
    core/AuthService.cpp
    core/ClientSession.h
    core/ClientSession.cpp
    core/ChatServer.h
    core/ChatServer.cpp
    core/AdminConsole.h
    core/AdminConsole.cpp
)

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(ChatServer PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)

# На этой стадии сервер — консольный (для отладки).
# В Пакете 4 переключим на GUI и поставим WIN32_EXECUTABLE ON.
set_target_properties(ChatServer PROPERTIES
    WIN32_EXECUTABLE OFF
)

messenger_deploy_qt_with_sql(ChatServer)