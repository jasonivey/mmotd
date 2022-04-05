# cmake/version.cmake

function (get_mmotd_version MAJOR_VERSION MINOR_VERSION PATCHLEVEL_VERSION)
    # version: 0.1.8-beta
    set(${MAJOR_VERSION}           0 PARENT_SCOPE)
    set(${MINOR_VERSION}           1 PARENT_SCOPE)
    set(${PATCHLEVEL_VERSION} 8-beta PARENT_SCOPE)
endfunction ()
