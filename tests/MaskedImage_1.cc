// -*- lsst-c++ -*-
#include "lsst/MaskedImage.h"

using namespace lsst;

template <typename ImagePixelT, typename MaskPixelT> class testPixProcFunc : public PixelProcessingFunc<ImagePixelT, MaskPixelT> {
public:
     typedef typename PixelChannelType<ImagePixelT>::type ImageChannelT;
     typedef typename PixelChannelType<MaskPixelT>::type MaskChannelT;
     
     testPixProcFunc(MaskedImage<ImagePixelT, MaskPixelT>& m) : PixelProcessingFunc<ImagePixelT, MaskPixelT>(m) {}
     
     void init() {
	  PixelProcessingFunc<ImagePixelT, MaskPixelT>::_maskPtr->getPlaneBitMask("CR", bitsCR);
     }
        
     bool operator ()(typename PixelProcessingFunc<ImagePixelT, MaskPixelT>::TupleT t) { 
	  return false;
     }
private:
    MaskChannelT bitsCR;
};


int main()
{
     typedef PixelGray<uint8> MaskPixelType;
     typedef PixelGray<float32> ImagePixelType;

     MaskedImage<ImagePixelType,MaskPixelType > testMaskedImage1(272, 1037);
     MaskedImage<ImagePixelType,MaskPixelType > testMaskedImage2(272, 1037);
     testMaskedImage2 += testMaskedImage1;

     testPixProcFunc<ImagePixelType, MaskPixelType> fooFunc(testMaskedImage1);   // should be a way to automatically convey template types
                                                                                 // from testMaskedImage1 to fooFunc
     fooFunc.init();
     testMaskedImage1.processPixels(fooFunc);
}
