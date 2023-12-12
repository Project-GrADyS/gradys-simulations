#pragma once

#ifndef __gradys_simulations_SINGLETON_H_
#define __gradys_simulations_SINGLETON_H_

#include "pybind11/embed.h"
#include <pybind11/iostream.h>

namespace py = pybind11;

namespace gradys_simulations {

class Singleton {

protected:
    Singleton() {
        py::initialize_interpreter();

        stream = new py::scoped_ostream_redirect(std::cout,
                py::module_::import("sys").attr("stdout"));

    }

    static Singleton *singleton_;

    py::scoped_ostream_redirect *stream;

    int counter;

public:
    Singleton(Singleton &other) = delete;

    void operator=(const Singleton&) = delete;

    void increase() {
        counter++;
    }
    ;
    int decrease() {
        return counter--;
    }
    ;
    int value() {
        return counter;
    }
    ;

    static Singleton* GetInstance();
    static void TryCloseInstance();
};

}
#endif /* SINGLETON_H_ */
