# Клиентская часть: GUI + сетевое ядро

qt_add_executable(ChatClient
    main.cpp
)

target_include_directories(ChatClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ChatClient PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

set_target_properties(ChatClient PROPERTIES
    WIN32_EXECUTABLE ON
)

messenger_deploy_qt(ChatClient)
