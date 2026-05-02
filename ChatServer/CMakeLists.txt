# Серверная часть с GUI на Qt Widgets.

qt_add_executable(ChatServer
    main.cpp

    # Core
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

    # GUI
    gui/ThemeManager.h
    gui/ThemeManager.cpp
    gui/LoginDialog.h
    gui/LoginDialog.cpp
    gui/MainWindow.h
    gui/MainWindow.cpp
    gui/UsersModel.h
    gui/UsersModel.cpp
    gui/UsersTab.h
    gui/UsersTab.cpp
    gui/BanDialog.h
    gui/BanDialog.cpp
    gui/MessagesModel.h
    gui/MessagesModel.cpp
    gui/MessagesTab.h
    gui/MessagesTab.cpp
    gui/ConnectionLogTab.h
    gui/ConnectionLogTab.cpp
)

# Ресурсы (qrc)
qt_add_resources(ChatServer "server_resources"
    PREFIX "/server"
    BASE   ${CMAKE_CURRENT_SOURCE_DIR}/resources
    FILES
        resources/theme_light.qss
        resources/theme_dark.qss
        resources/icons/server.svg
        resources/icons/user.svg
        resources/icons/message.svg
        resources/icons/log.svg
)

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
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

messenger_deploy_qt_with_sql(ChatServer)