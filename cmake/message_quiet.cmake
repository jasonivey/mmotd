cmake_minimum_required (VERSION 3.8)
project (MessageQuiet)

function (message)
    if (NOT MESSAGE_QUIET)
        _message (${ARGN})
    endif ()
endfunction ()
