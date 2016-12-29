#
# LSST Data Management System
# Copyright 2008, 2009, 2010 LSST Corporation.
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

from __future__ import absolute_import, division, print_function
import unittest

from builtins import range

import lsst.utils.tests
import lsst.afw.image as afwImage


class ImageIoTestCase(lsst.utils.tests.TestCase):
    """A test case for Image Persistence"""

    def checkImages(self, image, original):
        # Check that two images are identical
        self.assertEqual(image.getHeight(), original.getHeight())
        self.assertEqual(image.getWidth(), original.getWidth())
        self.assertEqual(image.getY0(), original.getY0())
        self.assertEqual(image.getX0(), original.getX0())
        for x in range(0, original.getWidth()):
            for y in range(0, image.getHeight()):
                self.assertEqual(image.get(x, y), original.get(x, y))

    def setUp(self):
        # Create the additionalData PropertySet
        self.cols = 4
        self.rows = 4

    def testIo(self):
        for Image in (afwImage.ImageU,
                      afwImage.ImageL,
                      afwImage.ImageI,
                      afwImage.ImageF,
                      afwImage.ImageD,
                      ):
            image = Image(self.cols, self.rows)
            for x in range(0, self.cols):
                for y in range(0, self.rows):
                    image.set(x, y, x + y)

            with lsst.utils.tests.getTempFilePath("_%s.fits" % (Image.__name__,)) as filename:
                image.writeFits(filename)
                readImage = Image(filename)

            self.checkImages(readImage, image)


class TestMemory(lsst.utils.tests.MemoryTestCase):
    pass


def setup_module(module):
    lsst.utils.tests.init()

if __name__ == "__main__":
    lsst.utils.tests.init()
    unittest.main()
