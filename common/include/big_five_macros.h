// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#pragma once

// Currently adding a custom (or = default;) destructor to a class will
//  disable any implicit move operations.

// Create defaults for the "Rule of 5"
#if !defined(DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR)
#define DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_DESTRUCTOR(ClassName) \
    ClassName() = default;                                             \
    ClassName(ClassName const &other) = default;                       \
    ClassName &operator=(ClassName const &other) = default;            \
    ClassName(ClassName &&other) = default;                            \
    ClassName &operator=(ClassName &&other) = default;                 \
    ~ClassName() = default
#endif

// Create defaults for the "Rule of 5" and mark the destructor virtual
#if !defined(DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR)
#define DEFAULT_CONSTRUCTORS_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(ClassName) \
    ClassName() = default;                                                     \
    ClassName(ClassName const &other) = default;                               \
    ClassName &operator=(ClassName const &other) = default;                    \
    ClassName(ClassName &&other) = default;                                    \
    ClassName &operator=(ClassName &&other) = default;                         \
    virtual ~ClassName() = default
#endif

// Create copy and move operations
#if !defined(NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR)
#define NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_DESTRUCTOR(ClassName) \
    ClassName(ClassName const &other) = default;                         \
    ClassName &operator=(ClassName const &other) = default;              \
    ClassName(ClassName &&other) = default;                              \
    ClassName &operator=(ClassName &&other) = default;                   \
    ~ClassName() = default
#endif

// Create copy and move operations and mark the destructor virtual
#if !defined(NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR)
#define NO_CONSTRUCTOR_DEFAULT_COPY_MOVE_OPERATORS_VIRTUAL_DESTRUCTOR(ClassName) \
    ClassName(ClassName const &other) = default;                                 \
    ClassName &operator=(ClassName const &other) = default;                      \
    ClassName(ClassName &&other) = default;                                      \
    ClassName &operator=(ClassName &&other) = default;                           \
    virtual ~ClassName() = default
#endif

// Default constructor and destructor, delete the copy and move operators
#if !defined(DEFAULT_CONSTRUCTOR_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS)
#define DEFAULT_CONSTRUCTOR_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS(ClassName) \
    ClassName() = default;                                                   \
    ClassName(ClassName const &other) = delete;                              \
    ClassName &operator=(ClassName const &other) = delete;                   \
    ClassName(ClassName &&other) = delete;                                   \
    ClassName &operator=(ClassName &&other) = delete;                        \
    ~ClassName() = default
#endif

// No default constructor, default destructor, delete the copy and move operators
#if !defined(NO_CONSTRUCTOR_DEFAULT_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS)
#define NO_CONSTRUCTOR_DEFAULT_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS(ClassName) \
    ClassName(ClassName const &other) = delete;                                 \
    ClassName &operator=(ClassName const &other) = delete;                      \
    ClassName(ClassName &&other) = delete;                                      \
    ClassName &operator=(ClassName &&other) = delete;                           \
    ~ClassName() = default
#endif

// Default constructor and virtual destructor, delete the copy and move operators
#if !defined(DEFAULT_CONSTRUCTOR_VIRTUAL_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS)
#define DEFAULT_CONSTRUCTOR_VIRTUAL_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS(ClassName) \
    ClassName() = default;                                                           \
    ClassName(ClassName const &other) = delete;                                      \
    ClassName &operator=(ClassName const &other) = delete;                           \
    ClassName(ClassName &&other) = delete;                                           \
    ClassName &operator=(ClassName &&other) = delete;                                \
    virtual ~ClassName() = default
#endif

// No default constructor and virtual destructor, delete the copy and move operators
#if !defined(NO_CONSTRUCTOR_DEFAULT_VIRTUAL_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS)
#define NO_CONSTRUCTOR_DEFAULT_VIRTUAL_DESTRUCTOR_DELETE_COPY_MOVE_OPERATORS(ClassName) \
    ClassName(ClassName const &other) = delete;                                         \
    ClassName &operator=(ClassName const &other) = delete;                              \
    ClassName(ClassName &&other) = delete;                                              \
    ClassName &operator=(ClassName &&other) = delete;                                   \
    virtual ~ClassName() = default
#endif

// Default destructor, delete constructor, copy and move operators
#if !defined(DEFAULT_DESTRUCTOR_DELETE_CONSTRUCTORS_COPY_MOVE_OPERATORS)
#define DEFAULT_DESTRUCTOR_DELETE_CONSTRUCTORS_COPY_MOVE_OPERATORS(ClassName) \
    ClassName() = delete;                                                     \
    ClassName(ClassName const &other) = delete;                               \
    ClassName &operator=(ClassName const &other) = delete;                    \
    ClassName(ClassName &&other) = delete;                                    \
    ClassName &operator=(ClassName &&other) = delete;                         \
    ~ClassName() = default
#endif

// Default virtual destructor, delete constructor, copy and move operators
#if !defined(DEFAULT_VIRTUAL_DESTRUCTOR_DELETE_CONSTRUCTORS_COPY_MOVE_OPERATORS)
#define DEFAULT_VIRTUAL_DESTRUCTOR_DELETE_CONSTRUCTORS_COPY_MOVE_OPERATORS(ClassName) \
    ClassName() = delete;                                                             \
    ClassName(ClassName const &other) = delete;                                       \
    ClassName &operator=(ClassName const &other) = delete;                            \
    ClassName(ClassName &&other) = delete;                                            \
    ClassName &operator=(ClassName &&other) = delete;                                 \
    virtual ~ClassName() = default
#endif
