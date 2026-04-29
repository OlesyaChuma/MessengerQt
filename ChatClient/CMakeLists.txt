# Клиентская часть.
# В Пакете 5 это станет GUI-клиентом — пока консольный для интеграционной проверки.

qt_add_executable(ChatClient
    main.cpp
    core/ChatClientCore.h
    core/ChatClientCore.cpp
    ConsoleUi.h
    ConsoleUi.cpp
)

target_include_directories(ChatClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(ChatClient PRIVATE
    MessengerCommon
    Qt6::Core
    Qt6::Network
)

# Консольный — оставляем видимое окно для отладки.
set_target_properties(ChatClient PROPERTIES
    WIN32_EXECUTABLE OFF
)

messenger_deploy_qt(ChatClient)