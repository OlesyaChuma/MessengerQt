# Хелпер для авто-деплоя Qt DLL рядом с exe на Windows.
# Вызывает windeployqt после сборки указанного таргета.

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${CMAKE_PREFIX_PATH}/bin" "$ENV{QTDIR}/bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "windeployqt found: ${WINDEPLOYQT_EXECUTABLE}")
    else()
        message(WARNING "windeployqt not found - Qt DLLs will NOT be deployed automatically")
    endif()
endif()

function(messenger_deploy_qt target)
    if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations --no-system-d3d-compiler
                    --no-opengl-sw --no-quick-import
                    $<IF:$<CONFIG:Debug>,--debug,--release>
                    "$<TARGET_FILE:${target}>"
            COMMENT "Running windeployqt for ${target}"
            VERBATIM
        )
    endif()
endfunction()