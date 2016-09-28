#ifndef _LCE_BINIARCHIVE_H_
#define _LCE_BINIARCHIVE_H_

#include <vector>
#include <string>
#include <assert.h>
#include "net/bytebuffer.h"
#include "net/split_member.h"

namespace lce
{
	namespace net
	{
		template< class BufferType >
		class BinIArchive
		{
		public:
		    BinIArchive(BufferType& buf)
		        :_buf(buf)
		    {
		    }
		    
		    BufferType& rdbuf(){ return _buf;}
		    BufferType& _buf;

		    typedef bool_<true> is_loading;
		    typedef bool_<false> is_saving;

		    
		    template <class T>
		    BinIArchive & operator &(T& t)
		    {
		        *this>>t;
		        return *this;
		    }


		    BinIArchive& operator &(char* t)
		    {
		        return *this>>t;
		    }
		    
		    BinIArchive& operator &(unsigned char* t)
		    {
		        return *this>>t;
		    }
		    
		    BinIArchive& operator &(const char* t)
		    {
		        return *this>>t;
		    }
		    
		    template <class T>
		    BinIArchive & operator >>(T& t)
		    {
		        return load(t);
		    }
		    

		    BinIArchive& operator >>(char* str)
		    {
		        return load(str);  
		    }

		    BinIArchive& operator >>(unsigned char* str)
		    {
		        return load((char*)str);  
		    }
		    
		    BinIArchive& operator >>(const char* str)
		    {
		        return *this>>const_cast<char*>(str);
		    }
		    

		    template <class T>
		    BinIArchive& load(T& t)
		    {
		        t.serialize(*this);
		        return *this;
		    }

			BinIArchive& load(char& c)
			{
			    c=_buf.get();
			    return *this;
			}


			BinIArchive& load(unsigned char& c)
			{
			    c=(unsigned char)_buf.get();
			    return *this;
			}


			BinIArchive& load(short& n)
			{
			    n=_buf.get_short();
			    return *this;
			}

			BinIArchive& load(unsigned short& n)
			{
			    n=(unsigned short)_buf.get_short();
			    return *this;
			}


			BinIArchive& load(int& n)
			{
			    n=_buf.get_int();
			    return *this;
			}


			BinIArchive& load(unsigned int& n)
			{
			    n=(unsigned)_buf.get_int();
			    return *this;
			}
			/*
			template<>
			inline
			BinIArchive& BinIArchive::load(long& n)
			{
			    n=_buf.getInt();
			    return *this;
			}

			template<>
			inline
			BinIArchive& BinIArchive::load(unsigned long& n)
			{
			    n=(unsigned)_buf.getInt();
			    return *this;
			}
			*/

			BinIArchive& load(int64_t& n)
			{
				n = (int64_t)_buf.get_long();
				return *this;
			}


			BinIArchive& load(uint64_t& n)
			{
				n = (uint64_t)_buf.get_long();
				return *this;
			}

			BinIArchive& load(char* str)
			{
			    int length;
			    *this>>length;
			    _buf.get(str, length, length);
			    str[length] = '\0';

			    return *this;
			}

			BinIArchive& load(std::string& str)
			{
			    int length;
			   *this>>length;
			    _buf.get(str, length);

			    return *this;
			}  

			BinIArchive& load(std::vector<char>& vec)
			{
				int length;
				*this>>length;
				assert(length>=0);
				vec.resize(length);
				if(length>0)
				{
					_buf.get(&vec[0], length, length);
				}
				return *this;
			}

			BinIArchive& load (ByteBuffer& bb)
			{
			    int length;
			    int pos = bb.position();
			    int lim = bb.limit();
			    assert(pos <= lim);
			    int rem = (pos <= lim ? lim - pos : 0);

			    *this>>length;
			    if(rem > 0)
			    {
			        _buf.get(bb.address() + pos, rem, length);
			    }

			    if (length > 0)
			    {
			        bb.position(pos + length);
			    }

			    bb.flip();
			    
			    return *this;
			}
		};	
	}
}

#endif


