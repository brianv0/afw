from __future__ import absolute_import, division, print_function

from past.builtins import basestring

from lsst.utils import monkeypatch

from ._schema import Field
from ._schemaMapper import SchemaMapper
from ._syntax import Schema_extract

__all__ = []  # import only for the side effects

@monkeypatch
class SchemaMapper:

    def addOutputField(self, field, type=None, doc=None, units="", size=None,
                 doReplace=False, parse_strict="raise"):
        """Add an un-mapped field to the output Schema.

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
        return field._addTo(self.editOutputSchema(), doReplace)

    extract = Schema_extract
