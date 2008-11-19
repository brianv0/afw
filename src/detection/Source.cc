// -*- lsst-c++ -*-
//
//##====----------------                                ----------------====##/
//!
//! \file
//! \brief Support for Source%s
//!
//##====----------------                                ----------------====##/

#include "lsst/daf/base.h"
#include "lsst/afw/detection/Source.h"

namespace lsst{
namespace afw{
namespace detection{


Source::Source() : 
    _sourceId(0),
    _ampExposureId(0),
    _filterId(0),
    _objectId(0),
    _movingObjectId(0),
    _procHistoryId(0),
    _ra(0.0),
    _decl(0.0),
    _raErr4detection(0.0),
    _decErr4detection(0.0),
    _raErr4wcs(0.0),
    _decErr4wcs(0.0),
    _xFlux(0.0),
    _xFluxErr(0.0),
    _yFlux(0.0),
    _yFluxErr(0.0),
    _raFlux(0.0),
    _raFluxErr(0.0),
    _decFlux(0.0),
    _decFluxErr(0.0),
    _xPeak(0.0),
    _yPeak(0.0),
    _raPeak(0.0),
    _decPeak(0.0),
    _xAstrom(0.0),
    _xAstromErr(0.0),
    _yAstrom(0.0),
    _yAstromErr(0.0),
    _raAstrom(0.0),
    _raAstromErr(0.0),
    _decAstrom(0.0),
    _decAstromErr(0.0),
    _taiMidPoint(0.0),
    _taiRange(0.0),
    _fwhmA(0.0),
    _fwhmB(0.0),
    _fwhmTheta(0.0),
    _psfMag(0.0),
    _psfMagErr(0.0),
    _apMag(0.0),
    _apMagErr(0.0),
    _modelMag(0.0),
    _modelMagErr(0.0),
    _petroMag(0.0),
    _petroMagErr(0.0),
    _apDia(0.0),        
    _snr(0.0),
    _chi2(0.0),
    _sky(0.0),
    _skyErr(0.0),
    _flag4association(0),
    _flag4detection(0),
    _flag4wcs(0),
{
    _nulls.set();
}


bool Source::operator==(Source const & d) const {
    if (this == &d)  {
        return true;
    }
    if (_sourceId         == d._sourceId         &&
        _filterId         == d._filterId         &&
        _procHistoryId    == d._procHistoryId    &&
        _ra               == d._ra               &&
        _dec              == d._dec              &&
        _raErr4Wcs        == d._raErr4Wcs        &&
        _decErr4Wcs       == d._decErr4Wcs       &&
        _taiMidPoint      == d._taiMidPoint      &&
        _fwhmA            == d._fwhmA            &&
        _fwhmB            == d._fwhmB            &&
        _fwhmTheta        == d._fwhmTheta        &&
        _psfMag           == d._psfMag           &&
        _psfMagErr        == d._psfMagErr        &&
        _apMag            == d._apMag            &&
        _apMagErr         == d._apMagErr         &&
        _modelMag         == d._modelMag         &&
        _modelMagErr      == d._modelMagErr      &&
        _snr              == d._snr              &&
        _chi2             == d._chi2)
    {
        if (_nulls == d._nulls) {
           
            return (isNull(AMP_EXPOSURE_ID)    || _ampExposureId    == d._ampExposureId   ) &&
                   (isNull(OBJECT_ID)          || _objectId         == d._objectId        ) &&
                   (isNull(MOVING_OBJECT_ID)   || _movingObjectId   == d._movingObjectId  ) &&
                   (isNull(RA_ERR_4_DETECTION) || _raErr4Detection  == d._raErr4Detection ) &&
                   (isNull(DEC_ERR_4_DETECTION)|| _decErr4Detection == d._decErr4Detection) &&
                   (isNull(X_FLUX)             || _xFlux            == d._xFlux           ) &&
                   (isNull(X_FLUX_ERR)         || _xFluxErr         == d._xFluxErr        ) &&                   
                   (isNull(Y_FLUX)             || _yFlux            == d._yFlux           ) &&
                   (isNull(Y_FLUX_ERR)         || _yFluxErr         == d._yFluxErr        ) &&
                   (isNull(X_PEAK)             || _xPeak            == d._xPeak           ) && 
                   (isNull(Y_PEAK)             || _yPeak            == d._yPeak           ) && 
                   (isNull(RA_PEAK)            || _raPeak           == d._raPeak          ) && 
                   (isNull(DEC_PEAK)           || _decPeak          == d._decPeak         ) &&   
                   (isNull(X_ASTROM)           || _xAstrom          == d._xAstrom         ) &&
                   (isNull(X_ASTROM_ERR)       || _xAstromErr       == d._xAstromErr      ) &&                   
                   (isNull(Y_ASTROM)           || _yAstrom          == d._yAstrom         ) &&
                   (isNull(Y_ASTROM_ERR)       || _yAstromErr       == d._yAstromErr      ) &&                                   
                   (isNull(RA_ASTROM)          || _raAstrom         == d._raAstrom        ) &&
                   (isNull(RA_ASTROM_ERR)      || _raAstromErr      == d._raAstromErr     ) &&                   
                   (isNull(DEC_ASTROM)         || _decAstrom        == d._decAstrom       ) &&
                   (isNull(DEC_ASTROM_ERR)     || _decAstromErr     == d._decAstromErr    ) &&                                   
                   (isNull(TAI_RANGE)          || _taiRange         == d._taiRange        ) &&
                   (isNull(PETRO_MAG)          || _petroMag         == d._petroMag        ) &&                   
                   (isNull(PETRO_MAG_ERR)      || _petroMagErr      == d._petroMagErr     ) &&
                   (isNull(AP_DIA)             || _apDia            == d._apDia           ) &&
                   (isNull(SKY)                || _sky              == d._sky             ) &&                   
                   (isNull(SKY_ERR)            || _skyErr           == d._skyErr          ) &&
                   (isNull(FLAG_4_ASSOCIATION) || _flag4association == d._flag4association) &&
                   (isNull(FLAG_4_DETECTION)   || _flag4detection   == d._flag4detection  ) &&
                   (isNull(FLAG_4_WCS)         || _flag4wcs         == d._flag4wcs        );
        }
    }
    return false;
}


// -- SourceVector ----------------
SourceVector::SourceVector()            : daf::base::Citizen(typeid(*this)), _vec()  {}
SourceVector::SourceVector(size_type n) : daf::base::Citizen(typeid(*this)), _vec(n) {}

SourceVector::SourceVector(size_type n, value_type const & val) :
    daf::base::Citizen(typeid(*this)),
    _vec(n, val)
{}


SourceVector::~SourceVector() {}


SourceVector::SourceVector(SourceVector const & v) :
    daf::base::Citizen(typeid(*this)),
    _vec(v._vec)
{}


SourceVector::SourceVector(Vector const & v) :
    daf::base::Citizen(typeid(*this)),
    _vec(v)
{}


SourceVector & SourceVector::operator=(SourceVector const & v) {
    if (this != &v) {
        _vec = v._vec;
    }
    return *this;
}


SourceVector & SourceVector::operator=(Vector const & v) {
    _vec = v;
    return *this;
}

} // namespace detection
} // namespace afw
} // namespace lsst
