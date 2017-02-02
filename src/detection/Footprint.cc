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


#include "lsst/afw/detection/Footprint.h"
#include "lsst/afw/table/io/CatalogVector.h"
#include "lsst/afw/table/io/OutputArchive.h"

namespace lsst { namespace afw { namespace detection {

Footprint::Footprint(std::shared_ptr<geom::SpanSet> inputSpans,
          geom::Box2I const & region): lsst::daf::base::Citizen(typeid(this)),
                                       _spans(inputSpans),
                                       _region(region),
                                       _peaks(PeakTable::makeMinimalSchema()) {}

Footprint::Footprint(std::shared_ptr<geom::SpanSet> inputSpans,
          afw::table::Schema const & peakSchema,
          geom::Box2I const & region): lsst::daf::base::Citizen(typeid(this)),
                                       _spans(inputSpans),
                                       _peaks(peakSchema),
                                       _region(region) {}

void Footprint::setSpans(std::shared_ptr<geom::SpanSet> otherSpanSet) {
    _spans = otherSpanSet;
}

std::shared_ptr<PeakRecord> Footprint::addPeak(float fx, float fy, float height) {
    std::shared_ptr<PeakRecord> p = getPeaks().addNew();
    p->setIx(fx);
    p->setIy(fy);
    p->setFx(fx);
    p->setFy(fy);
    p->setPeakValue(height);
    return p;
}

void Footprint::sortPeaks(afw::table::Key<float> const & key) {
    auto validatedKey = key.isValid() ? key : PeakTable::getPeakValueKey();
    getPeaks().sort([& validatedKey]
                    (detection::PeakRecord const & a, detection::PeakRecord const & b)
                    {return a.get(validatedKey) > b.get(validatedKey);}
                   );
}

void Footprint::shift(int dx, int dy) {
    setSpans(getSpans()->shiftedBy(dx, dy));
    for (auto & peak : getPeaks()) {
        peak.setIx(peak.getIx() + dx);
        peak.setIy(peak.getIy() + dy);
        peak.setFx(peak.getFx() + dx);
        peak.setFy(peak.getFy() + dy);
    }
}

void Footprint::clipTo(geom::Box2I const & box) {
    setSpans(getSpans()->clippedTo(box));
    removeOrphanPeaks();
}

bool Footprint::contains(geom::Point2I const & pix) const {
    return getSpans()->contains(pix);
}

std::unique_ptr<Footprint> Footprint::transform(std::shared_ptr<image::Wcs> source,
                                     std::shared_ptr<image::Wcs> target,
                                     geom::Box2I const & region,
                                     bool doClip) const {
    // Build a transform from the two wcs objects
    image::XYTransformFromWcsPair transform(target, source);
    // Transfrom the SpanSet first
    auto transformedSpan = getSpans()->transformedBy(transform);
    // Use this new SpanSet and the peakSchema to create a new Footprint
    std::unique_ptr<Footprint> newFootprint(new Footprint(transformedSpan, getPeaks().getSchema(), region));
    // now populate the new Footprint with transformed Peaks
    for (auto const & peak : getPeaks()) {
        // Transform the x y Point
        auto newPoint = transform.forwardTransform(geom::Point2D(peak.getFx(), peak.getFx()));
        newFootprint->addPeak(newPoint.getX(), newPoint.getY(), peak.getPeakValue());
    }
    if (doClip) {
        newFootprint->clipTo(region);
    }
    return newFootprint;
}

void Footprint::dilate(int r, geom::Stencil s) {
    setSpans(getSpans()->dilate(r, s));
}

void Footprint::dilate(geom::SpanSet const & other) {
    setSpans(getSpans()->dilate(other));
}

void Footprint::erode(int r, geom::Stencil s) {
    setSpans(getSpans()->erode(r, s));
    removeOrphanPeaks();
}

void Footprint::erode(geom::SpanSet const & other) {
    setSpans(getSpans()->erode(other));
    removeOrphanPeaks();
}

void Footprint::removeOrphanPeaks() {
    for (auto iter = getPeaks().begin(); iter != getPeaks().end(); ++iter) {
        if (!getSpans()->contains(geom::Point2I(iter->getIx(), iter->getIy()))) {
            iter = getPeaks().erase(iter);
            --iter;
        }
    }
}

std::vector<std::unique_ptr<Footprint>> Footprint::split() const {
    auto splitSpanSets = getSpans()->split();
    std::vector<std::unique_ptr<Footprint>> footprintList;
    footprintList.reserve(splitSpanSets.size());
    for (auto & spanPtr : splitSpanSets) {
        std::unique_ptr<Footprint> tmpFootprintPointer(new Footprint(spanPtr,
                                                                     getPeaks().getSchema(),
                                                                     getRegion()));
        tmpFootprintPointer->_peaks = getPeaks();
        // No need to remove any peaks, as there is only one Footprint, so it will
        // simply be a copy of the original
        if (splitSpanSets.size() > 1) {
            tmpFootprintPointer->removeOrphanPeaks();
        }
        footprintList.push_back(std::move(tmpFootprintPointer));
    }
    return footprintList;
}

bool Footprint::operator==(Footprint const & other) const {
    /* If the peakCatalogs are not the same length the Footprints can't be equal */
    if (getPeaks().size() != other.getPeaks().size()) {
        return false;
    }
    /* Check that for every peak in the PeakCatalog there is a corresponding peak
     * in the other, and if not return false
     */
    for (auto const & selfPeak : getPeaks()) {
        bool match = false;
        for (auto const & otherPeak : other.getPeaks()) {
            if (selfPeak.getI() == otherPeak.getI() &&
                selfPeak.getF() == otherPeak.getF() &&
                selfPeak.getPeakValue() == otherPeak.getPeakValue()) {
                match = true;
                break;
            }
        }
        if (!match) {
            return false;
        }
    }
    /* At this point the PeakCatalogs have evaluated true, compare the SpanSets
     */
    return *(getSpans()) == *(other.getSpans());
}

namespace {
std::string getFootprintPersistenceName() { return "Footprint"; }

class LegacyFootprintPersistenceHelper {
public:
    table::Schema spanSchema;
    table::Key<int> spanY;
    table::Key<int> spanX0;
    table::Key<int> spanX1;

    static LegacyFootprintPersistenceHelper const & get() {
        static LegacyFootprintPersistenceHelper instance;
        return instance;
    }

    // No copying
    LegacyFootprintPersistenceHelper (const LegacyFootprintPersistenceHelper&) = delete;
    LegacyFootprintPersistenceHelper& operator=(const LegacyFootprintPersistenceHelper&) = delete;

    // No moving
    LegacyFootprintPersistenceHelper (LegacyFootprintPersistenceHelper&&) = delete;
    LegacyFootprintPersistenceHelper& operator=(LegacyFootprintPersistenceHelper&&) = delete;

private:
        LegacyFootprintPersistenceHelper() :
        spanSchema(),
        spanY(spanSchema.addField<int>("y", "The row of the span", "pixel")),
        spanX0(spanSchema.addField<int>("x0", "First column of span (inclusive)", "pixel")),
        spanX1(spanSchema.addField<int>("x1", "Second column of span (inclusive)", "pixel"))
        {spanSchema.getCitizen().markPersistent();}
};

std::pair<afw::table::Schema&, table::Key<int>&> spanSetPersistenceHelper() {
    static afw::table::Schema spanSetIdSchema;
    static int initialize = true;
    static table::Key<int> idKey;
    if (initialize) {
        idKey = spanSetIdSchema.addField<int>("id", "id of the SpanSet catalog");
        initialize = false;
        spanSetIdSchema.getCitizen().markPersistent();
    }
    std::pair<afw::table::Schema&, table::Key<int>&> returnPair(spanSetIdSchema, idKey);
    return returnPair;
}
} // end anonymous namespace

class FootprintFactory : public table::io::PersistableFactory {
public:
    virtual std::shared_ptr<afw::table::io::Persistable>
    read(afw::table::io::InputArchive const & archive,
         afw::table::io::CatalogVector const & catalogs) const override {
        // Verify there are two catalogs
        LSST_ARCHIVE_ASSERT(catalogs.size() == 2u);
        std::shared_ptr<Footprint> loadedFootprint = detection::Footprint::readSpanSet(catalogs.front(),
                                                                                       archive);
        // Now read in the PeakCatalog records
        detection::Footprint::readPeaks(catalogs.back(), *loadedFootprint);
        return loadedFootprint;
    }

    explicit FootprintFactory(std::string const & name) : afw::table::io::PersistableFactory(name) {}

};

namespace{
// Insert the factory into the registry (instantiating an instance is sufficient, because the
// the code that does the work is in the base class ctor)
FootprintFactory registration(getFootprintPersistenceName());
} // end anonymous namespace

std::string Footprint::getPersistenceName() const { return getFootprintPersistenceName(); }

void Footprint::write(afw::table::io::OutputArchiveHandle & handle) const {
    // get the span schema and key
    auto const keys = spanSetPersistenceHelper();
    // create the output catalog
    afw::table::BaseCatalog spanSetCat = handle.makeCatalog(keys.first);
    // create a record that will hold the ID of the recursively saved SpanSet
    auto record = spanSetCat.addNew();
    record->set(keys.second, handle.put(getSpans()));
    handle.saveCatalog(spanSetCat);
    // save the peaks into a catalog
    afw::table::BaseCatalog peakCat = handle.makeCatalog(getPeaks().getSchema());
    peakCat.insert(peakCat.end(), getPeaks().begin(), getPeaks().end(), true);
    handle.saveCatalog(peakCat);
}

std::unique_ptr<Footprint> Footprint::readSpanSet(afw::table::BaseCatalog const & catalog,
                                                  afw::table::io::InputArchive const & archive) {
    int fieldCount = catalog.getSchema().getFieldCount();
    LSST_ARCHIVE_ASSERT(fieldCount == 1 || fieldCount == 3);
    std::shared_ptr<geom::SpanSet> loadedSpanSet;
    if (fieldCount == 1) {
        // This is a new style footprint with a SpanSet as a member, treat accordingly
        auto const schemaAndKey = spanSetPersistenceHelper();
        int persistedSpanSetId = catalog.front().get(schemaAndKey.second);
        loadedSpanSet = std::dynamic_pointer_cast<geom::SpanSet>(archive.get(persistedSpanSetId));
    } else {
        // This block is for an old style footprint load.
        auto const & keys = LegacyFootprintPersistenceHelper::get();
        std::vector<geom::Span> tempVec;
        tempVec.reserve(catalog.size());
        for (auto const & val : catalog) {
            tempVec.push_back(geom::Span(val.get(keys.spanY), val.get(keys.spanX0), val.get(keys.spanX1)));
        }
        loadedSpanSet = std::make_shared<geom::SpanSet>(std::move(tempVec));
    }
    auto loadedFootprint = std::unique_ptr<Footprint>(new Footprint(loadedSpanSet));
    return loadedFootprint;
}

void Footprint::readPeaks(afw::table::BaseCatalog const & peakCat, Footprint & loadedFootprint) {
    if (!peakCat.getSchema().contains(PeakTable::makeMinimalSchema())) {
        // need to handle an older form of Peak persistence for backwards compatibility
        afw::table::SchemaMapper mapper(peakCat.getSchema());
        mapper.addMinimalSchema(PeakTable::makeMinimalSchema());
        afw::table::Key<float> oldX = peakCat.getSchema()["x"];
        afw::table::Key<float> oldY = peakCat.getSchema()["y"];
        afw::table::Key<float> oldPeakValue = peakCat.getSchema()["value"];
        mapper.addMapping(oldX, "f.x");
        mapper.addMapping(oldY, "f.y");
        mapper.addMapping(oldPeakValue, "peakValue");
        loadedFootprint.setPeakSchema(mapper.getOutputSchema());
        auto peaks = loadedFootprint.getPeaks();
        peaks.reserve(peakCat.size());
        for (auto const & peak : peakCat) {
            auto newPeak = peaks.addNew();
            newPeak->assign(peak, mapper);
            newPeak->setIx(static_cast<int>(newPeak->getFx()));
            newPeak->setIy(static_cast<int>(newPeak->getFy()));
        }
        return;
    }
    loadedFootprint.setPeakSchema(peakCat.getSchema());
    auto & peaks = loadedFootprint.getPeaks();
    peaks.reserve(peakCat.size());
    for (auto const & peak : peakCat) {
        peaks.addNew()->assign(peak);
    }
}

}}} // End lsst::afw::detection namespace
