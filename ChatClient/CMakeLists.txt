# Клиентская часть с GUI на Qt Widgets.

qt_add_executable(ChatClient
    main.cpp

    # Core
    core/ChatClientCore.h
    core/ChatClientCore.cpp

    # GUI
    gui/ThemeManager.h
    gui/ThemeManager.cpp
    gui/TranslationManager.h
    gui/TranslationManager.cpp
    gui/LoginWindow.h
    gui/LoginWindow.cpp
    gui/MainWindow.h
    gui/MainWindow.cpp
    gui/UsersPanel.h
    gui/UsersPanel.cpp
    gui/ChatPanel.h
    gui/ChatPanel.cpp
)

# Ресурсы (qrc)
qt_add_resources(ChatClient "client_resources"
    PREFIX "/client"
    BASE   ${CMAKE_CURRENT_SOURCE_DIR}/resources
    FILES
        resources/client_light.qss
        resources/client_dark.qss
        resources/icons/app.svg
        resources/icons/send.svg
        resources/icons/attach.svg
        resources/icons/search.svg
)

# Переводы — .qm файлы генерируются вне CMake через update_translations.bat
set(CLIENT_QM_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/messenger_client_en.qm
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/messenger_client_ru.qm
)
foreach(QM_FILE ${CLIENT_QM_FILES})
    if(NOT EXISTS ${QM_FILE})
        file(WRITE ${QM_FILE} "")
    endif()
endforeach()
qt_add_resources(ChatClient "client_translations"
    PREFIX "/client/i18n"
    BASE   ${CMAKE_CURRENT_SOURCE_DIR}/translations
    FILES  ${CLIENT_QM_FILES}
)

target_include_directories(ChatClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

target_link_libraries(ChatClient PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# GUI-приложение — без cmd-окна
set_target_properties(ChatClient PROPERTIES
    WIN32_EXECUTABLE ON
)

messenger_deploy_qt(ChatClient)