add_custom_target(integration)

# this target generates all references files with HEAD version of the source code, use it when
# updating the tests while not updating the libs
add_custom_target(integration_create_ref)

# -----------------------------------------------------------------------------
# ------------- TinyPlyFileLoader ---------------------------------------------
# -----------------------------------------------------------------------------

set(INTEGRATION_TEST TinyPlyFileLoader)
# TinyPlyFileLoader read tests
add_executable(integration_${INTEGRATION_TEST} ${INTEGRATION_TEST}/main.cpp)
target_compile_options(integration_${INTEGRATION_TEST} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})
target_link_libraries(integration_${INTEGRATION_TEST} PUBLIC Core IO)

add_custom_target(integration_${INTEGRATION_TEST}_create_ref)
add_dependencies(integration_create_ref integration_${INTEGRATION_TEST}_create_ref)

add_custom_command(
    TARGET integration_${INTEGRATION_TEST}_create_ref
    PRE_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${INTEGRATION_TEST}/
    COMMAND
        bash -c
        "$<TARGET_FILE:integration_${INTEGRATION_TEST}>  2>&1 | cut -f4- -d: | grep -v \"Loading Time\" > ${CMAKE_CURRENT_SOURCE_DIR}/data/references/${INTEGRATION_TEST}.txt"
    COMMAND_EXPAND_LISTS VERBATIM
    COMMENT "generating references for integration_${INTEGRATION_TEST} integration test" DEPENDS
            integration_${INTEGRATION_TEST}
)

add_test(
    NAME integration_${INTEGRATION_TEST}_save_output
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${INTEGRATION_TEST}/
    COMMAND
        bash -c
        "$<TARGET_FILE:integration_${INTEGRATION_TEST}> 2>&1 | cut -f4- -d: | grep -v \"Loading Time\" > ${CMAKE_CURRENT_BINARY_DIR}/output.txt"
)

add_test(
    NAME integration_${INTEGRATION_TEST}_compare_to_ref
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${INTEGRATION_TEST}/
    COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_CURRENT_BINARY_DIR}/output.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/data/references/${INTEGRATION_TEST}.txt
)
set_tests_properties(
    integration_${INTEGRATION_TEST}_compare_to_ref
    PROPERTIES DEPENDS integration_${INTEGRATION_TEST}_save_output
)

add_dependencies(integration integration_${INTEGRATION_TEST})

# -----------------------------------------------------------------------------
# ------------- KeyMappingManager ---------------------------------------------
# -----------------------------------------------------------------------------

set(INTEGRATION_TEST KeyMappingManager)

find_program(LIBXML2_XMLLINT_EXECUTABLE xmllint)
find_program(SED_EXECUTABLE sed)

if(LIBXML2_XMLLINT_EXECUTABLE AND SED_EXECUTABLE)
    find_package(Filesystem COMPONENTS Final Experimental REQUIRED)
    find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED)

    set(Qt_LIBRARIES Qt::Core Qt::Widgets Qt::OpenGL Qt::Xml)

    # KeyMappingManager saveConfiguration tests
    add_executable(integration_${INTEGRATION_TEST} ${INTEGRATION_TEST}/main.cpp)
    target_compile_options(integration_${INTEGRATION_TEST} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

    target_link_libraries(
        integration_${INTEGRATION_TEST} PUBLIC Core Gui PRIVATE std::filesystem ${Qt_LIBRARIES}
    )
    target_compile_definitions(
        integration_${INTEGRATION_TEST}
        PRIVATE -DCXX_FILESYSTEM_HAVE_FS
                -DCXX_FILESYSTEM_IS_EXPERIMENTAL=$<BOOL:${CXX_FILESYSTEM_IS_EXPERIMENTAL}>
                -DCXX_FILESYSTEM_NAMESPACE=${CXX_FILESYSTEM_NAMESPACE}
    )

    add_test(
        NAME integration_${INTEGRATION_TEST}_save_output
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${INTEGRATION_TEST}/
        COMMAND
            bash -c
            "$<TARGET_FILE:integration_${INTEGRATION_TEST}>  ${CMAKE_CURRENT_BINARY_DIR} &&\
 \"${LIBXML2_XMLLINT_EXECUTABLE}\" --c14n11  ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid.xml > ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-out-lint.xml &&\
 \"${LIBXML2_XMLLINT_EXECUTABLE}\" --c14n11  ${CMAKE_CURRENT_SOURCE_DIR}/data/inputs/keymapping-valid.xml > ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-in-lint.xml &&\
 \"${SED_EXECUTABLE}\" -i.orig '/<!--.*-->/d' ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-in-lint.xml &&\
 \"${SED_EXECUTABLE}\" -i.orig '/<!--.*-->/d' ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-out-lint.xml &&\
 \"${SED_EXECUTABLE}\" -i.orig '/<!--/,/-->/d' ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-in-lint.xml &&\
 \"${SED_EXECUTABLE}\" -i.orig '/<!--/,/-->/d' ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-out-lint.xml; "
    )

    add_test(
        NAME integration_${INTEGRATION_TEST}_compare_to_ref
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${INTEGRATION_TEST}/
        COMMAND
            ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-out-lint.xml
            ${CMAKE_CURRENT_BINARY_DIR}/keymapping-valid-in-lint.xml
    )

    set_tests_properties(
        integration_${INTEGRATION_TEST}_compare_to_ref
        PROPERTIES DEPENDS integration_${INTEGRATION_TEST}_save_output
    )

    add_dependencies(integration integration_${INTEGRATION_TEST})
else()
    message(
        WARNING
            "Could not add integration test for KeyMappingManager: sed (${SED_EXECUTABLE}) or xmllint (${LIBXML2_XMLLINT_EXECUTABLE})"
    )
endif()

# -----------------------------------------------------------------------------
# ------------- run examples --------------------------------------------------
# -----------------------------------------------------------------------------
add_test(NAME integration_CoreExample WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
         COMMAND "$<TARGET_FILE:CoreExample>"
)
add_dependencies(integration CoreExample)

set(INTEGRATION_TEST "HeadlessExample")

if(UNIX AND TARGET ${INTEGRATION_TEST})

    find_package(OpenGL COMPONENTS EGL QUIET)
    find_program(COMPARE_EXECUTABLE compare)

    if(OpenGL_EGL_FOUND)
        add_test(NAME integration_${INTEGRATION_TEST} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                 COMMAND "$<TARGET_FILE:${INTEGRATION_TEST}>"
        )
        add_dependencies(integration ${INTEGRATION_TEST})
        add_custom_target(integration_${INTEGRATION_TEST}_create_ref)

        add_custom_command(
            TARGET integration_${INTEGRATION_TEST}_create_ref
            PRE_BUILD
            COMMAND
                bash -c "$<TARGET_FILE:${INTEGRATION_TEST}>" && cp
                "${CMAKE_CURRENT_BINARY_DIR}/frame.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/data/references/${INTEGRATION_TEST}.png" DEPENDS
                integration_${INTEGRATION_TEST}
        )
    else()
        message(
            STATUS
                "could not run ${INTEGRATION_TEST} integration test, EGL libraries not detected on this system"
        )
    endif()
    if(OpenGL_EGL_FOUND AND COMPARE_EXECUTABLE)
        add_test(
            NAME integration_${INTEGRATION_TEST}_compare_to_ref
            COMMAND
                bash -c
                "${COMPARE_EXECUTABLE} -metric PHASH \"${CMAKE_CURRENT_BINARY_DIR}/frame.png\" \"${CMAKE_CURRENT_SOURCE_DIR}/data/references/${INTEGRATION_TEST}.png\" null: > \"${CMAKE_CURRENT_BINARY_DIR}/value\" 2>&1;\
                    VALUE=$(cat \"${CMAKE_CURRENT_BINARY_DIR}/value\");\
                    if (( $(echo \"\${VALUE} > 0.01\" |bc -l) )); then false; fi"
        )
    else()
        message(
            STATUS
                "could not run ${INTEGRATION_TEST} integration test, missing tools EGL libraries, ${COMPARE_EXECUTABLE}"
        )
    endif()
endif()
# -----------------------------------------------------------------------------
# ------------- includes ------------------------------------------------------
# -----------------------------------------------------------------------------

set(INTEGRATION_TEST includes)

find_package(Filesystem COMPONENTS Final Experimental REQUIRED)
find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED)

macro(generate_includes name)
    include(${CMAKE_SOURCE_DIR}/src/${name}/filelist.cmake)
    string(TOLOWER ${name} lowername)
    set(filelist "${lowername}_headers")
    set(tmp ${${filelist}})
    list(TRANSFORM tmp PREPEND "#include <${name}/")
    list(TRANSFORM tmp APPEND ">\n")
    string(REPLACE ";" "" str_${lowername}_headers "${tmp}")
endmacro()

generate_includes(Core)
generate_includes(Engine)
generate_includes(Gui)
generate_includes(IO)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/includes.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/includes.cpp"
)

set(Qt_LIBRARIES Qt::Core Qt::Widgets Qt::OpenGL Qt::Xml)
add_executable(integration_${INTEGRATION_TEST} "${CMAKE_CURRENT_BINARY_DIR}/includes.cpp")
target_compile_options(integration_${INTEGRATION_TEST} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

target_link_libraries(
    integration_${INTEGRATION_TEST} PUBLIC Core Gui Engine PRIVATE std::filesystem ${Qt_LIBRARIES}
)

target_compile_definitions(
    integration_${INTEGRATION_TEST}
    PRIVATE -DCXX_FILESYSTEM_HAVE_FS
            -DCXX_FILESYSTEM_IS_EXPERIMENTAL=$<BOOL:${CXX_FILESYSTEM_IS_EXPERIMENTAL}>
            -DCXX_FILESYSTEM_NAMESPACE=${CXX_FILESYSTEM_NAMESPACE}
)

# there is no run here, the fact that it compiles is the test. maybe use try_compile, but harder to
# do ;)
add_dependencies(integration integration_${INTEGRATION_TEST})
