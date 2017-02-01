=======
SpanSet
=======
A ``SpanSet`` is a class which defines a region of an array in pixel coordinates.
As the name implies, a ``SpanSet`` is a collection of ``Span`` objects which
behaive as a mathematical set. Common set operators (intersect, intersectNot, union,
erode, dialate) are defined for pairs of ``SpanSet`` objects.

``SpanSet`` objects are immutable once they are created. As such, all method calls
on a ``SpanSet`` object which operate on a ``SpanSet`` create modify and return a new
``SpanSet`` object. 
