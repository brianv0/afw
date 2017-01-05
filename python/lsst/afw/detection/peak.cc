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
//#include <pybind11/stl.h>

#include "lsst/afw/table/BaseRecord.h"
#include "lsst/afw/table/BaseTable.h"
#include "lsst/afw/detection/Peak.h"
#include "lsst/afw/table/pybind11/catalog.h"
#include "lsst/afw/table/pybind11/columnView.h"

namespace py = pybind11;
using namespace py::literals;

namespace lsst {
namespace afw {
namespace detection {

namespace {

using PyPeakRecord = py::class_<PeakRecord, std::shared_ptr<PeakRecord>, table::BaseRecord>;
using PyPeakTable = py::class_<PeakTable, std::shared_ptr<PeakTable>, table::BaseTable>;
using PyPeakColumnView = py::class_<table::ColumnViewT<PeakRecord>,
                                    std::shared_ptr<table::ColumnViewT<PeakRecord>>,
                                    table::BaseColumnView>;
using PyPeakCatalog = py::class_<table::CatalogT<PeakRecord>,
                                 std::shared_ptr<table::CatalogT<PeakRecord>>>;

/**
Declare constructors and member and static functions for a pybind11 PeakRecord
*/
void declarePeakRecord(PyPeakRecord & cls) {
    table::pybind11::addCastFrom<table::BaseRecord>(cls);

    cls.def("getTable", &PeakRecord::getTable);
    cls.def("getId", &PeakRecord::getId);
    cls.def("setId", &PeakRecord::setId);
    cls.def("getIx", &PeakRecord::getIx);
    cls.def("getIy", &PeakRecord::getIy);
    cls.def("setIx", &PeakRecord::setIx);
    cls.def("setIy", &PeakRecord::setIy);
    cls.def("getI", &PeakRecord::getI);
    cls.def("getCentroid", (afw::geom::Point2I (PeakRecord::*)(bool) const) &PeakRecord::getCentroid);
    cls.def("getCentroid", (afw::geom::Point2D (PeakRecord::*)() const) &PeakRecord::getCentroid);
    cls.def("getFx", &PeakRecord::getFx);
    cls.def("getFy", &PeakRecord::getFy);
    cls.def("setFx", &PeakRecord::setFx);
    cls.def("setFy", &PeakRecord::setFy);
    cls.def("getF", &PeakRecord::getF);
    cls.def("getPeakValue", &PeakRecord::getPeakValue);
    cls.def("setPeakValue", &PeakRecord::setPeakValue);
}

/**
Declare constructors and member and static functions for a pybind11 PeakTable
*/
void declarePeakTable(PyPeakTable & cls) {
    table::pybind11::addCastFrom<table::BaseTable>(cls);

    cls.def_static("make", &PeakTable::make, "schema"_a, "forceNew"_a=false);
    cls.def_static("makeMinimalSchema", &PeakTable::makeMinimalSchema);
    cls.def_static("checkSchema", &PeakTable::checkSchema, "schema"_a);
    cls.def("getIdFactory",
            (std::shared_ptr<table::IdFactory> (PeakTable::*)()) &PeakTable::getIdFactory);
    cls.def("setIdFactory", &PeakTable::setIdFactory, "factory"_a);
    cls.def_static("getIdKey", &PeakTable::getIdKey);
    cls.def_static("getIxKey", &PeakTable::getIxKey);
    cls.def_static("getIyKey", &PeakTable::getIyKey);
    cls.def_static("getFxKey", &PeakTable::getFxKey);
    cls.def_static("getFyKey", &PeakTable::getFyKey);
    cls.def_static("getPeakValueKey", &PeakTable::getPeakValueKey);
}

}  // lsst::afw::detection::<anonymous>

PYBIND11_PLUGIN(_peak) {
    py::module mod("_peak", "Python wrapper for afw _peak library");

    /* Module level */
    PyPeakRecord clsPeakRecord(mod, "PeakRecord");
    PyPeakTable clsPeakTable(mod, "PeakTable", py::metaclass());
    PyPeakColumnView clsPeakColumnView(mod, "PeakColumnView");
    PyPeakCatalog clsPeakCatalog(mod, "PeakCatalog", py::dynamic_attr());

    /* Members */
    declarePeakRecord(clsPeakRecord);
    declarePeakTable(clsPeakTable);
    table::pybind11::declareColumnView(clsPeakColumnView);
    table::pybind11::declareCatalog(clsPeakCatalog);

    clsPeakRecord.attr("Table") = clsPeakTable;
    clsPeakRecord.attr("ColumnView") = clsPeakColumnView;
    clsPeakRecord.attr("Catalog") = clsPeakCatalog;
    clsPeakTable.attr("Record") = clsPeakRecord;
    clsPeakTable.attr("ColumnView") = clsPeakColumnView;
    clsPeakTable.attr("Catalog") = clsPeakCatalog;
    clsPeakCatalog.attr("Record") = clsPeakRecord;
    clsPeakCatalog.attr("Table") = clsPeakTable;
    clsPeakCatalog.attr("ColumnView") = clsPeakColumnView;

    return mod.ptr();
}
}}} // lsst::afw::detection