#
# LSST Data Management System
# Copyright 2016 LSST Corporation.
#
# This product includes software developed by the
# LSST Project (http://www.lsst.org/).
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the LSST License Statement and
# the GNU General Public License along with this program.  If not,
# see <http://www.lsstcorp.org/LegalNotices/>.
#
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from lsst.daf.base import DateTime
from lsst.afw.coord import Coord, IcrsCoord, Observatory, Weather
from lsst.afw.geom import Angle
from .imageLib import VisitInfo, RotType_UNKNOWN

__all__ = ["makeVisitInfo"]

nanFloat = float("nan")
nanAngle = Angle(nanFloat)


def makeVisitInfo(
    exposureId=0,
    exposureTime=nanFloat,
    darkTime=nanFloat,
    date=DateTime(),
    ut1=nanFloat,
    era=nanAngle,
    boresightRaDec=IcrsCoord(nanAngle, nanAngle),
    boresightAzAlt=Coord(nanAngle, nanAngle),
    boresightAirmass=nanFloat,
    boresightRotAngle=nanAngle,
    rotType=RotType_UNKNOWN,
    observatory=Observatory(nanAngle, nanAngle, nanFloat),
    weather=Weather(nanFloat, nanFloat, nanFloat),
):
    """Make a VisitInfo from keyword arguments

    This function will be replaced by a VisitInfo constructor once we switch to pybind11
    (it is too much hassle with SWIG).

    @param[in] exposureId  exposure ID (int, defaults to 0)
    @param[in] exposureTime  exposure duration (shutter open time); (float, sec, defaults to NaN)
    @param[in] darkTime  time from CCD flush to readout, including shutter open time (despite the name);
                    (float, sec, defaults to NaN)
    @param[in] date  TAI (international atomic time) MJD date at middle of exposure
                    (lsst.daf.base.DateTime; defaults to date of unix epoch)
    @param[in] ut1  UT1 (universal time) MJD date at middle of exposure (float, defaults to Nan)
    @param[in] era  earth rotation angle at middle of exposure
                    (lsst.afw.geom.Angle, defaults to Angle(Nan))
    @param[in] boresightRaDec  ICRS RA/Dec of boresight at middle of exposure
                    (lsst.afw.coord.IcrsCoord; defaults to IcrsCoord(Nan, Nan));
                    other Coord types are accepted and converted to Icrs
    @param[in] boresightAzAlt  refracted apparent topocentric Az/Alt of boresight at middle of exposure;
                    (lsst.afw.coord.Coord; defaults to Coord(Nan, Nan))
    @param[in] boresightAirmass  airmass at the boresight, relative to zenith at sea level
                    (float, defaults to Nan)
    @param[in] boresightRotAngle  rotation angle at boresight at middle of exposure;
                    see getBoresightRotAngle for details
                    (lsst.afw.geom.Angle, defaults to Angle(Nan))
    @param[in] rotType  rotation type; one of the lsst.afw.image.RotType_ constants,
                    defaults to RotType_UNKNOWN
    @param[in] observatory  observatory longitude, latitude and altitude,
                    (lsst.afw.coord.Observatory, defaults to Observatory(Angle(Nan), Angle(Nan), Nan))
    @param[in] weather  basic weather information for computing air mass,
                    (lsst.afw.coord.Weather, defaults to Weather(NaN, NaN, NaN))
    """
    return VisitInfo(
        exposureId,
        exposureTime,
        darkTime,
        date,
        ut1,
        era,
        boresightRaDec.toIcrs(),
        boresightAzAlt,
        boresightAirmass,
        boresightRotAngle,
        rotType,
        observatory,
        weather,
    )
