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
#include <pybind11/stl.h>
#include "numpy/arrayobject.h"
#include "ndarray/pybind11.h"
#include "ndarray/converter.h"


#include "lsst/afw/table/FieldBase.h"
#include "lsst/afw/table/Key.h"
#include "lsst/afw/table/BaseRecord.h"

namespace py = pybind11;

namespace lsst {
namespace afw {
namespace table {

namespace {

template <typename T>
using PyKey = py::class_<Key<T>, KeyBase<T>, FieldBase<T>>;

template <typename T>
void declareKeyAccessors(PyKey<T> & cls) {
    cls.def("get", [](Key<T> const & self, BaseRecord & record){ return record.get(self); });
    cls.def(
        "set",
        [](Key<T> const & self, BaseRecord & record, typename Key<T>::Value const & value) {
            record.set(self, value);
        }
    );
}

template <typename U>
void declareKeyAccessors(PyKey<Array<U>> & cls) {
    auto getter = [](Key<Array<U>> const & self, BaseRecord & record) -> ndarray::Array<U,1,1> {
        return record[self];
    };
    auto setter = [](Key<Array<U>> const & self, BaseRecord & record, py::object const & value) {
        if (self.getSize() == 0) {
            // Variable-length array field: do a shallow copy, which requires a non-const
            // contiguous array.
            record.set(self, py::cast<ndarray::Array<U,1,1>>(value));
        } else {
            // Fixed-length array field: do a deep copy, which can work with a const
            // noncontiguous array.  But we need to check the size first, since the
            // penalty for getting that wrong is assert->abort.
            auto v = py::cast<ndarray::Array<U const,1,0>>(value);
            ndarray::ArrayRef<U,1,1> ref = record[self];
            if (v.size() != ref.size()) {
                throw LSST_EXCEPT(
                    pex::exceptions::LengthError,
                    (boost::format("Array sizes do not agree: %s != %s") % v.size() % ref.size()).str()
                );
            }
            ref = v;
        }
        return;
    };
    cls.def("get", getter);
    cls.def("set", setter);
}

template <typename T>
PyKey<T> declareKey(py::module & mod, std::string const & suffix) {
    PyKey<T> clsKey(mod, ("Key_" + suffix).c_str());
    clsKey.def(py::init<>());
    clsKey.def(
        "__eq__",
        [](const Key<T> & self, Key<T> const & other)-> bool { return self == other; },
        py::is_operator()
    );
    clsKey.def(
        "__ne__",
        [](const Key<T> & self, Key<T> const & other)-> bool { return self != other; },
        py::is_operator()
    );
    clsKey.def("isValid", &Key<T>::isValid);
    clsKey.def("getOffset", &Key<T>::getOffset);
    declareKeyAccessors(clsKey);
    return clsKey;
};

} // anonymous

PYBIND11_PLUGIN(_key) {
    py::module mod("_key", "Python wrapper for afw _key library");

    if (_import_array() < 0) {
            PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
            return nullptr;
    };

    declareKey<std::uint16_t>(mod, "U");
    declareKey<std::int32_t>(mod, "I");
    declareKey<std::int64_t>(mod, "L");
    declareKey<float>(mod, "F");
    declareKey<double>(mod, "D");
    declareKey<std::string>(mod, "String");
    declareKey<lsst::afw::geom::Angle>(mod, "Angle");
    declareKey<lsst::afw::table::Array<std::uint16_t>>(mod, "ArrayU");
    declareKey<lsst::afw::table::Array<int>>(mod, "ArrayI");
    declareKey<lsst::afw::table::Array<float>>(mod, "ArrayF");
    declareKey<lsst::afw::table::Array<double>>(mod, "ArrayD");

    auto clsKeyFlag = declareKey<Flag>(mod, "Flag");
    clsKeyFlag.def("getBit", &Key<Flag>::getBit);

    return mod.ptr();
}

}}}  // namespace lsst::afw::table
