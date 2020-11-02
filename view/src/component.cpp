#include "view/include/component.h"

#include <cassert>
#include <ios>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
#if 0
static constexpr bool Initialize() {
    // Make sure you register this component for use by the model
    return true;
}

static constexpr bool initialized = Initialize();

static void is_initialized() {
    cout << "component initialized: " << std::boolalpha << initialized << endl;
}

extern bool everything_initialized() {
    if (initialized) {
        cout << "component initialized: " << std::boolalpha << initialized << endl;
    }
}
#endif

//shared_ptr<Component> RegisterComponent(string_view string_view) {
//}

Component::Component(string_view /*name*/, string_view /*value*/) {
    //if (auto i = components_.find(name.to_string()); i != end(components_)) {
    //    return (*i).first;
    //} else {
    //}
}

//shared_ptr<Component> RegisterComponent(string_view str_view) {
//}
