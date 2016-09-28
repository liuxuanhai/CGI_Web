
#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__


namespace lce
{
	namespace thread
	{
		class NonCopyable
		{

		    //! Restrict the copy constructor
		    NonCopyable(const NonCopyable&);

		    //! Restrict the assignment operator
		    const NonCopyable& operator=(const NonCopyable&);

		protected:

		    //! Create a NonCopyable object
		    NonCopyable() { }

		    //! Destroy a NonCopyable object
		    ~NonCopyable() { }

		}; /* NonCopyable */

	}
}



#endif
