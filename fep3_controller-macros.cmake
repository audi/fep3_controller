################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep_install(\<name\> \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK libraries (if neccessary)
#   to the folder \<destination\>
# Arguments:
# \li \<name\>:
# The name of the library to install.
# \li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(fep3_controller_install NAME DESTINATION)
    install(TARGETS ${NAME} DESTINATION ${DESTINATION})
    install(FILES $<TARGET_FILE:fep3_controller> DESTINATION ${DESTINATION})
    fep3_system_install(${NAME} ${DESTINATION})
endmacro(fep3_controller_install NAME DESTINATION)

macro(fep3_controller_deploy NAME)
    # no need to copy in build directory on linux since linker rpath takes care of that
    if (WIN32)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:fep3_controller>" "$<TARGET_FILE_DIR:${NAME}>"
        )
    endif()
    fep3_system_deploy(${NAME})
    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
endmacro(fep3_controller_deploy NAME)

