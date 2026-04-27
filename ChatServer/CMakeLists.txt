# Серверная часть: ядро + GUI

qt_add_executable(ChatServer
    main.cpp
)

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ChatServer PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

set_target_properties(ChatServer PROPERTIES
    WIN32_EXECUTABLE ON
)

messenger_deploy_qt(ChatServer)
