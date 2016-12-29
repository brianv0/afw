// -*- lsst-c++ -*-

#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/tuple/to_seq.hpp"

#include "lsst/afw/table/KeyBase.h"
#include "lsst/afw/table/detail/Access.h"
#include "lsst/afw/table/Flag.h"
#include "lsst/afw/table/BaseRecord.h"

namespace lsst { namespace afw { namespace table {

Key<FieldBase<Flag>::Element> KeyBase<Flag>::getStorage() const {
    return detail::Access::extractElement(*this, 0);
}

bool const KeyBase<Flag>::HAS_NAMED_SUBFIELDS;

template <typename T>
bool const KeyBase<T>::HAS_NAMED_SUBFIELDS;

template <typename U>
bool const KeyBase<Array<U>>::HAS_NAMED_SUBFIELDS;

template <typename U>
std::vector<U> KeyBase< Array<U> >::extractVector(BaseRecord const & record) const {
    Key< Array<U> > const * self = static_cast<Key< Array<U> > const *>(this);
    std::vector<U> result(self->getSize());
    typename Key< Array<U> >::ConstReference array = record[*self];
    std::copy(array.begin(), array.end(), result.begin());
    return result;
}

template <typename U>
void KeyBase< Array<U> >::assignVector(BaseRecord & record, std::vector<U> const & values) const {
    Key< Array<U> > const * self = static_cast<Key< Array<U> > const *>(this);
    std::copy(values.begin(), values.end(), record[*self].begin());
}

template <typename U>
Key<U> KeyBase< Array<U> >::operator[](int i) const {
    Key< Array<U> > const * self = static_cast<Key< Array<U> > const *>(this);
    if (self->isVariableLength()) {
        throw LSST_EXCEPT(
            lsst::pex::exceptions::LogicError,
            "Cannot get Keys to elements of variable-length arrays."
        );
    }
    if (i >= self->getSize() || i < 0) {
        throw LSST_EXCEPT(
            lsst::pex::exceptions::LengthError,
            "Array key index out of range."
        );
    }
    return detail::Access::extractElement(*this, i);
}

template <typename U>
Key< Array<U> > KeyBase< Array<U> >::slice(int begin, int end) const {
    Key< Array<U> > const * self = static_cast<Key< Array<U> > const *>(this);
    if (self->isVariableLength()) {
        throw LSST_EXCEPT(
            lsst::pex::exceptions::LogicError,
            "Cannot get Keys to slices of variable-length arrays."
        );
    }
    if (begin > self->getSize() || begin < 0) {
        throw LSST_EXCEPT(
            lsst::pex::exceptions::LengthError,
            "Array key begin index out of range."
        );
    }
    if (end > self->getSize() || end < 0) {
        throw LSST_EXCEPT(
            lsst::pex::exceptions::LengthError,
            "Array key end index out of range."
        );
    }
    return detail::Access::extractRange(*this, begin, end);
}

//----- Explicit instantiation ------------------------------------------------------------------------------

#define INSTANTIATE_KEY(r, data, elem)            \
    template class KeyBase< elem >;

BOOST_PP_SEQ_FOR_EACH(
    INSTANTIATE_KEY, _,
    BOOST_PP_TUPLE_TO_SEQ(AFW_TABLE_FIELD_TYPE_N, AFW_TABLE_FIELD_TYPE_TUPLE)
)

}}} // namespace lsst::afw::table
