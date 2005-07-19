#ifndef __NoClones_h__
#define __NoClones_h__

/* Explanation of NoClones {{{

C++ defines default copy constructors and assignment operators, which clone
every member of a class. Stroustrup describes this behaviour as the result of
"historical accident". For many non-trivial classes, especially those
containing pointers, this default behaviour is too naive. In fact it often leads
to heap corruption.

Sometimes it does not make any sense to copy an instance of a class. For
example if it contains a unique file handle, or other lock on a system resource.
Sometimes it is too much effort to write reliable replacement copy operations[1].
In either case a private copy constructor and a private assignment operator
prevent accidental copying. [2]

Deriving a class from this class has the same preventative effect. It is also a
bit neater and means that all the above explanation is centralised here.

IAM 09/2002

[1] An example of how it is very easy to make mistakes:
http://www.mistybeach.com/articles/WhyIDontLikeCPlusPlusForLargeProjects.html
If we don't need a copy feature it really isn't worth the hassle.
[2] The C++ Programming Language. Stroustrup. 3rd edition. Section 10.4.6.3

}}} */

class NoClones {
protected:                     // Default constructor doesn't need to be public, but can't be private.
  NoClones() {
  };                            // Lots of compiler complaints without default constructor.
private:
  NoClones(const NoClones &);
  NoClones & operator=(const NoClones &);
};

#endif /* #ifndef __NoClones_h__ */
