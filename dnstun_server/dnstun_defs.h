#ifndef _DNSTUN_DEFS_H_
#define _DNSTUN_DEFS_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MAX_LENGTH_OF_TYPE 10
#define MAX_LENGTH_OF_NAME 100
#define MAX_LENGTH_OF_ADDRESS 50
#define MAX_COUNT_OF_ADDRESSES_IN_RESPONSE 100
#define MAX_LENGTH_OF_HEADER 6
#define MAX_LENGTH_OF_ANSWER (MAX_LENGTH_OF_TYPE + MAX_LENGTH_OF_NAME + MAX_LENGTH_OF_HEADER + MAX_COUNT_OF_ADDRESSES_IN_RESPONSE * MAX_LENGTH_OF_ADDRESS)

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_DEFS_H_ */
