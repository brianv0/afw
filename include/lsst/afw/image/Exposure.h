// -*- LSST-C++ -*- // fixed format comment for emacs
/**
  * @file
  *
  * @brief Declaration of the templated Exposure Class for LSST.
  *
  * Create an Exposure from a lsst::afw::image::MaskedImage.
  *
  * @ingroup afw
  *
  * @author Nicole M. Silvestri, University of Washington
  *
  * Contact: nms@astro.washington.edu
  *
  * Created on: Mon Apr 23 1:01:14 2007
  *
  * @version 
  *
  * LSST Legalese here...
  */

#ifndef LSST_AFW_IMAGE_EXPOSURE_H
#define LSST_AFW_IMAGE_EXPOSURE_H

#include "boost/cstdint.hpp"
#include "boost/shared_ptr.hpp"

#include "lsst/daf/base/Persistable.h"
#include "lsst/daf/data/LsstBase.h"
#include "lsst/afw/image/MaskedImage.h"
#include "lsst/afw/image/Wcs.h"
#include "lsst/afw/image/TanWcs.h"
#include "lsst/afw/cameraGeom/Detector.h"
#include "lsst/afw/image/Filter.h"

namespace lsst {
namespace afw {
    namespace formatters {
        template<typename ImageT, typename MaskT, typename VarianceT> class ExposureFormatter;
    }
namespace image {
    class Calib;

    /// A class to contain the data, WCS, and other information needed to describe an %image of the sky
    template<typename ImageT, typename MaskT=lsst::afw::image::MaskPixel,
             typename VarianceT=lsst::afw::image::VariancePixel>
    class Exposure : public lsst::daf::base::Persistable,
                     public lsst::daf::data::LsstBase {
    public:
        typedef MaskedImage<ImageT, MaskT, VarianceT> MaskedImageT;
        typedef boost::shared_ptr<Exposure> Ptr;
        
        // Class Constructors and Destructor
        explicit Exposure(int const cols=0, int const rows=0, Wcs const& wcs=NoWcs);
        explicit Exposure(MaskedImageT & maskedImage, Wcs const& wcs=NoWcs);
        explicit Exposure(std::string const &baseName, int const hdu=0, BBox const& bbox=BBox(), bool const conformMasks=false);

        Exposure(Exposure const &src, BBox const& bbox, bool const deep=false);

        /// generalised copy constructor; defined here in the header so that the compiler can instantiate
        /// N(N-1)/2 conversions between N ImageBase types.
        ///
        /// We only support converting the Image part
        template<typename OtherPixelT>
        Exposure(Exposure<OtherPixelT, MaskT, VarianceT> const& rhs, //!< Input Exposure
                 const bool deep        //!< Must be true; needed to disambiguate
                ) :
            lsst::daf::data::LsstBase(typeid(this)),
            _maskedImage(rhs.getMaskedImage(), deep),
            _wcs(rhs.getWcs()->clone()),
            _detector(rhs.getDetector()),
            _filter(rhs.getFilter()),
            _calib(new lsst::afw::image::Calib(*rhs.getCalib()))
        {
            setMetadata(rhs.getMetadata()->deepCopy());
        }
        
        virtual ~Exposure(); 

        // Get Members
        /// Return the MaskedImage
        MaskedImageT getMaskedImage() { return _maskedImage; };
        /// Return the MaskedImage
        MaskedImageT getMaskedImage() const { return _maskedImage; };
        Wcs::Ptr getWcs() const;
        /// Return the Exposure's Detector information
        lsst::afw::cameraGeom::Detector::Ptr getDetector() const { return _detector; }
        /// Return the Exposure's filter
        Filter getFilter() const { return _filter; }

        /// Return the Exposure's width
        int getWidth() const { return _maskedImage.getWidth(); }
        /// Return the Exposure's height
        int getHeight() const { return _maskedImage.getHeight(); }
        
        /**
         * Return the Exposure's row-origin
         *
         * \sa getXY0()
         */
        int getX0() const { return _maskedImage.getX0(); }
        /**
         * Return the Exposure's column-origin
         *
         * \sa getXY0()
         */
        int getY0() const { return _maskedImage.getY0(); }

        /**
         * Return the Exposure's origin
         *
         * This will usually be (0, 0) except for images created using the
         * <tt>Exposure(fileName, hdu, BBox, mode)</tt> ctor or <tt>Exposure(Exposure, BBox)</tt> cctor
         * The origin can be reset with \c setXY0
         */
        PointI getXY0() const { return _maskedImage.getXY0(); }

        /**
         * Set the Exposure's origin (including correcting the Wcs)
         *
         * The origin is usually set by the constructor, so you shouldn't need this function
         *
         * \note There are use cases (e.g. memory overlays) that may want to set these values, but
         * don't do so unless you are an Expert.
         */
        void setXY0(int const x0, int const y0) {
            PointI const old = _maskedImage.getXY0();
            _maskedImage.setXY0(x0, y0);
            _wcs->shiftReferencePixel(x0 - old[0], y0 - old[1]);
        }
        /**
         * Set the Exposure's origin
         *
         * The origin is usually set by the constructor, so you shouldn't need this function
         *
         * \note There are use cases (e.g. memory overlays) that may want to set these values, but
         * don't do so unless you are an Expert.
         */
        void setXY0(PointI const origin) {
            setXY0(origin[0], origin[1]);
        }
        // Set Members
        void setMaskedImage(MaskedImageT &maskedImage);
        void setWcs(Wcs const& wcs);
        /// Set the Exposure's Detector information
        void setDetector(lsst::afw::cameraGeom::Detector::Ptr detector) { _detector = detector; }
        /// Set the Exposure's filter
        void setFilter(Filter const& filter) { _filter = filter; }
        /// Return the Exposure's Calib object
        boost::shared_ptr<Calib> getCalib() { return _calib; }
        boost::shared_ptr<const Calib> getCalib() const { return _calib; }
        
        // Has Member (inline)
        bool hasWcs() const { return (*_wcs ? true : false); 
        }
        
        // FITS
        void writeFits(std::string const &expOutFile) const;
        
    private:
        LSST_PERSIST_FORMATTER(lsst::afw::formatters::ExposureFormatter<ImageT, MaskT, VarianceT>)

        MaskedImageT _maskedImage;             
        Wcs::Ptr _wcs;
        cameraGeom::Detector::Ptr _detector;
        Filter _filter;
        boost::shared_ptr<Calib> _calib;
    };

/**
 * A function to return an Exposure of the correct type (cf. std::make_pair)
 */
    template<typename MaskedImageT>
    Exposure<typename MaskedImageT::Image::Pixel>* makeExposure(MaskedImageT & mimage, ///< the Exposure's image
                                                                Wcs const& wcs=NoWcs ///< the Exposure's WCS
                                                               ) {
        return new Exposure<typename MaskedImageT::Image::Pixel>(mimage, wcs);
    }

}}} // lsst::afw::image

#endif // LSST_AFW_IMAGE_EXPOSURE_H
