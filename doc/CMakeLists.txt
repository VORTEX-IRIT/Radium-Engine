# inspiration https://github.com/kracejic/EmptyDoxygenCMake/blob/master/CMakeLists.txt

cmake_minimum_required(VERSION 3.18)
project(radium-doc)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cmake-message-helpers/MessageHelpers.cmake)
message_prefix_push("Doc")

# enable if doxygen found
find_package(Doxygen 1.8 OPTIONAL_COMPONENTS dot)

message(NOTICE "")
message_title(" documentation ${PROJECT_NAME} ")
message_info("Can build doc?                            ${DOXYGEN_FOUND}")

if(DOXYGEN_FOUND)
    # Search for plantUML for creating UML diagrams from doxygen
    get_target_property(DOT_IMPORTED Doxygen::dot "IMPORTED")
    message_setting("DOT_IMPORTED")
    find_file(PLANT_UML_PATH NAMES plantuml.jar plantuml PATH_SUFFIXES PlantUML plantuml Plantuml
              PATHS /usr/share /usr/local/share/ /usr/local/bin /opt/local/bin c/Program\ Files*
    )
    message_setting("PLANT_UML_PATH")
    if(PLANT_UML_PATH)
        set(DOXYGEN_PLANTUML_JAR_PATH "${PLANT_UML_PATH}")
    endif()

    # configure doxygen target
    set(RADIUM_DOC_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)
    set(RADIUM_MAIN_DOC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/main.md)

    # Get awesome css
    set(RADIUM_DOXYGEN_AWESOME_CSS_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/../external/doxygen-awesome-css"
    )
    set(RADIUM_DOXYGEN_AWESOME_CSS "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome.css")
    set(USE_AWESOME_CSS YES)
    if(NOT EXISTS "${RADIUM_DOXYGEN_AWESOME_CSS}")
        set(USE_AWESOME_CSS OFF)
        if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
            execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init ${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMOD_RESULT
            )
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                message(
                    FATAL_ERROR
                        "git submodule update --init ${DOXYGEN_AWESOME_CSS_ROOT} failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
                )
            else()
                set(USE_AWESOME_CSS YES)
            endif()
        else()
            set(RADIUM_DOXYGEN_AWESOME_CSS "")
        endif()
    endif()
    message_setting("USE_AWESOME_CSS")

    # set RADIUM_PROJECT_NUMBER from VERSION file
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" INPUT_Radium_VERSION)
    set(RADIUM_PROJECT_NUMBER ${INPUT_Radium_VERSION})

    # be nice with IDE: add .md files
    file(GLOB_RECURSE md_files ${CMAKE_CURRENT_SOURCE_DIR}/*/*.md)

    set(md_pages_order
        "${CMAKE_CURRENT_SOURCE_DIR}/main.md" "${CMAKE_CURRENT_SOURCE_DIR}/basics.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/concepts.md" "${CMAKE_CURRENT_SOURCE_DIR}/developer.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES.md"
    )

    set(DOXYGEN_ALIASES [[complexity="@par Complexity:^^"]])
    set(DOXYGEN_BUILTIN_STL_SUPPORT "YES")
    set(DOXYGEN_CALL_GRAPH "YES")
    set(DOXYGEN_COLS_IN_ALPHA_INDEX "2")
    set(DOXYGEN_DOCSET_BUNDLE_ID "com.storm-irit.RadiumEngine")
    set(DOXYGEN_DOCSET_PUBLISHER_NAME "IRIT")
    set(DOXYGEN_DOT_GRAPH_MAX_NODES "60")
    set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
    set(DOXYGEN_EXAMPLE_RECURSIVE "YES")
    set(DOXYGEN_EXAMPLE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/../tests" "${CMAKE_CURRENT_SOURCE_DIR}/../examples/"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    )
    set(DOXYGEN_EXAMPLE_PATTERNS "*.cpp" "*.h" "*.cu")
    set(DOXYGEN_EXCLUDE_PATTERNS "*/src/Headless/CLI/*")
    set(DOXYGEN_EXPAND_ONLY_PREDEF "YES")
    set(DOXYGEN_EXTENSION_MAPPING "dox=C++" "inl=C++")
    set(DOXYGEN_EXTRACT_ALL "NO")
    set(DOXYGEN_EXTRACT_STATIC "YES")
    set(DOXYGEN_EXT_LINKS_IN_WINDOW "YES")
    set(DOXYGEN_HIDE_UNDOC_CLASSES "YES")
    set(DOXYGEN_HIDE_UNDOC_RELATIONS "NO")
    set(DOXYGEN_HTML_COLORSTYLE "LIGHT")
    set(DOXYGEN_HTML_COPY_CLIPBOARD "NO")
    set(DOXYGEN_HTML_DYNAMIC_SECTIONS "YES")
    set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/header.html")
    set(DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/images")
    set(DOXYGEN_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../src")
    set(DOXYGEN_INTERACTIVE_SVG "YES")
    set(DOXYGEN_MACRO_EXPANSION "YES")
    set(DOXYGEN_MATHJAX_EXTENSIONS "TeX/AMSsymbols" "TeX/AMSmath")
    set(DOXYGEN_MATHJAX_RELPATH "https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/")
    set(DOXYGEN_PREDEFINED "PARSED_BY_DOXYGEN MULTIARCH= ")
    set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/images/radium-logo.webp")
    set(DOXYGEN_PROJECT_NAME "Radium Engine")
    set(DOXYGEN_PROJECT_NUMBER "${RADIUM_PROJECT_NUMBER}")
    set(DOXYGEN_RECURSIVE "YES")
    set(DOXYGEN_REFERENCES_LINK_SOURCE "NO")
    set(DOXYGEN_SHOW_GROUPED_MEMB_INC "YES")
    set(DOXYGEN_SOURCE_BROWSER "YES")
    set(DOXYGEN_STRIP_FROM_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../src")
    set(DOXYGEN_TEMPLATE_RELATIONS "YES")
    set(DOXYGEN_TOC_INCLUDE_HEADINGS "2")
    set(DOXYGEN_UML_LIMIT_NUM_FIELDS "50")
    set(DOXYGEN_UML_LOOK "YES")
    set(DOXYGEN_USE_MATHJAX "YES")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${RADIUM_MAIN_DOC_FILE}")
    set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED "NO")

    if(USE_AWESOME_CSS)
        set(DOXYGEN_GENERATE_TREEVIEW "YES")
        set(DOXYGEN_DISABLE_INDEX "NO")
        set(DOXYGEN_FULL_SIDEBAR "NO")
        set(DOXYGEN_HTML_EXTRA_STYLESHEET
            "${RADIUM_DOXYGEN_AWESOME_CSS}"
            "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome-sidebar-only.css"
            "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome-sidebar-only-darkmode-toggle.css"
            "${CMAKE_CURRENT_SOURCE_DIR}/custom.css"
        )
        set(DOXYGEN_HTML_EXTRA_FILES
            "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome-darkmode-toggle.js"
            "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome-fragment-copy-button.js"
            "${RADIUM_DOXYGEN_AWESOME_CSS_ROOT}/doxygen-awesome-paragraph-link.js"
        )
    endif()
    # resolve std to cpp ref
    # https://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml
    set(DOXYGEN_TAGFILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/"
    )

    doxygen_add_docs(
        RadiumDoc ${md_pages_order} ${md_files} ${CMAKE_CURRENT_SOURCE_DIR}/../src/
        ${CURRENT_SRC_DIR} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
    # Where docs will be installed.
    if(RADIUM_INSTALL_DOC)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html TYPE DOC OPTIONAL)
    endif()
endif()
message_end()
message(NOTICE "")

message_prefix_pop()
