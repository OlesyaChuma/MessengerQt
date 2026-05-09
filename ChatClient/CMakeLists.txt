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
        gui/AvatarRenderer.h
        gui/AvatarRenderer.cpp
        gui/UsersListModel.h
        gui/UsersListModel.cpp
        gui/UserItemDelegate.h
        gui/UserItemDelegate.cpp
        gui/UsersPanel.h
        gui/UsersPanel.cpp
        gui/MessageBubble.h
        gui/MessageBubble.cpp
        gui/MessagesView.h
        gui/MessagesView.cpp
        gui/ChatInputWidget.h
        gui/ChatInputWidget.cpp
        gui/ChatPanel.h
        gui/ChatPanel.cpp
        gui/MessageEditDialog.h
        gui/MessageEditDialog.cpp
        gui/EmojiData.h
        gui/EmojiData.cpp
        gui/EmojiPicker.h
        gui/EmojiPicker.cpp
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
        resources/emoji/1F600.png
        resources/emoji/1F601.png
        resources/emoji/1F602.png
        resources/emoji/1F923.png
        resources/emoji/1F60A.png
        resources/emoji/1F60D.png
        resources/emoji/1F618.png
        resources/emoji/1F61C.png
        resources/emoji/1F914.png
        resources/emoji/1F60E.png
        resources/emoji/1F62D.png
        resources/emoji/1F622.png
        resources/emoji/1F621.png
        resources/emoji/1F634.png
        resources/emoji/1F92F.png
        resources/emoji/1F973.png
        resources/emoji/1F44D.png
        resources/emoji/1F44E.png
        resources/emoji/1F44F.png
        resources/emoji/1F64C.png
        resources/emoji/1F64F.png
        resources/emoji/1F44A.png
        resources/emoji/270A.png
        resources/emoji/270C.png
        resources/emoji/1F91D.png
        resources/emoji/1F91F.png
        resources/emoji/1F90C.png
        resources/emoji/1F44B.png
        resources/emoji/1F4AA.png
        resources/emoji/1F91E.png
        resources/emoji/1F446.png
        resources/emoji/1F447.png
        resources/emoji/2764.png
        resources/emoji/1F9E1.png
        resources/emoji/1F49B.png
        resources/emoji/1F49A.png
        resources/emoji/1F499.png
        resources/emoji/1F49C.png
        resources/emoji/1F5A4.png
        resources/emoji/1F90D.png
        resources/emoji/1F498.png
        resources/emoji/1F495.png
        resources/emoji/1F496.png
        resources/emoji/1F497.png
        resources/emoji/1F49E.png
        resources/emoji/1F49F.png
        resources/emoji/1F494.png
        resources/emoji/1F49D.png
        resources/emoji/1F436.png
        resources/emoji/1F431.png
        resources/emoji/1F42D.png
        resources/emoji/1F430.png
        resources/emoji/1F98A.png
        resources/emoji/1F43B.png
        resources/emoji/1F43C.png
        resources/emoji/1F428.png
        resources/emoji/1F981.png
        resources/emoji/1F42F.png
        resources/emoji/1F434.png
        resources/emoji/1F42E.png
        resources/emoji/1F437.png
        resources/emoji/1F438.png
        resources/emoji/1F435.png
        resources/emoji/1F414.png
        resources/emoji/1F525.png
        resources/emoji/2B50.png
        resources/emoji/2728.png
        resources/emoji/1F4A5.png
        resources/emoji/1F389.png
        resources/emoji/1F38A.png
        resources/emoji/1F381.png
        resources/emoji/2615.png
        resources/emoji/1F37A.png
        resources/emoji/1F355.png
        resources/emoji/1F354.png
        resources/emoji/1F370.png
        resources/emoji/2705.png
        resources/emoji/274C.png
        resources/emoji/2753.png
        resources/emoji/2757.png
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
