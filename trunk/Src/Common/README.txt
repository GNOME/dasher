Expat
-----
Dasher needs an XML parser. I chose Expat.

Cons:
	C not "nice" C++.
	Doesn't help write XML.
	Is not a validating parser.
Pros:
	Small, quick relatively easy to get working
	For the simple alphabet DTD I can write the XML myself.

There are very comprehensive C++ libraries, eg:
http://www.alphaworks.ibm.com/tech/xml4c
However, for "comprehensive" read "10 MB"


Unicode
-------
UTF-8 <--> UTF-32 <--> UTF-16 conversion is very easy in theory.
When learning about Unicode I wrote some code that does it based
on RFC's as an exercise. However, the supplied code from the
Unicode consortium is better. It comes with a test-rig, so you
can check it on your platform. Use my corrected version of the
test-rig.

Note that Dasher does not need to do any of the above conversion,
but GUI's commonly will do. Some OS's such as Win95 and WinCE have
no native UTF-8 conversion functions.