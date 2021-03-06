#ifndef __OS_DATA_TYPE_H__
#define __OS_DATA_TYPE_H__



#ifdef WIN32 
	#define int8    char
	#define Int8    int8
	#define uint8   unsigned char 
	#define UInt8    uint8

	#define int16   short
	#define Int16   int16
	#define uint16  unsigned short
	#define UInt16  uint16 

	#define int32   int
	#define Int32   int32
	#define uint32  unsigned int 
	#define UInt32  uint32

	#define int64   __int64
	#define Int64   int64
	#define uint64  unsigned __int64
	#define UInt64  uint64
	typedef float               Float32;
  typedef double              Float64;
	typedef UInt32              Bool;
	typedef pthread_t		THREAD_ID;
	typedef int32 OS_Error;
	
#else
	#define int8    char
	#define Int8    int8
	#define uint8   unsigned char 
	#define UInt8    uint8

	#define int16   short
	#define Int16   int16
	#define uint16  unsigned short
	#define UInt16  uint16 

	#define int32   int
	#define Int32   int32
	#define uint32  unsigned int 
	#define UInt32  uint32

	#define int64   long long
	#define Int64   int64
	#define uint64  unsigned long long
	#define UInt64  uint64
	typedef float               Float32;
  typedef double              Float64;
	typedef UInt32              Bool;
//	typedef pthread_t		THREAD_ID;
	typedef int32 OS_Error;

#endif

#ifdef __x86_64
	#define			OS_INT_PTR		uint64
#else
	#define			OS_INT_PTR		uint32
#endif


#ifdef __x86_64
	#define			MP4FILE_HANDLE		uint64
#else
	#define			MP4FILE_HANDLE		uint32
#endif

#endif
