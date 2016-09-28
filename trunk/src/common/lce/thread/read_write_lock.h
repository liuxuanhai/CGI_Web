#ifndef __RWLOCK_HPP__
#define __RWLOCK_HPP__


#include <pthread.h>
#include <assert.h>

#include "thread/exceptions.h"
#include "thread/noncopyable.h"

namespace lce
{
	namespace thread
	{
		class ReadWriteLock : private NonCopyable 
		{
			
		public:
			inline
			ReadWriteLock () : _rdlock(*this)
					 , _wrlock(*this) {
				int ret = pthread_rwlock_init (&_rwlock, NULL);
				if (ret != 0) {
					throw Initialization_Exception();
				}
			}

			~ReadWriteLock () {
				int ret = pthread_rwlock_destroy (&_rwlock);
				if (ret != 0) {
					//assert(0);
				}
			}

			// �ṩͨ���Ķ�д���ӿ�

			inline
			void acquire_read () {
				int ret = pthread_rwlock_rdlock (&_rwlock);
				if (ret != 0) {
					throw Synchronization_Exception();
				}
			}

			inline
			void acquire_write () {
				int ret = pthread_rwlock_wrlock (&_rwlock);
				if (ret != 0) {
					throw Synchronization_Exception();
				}
			}

			inline
			bool try_acquire_read (unsigned long /*timeout*/ = 0) {
				int ret = pthread_rwlock_tryrdlock (&_rwlock);
				return ret == 0;
			}

			inline
			bool try_acquire_write (unsigned long /*timeout*/ = 0) {
				int ret = pthread_rwlock_trywrlock (&_rwlock);
				return ret == 0;
			}

			inline
			void release () {
				int ret = pthread_rwlock_unlock (&_rwlock);
				if (ret != 0) {
					throw Synchronization_Exception();
				}
			}

			
			// �ṩ����Ķ�д������ʹ������񻥳���һ����ʹ�ã�������
			// Guardһ��ʹ�á�

			
			class ReadLock : private NonCopyable {
			private:
				ReadWriteLock &_lock;

			public:
				inline
				ReadLock (ReadWriteLock &lock) : _lock(lock) {}

				inline
				~ReadLock () {}
				
				inline
				void acquire () {
					_lock.acquire_read();
				}

				inline
				bool try_acquire (unsigned long timeout = 0) {
					return _lock.try_acquire_read(timeout);
				}

				inline
				void release () {
					_lock.release();
				}
				
			};

			class WriteLock : private NonCopyable {
			private:
				ReadWriteLock &_lock;
			public:
				WriteLock (ReadWriteLock &lock) : _lock(lock) {}

				~WriteLock () {}
				
				inline
				void acquire () {
					_lock.acquire_write();
				}

				inline
				bool try_acquire (unsigned long timeout = 0) {
					return _lock.try_acquire_write(timeout);
				}

				inline
				void release () {
					_lock.release();
				}
			};

			// ��ȡ�ڲ���������
			ReadLock &read_lock () {
				return _rdlock;
			}

			// ��ȡ�ڲ�д������
			WriteLock &write_lock () {
				return _wrlock;
			}
			
		private:
			pthread_rwlock_t _rwlock;

			// ����Ķ�д����
			ReadLock _rdlock;
			WriteLock _wrlock;
		};
	}
}


#endif	// __RWLOCK_HPP__
