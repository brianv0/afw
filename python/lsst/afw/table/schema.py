from __future__ import absolute_import, division, print_function

from builtins import str as futurestr
from past.builtins import basestring

import numpy as np
import astropy.units

from ..geom import Angle
from lsst.utils import monkeypatch

from ._schema import Key, Field, SchemaItem, Schema
from ._syntax import Schema_extract

__all__ = []  # import only for the side effects

# Map python types to C++ type identifiers
aliases = {
    str: "String",
    futurestr: "String",
    np.uint16: "U",
    np.int32: "I",
    np.int64: "L",
    np.float32: "F",
    np.float64: "D",
    Angle: "Angle",
}

for k, v in aliases.items():
    Key[k] = Key[v]
    Field[k] = Field[v]
    SchemaItem[k] = SchemaItem[v]
del k
del v

@monkeypatch
class Schema:

    def getOrderedNames(self):
        """Return a list of field names in the order the fields were added to the Schema.
        """
        names = []
        def func(item):
            names.append(item.field.getName())
        self.forEach(func)
        return names

    def __iter__(self):
        """Iterate over the items in the Schema.
        """
        items = []
        self.forEach(items.append)
        return iter(items)

    def checkUnits(self, parse_strict='raise'):
        """Check that all units in the Schema are valid Astropy unit strings.
        """
        def func(item):
            astropy.units.Unit(item.field.getUnits(), parse_strict=parse_strict)
        self.forEach(func)

    def addField(self, field, type=None, doc="", units="", size=None,
                 doReplace=False, parse_strict="raise"):
        """Add a field to the Schema.

        Parameters
        ----------
        field : str,Field
            The string name of the Field, or a fully-constructed Field object.
            If the latter, all other arguments besides doReplace are ignored.
        type\n : str,type
            The type of field to create.  Valid types are the keys of the
            afw.table.Field dictionary.
        doc : str
            Documentation for the field.
        unit : str
            Units for the field, or an empty string if unitless.
        size : int
            Size of the field; valid for string and array fields only.
        doReplace : bool
            If a field with this name already exists, replace it instead of
            raising pex.exceptions.InvalidParameterError.
        parse_strict : str
            One of 'raise' (default), 'warn', or 'strict', indicating how to
            handle unrecognized unit strings.  See also astropy.units.Unit.
        """
        if isinstance(field, basestring):
            field = Field[type](field, doc=doc, units=units, size=size, parse_strict=parse_strict)
        return field._addTo(self, doReplace)

    extract = Schema_extract
