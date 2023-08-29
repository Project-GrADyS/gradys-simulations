#include "Singleton.h"

namespace gradys_simulations {

Singleton *Singleton::singleton_ = nullptr;
;

Singleton* Singleton::GetInstance() {
    if (singleton_ == nullptr) {
        singleton_ = new Singleton();
    }
    singleton_->increase();
    return singleton_;
}

void Singleton::TryCloseInstance() {
    singleton_->decrease();
    if (singleton_->value() == 0 && singleton_ != nullptr) {
        free(singleton_->stream);
        py::finalize_interpreter();

    }
}
}
