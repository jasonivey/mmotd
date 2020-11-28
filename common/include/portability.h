#pragma once

#if defined(__has_cpp_attribute)
#    if __has_cpp_attribute(nodiscard)
#        define FOLLY_NODISCARD [[nodiscard]]
#    endif
#endif
#if !defined FOLLY_NODISCARD
#    if defined(_MSC_VER) && (_MSC_VER >= 1700)
#        define FOLLY_NODISCARD _Check_return_
#    elif defined(__GNUC__)
#        define FOLLY_NODISCARD __attribute__((__warn_unused_result__))
#    else
#        define FOLLY_NODISCARD
#    endif
#endif
