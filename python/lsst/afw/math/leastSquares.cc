/* 
 * LSST Data Management System
 * Copyright 2008-2016  AURA/LSST.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <https://www.lsstcorp.org/LegalNotices/>.
 */

#include <pybind11/pybind11.h>
//#include <pybind11/operators.h>
//#include <pybind11/stl.h>

#include "numpy/arrayobject.h"
#include "ndarray/pybind11.h"
#include "ndarray/converter.h"
//#include "ndarray/eigen.h"

#include "lsst/afw/math/LeastSquares.h"

namespace py = pybind11;
using namespace pybind11::literals;

using namespace lsst::afw::math;

template <typename T1, typename T2, int C1, int C2>
void declareLeastSquares(py::module &mod){
    py::class_<LeastSquares> cls(mod, "LeastSquares", py::metaclass());
    py::enum_<LeastSquares::Factorization>(cls, "Factorization")
        .value("NORMAL_EIGENSYSTEM", LeastSquares::Factorization::NORMAL_EIGENSYSTEM)
        .value("NORMAL_CHOLESKY", LeastSquares::Factorization::NORMAL_CHOLESKY)
        .value("DIRECT_SVD", LeastSquares::Factorization::DIRECT_SVD)
        .export_values();
    cls.def_static("fromDesignMatrix",
                   (LeastSquares (*)(ndarray::Array<T1,2,C1> const &,
                                     ndarray::Array<T2,1,C2> const &,
                                     LeastSquares::Factorization))
                                         &LeastSquares::fromDesignMatrix<T1, T2, C1, C2>,
                   "design"_a, "data"_a,"factorization"_a=LeastSquares::NORMAL_EIGENSYSTEM);
    cls.def_static("fromNormalEquations",
                   (LeastSquares (*)(ndarray::Array<T1,2,C1> const &,
                                     ndarray::Array<T2,1,C2> const &,
                                     LeastSquares::Factorization))
                                         &LeastSquares::fromNormalEquations<T1, T2, C1, C2>,
                   "fisher"_a, "rhs"_a,"factorization"_a=LeastSquares::NORMAL_EIGENSYSTEM);
    cls.def("getRank", &LeastSquares::getRank);
    cls.def("setDesignMatrix",
            (void (LeastSquares::*)(ndarray::Array<T1,2,C1> const &,
                                    ndarray::Array<T2,1,C2> const &))
                                        &LeastSquares::setDesignMatrix<T1, T2, C1, C2>);
    cls.def("getDimension", &LeastSquares::getDimension);
    cls.def("setNormalEquations",
            (void (LeastSquares::*)(ndarray::Array<T1,2,C1> const &,
                                    ndarray::Array<T2,1,C2> const &))
                                        &LeastSquares::setNormalEquations<T1, T2, C1, C2>);
    cls.def("getSolution", &LeastSquares::getSolution);
    cls.def("getFisherMatrix", &LeastSquares::getFisherMatrix);
    cls.def("getCovariance", &LeastSquares::getCovariance);
    cls.def("getFactorization", &LeastSquares::getFactorization);
    cls.def("getDiagnostic", &LeastSquares::getDiagnostic);
    cls.def("getThreshold", &LeastSquares::getThreshold);
    cls.def("setThreshold", &LeastSquares::setThreshold);
};

PYBIND11_PLUGIN(_leastSquares) {
    py::module mod("_leastSquares", "Python wrapper for afw _leastSquares library");
    
    if (_import_array() < 0) {
            PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
            return nullptr;
    };
    
    declareLeastSquares<double,double,0,0>(mod);

    /* Module level */

    /* Member types and enums */

    /* Constructors */

    /* Operators */

    /* Members */

    return mod.ptr();
}