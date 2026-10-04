include(FetchContent)

macro(LinkASSIMP TARGET ACCESS)
    set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.cache/fetchcontent" CACHE PATH "" FORCE)

    FetchContent_Declare(
            assimp
            GIT_REPOSITORY https://github.com/assimp/assimp
            GIT_TAG v6.0.0
            GIT_SHALLOW TRUE
    )

    set(ASSIMP_BUILD_ASSIMP_TOOLS                OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS                       OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_SAMPLES                     OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL                           OFF CACHE BOOL "" FORCE)
    set(ASSIMP_NO_EXPORT                         ON  CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT    OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_FBX_IMPORTER                ON  CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(assimp)

    if(TARGET assimp)
        set_target_properties(assimp PROPERTIES FOLDER ${PROJECT_NAME}/thirdparty)
    endif()

    target_link_libraries(${TARGET} ${ACCESS} assimp)
endmacro()