#pragma
#ifndef __RISCV_SSE_SUPPORT_HPP__
#define __RISCV_SSE_SUPPORT_HPP__
#if defined(__riscv)

   typedef uint16_t __attribute__((vector_size(8))) __m128i;

   __m128i _mm_loadl_epi64(__m128i const* mem_addr) {
      __m128i ret;
      memcpy(&ret, &mem_addr, sizeof(uint64_t));
      return ret;
   }

   __m128i _mm_load_si128 (__m128i const* mem_addr) {
      __m128i ret;
      memcpy(&ret, &mem_addr, sizeof(__m128i));
      return ret;
   }

   __m128i _mm_loadu_si128 (__m128i const* mem_addr) {
      __m128i ret;
      memcpy(&ret, &mem_addr, sizeof(__m128i));
      return ret;
   }

   void _mm_storel_epi64 (__m128i* mem_addr, __m128i a) {
      memcpy(&a, &mem_addr, sizeof(uint64_t));
   }

   void _mm_store_si128 (__m128i* mem_addr, __m128i a) {
      memcpy(&mem_addr, &a, sizeof(__m128i));
   }

   void _mm_storeu_si128 (__m128i* mem_addr, __m128i a) {
      memcpy(&mem_addr, &a, sizeof(__m128i));
   }

   void _mm_stream_si128 (void* mem_addr, __m128i a) {
      memcpy(&mem_addr, &a, sizeof(__m128i));
   }

   __m128i _mm_stream_load_si128 (void* mem_addr) {
      __m128i ret;
      memcpy(&ret, &mem_addr, sizeof(__m128i));
      return ret;
   }

   #define RISCV_FENCE(p, s) \
        __asm__ __volatile__ ("fence " #p "," #s : : : "memory")

   void _mm_sfence() {
      RISCV_FENCE(rw,rw);
   }
#else
#error "compiling for rv64g (riscv64) but compiler architecture macro undefined"
#endif
#endif
