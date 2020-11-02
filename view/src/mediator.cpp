// vim: awa:sts=4:ts=4:sw=4:et:cin:fdm=manual:tw=120:ft=cpp
#include "view/include/mediator.h"

void IMediator::LoadAllComponents() {
}

class Mediator : public IMediator {
public:
    virtual ~Mediator() = default;

    const Componenets &GetAllComponenets() const override { return mComponents; }
    void LoadAllComponents() override;

private:
    Componenets mComponents;
};
