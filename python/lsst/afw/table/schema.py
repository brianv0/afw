from __future__ import absolute_import, division, print_function

from builtins import str as futurestr

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

    extract = Schema_extract
