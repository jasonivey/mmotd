# cmake/add_includes_and_flags.cmake
include_guard (DIRECTORY)

cmake_minimum_required (VERSION 3.8)

function (get_cxx_system_include_paths CXX_SYSTEM_INCLUDE_PATHS)
    set (SYSTEM_INCLUDE_PATHS "")
    foreach (dir IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
        get_filename_component(abs_path_dir "${dir}" REALPATH BASE_DIR "/")
        string(APPEND SYSTEM_INCLUDE_PATHS "${abs_path_dir} ")
        message (TRACE "added cxx system include=${abs_path_dir}")
    endforeach ()
    message (TRACE "entire cxx system include=${SYSTEM_INCLUDE_PATHS}")
    set (CXX_SYSTEM_INCLUDE_PATHS ${SYSTEM_INCLUDE_PATHS} PARENT_SCOPE)
endfunction ()

function (get_c_system_include_paths C_SYSTEM_INCLUDE_PATHS)
    set (SYSTEM_INCLUDE_PATHS "")
    foreach (dir IN LISTS CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES)
        get_filename_component(abs_path_dir "${dir}" REALPATH BASE_DIR "/")
        string(APPEND SYSTEM_INCLUDE_PATHS "${abs_path_dir} ")
        message (TRACE "added c system include=${abs_path_dir}")
    endforeach ()
    message (TRACE "entire c system include=${SYSTEM_INCLUDE_PATHS}")
    set (C_SYSTEM_INCLUDE_PATHS ${SYSTEM_INCLUDE_PATHS} PARENT_SCOPE) 
endfunction ()

macro (is_include_in_c_flags INCLUDE_PATH C_INCLUDE_ALREADY_ADDED)
    get_c_system_include_paths(C_SYSTEM_INCLUDE_PATHS)
    string (FIND "${C_SYSTEM_INCLUDE_PATHS}" "${INCLUDE_PATH}" FLAG_INDEX)
    if ("${FLAG_INDEX}" EQUAL "-1")
        message (TRACE "include \"${INCLUDE_PATH}\" was not found in c system path \"${C_SYSTEM_INCLUDE_PATHS}\"")
        set (${C_INCLUDE_ALREADY_ADDED} FALSE)
    else ()
        message (TRACE "include \"${INCLUDE_PATH}\" was found in c system path: \"${C_SYSTEM_INCLUDE_PATHS}\"")
        set (${C_INCLUDE_ALREADY_ADDED} TRUE)
    endif ()
endmacro ()

macro (is_include_in_cxx_flags INCLUDE_PATH CXX_INCLUDE_ALREADY_ADDED)
    get_cxx_system_include_paths(CXX_SYSTEM_INCLUDE_PATHS)
    string (FIND "${CXX_SYSTEM_INCLUDE_PATHS}" "${INCLUDE_PATH}" FLAG_INDEX)
    if ("${FLAG_INDEX}" EQUAL "-1")
        message (TRACE "include \"${INCLUDE_PATH}\" was not found in cxx system path: \"${CXX_SYSTEM_INCLUDE_PATHS}\"")
        set (${CXX_INCLUDE_ALREADY_ADDED} FALSE)
    else ()
        message (TRACE "include \"${INCLUDE_PATH}\" was found in in cxx system path: \"${CXX_SYSTEM_INCLUDE_PATHS}\"")
        set (${CXX_INCLUDE_ALREADY_ADDED} TRUE)
    endif ()
endmacro ()

macro (is_flag_in_c_flags FLAG C_FLAG_ALREADY_ADDED)
    string (FIND "${CMAKE_C_FLAGS}" "${FLAG}" FLAG_INDEX)
    if ("${FLAG_INDEX}" EQUAL "-1")
        message (TRACE "flag: \"${FLAG}\" was not found in cflags: \"${CMAKE_C_FLAGS}\"")
        set (${C_FLAG_ALREADY_ADDED} FALSE)
    else ()
        message (TRACE "flags: \"${FLAG}\" was found in cflags: \"${CMAKE_C_FLAGS}\"")
        set (${C_FLAG_ALREADY_ADDED} TRUE)
    endif ()
endmacro ()

macro (is_flag_in_cxx_flags FLAG CXX_FLAG_ALREADY_ADDED)
    string (FIND "${CMAKE_CXX_FLAGS}" "${FLAG}" FLAG_INDEX)
    if ("${FLAG_INDEX}" EQUAL "-1")
        message (TRACE "flag: \"${FLAG}\" was not found in cxxflags: \"${CMAKE_CXX_FLAGS}\"")
        set (${CXX_FLAG_ALREADY_ADDED} FALSE)
    else ()
        message (TRACE "flag: \"${FLAG}\" was found in cxxflags: \"${CMAKE_CXX_FLAGS}\"")
        set (${CXX_FLAG_ALREADY_ADDED} TRUE)
    endif ()
endmacro ()

macro (add_cmake_c_flags FLAG)
    is_flag_in_c_flags("${FLAG}" C_FLAG_ALREADY_ADDED)
    message (TRACE "flag \"${FLAG}\" has been added to C_FLAGS: ${ALREADY_ADDED}")
    if (${C_FLAG_ALREADY_ADDED})
       message (DEBUG "flag: \"${FLAG}\" has already been added to C_FLAGS (${CMAKE_C_FLAGS})")
    else ()
        message (TRACE "flag: \"${FLAG}\" has not been added to C_FLAGS (${CMAKE_C_FLAGS})")
        string(APPEND CMAKE_C_FLAGS " ${FLAG}")
        message (DEBUG "flag: \"${FLAG}\" is now part of C_FLAGS (${CMAKE_C_FLAGS})")
    endif ()
endmacro ()

macro (add_cmake_cxx_flags FLAG)
    is_flag_in_cxx_flags("${FLAG}" CXX_FLAG_ALREADY_ADDED)
    message (TRACE "flag: \"${FLAG}\" has been added to CXX_FLAGS: ${CXX_FLAG_ALREADY_ADDED}")
    if (${CXX_FLAG_ALREADY_ADDED})
        message (DEBUG "flag: \"${FLAG}\" has already been added to CXX_FLAGS (${CMAKE_CXX_FLAGS})")
    else ()
        message (TRACE "flag: \"${FLAG}\" has not been added to CXX_FLAGS (${CMAKE_CXX_FLAGS})")
        string(APPEND CMAKE_CXX_FLAGS " ${FLAG}")
        message (DEBUG "flag: \"${FLAG}\" is now part of CXX_FLAGS (${CMAKE_CXX_FLAGS})")
    endif ()
endmacro ()

macro (add_cmake_c_cxx_flags FLAG)
    add_cmake_cxx_flags("${FLAG}")
    add_cmake_c_flags("${FLAG}")
endmacro ()

macro (add_cmake_c_cxx_include_directory INCLUDE_PATH IS_SYSTEM_INCLUDE)
    is_include_in_cxx_flags(${INCLUDE_PATH} CXX_INCLUDE_ALREADY_ADDED)
    if (NOT ${CXX_INCLUDE_ALREADY_ADDED})
        message (DEBUG "include directory has not been added to CXX flags: ${INCLUDE_PATH}")
        if (${IS_SYSTEM_INCLUDE})
            add_cmake_cxx_flags("-isystem ${INCLUDE_PATH}")
        else ()
            add_cmake_cxx_flags("-I${INCLUDE_PATH}")
        endif ()
    else ()
        message (DEBUG "include directory already been added to CXX flags: ${INCLUDE_PATH}")
    endif ()
    is_include_in_c_flags(${INCLUDE_PATH} C_INCLUDE_ALREADY_ADDED)
    if (NOT ${C_INCLUDE_ALREADY_ADDED})
        message (DEBUG "include directory has not been added to C flags: ${INCLUDE_PATH}")
        if (${IS_SYSTEM_INCLUDE})
             add_cmake_c_flags("-isystem ${INCLUDE_PATH}")
        else ()
             add_cmake_c_flags("-I${INCLUDE_PATH}")
        endif ()
    else ()
        message (DEBUG "include directory already been added to C flags: ${INCLUDE_PATH}")
    endif ()
endmacro ()

