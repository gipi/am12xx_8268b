/**
 * Crypto code for HDCP2 XTASK.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.  Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.  The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors.  The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials,  either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

#include <linux/types.h>
#include <linux/string.h>
#include <linux/time.h>

/**
 * This file contains generic C for implementing the HDCP
 * state machine.
 */
#include "hdcp2_hal.h"


/**
 * Bigdigits library. See included copyright notice.
 */
#include "bigdigits.h"



/**
 * Data structure for RSA decrypt.
 *
 * Contains P, Q, dP, dQ, qInv from the public / private keypairs.
 */

typedef struct {
   DIGIT_T *p;
   DIGIT_T *q;
   DIGIT_T *dP;
   DIGIT_T *dQ;
   DIGIT_T *Qinv;
} crypto_RSAKey;




/**
 * size of the key components for RSA decryption.
 * This should equal key size / 32.
 */
#define MOD_SIZE (1024/32)

/** Prototypes not included in crypto.h */
static int crypto_rsaPrivateDecryptOaepSha256( uint8_t const * pIn, uint8_t* pOut, const crypto_RSAKey *key );

/**
 * Swap byte order of 'length' bytes. This function is intended
 * to convert a SHA-256 digest into the correct byte order expected
 * as input to other functions. Length should be even.
 *
 * @param[inout] ptr Data to be byte-swapped
 * @param[inout] length Size of data to be modified.
 * @return H2_OK
 */
H2status crypto_swapBytes( H2uint8 *ptr, int length )
{
   int ii;

   H2status status = H2_ERROR;

   do
   {
      if( length <= 2 )
      {
         status = H2_ERROR;
         break;
      }

      for(ii=0;ii<length/2;ii++)
      {
         char c = ptr[ii];
         ptr[ii] = ptr[length-ii-1];
         ptr[length-ii-1] = c;
      }

      status = H2_OK;

   }while(0);

   return status;
}


/**
 *
 * Mask Generation Function for RSA-OAEP-SHA256.
 *
 * @see PKCS#1v2.1 for an explanation of the MGF.
 * @param[inout] mask Pointer to mask data
 * @param[in] length Size of mask data
 * @param[in] seed Seed for SHA256
 * @param[in] seedLen Size of seed data.
 * @scope static.
 * @return H2_OK.
 */
static H2status MGF1_SHA256( H2uint8 *mask, H2uint32 length, const H2uint8 *seed, int seedLen )
{
   H2uint32 ii=0, outBytes = 0;
   H2uint8       cnt[4];
   H2Sha256Ctx_t ctx;

   static H2uint8 digest[SHA256_DIGEST_SIZE];

   crypto_sha256Init( &ctx );

   for ( ii=0; outBytes < length; ii++)
   {
      /** Copy current byte offset to the counter */
      /** Since the copy is done this way, byte order is irrelevant */
      cnt[0] = (H2uint8) (( ii >> 24 ) & 0xFF );
      cnt[1] = (H2uint8) (( ii >> 16 ) & 0xFF );
      cnt[2] = (H2uint8) (( ii >> 8 ) & 0xFF );
      cnt[3] = (H2uint8) (( ii ) & 0xFF );
      /** Clear the context */
      memset( &ctx, 0, sizeof( ctx ));
      crypto_sha256Init( &ctx );
      /** Add the seed */
      crypto_sha256Update( &ctx, seed, seedLen );
      /** Add the counter */
      crypto_sha256Update( &ctx, cnt, 4 );
      if ( outBytes + SHA256_DIGEST_SIZE <= length )
      {
         /** If at least SHA256_DIGEST_SIZE output bytes are needed, copy to output and continue */
         crypto_sha256Final( &ctx, digest );

         memcpy( mask+outBytes, digest, SHA256_DIGEST_SIZE );
         outBytes += SHA256_DIGEST_SIZE;
      }
      else
      {
         /** Otherwise, only copy as many bytes as are still needed and terminate */
         crypto_sha256Final( &ctx, digest );

         memcpy( mask+outBytes, digest, length-outBytes );
         outBytes = length;
      }
   }
   return H2_OK;
}


/**
 * d = s1^r2 over len bytes.
 *
 * This implementation is not very efficient.
 *
 * @param[in] s1 Input to xor
 * @param[in] s2 Input to xor
 * @param[out] d Output = s1 ^ s2
 * @param[in] len Length of data to xor
 * @return H2_OK or H2_ERROR if s1, s2, or d are NULL.
 *
 */
H2status crypt_xor( const uint8_t *s1, const uint8_t *s2, uint8_t *d, int len )
{
   int ii;

   if ( !s1 || !s2 || !d )
   {
      return H2_ERROR;
   }

   /** @todo: Optimize by using 32 bit-xors if needed. */
   for ( ii = 0 ; ii < len ; ii++ )
   {
      d[ii] = s1[ii] ^ s2[ii];
   }

   return H2_OK;

}


/**
 * RSA private key decrypt a memory buffer. PKCS#1 1.5 padding is used.
 * The key size is 1024 bits so all buffers are to be this length.
 *
 * @param[in]  pIn   Pointer to memory buffer to decrypt.
 * @param[out] pOut  Pointer to clear text buffer.
 *
 * Notes: Only a NULL label is supported. This function is non-reentrant.
 *
 * @retval Size of decrypted buffer. Zero if decryption failed.
 *
 */

static int crypto_rsaPrivateDecryptOaepSha256( uint8_t const * pIn, uint8_t* pOut, const crypto_RSAKey *key )
{
   int rc = 0;
   int bad = 0;
   /** The below values are static to move them off of the stack. */
   static H2uint8 seedMask[SHA256_DIGEST_SIZE];
   static H2uint8 seed[SHA256_DIGEST_SIZE];
   static H2uint8 dbMask[1024/8];
   static H2uint8 db[128-SHA256_DIGEST_SIZE-1];
   static H2uint8 pTemp[1024/8];
   static H2uint8 *ptr;
   H2uint8 *maskedSeed;
   H2uint8 *maskedDB;

   static const H2uint8 sha256NullHash[SHA256_DIGEST_SIZE] = { 0xe3, 0xb0, 0xc4, 0x42,
      0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f,
      0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
      0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55 };

   static DIGIT_T c[MOD_SIZE];
   static DIGIT_T m[MOD_SIZE];
   static DIGIT_T m1[MOD_SIZE];
   static DIGIT_T m2[MOD_SIZE];
   static DIGIT_T h[MOD_SIZE];
   static DIGIT_T hq[2*MOD_SIZE];       /* Multiply requires 2x size of it's inputs */

   /** Copy */
   mpConvFromOctets( c, MOD_SIZE, pIn, 1024/8 );
  
   /** TODO: Add Blinding to avoid giving away hints based on decrypt time. */

   /** Decrypt using Chinese Remainder Theorem */
   /** Let m_1 = c^dP mod p. */
   mpModExp( m1, c, key->dP, key->p, MOD_SIZE );

   /** Let m_2 = c^dQ mod q. */
   mpModExp( m2, c, key->dQ, key->q, MOD_SIZE );

   if ( mpCompare( m1, m2, MOD_SIZE ) < 0 )
   {
      mpAdd( m1, m1, key->p, MOD_SIZE );
   }

   mpSubtract( m1, m1, m2, MOD_SIZE );

   /** Let h - qInv( m_1 - m_2 ) mod p */
   mpModMult( h, key->Qinv, m1, key->p, MOD_SIZE );
   mpMultiply( hq, h, key->q, MOD_SIZE );

   /** Let m = m_2 + hq */
   mpAdd( m, m2, hq, MOD_SIZE );

   /** Copy the output to pTemp */
   mpConvToOctets(m, MOD_SIZE, pTemp, sizeof(pTemp));
   
   /**
    * pTemp still has OAEP-SHA256 padding.
    *
    * pTemp is:
    * Y || maskedSeed || maskedDB
    * Y is one octet, and should be zero.
    * maskedSeed is hLen ( SHA256_DIGEST_SIZE) bytes.
    * maskedDB is k-hLen-1 bytes
    */

   /** split it up */
   maskedSeed = pTemp+1;
   maskedDB = pTemp+1+SHA256_DIGEST_SIZE;

   /** Do not exit on error conditions, or reveal what the errors were,
    * to avoid leaking timing information.
    */

   if ( *pTemp != 0 )
   {
      bad = 1;
   }

   /** Reverse OAEP-SHA256 */
   /** seedMask = MGF( maskedDB, hLen ) */
   MGF1_SHA256( seedMask, SHA256_DIGEST_SIZE, maskedDB, 1024/8-SHA256_DIGEST_SIZE-1 );

   /** seed = maskedSeed xor seedMask */
   crypt_xor( maskedSeed, seedMask, seed, SHA256_DIGEST_SIZE );

   /** dbMask = MGF( seed, k-hLen-1 ) */
   MGF1_SHA256( dbMask, 1024/8-SHA256_DIGEST_SIZE-1, seed, SHA256_DIGEST_SIZE );

   /** DB = maskedDB XOR dbMask */
   crypt_xor( dbMask, maskedDB, db, 1024/8-SHA256_DIGEST_SIZE-1 );

   /**
    * DB should contain:
    * lHash' || PS || 0x01 || M
    *
    * Where:
    * lhash' = SHA256(Label), which should be SHA256( NULL, 0 )
    * PS is a (possibly empty) padding string of all zeroes
    * 0x01 immediately precedes the data
    * M is the message
    */


   /* Verify the hash */
   if ( memcmp( db, sha256NullHash, SHA256_DIGEST_SIZE ) != 0)
   {
//#warning REMOVE THIS DEBUG BEFORE PRODUCTION!
      int ii;
      printk("SHA256 fail.\n");
      printk("NULL HASH\n");
      for(ii=0;ii<SHA256_DIGEST_SIZE;ii++)
      {
         printk("0x%x ", sha256NullHash[ii]);
      }
      printk("\nCOMPUTED HASH\n");
      for(ii=0;ii<SHA256_DIGEST_SIZE;ii++)
      {
         printk("0x%x ", db[ii]);
      }
      printk("\n");

      bad = 1;
      return rc;
   }

   ptr = db + SHA256_DIGEST_SIZE;
   do
   {
      /** Search for the first non-zero octet, and make sure it's 0x01.
       * Since PS is possibly empty, we do not check for zero until AFTER
       * checking for 0x01.
       */
      if ( ptr[0] != 0x00 )
      {
         if ( ptr[0] != 0x01 )
         {
            /** For security purposes, we cannot reveal which part of
             * the decryption has failed.
             *
             * However, if LOCALDBG is set, the keys and ciphertext are
             * printed out, so it is not an issue.
             *
             */
            printk("Found non-zero that wasn't 0x01 fail.\n");
            bad = 1;
         }
         /** Found it! First non-zero octet was 0x01 */
         break;
      }
      ptr++;
   } while( ptr < db + sizeof(db));

   if ( ptr >= db + sizeof(db))
   {
      bad = 1;
   }
   else
   {
      //printk("SUCCESS! %d - bad=%d\n", ptr - db, bad);
      ptr++;
      /** Copy the results */
      memcpy( pOut, ptr, db + sizeof(db) - ( ptr ));
      rc = db + sizeof(db) - (ptr);
   }
   if ( bad )
   {
      rc = 0;
   }
   //printk("Rc=%d\n", rc);
   return rc;

}

/**
 * Decrypt EKpubKm using AES CTR decrypt with kPrivRx as AES key
 *
 * @param[in] KprivRx Private key used for decryption in p,q,dP,dQ,qInv form
 * @param[out] km Decrypted km. Should be MASTERKEY_SIZE bytes
 * @param[in] EKpubKm Encrypted Km to decrypt. Should be sent plaintext from TX
 * @return H2_OK or H2_ERROR
 */
H2status Decrypt_EKpubKm_kPrivRx( const H2uint8* KprivRx, H2uint8* km, const H2uint8 *EKpubKm)
{
   H2status rc = H2_ERROR;

   int kmSize = MASTERKEY_SIZE;
	struct timeval value;
	unsigned int start,end;

   do
   {
      crypto_RSAKey key;


      /**
       * BIGDITS buffers for private key.
       * MOD_SIZE = 32. DIGIT_T = 32 bit, 32*32 = 1024 bit total.
       *
       */
      DIGIT_T keyP[MOD_SIZE];
      DIGIT_T keyQ[MOD_SIZE];
      DIGIT_T keydP[MOD_SIZE];
      DIGIT_T keydQ[MOD_SIZE];
      DIGIT_T keyQinv[MOD_SIZE];
      key.p = keyP;
      key.q = keyQ;
      key.dP = keydP;
      key.dQ = keydQ;
      key.Qinv = keyQinv;

      // Input parameter validation
      if( KprivRx == NULL || km == NULL || EKpubKm == NULL)
      {
         //H2DBGLOG(( WARN, "ERROR! Decrypt_EKpubKm_kPrivRx: Invaid Input parameters"));
         break;
      }

      /**
       * Initialize data for the key - P, Q, dP, dQ and qInv
       */
      mpConvFromOctets( key.p, MOD_SIZE, KprivRx, KPUBRX_P );
      mpConvFromOctets( key.q, MOD_SIZE, KprivRx+KPUBRX_P, KPUBRX_Q );
      mpConvFromOctets( key.dP, MOD_SIZE, KprivRx+KPUBRX_P+KPUBRX_Q, KPUBRX_dP );
      mpConvFromOctets( key.dQ, MOD_SIZE, KprivRx+KPUBRX_P+KPUBRX_Q+KPUBRX_dP, KPUBRX_dQ );
      mpConvFromOctets( key.Qinv, MOD_SIZE, KprivRx+KPUBRX_P+KPUBRX_Q+KPUBRX_dP+KPUBRX_dQ, KPUBRX_qInv );

      /**
       * Perform decrypt. Return value is the size of decrypted data.
       */
	//do_gettimeofday(&value);
	//start = (unsigned int)(value.tv_sec*1000 + value.tv_usec/1000);

      kmSize = crypto_rsaPrivateDecryptOaepSha256( EKpubKm, km, &key );
	  
	//do_gettimeofday(&value);
	//end = (unsigned int)(value.tv_sec*1000 + value.tv_usec/1000);
	//printk(">:%d\n",end-start);

      /**
       * Check return size!
       */
      if ( kmSize != MASTERKEY_SIZE )
      {
         rc = H2_ERROR;
      }
      else
      {
         rc = H2_OK;
      }

   } while ( 0 );
   return rc;
}



