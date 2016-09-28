#ifndef _LCE_BOUTOARCHIVE_H_
#define _LCE_BOUTOARCHIVE_H_

#include <iostream>
#include <vector>
#include "net/bytebuffer.h"
#include "net/is_primitive.h"
#include "net/split_member.h"
#include <assert.h>

namespace lce
{
	namespace net
	{
		template< class BufferType >
		class BinOArchive
		{
		public:
		    BufferType& _buf;
		public:
		    BinOArchive(BufferType& buf)
		        :_buf(buf)
		    {
		    }
		    typedef bool_<false> is_loading;
		    typedef bool_<true> is_saving;

		    BufferType& rdbuf(){ return _buf;}

		    template<class T>
		    BinOArchive& operator&(T& t)
		    {
		        return *this<<t;
		    }

		    BinOArchive& operator &(char* t)
		    {
		        return *this<<t;
		    }

		    BinOArchive& operator &(unsigned char* t)
		    {
		        return *this<<t;
		    }
		    
		    BinOArchive& operator &(const char* t)
		    {
		        return *this<<t;
		    }
		    
		    template <class T>
		    BinOArchive & operator <<(T& t)
		    {
		        return save(t);
		    }

		    BinOArchive& operator <<(const char* str)
		    {
				save(str);
		        return *this;
		    }

		    BinOArchive& operator <<(unsigned char* str)
		    {
				save((const char*)(str));
		        return *this;
		    }

		    BinOArchive& operator <<(const unsigned char* str)
		    {
				save((const char*)(str));
		        return *this;
		    }
		    
		    BinOArchive& operator <<(char* str)
		    {
		        return *this<<static_cast<const char*>(str);
		    }
		    
		    template <class T>
		    BinOArchive & save(T& t)
		    {
		        t.serialize(*this);
		        return *this;
		    }

			
			BinOArchive& save (char& c)
			{
			     _buf.put(c);

			     return *this;
			}

			BinOArchive& save (unsigned char& c)
			{
			     _buf.put((char)c);

			     return *this;
			}


			BinOArchive& save (short& n)
			{
			     _buf.put_short(n);

			     return *this;
			}


			BinOArchive& save (unsigned short& n)
			{
			     _buf.put_short((unsigned short)n);

			     return *this;
			}


			BinOArchive& save (int& n)
			{
			     _buf.put_int(n);

			     return *this;
			}


			BinOArchive& save (unsigned int& n)
			{
			     _buf.put_int((unsigned int)n);

			     return *this;
			}
			/*
			template<>
			inline
			BinOArchive& BinOArchive::save (long& n)
			{
			     _buf.putInt(n);

			     return *this;
			}

			template<>
			inline
			BinOArchive& BinOArchive::save (unsigned long& n)
			{
			     _buf.putInt((unsigned int)n);

			    return *this;
			}
			*/

			BinOArchive& save(int64_t& n)
			{
			     _buf.put_long((int64_t)n);

			    return *this;
			}


			BinOArchive& save(uint64_t& n)
			{
			     _buf.put_long((uint64_t)n);

			    return *this;
			}


			BinOArchive& save (const std::string& str)
			{
			    int length = str.length();
			    *this<<length;

			    if(length > 0)
			    {
			        _buf.put(str);
			    }

			    return *this;
			}


			BinOArchive& save (std::string& str)
			{
				return save((const std::string&)(str));
			}

			BinOArchive& save(std::vector<char>& vec)
			{
				int length = vec.size();
				assert(length>=0);
			    *this<<length;
			    if(length > 0)
			    {
				    _buf.put(&vec[0], length, length);
			    }
			    return *this;
			}

			BinOArchive& save (const char *str)
			{
			    int  length = 0;
			    if(str)
			        length = strlen(str);

			    *this<<length;
			    if(length > 0)
			    {
			        _buf.put(str, length, length);
			    }

			    return *this;
			}
			
			BinOArchive& save (ByteBuffer& bb)
			{
			    int pos = bb.position();
			    int lim = bb.limit();
			    assert(pos <= lim);
			    int rem = (pos <= lim ? lim - pos : 0);

			    *this<<rem;
			    if(rem > 0)
			    {
			        _buf.put(bb.address() + pos, rem, rem);
			    }

			    return *this;
			}
		};
	}
}

#endif

