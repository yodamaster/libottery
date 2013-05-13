#ifndef OTTERY_INTERNAL_H_HEADER_INCLUDED_
#define OTTERY_INTERNAL_H_HEADER_INCLUDED_
#include <stdint.h>
#include <sys/types.h>
#include "ottery-config.h"

/**
 * Interface to the operating system's strong RNG.  If this were fast,
 * we'd just use it for everything, and forget about having a userspace
 * PRNG.  Unfortunately, it typically isn't.
 *
 * @param bytes A buffer to receive random bytes.
 * @param n The number of bytes to write
 * @return 0 on success, -1 on failure.  On failure, it is not safe to treat
 *   the contents of the buffer as random at all.
 */
int ottery_os_randbytes_(uint8_t *bytes, size_t n);

#if ! defined(OTTERY_NO_VECS)          \
       && (defined(__ARM_NEON__) ||    \
           defined(__ALTIVEC__)  ||    \
           defined(__SSE2__))
#define OTTERY_HAVE_SIMD_IMPL
#endif

/**
 * Information on a single pseudorandom function that we can use to generate
 * a bytestream which (we hope) an observer can't distinguish from random
 * bytes.
 *
 * Broadly speaking, every ottery_prf has an underlying function from an
 * (state_bytes)-byte state and a 4 byte counter to an (output_len /
 * idx_byte)-byte output block.  This function is computed (output_len)
 * bytes at a time, so you need to advance the counter (idx_byte) bytes
 * between calls.
 **/
struct ottery_prf {
  /** The name of this algorithm. */
  const char *name;
  /** The name of the implementation of this algorithm*/
  const char *impl;
  /** The length of the object that's used to hold the state (keys, nonces,
   * subkeys as needed, etc) for this PRF. This can be longer than
   * state_bytes because of key expansion or structure padding.  It must be
   * no greater than MAX_STATE_LEN. */
  unsigned state_len;
  /** The number of bytes used to generate a state object. It must be no
   * greater than MAX_STATE_BYTES.  It must be no grater than output_len. */
  unsigned state_bytes;
  /** The number of bytes generated by a single call to the generate
   *  function.
   */
  unsigned output_len;
  /** The number of counter values consumed by a single call to the generate
   * function. */
  unsigned idx_step;
  /** Pointer to a function to to intialize a state structure for the PRF.
   *
   * @param state An object of size at least (state_len) that will
   *     hold the state and any derived values.  It must be aligned to
   *     a 16-byte boundary.
   * @param bytes An array of (state_bytes) random bytes.
   */
  void (*setup)(void *state, const uint8_t *bytes);
  /** Pointer to a function that calculates the PRF.
   *
   * @param state A state object previously initialized by the setup
   *     function.
   * @param output An array of (output_len) bytes in which to store the
   *     result of the function
   * @param idx A counter value for the function.
   */
  void (*generate)(void *state, uint8_t *output, uint32_t idx);
};

/** Largest possible state_bytes value. */
#define MAX_STATE_BYTES 64
/** Largest possible state_len value. */
#define MAX_STATE_LEN 256
/** Largest possible output_len value. */
#define MAX_OUTPUT_LEN 256

/**
 * @brief pure-C portable ChaCha implementations.
 *
 * @{
 */
extern const struct ottery_prf ottery_prf_chacha8_merged_;
extern const struct ottery_prf ottery_prf_chacha12_merged_;
extern const struct ottery_prf ottery_prf_chacha20_merged_;
/**@}*/

#ifdef OTTERY_HAVE_SIMD_IMPL
/**
 * @brief SIMD-basd ChaCha implementations.
 *
 * These are much, much faster.
 *
 * @{ */
extern const struct ottery_prf ottery_prf_chacha8_krovetz_;
extern const struct ottery_prf ottery_prf_chacha12_krovetz_;
extern const struct ottery_prf ottery_prf_chacha20_krovetz_;
/** @} */
/** @brief Default ChaCha implementations.
 * @{ */
#define ottery_prf_chacha8_ ottery_prf_chacha8_krovetz_
#define ottery_prf_chacha12_ ottery_prf_chacha12_krovetz_
#define ottery_prf_chacha20_ ottery_prf_chacha20_krovetz_
/** @} */
#else
#define ottery_prf_chacha8_ ottery_prf_chacha8_merged_
#define ottery_prf_chacha12_ ottery_prf_chacha12_merged_
#define ottery_prf_chacha20_ ottery_prf_chacha20_merged_
#endif

#endif
