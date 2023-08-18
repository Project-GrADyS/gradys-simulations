#include "pybind11/embed.h"

namespace py = pybind11;

class Singleton
{

protected:
    Singleton()
    {
        py::initialize_interpreter();
    }

    static Singleton* singleton_;

    int counter;

public:
    Singleton(Singleton &other) = delete;

    void operator=(const Singleton &) = delete;

    void increase() {counter++;};
    int decrease() {return counter--;};
    int value() {return counter;};

    static Singleton *GetInstance();
    static void TryCloseInstance();
};

Singleton* Singleton::singleton_= nullptr;;

Singleton *Singleton::GetInstance()
{
    if(singleton_==nullptr){
        singleton_ = new Singleton();
    }
    singleton_->increase();
    return singleton_;
}

void Singleton::TryCloseInstance()
{
    singleton_->decrease();
    if(singleton_->value()==0 && singleton_!=nullptr){
        py::finalize_interpreter();
    }
}
