Building Dasher for PPC

I set up a new "Deployment" configuration in order to build a universal binary, following http://developer.apple.com/documentation/MacOSX/Conceptual/universal_binary/universal_binary_compiling/universal_binary_build.html#//apple_ref/doc/uid/TP40002217-CH206-BCICFABA

If you now select "OS 10.4" as your Active SDK, you should get a choice of i386 and PPC architectures you can build for (however, it seems that choosing either, builds both - perhaps unsurprising, as it's a universal binary!).

A complication is that OS 10.4 doesn't include the expat XML library we're using (whereas 10.5 has it built in - however, 10.5 doesn't run on PPC...). Hence, such users will have to install expat first, from expat-2.0.1-3.dmg (on the Dasher website). However, XCode doesn't think they'll have done this, and AFAICT the only sort of dependency it allows is on another project/target within XCode (this'd require setting up Expat as a Framework to be built by XCode - which seems a bit unnecessary when 10.5 users don't need it). So, I hacked up the "10.4u" (universal) SDK to include the files that expat-2.0.1-3.dmg installs onto the user's machine, as follows...

(On your 10.5 build machine) Mount the expat DMG, and on the resulting volume, right-click expat.pkg and "Show Package Contents". Grab the file Contents/Archive.pax.gz, and copy to somewhere on your harddisk. Then, gunzip it to get Archive.pax. If you double-click this in the Finder, it opens with OS X's inbuilt "Archive Utility", and produces a directory "usr". As root, copy the directory usr/local/include and the file usr/local/lib/libexpat.a into the corresponding locations in /Developer/SDKs/MacOS10.4u.sdk/usr/local. This should persuade XCode to build Dasher using the 10.4 SDK....

and for now, I'm distributing it as a .zip, simply by going to dasher/Src/MacOSX/build/Deployment/, right-clicking Dasher.app and selecting 'Compress "Dasher"'...

--Alan Lawrence, May 2009.
