#pragma once

#ifndef __gradys_simulations_SINGLETON_H_
#define __gradys_simulations_SINGLETON_H_

#include "pybind11/embed.h"

namespace py = pybind11;

namespace gradys_simulations {

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

}
#endif /* SINGLETON_H_ */
