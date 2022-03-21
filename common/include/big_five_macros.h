// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

// Currently adding a custom (or = default;) destructor to a class will
//  disable any implicit move operations.

// Mark the constructor, copy operations, move operations and destructor as default
#if !defined(DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR)
#define DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(ClassName)                                                 \
    ClassName() = default;                                                                                             \
    ClassName(ClassName const &other) = default;                                                                       \
    ClassName &operator=(ClassName const &other) = default;                                                            \
    ClassName(ClassName &&other) = default;                                                                            \
    ClassName &operator=(ClassName &&other) = default;                                                                 \
    ~ClassName() = default
#endif

// Mark the constructor, copy operations, move operations and 'virtual' destructor as default
#if !defined(DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR)
#define DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(ClassName)                                         \
    ClassName() = default;                                                                                             \
    ClassName(ClassName const &other) = default;                                                                       \
    ClassName &operator=(ClassName const &other) = default;                                                            \
    ClassName(ClassName &&other) = default;                                                                            \
    ClassName &operator=(ClassName &&other) = default;                                                                 \
    virtual ~ClassName() = default
#endif

// Leave constructor undefined but mark copy operations, move operations and destructor as default
#if !defined(NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR)
#define NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(ClassName)                                               \
    ClassName(ClassName const &other) = default;                                                                       \
    ClassName &operator=(ClassName const &other) = default;                                                            \
    ClassName(ClassName &&other) = default;                                                                            \
    ClassName &operator=(ClassName &&other) = default;                                                                 \
    ~ClassName() = default
#endif

// Leave constructor undefined but mark copy operations, move operations and 'virtual' destructor as default
#if !defined(NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR)
#define NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(ClassName)                                       \
    ClassName(ClassName const &other) = default;                                                                       \
    ClassName &operator=(ClassName const &other) = default;                                                            \
    ClassName(ClassName &&other) = default;                                                                            \
    ClassName &operator=(ClassName &&other) = default;                                                                 \
    virtual ~ClassName() = default
#endif

// Leave constructor and destructor undefined but mark copy and move operations as default
#if !defined(NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_NO_DESTRUCTOR)
#define NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_NO_DESTRUCTOR(ClassName)                                            \
    ClassName(ClassName const &other) = default;                                                                       \
    ClassName &operator=(ClassName const &other) = default;                                                            \
    ClassName(ClassName &&other) = default;                                                                            \
    ClassName &operator=(ClassName &&other) = default
#endif

// Default constructor and destructor, delete the copy and move operators
#if !defined(DEFAULT_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR)
#define DEFAULT_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR(ClassName)                                   \
    ClassName() = default;                                                                                             \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    ~ClassName() = default
#endif

// Default constructor and virtual destructor, delete the copy and move operators
#if !defined(DEFAULT_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR)
#define DEFAULT_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR(ClassName)                           \
    ClassName() = default;                                                                                             \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    virtual ~ClassName() = default
#endif

// No default constructor, default destructor, delete the copy and move operators
#if !defined(NO_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR)
#define NO_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR(ClassName)                                        \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    ~ClassName() = default
#endif

// No default constructor, default virtual destructor, delete the copy and move operators
#if !defined(NO_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR)
#define NO_CONSTRUCTOR_DELETE_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR(ClassName)                                \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    virtual ~ClassName() = default
#endif

// Default destructor, delete constructor, copy and move operators
#if !defined(DELETE_CONSTRUCTOR_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR)
#define DELETE_CONSTRUCTOR_COPY_MOVE_OPERATORS_DEFAULT_DESTRUCTOR(ClassName)                                           \
    ClassName() = delete;                                                                                              \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    ~ClassName() = default
#endif

// Default virtual destructor, delete constructor, copy and move operators
#if !defined(DELETE_CONSTRUCTOR_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR)
#define DELETE_CONSTRUCTOR_COPY_MOVE_OPERATORS_DEFAULT_VIRTUAL_DESTRUCTOR(ClassName)                                   \
    ClassName() = delete;                                                                                              \
    ClassName(ClassName const &other) = delete;                                                                        \
    ClassName &operator=(ClassName const &other) = delete;                                                             \
    ClassName(ClassName &&other) = delete;                                                                             \
    ClassName &operator=(ClassName &&other) = delete;                                                                  \
    virtual ~ClassName() = default
#endif
