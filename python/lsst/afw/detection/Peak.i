// -*- lsst-c++ -*-
/*
 * LSST Data Management System
 * Copyright 2008-2014 LSST Corporation.
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
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */

/*
 * Wrappers for PeakRecord, PeakTable, PeakCatalog, and PeakColumnView
 */

%import "lsst/afw/table/tableLib.i"

%{
#include "lsst/afw/detection/Peak.h"
%}

// =============== PeakTable and PeakRecord =============================================================

%shared_ptr(lsst::afw::detection::PeakTable)
%shared_ptr(lsst::afw::detection::PeakRecord)

%include "lsst/afw/detection/Peak.h"

// These %extend blocks are a workaround for HSC-1117: Swig doesn't recognize PeakTable/PeakRecord
// as subclasses of BaseTable/BaseRecord, and I'm completely mystified as to why.
%extend lsst::afw::detection::PeakRecord {
    lsst::afw::table::BaseRecord * asBaseRecord() {
        return self;
    }
    %pythoncode %{
    def __getattr__(self, name):
        return getattr(self.asBaseRecord(), name)
    %}
}
%extend lsst::afw::detection::PeakTable {
    lsst::afw::table::BaseTable * asBaseTable() {
        return self;
    }
    %pythoncode %{
    def __getattr__(self, name):
        return getattr(self.asBaseTable(), name)
    %}
}

%template(PeakColumnView) lsst::afw::table::ColumnViewT<lsst::afw::detection::PeakRecord>;

%addCastMethod(lsst::afw::detection::PeakTable, lsst::afw::table::BaseTable)
%addCastMethod(lsst::afw::detection::PeakRecord, lsst::afw::table::BaseRecord)

%addStreamRepr(lsst::afw::detection::PeakRecord);

// =============== Catalogs =================================================================================

namespace lsst { namespace afw { namespace table {

using afw::detection::PeakRecord;
using afw::detection::PeakTable;

%declareCatalog(CatalogT, Peak)

}}} // namespace lsst::afw::table


namespace lsst { namespace afw { namespace detection {

typedef lsst::afw::table::CatalogT<PeakRecord> PeakCatalog;

}}} // namespace lsst::afw::detection
