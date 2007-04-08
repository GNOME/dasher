ConvertUTF.c and ConvertUTF.h are unchanged.

harness.c has a small bug fix in the test rig (that took me too long to find).
This bug was subtle and only evident in release builds of harness.c in MSVC++ 6
(which made debugging annoying). It was worth checking out though to confirm
that the problem was not with ConvertUTF.

MSVC++ can give some warnings about assigning a longs to chars when compiling
ConvertUTF.c - they all seem safe to me.

See the files themselves for documentation and copyright.

IAM 09/2002