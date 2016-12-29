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
#include <pybind11/functional.h>

#include <sstream>

#include "numpy/arrayobject.h"
#include "ndarray/pybind11.h"
#include "ndarray/converter.h"

#include "lsst/afw/fits.h"
#include "lsst/afw/table/Schema.h"
#include "lsst/afw/table/BaseRecord.h"

namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst {
namespace afw {
namespace table {

namespace {

using PySchema = py::class_<Schema>;

using PySubSchema = py::class_<SubSchema>;

template <typename T>
using PyFieldBase = py::class_<FieldBase<T>>;

template <typename T>
using PyKeyBase = py::class_<KeyBase<T>>;

template <typename T>
using PyField = py::class_<Field<T>,FieldBase<T>>;

template <typename T>
using PyKey = py::class_<Key<T>, KeyBase<T>, FieldBase<T>>;

template <typename T>
using PySchemaItem = py::class_<SchemaItem<T>>;


// Specializations for FieldBase

template <typename T>
void specialize(PyFieldBase<T> & cls) {
    cls.def(py::init<>());
}

template <typename T>
void specialize(PyFieldBase<Array<T>> & cls) {
    cls.def(py::init<int>(), "size"_a=0);
    cls.def("getSize", &FieldBase<Array<T>>::getSize);
    cls.def("isVariableLength", &FieldBase<Array<T>>::isVariableLength);
}

void specialize(PyFieldBase<std::string> & cls) {
    cls.def(py::init<int>(), "size"_a=-1);
    cls.def("getSize", &FieldBase<std::string>::getSize);
}


// Specializations for KeyBase

template <typename T>
void specialize(PyKeyBase<T> &) {}

template <typename T>
void specialize(PyKeyBase<Array<T>> & cls) {
    cls.def("__getitem__", &KeyBase<Array<T>>::operator[]);
    cls.def("__getitem__", &KeyBase<Array<T>>::slice);
}


// Specializations for Key

template <typename T>
void addKeyAccessors(PyKey<T> & cls) {
    cls.def("get", [](Key<T> const & self, BaseRecord & record){ return record.get(self); });
    cls.def(
        "set",
        [](Key<T> const & self, BaseRecord & record, typename Key<T>::Value const & value) {
            record.set(self, value);
        }
    );
}

template <typename U>
void addKeyAccessors(PyKey<Array<U>> & cls) {
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
void specialize(PyKey<T> & cls) {
    addKeyAccessors(cls);
}

void specialize(PyKey<Flag> & cls) {
    addKeyAccessors(cls);
    cls.def("getBit", &Key<Flag>::getBit);
}


// Wrap all helper classes for a Schema field type.
template <typename T>
void wrapSchemaType(py::module & mod) {
    std::string suffix = FieldBase<T>::getTypeString();
    py::str pySuffix(suffix);

    py::object astropyUnit = py::module::import("astropy.units").attr("Unit");

    // FieldBase
    PyFieldBase<T> clsFieldBase(mod, ("FieldBase" + suffix).c_str());
    clsFieldBase.def_static("getTypeString", &FieldBase<T>::getTypeString);
    specialize(clsFieldBase);

    // KeyBase
    PyKeyBase<T> clsKeyBase(mod, ("KeyBase" + suffix).c_str());
    clsKeyBase.def_readonly_static("HAS_NAMED_SUBFIELDS", &KeyBase<T>::HAS_NAMED_SUBFIELDS);
    specialize(clsKeyBase);

    // Field
    PyField<T> clsField(mod, ("Field" + suffix).c_str());
    mod.attr("Field")[pySuffix] = clsField;
    clsField.def(
        "__init__",
        [astropyUnit]( // capture by value to refcount in Python instead of dangle in C++
            Field<T> & self,
            std::string const & name,
            std::string const & doc,
            py::str const & units,
            py::object const & size,
            py::str const & parse_strict
        ) {
            astropyUnit(units, "parse_strict"_a=parse_strict);
            std::string u = py::cast<std::string>(units);
            if (size == py::none()) {
                new (&self) Field<T>(name, doc, u);
            } else {
                int s = py::cast<int>(size);
                new (&self) Field<T>(name, doc, u, s);
            }
        },
        "name"_a, "doc"_a, "units"_a="", "size"_a=py::none(), "parse_strict"_a="raise"
    );
    clsField.def(
        "_addTo",
        [](Field<T> const & self, Schema & schema, bool doReplace) -> Key<T> {
            return schema.addField(self, doReplace);
        }
    );
    clsField.def("getName", &Field<T>::getName);
    clsField.def("getDoc", &Field<T>::getDoc);
    clsField.def("getUnits", &Field<T>::getUnits);
    clsField.def("copyRenamed", &Field<T>::copyRenamed);
    clsField.def(
        "__repr__",
        [](Field<T> const & self) -> std::string {
            std::ostringstream os;
            os << self;
            return os.str();
        }
    );

    // Key
    PyKey<T> clsKey(mod, ("Key_" + suffix).c_str());
    mod.attr("Key")[pySuffix] = clsKey;
    clsKey.def(py::init<>());
    clsKey.def(
        "__eq__",
        [](Key<T> const & self, Key<T> const & other)-> bool { return self == other; },
        py::is_operator()
    );
    clsKey.def(
        "__ne__",
        [](Key<T> const & self, Key<T> const & other)-> bool { return self != other; },
        py::is_operator()
    );
    clsKey.def("isValid", &Key<T>::isValid);
    clsKey.def("getOffset", &Key<T>::getOffset);
    clsKey.def(
        "__repr__",
        [](Key<T> const & self) -> std::string {
            std::ostringstream os;
            os << self;
            return os.str();
        }
    );
    clsKey.def(
        "_findIn",
        [](Key<T> const & self, Schema const & schema) {
            return schema.find(self);
        }
    );
    specialize(clsKey);

    // SchemaItem
    PySchemaItem<T> clsSchemaItem(mod, ("SchemaItem"+suffix).c_str());
    mod.attr("SchemaItem")[pySuffix] = clsSchemaItem;
    clsSchemaItem.def_readonly("key", &SchemaItem<T>::key);
    clsSchemaItem.def_readonly("field", &SchemaItem<T>::field);
    clsSchemaItem.def(
        "__getitem__",
        [](py::object const & self, int index) -> py::object {
            if (index == 0) {
                return self.attr("key");
            } else if (index == 1) {
                return self.attr("field");
            }
            throw LSST_EXCEPT(
                pex::exceptions::LengthError,
                "SchemaItem index must be 0 or 1."
            );
        }
    );
    clsSchemaItem.def(
        "__len__",
        [](py::object const & self) -> int {
            return 2;
        }
    );
    clsSchemaItem.def(
        "__repr__",
        [](py::object const & self) -> py::str {
            return py::str("SchemaItem(key={0.key}, field={0.field})").format(self);
        }
    );
}

// Helper class for Schema::find(name, func) that converts the result to Python.
// In C++14, this should be converted to a universal lambda.
struct MakePythonSchemaItem {

    template <typename T>
    void operator()(SchemaItem<T> const & item) {
        result = py::cast(item);
    }

    py::object result;
};




} // anonymous

PYBIND11_PLUGIN(_schema) {
    py::module mod("_schema", "Python wrapper for afw _schema library");

    if (_import_array() < 0) {
            PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
            return nullptr;
    };

    /* Module level */
    py::dict fieldDict;
    mod.attr("Field") = fieldDict;
    mod.attr("Key") = py::dict();
    mod.attr("SchemaItem") = py::dict();
    PySchema clsSchema(mod, "Schema");
    PySubSchema clsSubSchema(mod, "SubSchema");

    wrapSchemaType<std::uint16_t>(mod);
    wrapSchemaType<std::int32_t>(mod);
    wrapSchemaType<std::int64_t>(mod);
    wrapSchemaType<float>(mod);
    wrapSchemaType<double>(mod);
    wrapSchemaType<std::string>(mod);
    wrapSchemaType<geom::Angle>(mod);
    wrapSchemaType<Array<std::uint16_t>>(mod);
    wrapSchemaType<Array<int>>(mod);
    wrapSchemaType<Array<float>>(mod);
    wrapSchemaType<Array<double>>(mod);
    wrapSchemaType<Flag>(mod);

    /* Member types and enums */
    py::enum_<Schema::ComparisonFlags>(clsSchema, "ComparisonFlags")
        .value("EQUAL_KEYS", Schema::ComparisonFlags::EQUAL_KEYS)
        .value("EQUAL_NAMES", Schema::ComparisonFlags::EQUAL_NAMES)
        .value("EQUAL_DOCS", Schema::ComparisonFlags::EQUAL_DOCS)
        .value("EQUAL_UNITS", Schema::ComparisonFlags::EQUAL_UNITS)
        .value("EQUAL_FIELDS", Schema::ComparisonFlags::EQUAL_FIELDS)
        .value("EQUAL_ALIASES", Schema::ComparisonFlags::EQUAL_ALIASES)
        .value("IDENTICAL", Schema::ComparisonFlags::IDENTICAL)
        .export_values()
        .def("__invert__", [](Schema::ComparisonFlags a) -> std::uint8_t { return ~a; }, py::is_operator())
        .def("__and__", [](Schema::ComparisonFlags a, std::uint8_t b) -> std::uint8_t { return a & b; },
             py::is_operator())
        .def("__and__",
             [](Schema::ComparisonFlags a, Schema::ComparisonFlags b) -> std::uint8_t { return a & b; },
             py::is_operator())
        .def("__rand__", [](Schema::ComparisonFlags a, std::uint8_t b) -> std::uint8_t { return b & a; },
             py::is_operator())
        ;

    /* Constructors */
    clsSchema.def(py::init<>());
    clsSchema.def(py::init<Schema const &>());

    /* Operators */
    clsSchema.def("__getitem__", [](Schema & self, std::string const & name) { return self[name]; });
    clsSchema.def("__eq__",
                 [](Schema const & self, Schema const & other) { return self == other; },
                 py::is_operator());
    clsSchema.def("__ne__",
                 [](Schema const & self, Schema const & other) { return self != other; },
                 py::is_operator());

    /* Members */
    clsSchema.def("getRecordSize", &Schema::getRecordSize);
    clsSchema.def("getFieldCount", &Schema::getFieldCount);
    clsSchema.def("getFlagFieldCount", &Schema::getFlagFieldCount);
    clsSchema.def("getNonFlagFieldCount", &Schema::getNonFlagFieldCount);
    // Instead of wrapping all the templated overrides of Schema::addField,
    // and forcing pybind11 to do (slow) override resolution, we use the visitor
    // pattern to invert the call: Schema calls Field._addTo (in Python, using
    // the pybind11 C++ Python API).  We combine this with the argument parsing
    // logic to construct a new Field if string arguments are passed instead.
    clsSchema.def(
        "addField",
        [fieldDict](  // capture by value (will refcount) to avoid dangling
            py::object const & self,
            py::object field,
            py::object const & type,
            py::object const & doc,
            py::object const & unit,
            py::object const & size,
            py::object const & doReplace,
            py::object const & parse_strict
        ) -> py::object {
            if (py::isinstance<py::str>(field) || py::isinstance<py::bytes>(field)) {
                field = fieldDict[type](field, doc, unit, size, parse_strict);
            }
            return field.attr("_addTo")(self, doReplace);
        },
        "field"_a, "type"_a=py::none(), "doc"_a="", "units"_a="", "size"_a=py::none(),
        "doReplace"_a=false, "parse_strict"_a="raise",

        "Add a field to the Schema.\n"
        "\n"
        "Parameters\n"
        "----------\n"
        "field : str,Field\n"
        "    The string name of the Field, or a fully-constructed Field object.\n"
        "    If the latter, all other arguments besides doReplace are ignored.\n"
        "type\n : str,type\n"
        "    The type of field to create.  Valid types are the keys of the\n"
        "    afw.table.Field dictionary.\n"
        "doc : str\n"
        "    Documentation for the field.\n"
        "unit : str\n"
        "    Units for the field, or an empty string if unitless.\n"
        "size : int\n"
        "    Size of the field; valid for string and array fields only.\n"
        "doReplace : bool\n"
        "    If a field with this name already exists, replace it instead of\n"
        "    raising pex.exceptions.InvalidParameterError.\n"
        "parse_strict : str\n"
        "    One of 'raise' (default), 'warn', or 'strict', indicating how to\n"
        "    handle unrecognized unit strings.  See also astropy.units.Unit\n."
    );
    clsSchema.def(
        "find",
        [](py::object const & self, py::object const & key) -> py::object {
            if (py::isinstance<py::str>(key) || py::isinstance<py::bytes>(key)) {
                Schema const & s = py::cast<Schema const &>(self);
                std::string name = py::cast<std::string>(key);
                MakePythonSchemaItem func;
                s.find(name, func);
                return func.result;
            }
            return key.attr("_findIn")(self);
        }
    );
    clsSchema.def("getNames", &Schema::getNames, "topOnly"_a=false);
    clsSchema.def("getAliasMap", &Schema::getAliasMap);
    clsSchema.def("setAliasMap", &Schema::setAliasMap, "aliases"_a);
    clsSchema.def("disconnectAliases", &Schema::disconnectAliases);
    clsSchema.def("forEach", [](Schema & self, py::object & obj) { self.forEach(obj); });
    clsSchema.def("compare", &Schema::compare, "other"_a, "flags"_a=Schema::ComparisonFlags::EQUAL_KEYS);
    clsSchema.def("contains", (int (Schema::*)(Schema const &, int) const) &Schema::contains,
                  "other"_a, "flags"_a=Schema::ComparisonFlags::EQUAL_KEYS);
    clsSchema.def(
        "__contains__",
        [](py::object const & self, py::object const & key) {
            try {
                self.attr("find")(key);
            } catch (py::error_already_set & err) {
                return false;
            }
            return true;
        }
    );
    clsSchema.def_static("readFits",
                         (Schema (*)(std::string const &, int)) &Schema::readFits,
                         "filename"_a, "hdu"_a=0);
    clsSchema.def_static("readFits",
                         (Schema (*)(fits::MemFileManager &, int)) &Schema::readFits,
                         "manager"_a, "hdu"_a=0);

    clsSchema.def("join",
                  (std::string (Schema::*)(std::string const &, std::string const &) const) &Schema::join,
                  "a"_a, "b"_a);
    clsSchema.def("join",
                  (std::string (Schema::*)(std::string const &,
                                           std::string const &,
                                           std::string const &) const) &Schema::join,
                  "a"_a, "b"_a, "c"_a);
    clsSchema.def("join",
                  (std::string (Schema::*)(std::string const &,
                                           std::string const &,
                                           std::string const &,
                                           std::string const &) const) &Schema::join,
                  "a"_a, "b"_a, "c"_a, "d"_a);
    clsSchema.def("__repr__", [](Schema const & self) {
        std::ostringstream os;
        os << self;
        return os.str();
    });

    clsSubSchema.def("getNames", &SubSchema::getNames, "topOnly"_a=false);
    clsSubSchema.def("getPrefix", &SubSchema::getPrefix);
    clsSubSchema.def(
        "asKey",
        [](SubSchema const & self) -> py::object {
            MakePythonSchemaItem func;
            self.apply(func);
            return func.result.attr("key");
        }
    );
    clsSubSchema.def(
        "asField",
        [](SubSchema const & self) -> py::object {
            MakePythonSchemaItem func;
            self.apply(func);
            return func.result.attr("field");
        }
    );
    clsSubSchema.def(
        "find",
        [](SubSchema const & self, std::string const & name) -> py::object {
            MakePythonSchemaItem func;
            self.find(name, func);
            return func.result;
        }
    );
    clsSubSchema.def("__getitem__", [](SubSchema & self, std::string const & name) { return self[name]; });

    return mod.ptr();
}

}}}  // namespace lsst::afw::table
