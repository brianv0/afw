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

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "numpy/arrayobject.h"
#include "ndarray/pybind11.h"
#include "ndarray/converter.h"

#include "lsst/utils/pybind11.h"
#include "lsst/afw/table/Flag.h"
#include "lsst/afw/table/Field.h"
#include "lsst/afw/table/SchemaMapper.h"
#include "lsst/afw/table/BaseColumnView.h"
#include "lsst/afw/table/BaseRecord.h"
#include "lsst/afw/table/BaseTable.h"
#include "lsst/afw/table/BaseRecord.h"
#include "lsst/afw/table/pybind11/catalog.h"
#include "lsst/afw/table/pybind11/columnView.h"

namespace py = pybind11;

namespace lsst {
namespace afw {
namespace table {

namespace {

/**
Unlike most pybind11 wrapper classes, which have one .cc file per header file,
this module wraps both BaseRecord.h and BaseTable.h (as well as CatalogT<BaseRecord> from Catalog.h).

This allows us to define BaseCatalog.Table = clsBaseTable, which is needed to support `cast` in Python,
and makes wrapping Base catalogs more similar to all other types of catalog.
*/

using PyBaseRecord = py::class_<BaseRecord, std::shared_ptr<BaseRecord>>;
using PyBaseTable = py::class_<BaseTable, std::shared_ptr<BaseTable>>;
using PyBaseColumnView = py::class_<ColumnViewT<BaseRecord>,
                                    std::shared_ptr<ColumnViewT<BaseRecord>>,
                                    BaseColumnView>;
using BaseCatalog = CatalogT<BaseRecord>;
using PyBaseCatalog = py::class_<BaseCatalog, std::shared_ptr<BaseCatalog>>;

template <typename T>
void declareBaseRecordOverloads(PyBaseRecord & cls, std::string const & suffix) {
    typedef typename Field<T>::Value (BaseRecord::*Getter)(Key<T> const &) const;
    typedef void (BaseRecord::*Setter)(Key<T> const &, typename Field<T>::Value const &);
    cls.def(("get" + suffix).c_str(), (Getter)&BaseRecord::get);
    cls.def(("set" + suffix).c_str(), (Setter)&BaseRecord::set);
}

template <typename T>
void declareBaseRecordArrayOverloads(PyBaseRecord & cls, std::string const & suffix) {
    auto getter = [](BaseRecord & self, Key<Array<T>> const & key) -> ndarray::Array<T,1,1> {
        return self[key];
    };
    auto setter = [](BaseRecord & self, Key<Array<T>> const & key, py::object const & value) {
        if (key.getSize() == 0) {
            // Variable-length array field: do a shallow copy, which requires a non-const
            // contiguous array.
            self.set(key, py::cast<ndarray::Array<T,1,1>>(value));
        } else {
            // Fixed-length array field: do a deep copy, which can work with a const
            // noncontiguous array.  But we need to check the size first, since the
            // penalty for getting that wrong is assert->abort.
            auto v = py::cast<ndarray::Array<T const,1,0>>(value);
            ndarray::ArrayRef<T,1,1> ref = self[key];
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
    cls.def(("get" + suffix).c_str(), getter);
    cls.def(("set" + suffix).c_str(), setter);
}

/**
Declare member and static functions for a pybind11 wrapper of BaseRecord
*/
void declareBaseRecord(PyBaseRecord & cls) {
    table::pybind11::addCastFrom<BaseRecord>(cls);

    utils::addSharedPtrEquality<BaseRecord>(cls);

    cls.def("assign", (void (BaseRecord::*)(BaseRecord const &)) &BaseRecord::assign);
    cls.def("assign", (void (BaseRecord::*)(BaseRecord const &, SchemaMapper const &)) &BaseRecord::assign);
    cls.def("getSchema", &BaseRecord::getSchema);
    cls.def("getTable", &BaseRecord::getTable);
    cls.def_property_readonly("schema", &BaseRecord::getSchema);
    cls.def_property_readonly("table", &BaseRecord::getTable);

    declareBaseRecordOverloads<double>(cls, "D");
    declareBaseRecordOverloads<float>(cls, "F");
    declareBaseRecordOverloads<lsst::afw::table::Flag>(cls, "Flag");
    declareBaseRecordOverloads<std::uint16_t>(cls, "U");
    declareBaseRecordOverloads<std::int32_t>(cls, "I");
    declareBaseRecordOverloads<std::int64_t>(cls, "L");
    declareBaseRecordOverloads<std::string>(cls, "String");
    declareBaseRecordOverloads<lsst::afw::geom::Angle>(cls, "Angle");
    declareBaseRecordArrayOverloads<std::uint16_t>(cls, "ArrayU");
    declareBaseRecordArrayOverloads<int>(cls, "ArrayI");
    declareBaseRecordArrayOverloads<float>(cls, "ArrayF");
    declareBaseRecordArrayOverloads<double>(cls, "ArrayD");

    // These are master getters and setters that can take either strings, Keys, or
    // FunctorKeys, and dispatch to key.get.
    auto getter = [](py::object const & self, py::object key) -> py::object {
        py::object schema = self.attr("schema");
        if (py::isinstance<py::str>(key) || py::isinstance<py::bytes>(key)) {
            key = schema.attr("find")(key).attr("key");
        }
        return key.attr("get")(self);
    };
    auto setter = [](py::object const & self, py::object key, py::object const & value) -> void {
        py::object schema = self.attr("schema");
        if (py::isinstance<py::str>(key) || py::isinstance<py::bytes>(key)) {
            key = schema.attr("find")(key).attr("key");
        }
        key.attr("set")(self, value);
    };

    // The distinction between get/set and operator[] is meaningful in C++, because "record[k] = v"
    // operates by returning an object that can be assigned to.
    // But there's no meaningful difference between get/set and __getitem__/__setitem__.
    cls.def("get", getter);
    cls.def("__getitem__", getter);
    cls.def("set", setter);
    cls.def("__setitem__", setter);

}

/**
Declare member and static functions for a pybind11 wrapper of BaseTable
*/
void declareBaseTable(PyBaseTable & cls) {
    table::pybind11::addCastFrom<BaseTable>(cls);

    utils::addSharedPtrEquality<BaseTable>(cls);

    cls.def_static("make", &BaseTable::make);

    cls.def("getMetadata", &BaseTable::getMetadata);
    cls.def("setMetadata", &BaseTable::setMetadata, "metadata"_a);
    //cls.def("popMetadata", &BaseTable::popMetadata);
    cls.def("makeRecord", &BaseTable::makeRecord);
    cls.def("copyRecord",
            (std::shared_ptr<BaseRecord> (BaseTable::*)(BaseRecord const &)) &BaseTable::copyRecord);
    cls.def("copyRecord",
            (std::shared_ptr<BaseRecord> (BaseTable::*)(BaseRecord const &, SchemaMapper const &))
                &BaseTable::copyRecord);
    cls.def("getSchema", &BaseTable::getSchema);
    cls.def_property_readonly("schema", &BaseTable::getSchema);
    cls.def("getBufferSize", &BaseTable::getBufferSize);
    cls.def("clone", &BaseTable::clone);
    cls.def("preallocate", &BaseTable::preallocate);

    cls.def_static("_castFrom",
                   [](std::shared_ptr<BaseTable> base) {
                        return std::dynamic_pointer_cast<BaseTable>(base);
                   });
}

}  // namespace lsst::afw::table::<anonymous>

PYBIND11_PLUGIN(_base) {
    py::module mod("_base", "Python wrapper for afw _base library");
    
    if (_import_array() < 0) {
            PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
            return nullptr;
    };

    PyBaseRecord clsBaseRecord(mod, "BaseRecord");
    PyBaseTable clsBaseTable(mod, "BaseTable");
    PyBaseColumnView clsBaseColumnView(mod, "BaseColumnView");
    PyBaseCatalog clsBaseCatalog(mod, "BaseCatalog", py::dynamic_attr());

    declareBaseTable(clsBaseTable);
    declareBaseRecord(clsBaseRecord);
    table::pybind11::declareColumnView(clsBaseColumnView);
    table::pybind11::declareCatalog(clsBaseCatalog);

    clsBaseRecord.attr("Table") = clsBaseTable;
    clsBaseRecord.attr("ColumnView") = clsBaseColumnView;
    clsBaseRecord.attr("Catalog") = clsBaseCatalog;
    clsBaseTable.attr("Record") = clsBaseRecord;
    clsBaseTable.attr("ColumnView") = clsBaseColumnView;
    clsBaseTable.attr("Catalog") = clsBaseCatalog;
    clsBaseCatalog.attr("Record") = clsBaseRecord;
    clsBaseCatalog.attr("Table") = clsBaseTable;
    clsBaseCatalog.attr("ColumnView") = clsBaseColumnView;

    return mod.ptr();
}

}}}  // namespace lsst::afw::table
