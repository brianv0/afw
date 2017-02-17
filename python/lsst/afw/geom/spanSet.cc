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

#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "numpy/arrayobject.h"
#include "ndarray/pybind11.h"
#include "ndarray/converter.h"
#include "lsst/afw/geom/ellipses/Quadrupole.h"

#include "lsst/afw/geom/SpanSet.h"


namespace py = pybind11;
using namespace pybind11::literals;

namespace lsst { namespace afw { namespace geom {

namespace {
    template <typename Pixel, typename PyClass>
    void declareFlattenMethod(PyClass & cls) {
        cls.def("flatten", (ndarray::Array<Pixel, 1, 1> (SpanSet::*)(ndarray::Array<Pixel, 2, 0> const &,
                                                                     Point2I const &) const)
                                                                     &SpanSet::flatten<Pixel, 2, 0>,
                                                                     py::arg("input"),
                                                                     py::arg("xy0") = Point2I());
        cls.def("flatten", (ndarray::Array<Pixel, 2, 2> (SpanSet::*)(ndarray::Array<Pixel, 3, 0> const &,
                                                                     Point2I const &) const)
                                                                     &SpanSet::flatten<Pixel, 3, 0>,
                                                                     py::arg("input"),
                                                                     py::arg("xy0") = Point2I());
        cls.def("flatten", (void (SpanSet::*)(ndarray::Array<Pixel, 1, 0> const &,
                                              ndarray::Array<Pixel, 2, 0> const &,
                                              Point2I const &) const) &SpanSet::flatten<Pixel, Pixel, 2, 0, 0>,
                                              py::arg("output"),
                                              py::arg("input"),
                                              py::arg("xy0") = Point2I());
        cls.def("flatten", (void (SpanSet::*)(ndarray::Array<Pixel, 2, 0> const &,
                                              ndarray::Array<Pixel, 3, 0> const &,
                                              Point2I const &) const) &SpanSet::flatten<Pixel, Pixel, 3, 0, 0>,
                                              py::arg("output"),
                                              py::arg("input"),
                                              py::arg("xy0") = Point2I());
    }

    template <typename Pixel, typename PyClass>
    void declareUnflattenMethod(PyClass & cls) {
        cls.def("unflatten",
                (ndarray::Array<Pixel, 2, 2> (SpanSet::*)(ndarray::Array<Pixel, 1, 0> const & input) const)
                &SpanSet::unflatten<Pixel, 1, 0>);
        cls.def("unflatten",
                (ndarray::Array<Pixel, 3, 3> (SpanSet::*)(ndarray::Array<Pixel, 2, 0> const & input) const)
                &SpanSet::unflatten<Pixel, 2, 0>);
        cls.def("unflatten", (void (SpanSet::*)(ndarray::Array<Pixel, 2, 0> const & ,
                                                ndarray::Array<Pixel, 1, 0> const & ,
                                                Point2I const &) const) &SpanSet::unflatten<Pixel, Pixel, 1, 0, 0>,
                                                py::arg("output"),
                                                py::arg("input"),
                                                py::arg("xy0") = Point2I());
        cls.def("unflatten", (void (SpanSet::*)(ndarray::Array<Pixel, 3, 0> const & ,
                                                ndarray::Array<Pixel, 2, 0> const & ,
                                                Point2I const &) const) &SpanSet::unflatten<Pixel, Pixel, 2, 0, 0>,
                                                py::arg("output"),
                                                py::arg("input"),
                                                py::arg("xy0") = Point2I());
    }

    template <typename Pixel, typename PyClass>
    void declareSetMaskMethod(PyClass & cls) {
        cls.def("setMask", (void (SpanSet::*)(lsst::afw::image::Mask<Pixel> &, Pixel) const)
                           &SpanSet::setMask<Pixel>);
    }

    template <typename Pixel, typename PyClass>
    void declareClearMaskMethod(PyClass & cls) {
        cls.def("clearMask",
                (void (SpanSet::*)(lsst::afw::image::Mask<Pixel> &, Pixel) const) &SpanSet::clearMask<Pixel>);
    }

    template <typename Pixel, typename PyClass>
    void declareIntersectMethod(PyClass & cls) {
        cls.def("intersect", (std::shared_ptr<SpanSet> (SpanSet::*)(lsst::afw::image::Mask<Pixel> const &,
                                                                    Pixel const &) const)
                                                                    &SpanSet::intersect<Pixel>);
        // Default to compare any bit set
        cls.def("intersect", []
                             (SpanSet const & self, lsst::afw::image::Mask<Pixel> const & mask)
                             {
                                 auto tempSpanSet = geom::maskToSpanSet(mask);
                                 return self.intersect(*tempSpanSet);
                             });
    }

    template <typename Pixel, typename PyClass>
    void declareIntersectNotMethod(PyClass & cls) {
        cls.def("intersectNot", (std::shared_ptr<SpanSet> (SpanSet::*)(lsst::afw::image::Mask<Pixel> const &,
                                                                       Pixel const &) const)
                                                                       &SpanSet::intersectNot<Pixel>);
        // Default to compare any bit set
        cls.def("intersectNot", []
                                (SpanSet const & self, lsst::afw::image::Mask<Pixel> const & mask)
                                {
                                    auto tempSpanSet = geom::maskToSpanSet(mask);
                                    return self.intersectNot(*tempSpanSet);
                                });
    }

    template <typename Pixel, typename PyClass>
    void declareUnionMethod(PyClass & cls) {
        cls.def("union", (std::shared_ptr<SpanSet> (SpanSet::*)(lsst::afw::image::Mask<Pixel> const &,
                                                                Pixel const &) const)
                                                                &SpanSet::union_<Pixel>);
        // Default to compare any bit set
        cls.def("union", []
                         (SpanSet const & self, lsst::afw::image::Mask<Pixel> const & mask)
                         {
                             auto tempSpanSet = geom::maskToSpanSet(mask);
                             return self.union_(*tempSpanSet);
                         });
    }

    template <typename ImageT, typename PyClass>
    void declareCopyImage(PyClass & cls) {
        cls.def("copyImage", &SpanSet::copyImage<ImageT>);
    }

    template <typename ImageT, typename PyClass>
    void declareCopyMaskedImage(PyClass & cls) {
        using MaskPixel = lsst::afw::image::MaskPixel;
        using VariancePixel = lsst::afw::image::VariancePixel;
        cls.def("copyMaskedImage", &SpanSet::copyMaskedImage<ImageT, MaskPixel, VariancePixel>);
    }

    template <typename ImageT, typename PyClass>
    void declareSetImage(PyClass & cls) {
        cls.def("setImage", &SpanSet::setImage<ImageT>, "image"_a, "val"_a,
                "region"_a = lsst::afw::geom::Box2I(), "doClip"_a=false);
    }

    template <typename Pixel>
    void declareMaskToSpanSetFunction(py::module & mod) {
        mod.def("maskToSpanSet",
                []
                (lsst::afw::image::Mask<Pixel> mask)
                {
                    return maskToSpanSet(mask);
                });
        mod.def("maskToSpanSet",
                []
                (lsst::afw::image::Mask<Pixel> mask, Pixel const & bitmask)
                {
                    auto functor = [&bitmask](Pixel const & pixval){ return (pixval & bitmask) == bitmask; };
                    return maskToSpanSet(mask, functor);
                });
    }

    template <typename Pixel, typename PyClass>
    void declareMaskMethods(PyClass & cls) {
        declareSetMaskMethod<Pixel>(cls);
        declareClearMaskMethod<Pixel>(cls);
        declareIntersectMethod<Pixel>(cls);
        declareIntersectNotMethod<Pixel>(cls);
        declareUnionMethod<Pixel>(cls);
    }

    template <typename Pixel, typename PyClass>
    void declareImageTypes(PyClass & cls) {
        declareFlattenMethod<Pixel>(cls);
        declareUnflattenMethod<Pixel>(cls);
        declareCopyImage<Pixel>(cls);
        declareCopyMaskedImage<Pixel>(cls);
        declareSetImage<Pixel>(cls);
    }

} // end anonymous namespace

PYBIND11_PLUGIN(_spanSet) {
    using MaskPixel = lsst::afw::image::MaskPixel;
    py::module mod("_spanSet", "Python wrapper for afw _spanSet library");

    if(_import_array() < 0) {
        PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
        return nullptr;
    }

    py::enum_<Stencil>(mod, "Stencil")
        .value("CIRCLE", Stencil::CIRCLE)
        .value("BOX", Stencil::BOX)
        .value("MANHATTAN", Stencil::MANHATTAN);

    py::class_<SpanSet, std::shared_ptr<SpanSet>> clsSpanSet(mod, "SpanSet");

    /* SpanSet Constructors */
    clsSpanSet.def(py::init<>());
    clsSpanSet.def(py::init<Box2I>());
    clsSpanSet.def(py::init<std::vector<Span>, bool>(), py::arg("spans"), py::arg("normalize") = true);

    /* SpanSet Methods */
    clsSpanSet.def("getArea", &SpanSet::getArea);
    clsSpanSet.def("getBBox", &SpanSet::getBBox);
    clsSpanSet.def("isContiguous", &SpanSet::isContiguous);
    clsSpanSet.def("shiftedBy", (std::shared_ptr<SpanSet> (SpanSet::*)(int, int) const) &SpanSet::shiftedBy);
    clsSpanSet.def("shiftedBy",
                   (std::shared_ptr<SpanSet> (SpanSet::*)(Extent2I const &) const) &SpanSet::shiftedBy);
    clsSpanSet.def("clippedTo", &SpanSet::clippedTo);
    clsSpanSet.def("transformedBy",
                   (std::shared_ptr<SpanSet> (SpanSet::*)(LinearTransform const &) const)
                   &SpanSet::transformedBy);
    clsSpanSet.def("transformedBy",
                   (std::shared_ptr<SpanSet> (SpanSet::*)(AffineTransform const &) const)
                   &SpanSet::transformedBy);
    clsSpanSet.def("transformedBy",
                   (std::shared_ptr<SpanSet> (SpanSet::*)(XYTransform const &) const)
                   &SpanSet::transformedBy);
    clsSpanSet.def("overlaps", &SpanSet::overlaps);
    clsSpanSet.def("contains", (bool (SpanSet::*)(SpanSet const &) const) &SpanSet::contains);
    clsSpanSet.def("contains", (bool (SpanSet::*)(Point2I const &) const) &SpanSet::contains);
    clsSpanSet.def("computeCentroid", &SpanSet::computeCentroid);
    clsSpanSet.def("computeShape", &SpanSet::computeShape);
    clsSpanSet.def("dilate", (std::shared_ptr<SpanSet> (SpanSet::*)(int, Stencil) const) &SpanSet::dilate,
                   py::arg("r"), py::arg("s") = Stencil::CIRCLE);
    clsSpanSet.def("dilate", (std::shared_ptr<SpanSet> (SpanSet::*)(SpanSet const &) const) &SpanSet::dilate);
    clsSpanSet.def("erode", (std::shared_ptr<SpanSet> (SpanSet::*)(int, Stencil) const) &SpanSet::erode,
                   py::arg("r"), py::arg("s") = Stencil::CIRCLE);
    clsSpanSet.def("erode", (std::shared_ptr<SpanSet> (SpanSet::*)(SpanSet const &) const) &SpanSet::erode);
    clsSpanSet.def("intersect",
                   (std::shared_ptr<SpanSet> (SpanSet::*)(SpanSet const &) const) &SpanSet::intersect);
    clsSpanSet.def("intersectNot",
                    (std::shared_ptr<SpanSet> (SpanSet::*)(SpanSet const &) const) &SpanSet::intersectNot);
    clsSpanSet.def("union", (std::shared_ptr<SpanSet> (SpanSet::*)(SpanSet const &) const) &SpanSet::union_);
    clsSpanSet.def_static("spanSetFromShape",
                          (std::shared_ptr<SpanSet> (*)(int, Stencil)) &SpanSet::spanSetFromShape,
                           "radius"_a, "stencil"_a=Stencil::CIRCLE);
    clsSpanSet.def_static("spanSetFromShape",
                          (std::shared_ptr<SpanSet> (*)(geom::ellipses::Ellipse const &)) &SpanSet::spanSetFromShape);
    clsSpanSet.def("split", &SpanSet::split);
    clsSpanSet.def("findEdgePixels", &SpanSet::findEdgePixels);

    /* SpanSet Operators */
    clsSpanSet.def("__eq__",
                   [](SpanSet const & self, SpanSet const & other) -> bool {return self == other;},
                   py::is_operator());
    clsSpanSet.def("__ne__",
                   [](SpanSet const & self, SpanSet const & other) -> bool {return self != other;},
                   py::is_operator());
    clsSpanSet.def("__iter__",
                   [](SpanSet & self){return py::make_iterator(self.begin(), self.end());},
                   py::keep_alive<0, 1>());
    clsSpanSet.def("__len__",
                   [](SpanSet const & self) -> decltype(self.size()) { return self.size(); });
    clsSpanSet.def("__contains__",
                   [](SpanSet & self, SpanSet const & other)->bool {return self.contains(other); });
    clsSpanSet.def("__contains__",
                   [](SpanSet & self, Point2I & other)->bool {return self.contains(other); });
    clsSpanSet.def("__repr__",
                   []
                   (SpanSet const & self)-> std::string
                   {
                       std::ostringstream os;
                       lsst::afw::image::Mask<MaskPixel> tempMask(self.getBBox());
                       self.setMask(tempMask, static_cast<MaskPixel>(1));
                       auto array = tempMask.getArray();
                       auto dims = array.getShape();
                       for (std::size_t i = 0; i < dims[0]; ++i){
                           os << "[";
                           for (std::size_t j = 0; j< dims[1]; ++j) {
                               os << array[i][j];
                               if (j != dims[1] - 1){
                                   os << ", ";
                               }
                           }
                           os << "]" << std::endl;
                       }
                       return os.str();
                   });
    clsSpanSet.def("__str__",
                   []
                   (SpanSet const & self)->std::string
                   {
                       std::ostringstream os;
                       for (auto const & span : self) {
                           os << span.getY() << ": " << span.getMinX()
                              << ".." << span.getMaxX() << std::endl;
                       }
                       return os.str();
                   });
    // Instantiate all the templates

    declareMaskMethods<MaskPixel>(clsSpanSet);

    declareImageTypes<uint16_t>(clsSpanSet);
    declareImageTypes<uint64_t>(clsSpanSet);
    declareImageTypes<int>(clsSpanSet);
    declareImageTypes<float>(clsSpanSet);
    declareImageTypes<double>(clsSpanSet);

    // Extra instantiation for flatten unflatten methods
    declareFlattenMethod<long>(clsSpanSet);
    declareUnflattenMethod<long>(clsSpanSet);

    /* Free Functions */
    declareMaskToSpanSetFunction<MaskPixel>(mod);

    return mod.ptr();
}

}}} // end lsst::afw::geom
