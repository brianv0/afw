#
# LSST Data Management System
#
# Copyright 2008-2016  AURA/LSST.
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
# see <https://www.lsstcorp.org/LegalNotices/>.
#

from __future__ import absolute_import, division, print_function
import unittest
import numpy as np

from builtins import zip
from builtins import range

import lsst.utils.tests
import lsst.afw.geom as afwGeom
import lsst.afw.geom.ellipses as afwGeomEllipses
import lsst.afw.image as afwImage


class SpanSetTestCase(lsst.utils.tests.TestCase):
    '''
    This is a python level unit test of the SpanSets class. It is mean to work in conjuction
    with the c++ unit test. The C++ test has much more coverage, and tests some features which
    are not exposed to python. This test serves mainly as a way to test that the python bindings
    correctly work. Many of these tests are smaller versions of the C++ tests.
    '''
    def testNullSpanSet(self):
        nullSS = afwGeom.SpanSet()
        self.assertEqual(nullSS.getArea(), 0)
        self.assertEqual(len(nullSS), 0)
        self.assertEqual(nullSS.getBBox().getDimensions().getX(), 0)
        self.assertEqual(nullSS.getBBox().getDimensions().getY(), 0)

    def testBBoxSpanSet(self):
        boxSS = afwGeom.SpanSet(afwGeom.Box2I(afwGeom.Point2I(2, 2), afwGeom.Point2I(6, 6)))
        self.assertEqual(boxSS.getArea(), 25)
        bBox = boxSS.getBBox()
        self.assertEqual(bBox.getMinX(), 2)
        self.assertEqual(bBox.getMinY(), 2)

    def testIteratorConstructor(self):
        spans = [afwGeom.Span(0, 2, 4), afwGeom.Span(1, 2, 4), afwGeom.Span(2, 2, 4)]
        spanSetFromList = afwGeom.SpanSet(spans)
        spanSetFromArray = afwGeom.SpanSet(np.array(spans))

        self.assertEqual(spanSetFromList.getBBox().getMinX(), 2)
        self.assertEqual(spanSetFromList.getBBox().getMaxX(), 4)
        self.assertEqual(spanSetFromList.getBBox().getMinY(), 0)

        self.assertEqual(spanSetFromArray.getBBox().getMinX(), 2)
        self.assertEqual(spanSetFromArray.getBBox().getMaxX(), 4)
        self.assertEqual(spanSetFromArray.getBBox().getMinY(), 0)

    def testIsContiguous(self):
        spanSetConList = [afwGeom.Span(0, 2, 5), afwGeom.Span(1, 5, 8)]
        spanSetCon = afwGeom.SpanSet(spanSetConList)
        self.assertTrue(spanSetCon.isContiguous())

        spanSetNotConList = [afwGeom.Span(0, 2, 5), afwGeom.Span(1, 20, 25)]
        spanSetNotCon = afwGeom.SpanSet(spanSetNotConList)
        self.assertFalse(spanSetNotCon.isContiguous())

    def testSplit(self):
        spanSetOne = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 2)
        spanSetTwo = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(8, 8)

        spanSetList = []
        for spn in spanSetOne:
            spanSetList.append(spn)
        for spn in spanSetTwo:
            spanSetList.append(spn)
        spanSetTogether = afwGeom.SpanSet(spanSetList)

        spanSetSplit = spanSetTogether.split()
        self.assertEqual(len(spanSetSplit), 2)

        for a, b in zip(spanSetOne, spanSetSplit[0]):
            self.assertEqual(a, b)

        for a, b in zip(spanSetTwo, spanSetSplit[1]):
            self.assertEqual(a, b)

    def testTransform(self):
        transform = afwGeom.LinearTransform(np.array([[2.0, 0.0], [0.0, 2.0]]))
        spanSetPreScale = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.CIRCLE)
        spanSetPostScale = spanSetPreScale.transformedBy(transform)

        self.assertEqual(spanSetPostScale.getBBox().getMinX(), -4)
        self.assertEqual(spanSetPostScale.getBBox().getMinY(), -4)

    def testOverlaps(self):
        spanSetNoShift = afwGeom.SpanSet.spanSetFromShape(4, afwGeom.Stencil.CIRCLE)
        spanSetShift = spanSetNoShift.shiftedBy(2, 2)

        self.assertTrue(spanSetNoShift.overlaps(spanSetShift))

    def testContains(self):
        spanSetLarge = afwGeom.SpanSet.spanSetFromShape(4, afwGeom.Stencil.CIRCLE)
        spanSetSmall = afwGeom.SpanSet.spanSetFromShape(1, afwGeom.Stencil.CIRCLE)

        self.assertTrue(spanSetLarge.contains(spanSetSmall))
        self.assertFalse(spanSetSmall.contains(afwGeom.Point2I(100, 100)))

    def testComputeCentroid(self):
        spanSetShape = afwGeom.SpanSet.spanSetFromShape(4, afwGeom.Stencil.CIRCLE).shiftedBy(2, 2)
        center = spanSetShape.computeCentroid()

        self.assertEqual(center.getX(), 2)
        self.assertEqual(center.getY(), 2)

    def testComputeShape(self):
        spanSetShape = afwGeom.SpanSet.spanSetFromShape(1, afwGeom.Stencil.CIRCLE)
        quad = spanSetShape.computeShape()

        self.assertEqual(quad.getIxx(), 0.4)
        self.assertEqual(quad.getIyy(), 0.4)
        self.assertEqual(quad.getIxy(), 0)

    def testDilate(self):
        spanSetPreDilate = afwGeom.SpanSet.spanSetFromShape(1, afwGeom.Stencil.CIRCLE)
        spanSetPostDilate = spanSetPreDilate.dilate(1)

        bBox = spanSetPostDilate.getBBox()
        self.assertEqual(bBox.getMinX(), -2)
        self.assertEqual(bBox.getMinY(), -2)

    def testErode(self):
        spanSetPreErode = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.CIRCLE)
        spanSetPostErode = spanSetPreErode.erode(1)

        bBox = spanSetPostErode.getBBox()
        self.assertEqual(bBox.getMinX(), -1)
        self.assertEqual(bBox.getMinY(), -1)

    def testFlatten(self):
        # Give an initial value to an input array
        inputArray = np.ones((6, 6)) * 9
        inputArray[1, 1] = 1
        inputArray[1, 2] = 2
        inputArray[2, 1] = 3
        inputArray[2, 2] = 4

        inputSpanSet = afwGeom.SpanSet([afwGeom.Span(0, 0, 1), afwGeom.Span(1, 0, 1)])
        flatArr = inputSpanSet.flatten(inputArray, afwGeom.Point2I(-1, -1))

        self.assertEqual(flatArr.size, inputSpanSet.getArea())

        # Test flatttening a 3D array
        spanSetArea = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX)
        spanSetArea = spanSetArea.shiftedBy(2, 2)

        testArray = np.arange(5*5*3).reshape(5, 5, 3)
        flattened2DArray = spanSetArea.flatten(testArray)

        truthArray = np.arange(5*5*3).reshape(5*5, 3)
        self.assertFloatsAlmostEqual(flattened2DArray, truthArray)

    def testUnflatten(self):
        inputArray = np.ones(6) * 4
        inputSpanSet = afwGeom.SpanSet([afwGeom.Span(9, 2, 3),
                                        afwGeom.Span(10, 3, 4),
                                        afwGeom.Span(11, 2, 3)])
        outputArray = inputSpanSet.unflatten(inputArray)

        arrayShape = outputArray.shape
        bBox = inputSpanSet.getBBox()
        self.assertEqual(arrayShape[0], bBox.getHeight())
        self.assertEqual(arrayShape[1], bBox.getWidth())

        # Test unflattening a 2D array
        spanSetArea = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX)
        spanSetArea = spanSetArea.shiftedBy(2, 2)

        testArray = np.arange(5*5*3).reshape(5*5, 3)
        unflattened3DArray = spanSetArea.unflatten(testArray)

        truthArray = np.arange(5*5*3).reshape(5, 5, 3)
        self.assertFloatsAlmostEqual(unflattened3DArray, truthArray)

    def populateMask(self):
        msk = afwImage.MaskU(10, 10, 1)
        spanSetMask = afwGeom.SpanSet.spanSetFromShape(3, afwGeom.Stencil.CIRCLE).shiftedBy(5, 5)
        spanSetMask.setMask(msk, 2)
        return msk, spanSetMask

    def testSetMask(self):
        mask, spanSetMask = self.populateMask()
        mskArray = mask.getArray()
        for i in range(mskArray.shape[0]):
            for j in range(mskArray.shape[1]):
                if afwGeom.Point2I(i, j) in spanSetMask:
                    self.assertEqual(mskArray[i, j], 3)
                else:
                    self.assertEqual(mskArray[i, j], 1)

    def testClearMask(self):
        mask, spanSetMask = self.populateMask()
        spanSetMask.clearMask(mask, 2)
        mskArray = mask.getArray()
        for i in range(mskArray.shape[0]):
            for j in range(mskArray.shape[1]):
                self.assertEqual(mskArray[i, j], 1)

    def makeOverlapSpanSets(self):
        firstSpanSet = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 4)
        secondSpanSet = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 2)
        return firstSpanSet, secondSpanSet


    def makeMaskAndSpanSetForOperationTest(self):
        firstMaskPart = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 2)
        secondMaskPart = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 8)
        spanSetMaskOperation = afwGeom.SpanSet.spanSetFromShape(2, afwGeom.Stencil.BOX).shiftedBy(2, 5)

        mask = afwImage.MaskU(20,20)
        firstMaskPart.setMask(mask, 3)
        secondMaskPart.setMask(mask, 3)
        spanSetMaskOperation.setMask(mask, 4)

        return mask, spanSetMaskOperation


    def testIntersection(self):
        firstSpanSet, secondSpanSet = self.makeOverlapSpanSets()

        overlap = firstSpanSet.intersect(secondSpanSet)
        for i, span in enumerate(overlap):
            self.assertEqual(span.getY(), i+2)
            self.assertEqual(span.getMinX(), 0)
            self.assertEqual(span.getMaxX(), 4)

        mask, spanSetMaskOperation = self.makeMaskAndSpanSetForOperationTest()
        spanSetIntersectMask = spanSetMaskOperation.intersect(mask, 2)

        expectedYRange = [3, 4, 6, 7]
        for expected, val in zip(expectedYRange, spanSetIntersectMask):
            self.assertEqual(expected, val.getY())


    def testIntersectNot(self):
        firstSpanSet, secondSpanSet = self.makeOverlapSpanSets()

        overlap = firstSpanSet.intersectNot(secondSpanSet)
        for yVal, span in enumerate(overlap):
            self.assertEqual(span.getY(), yVal+5)
            self.assertEqual(span.getMinX(), 0)
            self.assertEqual(span.getMaxX(), 4)

        mask, spanSetMaskOperation = self.makeMaskAndSpanSetForOperationTest()

        spanSetIntersectNotMask = spanSetMaskOperation.intersectNot(mask, 2)

        self.assertEqual(len(spanSetIntersectNotMask), 1)
        self.assertEqual(next(iter(spanSetIntersectNotMask)).getY(), 5)


    def testUnion(self):
        firstSpanSet, secondSpanSet = self.makeOverlapSpanSets()

        overlap = firstSpanSet.union(secondSpanSet)

        for yVal, span in enumerate(overlap):
            self.assertEqual(span.getY(), yVal)
            self.assertEqual(span.getMinX(), 0)
            self.assertEqual(span.getMaxX(), 4)

        mask, spanSetMaskOperation = self.makeMaskAndSpanSetForOperationTest()

        spanSetUnion = spanSetMaskOperation.union(mask, 2)

        for yVal, span in enumerate(spanSetUnion):
            self.assertEqual(span.getY(), yVal)


    def testMaskToSpanSet(self):
        mask, _ = self.makeMaskAndSpanSetForOperationTest()
        spanSetFromMask = afwGeom.maskToSpanSet(mask)

        for yCoord, span in enumerate(spanSetFromMask):
            self.assertEqual(span, afwGeom.Span(yCoord, 0, 4))


    def testEquality(self):
        firstSpanSet, secondSpanSet = self.makeOverlapSpanSets()
        secondSpanSetShift = secondSpanSet.shiftedBy(0, 2)

        self.assertFalse(firstSpanSet == secondSpanSet)
        self.assertTrue(firstSpanSet != secondSpanSet)
        self.assertTrue(firstSpanSet == secondSpanSetShift)
        self.assertFalse(firstSpanSet != secondSpanSetShift)

    def testSpanSetFromEllipse(self):
        axes = afwGeomEllipses.Axes(6, 6, 0)
        ellipse = afwGeomEllipses.Ellipse(axes, afwGeom.Point2D(5, 6))
        spanSet = afwGeom.SpanSet.spanSetFromShape(ellipse)
        for ss, es in zip(spanSet, afwGeomEllipses.PixelRegion(ellipse)):
            self.assertEqual(ss, es)

    def testFindEdgePixels(self):
        spanSet = afwGeom.SpanSet.spanSetFromShape(6, afwGeom.Stencil.CIRCLE)
        spanSetEdge = spanSet.findEdgePixels()

        truthSpans = [afwGeom.Span(-6, 0, 0),
                      afwGeom.Span(-5, -3, -1),
                      afwGeom.Span(-5, 1, 3),
                      afwGeom.Span(-4, -4, -4),
                      afwGeom.Span(-4, 4, 4),
                      afwGeom.Span(-3, -5, -5),
                      afwGeom.Span(-3, 5, 5),
                      afwGeom.Span(-2, -5, -5),
                      afwGeom.Span(-2, 5, 5),
                      afwGeom.Span(-1, -5, -5),
                      afwGeom.Span(-1, 5, 5),
                      afwGeom.Span(0, -6, -6),
                      afwGeom.Span(0, 6, 6),
                      afwGeom.Span(1, -5, -5),
                      afwGeom.Span(1, 5, 5),
                      afwGeom.Span(2, -5, -5),
                      afwGeom.Span(2, 5, 5),
                      afwGeom.Span(3, -5, -5),
                      afwGeom.Span(3, 5, 5),
                      afwGeom.Span(4, -4, -4),
                      afwGeom.Span(4, 4, 4),
                      afwGeom.Span(5, -3, -1),
                      afwGeom.Span(5, 1, 3),
                      afwGeom.Span(6, 0, 0)]
        truthSpanSet = afwGeom.SpanSet(truthSpans)
        self.assertEqual(spanSetEdge, truthSpanSet)


class TestMemory(lsst.utils.tests.MemoryTestCase):
    pass


def set_module(module):
    lsst.utils.tests.init()

if __name__ == "__main__":
    lsst.utils.tests.init()
    unittest.main()
