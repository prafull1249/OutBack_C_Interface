/*	OBencrypt.h	version 0.87	Feb. 13, 2012
 *
 *	Written by Daniel Lloyd. 2011-2012.
 *	Property of OutBack Power Systems Inc.
 */

#ifndef OutBack_OBencrypt_h
#define OutBack_OBencrypt_h

#define BIAS			(0x0BCC)
#define ROTATE_R7(x)	((((x) & 0x007F) << 9) | ((x) >> 7))
#define ROTATE_L7(x)	((((x) & 0x01FF) << 7) | ((x) >> 9))

#define ENCRYPT(x)		((x) = (ROTATE_R7((x) ^ BIAS) ^ encKey))
#define DECRYPT(x)		((x) = (ROTATE_L7((x) ^ encKey) ^ BIAS))

#define ENCRYPTION_ENABLED

#endif
