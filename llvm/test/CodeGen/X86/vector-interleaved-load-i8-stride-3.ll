; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-- -mattr=+avx2 | FileCheck --check-prefixes=AVX2,AVX2-SLOW %s
; RUN: llc < %s -mtriple=x86_64-- -mattr=+avx2,+fast-variable-crosslane-shuffle,+fast-variable-perlane-shuffle | FileCheck --check-prefixes=AVX2,AVX2-FAST %s
; RUN: llc < %s -mtriple=x86_64-- -mattr=+avx2,+fast-variable-perlane-shuffle | FileCheck --check-prefixes=AVX2,AVX2-FAST %s

; These patterns are produced by LoopVectorizer for interleaved loads.

define void @load_i8_stride3_vf2(<6 x i8>* %in.vec, <2 x i8>* %out.vec0, <2 x i8>* %out.vec1, <2 x i8>* %out.vec2) nounwind {
; AVX2-LABEL: load_i8_stride3_vf2:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX2-NEXT:    vpshufb {{.*#+}} xmm1 = xmm0[0,3,u,u,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm2 = xmm0[1,4,u,u,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[2,5,u,u,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpextrw $0, %xmm1, (%rsi)
; AVX2-NEXT:    vpextrw $0, %xmm2, (%rdx)
; AVX2-NEXT:    vpextrw $0, %xmm0, (%rcx)
; AVX2-NEXT:    retq
  %wide.vec = load <6 x i8>, <6 x i8>* %in.vec, align 32

  %strided.vec0 = shufflevector <6 x i8> %wide.vec, <6 x i8> poison, <2 x i32> <i32 0, i32 3>
  %strided.vec1 = shufflevector <6 x i8> %wide.vec, <6 x i8> poison, <2 x i32> <i32 1, i32 4>
  %strided.vec2 = shufflevector <6 x i8> %wide.vec, <6 x i8> poison, <2 x i32> <i32 2, i32 5>

  store <2 x i8> %strided.vec0, <2 x i8>* %out.vec0, align 32
  store <2 x i8> %strided.vec1, <2 x i8>* %out.vec1, align 32
  store <2 x i8> %strided.vec2, <2 x i8>* %out.vec2, align 32

  ret void
}

define void @load_i8_stride3_vf4(<12 x i8>* %in.vec, <4 x i8>* %out.vec0, <4 x i8>* %out.vec1, <4 x i8>* %out.vec2) nounwind {
; AVX2-LABEL: load_i8_stride3_vf4:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovdqa (%rdi), %xmm0
; AVX2-NEXT:    vpshufb {{.*#+}} xmm1 = xmm0[0,3,6,9,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm2 = xmm0[1,4,7,10,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[2,5,8,11,u,u,u,u,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vmovd %xmm1, (%rsi)
; AVX2-NEXT:    vmovd %xmm2, (%rdx)
; AVX2-NEXT:    vmovd %xmm0, (%rcx)
; AVX2-NEXT:    retq
  %wide.vec = load <12 x i8>, <12 x i8>* %in.vec, align 32

  %strided.vec0 = shufflevector <12 x i8> %wide.vec, <12 x i8> poison, <4 x i32> <i32 0, i32 3, i32 6, i32 9>
  %strided.vec1 = shufflevector <12 x i8> %wide.vec, <12 x i8> poison, <4 x i32> <i32 1, i32 4, i32 7, i32 10>
  %strided.vec2 = shufflevector <12 x i8> %wide.vec, <12 x i8> poison, <4 x i32> <i32 2, i32 5, i32 8, i32 11>

  store <4 x i8> %strided.vec0, <4 x i8>* %out.vec0, align 32
  store <4 x i8> %strided.vec1, <4 x i8>* %out.vec1, align 32
  store <4 x i8> %strided.vec2, <4 x i8>* %out.vec2, align 32

  ret void
}

define void @load_i8_stride3_vf8(<24 x i8>* %in.vec, <8 x i8>* %out.vec0, <8 x i8>* %out.vec1, <8 x i8>* %out.vec2) nounwind {
; AVX2-LABEL: load_i8_stride3_vf8:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovdqa (%rdi), %xmm0
; AVX2-NEXT:    vmovdqa 16(%rdi), %xmm1
; AVX2-NEXT:    vpshufb {{.*#+}} xmm2 = zero,zero,zero,zero,zero,zero,xmm1[2,5,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm3 = xmm0[0,3,6,9,12,15],zero,zero,xmm0[u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpor %xmm2, %xmm3, %xmm2
; AVX2-NEXT:    vpshufb {{.*#+}} xmm3 = zero,zero,zero,zero,zero,xmm1[0,3,6,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm4 = xmm0[1,4,7,10,13],zero,zero,zero,xmm0[u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpor %xmm3, %xmm4, %xmm3
; AVX2-NEXT:    vpshufb {{.*#+}} xmm1 = zero,zero,zero,zero,zero,xmm1[1,4,7,u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[2,5,8,11,14],zero,zero,zero,xmm0[u,u,u,u,u,u,u,u]
; AVX2-NEXT:    vpor %xmm1, %xmm0, %xmm0
; AVX2-NEXT:    vmovq %xmm2, (%rsi)
; AVX2-NEXT:    vmovq %xmm3, (%rdx)
; AVX2-NEXT:    vmovq %xmm0, (%rcx)
; AVX2-NEXT:    retq
  %wide.vec = load <24 x i8>, <24 x i8>* %in.vec, align 32

  %strided.vec0 = shufflevector <24 x i8> %wide.vec, <24 x i8> poison, <8 x i32> <i32 0, i32 3, i32 6, i32 9, i32 12, i32 15, i32 18, i32 21>
  %strided.vec1 = shufflevector <24 x i8> %wide.vec, <24 x i8> poison, <8 x i32> <i32 1, i32 4, i32 7, i32 10, i32 13, i32 16, i32 19, i32 22>
  %strided.vec2 = shufflevector <24 x i8> %wide.vec, <24 x i8> poison, <8 x i32> <i32 2, i32 5, i32 8, i32 11, i32 14, i32 17, i32 20, i32 23>

  store <8 x i8> %strided.vec0, <8 x i8>* %out.vec0, align 32
  store <8 x i8> %strided.vec1, <8 x i8>* %out.vec1, align 32
  store <8 x i8> %strided.vec2, <8 x i8>* %out.vec2, align 32

  ret void
}

define void @load_i8_stride3_vf16(<48 x i8>* %in.vec, <16 x i8>* %out.vec0, <16 x i8>* %out.vec1, <16 x i8>* %out.vec2) nounwind {
; AVX2-LABEL: load_i8_stride3_vf16:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovdqa (%rdi), %xmm0
; AVX2-NEXT:    vmovdqa 16(%rdi), %xmm1
; AVX2-NEXT:    vmovdqa 32(%rdi), %xmm2
; AVX2-NEXT:    vmovdqa {{.*#+}} xmm3 = [0,3,6,9,12,15,2,5,8,11,14,1,4,7,10,13]
; AVX2-NEXT:    vpshufb %xmm3, %xmm0, %xmm0
; AVX2-NEXT:    vpshufb %xmm3, %xmm1, %xmm1
; AVX2-NEXT:    vpshufb %xmm3, %xmm2, %xmm2
; AVX2-NEXT:    vpalignr {{.*#+}} xmm3 = xmm2[11,12,13,14,15],xmm0[0,1,2,3,4,5,6,7,8,9,10]
; AVX2-NEXT:    vpalignr {{.*#+}} xmm0 = xmm0[11,12,13,14,15],xmm1[0,1,2,3,4,5,6,7,8,9,10]
; AVX2-NEXT:    vpalignr {{.*#+}} xmm1 = xmm1[11,12,13,14,15],xmm2[0,1,2,3,4,5,6,7,8,9,10]
; AVX2-NEXT:    vpalignr {{.*#+}} xmm2 = xmm3[11,12,13,14,15],xmm1[0,1,2,3,4,5,6,7,8,9,10]
; AVX2-NEXT:    vmovdqa {{.*#+}} xmm4 = [255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0]
; AVX2-NEXT:    vpblendvb %xmm4, %xmm0, %xmm1, %xmm1
; AVX2-NEXT:    vpalignr {{.*#+}} xmm0 = xmm0[11,12,13,14,15],xmm3[0,1,2,3,4,5,6,7,8,9,10]
; AVX2-NEXT:    vpalignr {{.*#+}} xmm0 = xmm0[10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9]
; AVX2-NEXT:    vmovdqa %xmm0, (%rsi)
; AVX2-NEXT:    vmovdqa %xmm1, (%rdx)
; AVX2-NEXT:    vmovdqa %xmm2, (%rcx)
; AVX2-NEXT:    retq
  %wide.vec = load <48 x i8>, <48 x i8>* %in.vec, align 32

  %strided.vec0 = shufflevector <48 x i8> %wide.vec, <48 x i8> poison, <16 x i32> <i32 0, i32 3, i32 6, i32 9, i32 12, i32 15, i32 18, i32 21, i32 24, i32 27, i32 30, i32 33, i32 36, i32 39, i32 42, i32 45>
  %strided.vec1 = shufflevector <48 x i8> %wide.vec, <48 x i8> poison, <16 x i32> <i32 1, i32 4, i32 7, i32 10, i32 13, i32 16, i32 19, i32 22, i32 25, i32 28, i32 31, i32 34, i32 37, i32 40, i32 43, i32 46>
  %strided.vec2 = shufflevector <48 x i8> %wide.vec, <48 x i8> poison, <16 x i32> <i32 2, i32 5, i32 8, i32 11, i32 14, i32 17, i32 20, i32 23, i32 26, i32 29, i32 32, i32 35, i32 38, i32 41, i32 44, i32 47>

  store <16 x i8> %strided.vec0, <16 x i8>* %out.vec0, align 32
  store <16 x i8> %strided.vec1, <16 x i8>* %out.vec1, align 32
  store <16 x i8> %strided.vec2, <16 x i8>* %out.vec2, align 32

  ret void
}

define void @load_i8_stride3_vf32(<96 x i8>* %in.vec, <32 x i8>* %out.vec0, <32 x i8>* %out.vec1, <32 x i8>* %out.vec2) nounwind {
; AVX2-LABEL: load_i8_stride3_vf32:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovdqa (%rdi), %xmm0
; AVX2-NEXT:    vmovdqa 16(%rdi), %xmm1
; AVX2-NEXT:    vmovdqa 32(%rdi), %xmm2
; AVX2-NEXT:    vinserti128 $1, 48(%rdi), %ymm0, %ymm0
; AVX2-NEXT:    vinserti128 $1, 64(%rdi), %ymm1, %ymm1
; AVX2-NEXT:    vinserti128 $1, 80(%rdi), %ymm2, %ymm2
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm3 = [0,3,6,9,12,15,2,5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,5,8,11,14,1,4,7,10,13]
; AVX2-NEXT:    vpshufb %ymm3, %ymm0, %ymm0
; AVX2-NEXT:    vpshufb %ymm3, %ymm1, %ymm1
; AVX2-NEXT:    vpshufb %ymm3, %ymm2, %ymm2
; AVX2-NEXT:    vpalignr {{.*#+}} ymm3 = ymm2[11,12,13,14,15],ymm0[0,1,2,3,4,5,6,7,8,9,10],ymm2[27,28,29,30,31],ymm0[16,17,18,19,20,21,22,23,24,25,26]
; AVX2-NEXT:    vpalignr {{.*#+}} ymm0 = ymm0[11,12,13,14,15],ymm1[0,1,2,3,4,5,6,7,8,9,10],ymm0[27,28,29,30,31],ymm1[16,17,18,19,20,21,22,23,24,25,26]
; AVX2-NEXT:    vpalignr {{.*#+}} ymm1 = ymm1[11,12,13,14,15],ymm2[0,1,2,3,4,5,6,7,8,9,10],ymm1[27,28,29,30,31],ymm2[16,17,18,19,20,21,22,23,24,25,26]
; AVX2-NEXT:    vpalignr {{.*#+}} ymm2 = ymm3[11,12,13,14,15],ymm1[0,1,2,3,4,5,6,7,8,9,10],ymm3[27,28,29,30,31],ymm1[16,17,18,19,20,21,22,23,24,25,26]
; AVX2-NEXT:    vmovdqa {{.*#+}} ymm4 = [255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0]
; AVX2-NEXT:    vpblendvb %ymm4, %ymm0, %ymm1, %ymm1
; AVX2-NEXT:    vpalignr {{.*#+}} ymm0 = ymm0[11,12,13,14,15],ymm3[0,1,2,3,4,5,6,7,8,9,10],ymm0[27,28,29,30,31],ymm3[16,17,18,19,20,21,22,23,24,25,26]
; AVX2-NEXT:    vpalignr {{.*#+}} ymm0 = ymm0[10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,26,27,28,29,30,31,16,17,18,19,20,21,22,23,24,25]
; AVX2-NEXT:    vmovdqa %ymm0, (%rsi)
; AVX2-NEXT:    vmovdqa %ymm1, (%rdx)
; AVX2-NEXT:    vmovdqa %ymm2, (%rcx)
; AVX2-NEXT:    vzeroupper
; AVX2-NEXT:    retq
  %wide.vec = load <96 x i8>, <96 x i8>* %in.vec, align 32

  %strided.vec0 = shufflevector <96 x i8> %wide.vec, <96 x i8> poison, <32 x i32> <i32 0, i32 3, i32 6, i32 9, i32 12, i32 15, i32 18, i32 21, i32 24, i32 27, i32 30, i32 33, i32 36, i32 39, i32 42, i32 45, i32 48, i32 51, i32 54, i32 57, i32 60, i32 63, i32 66, i32 69, i32 72, i32 75, i32 78, i32 81, i32 84, i32 87, i32 90, i32 93>
  %strided.vec1 = shufflevector <96 x i8> %wide.vec, <96 x i8> poison, <32 x i32> <i32 1, i32 4, i32 7, i32 10, i32 13, i32 16, i32 19, i32 22, i32 25, i32 28, i32 31, i32 34, i32 37, i32 40, i32 43, i32 46, i32 49, i32 52, i32 55, i32 58, i32 61, i32 64, i32 67, i32 70, i32 73, i32 76, i32 79, i32 82, i32 85, i32 88, i32 91, i32 94>
  %strided.vec2 = shufflevector <96 x i8> %wide.vec, <96 x i8> poison, <32 x i32> <i32 2, i32 5, i32 8, i32 11, i32 14, i32 17, i32 20, i32 23, i32 26, i32 29, i32 32, i32 35, i32 38, i32 41, i32 44, i32 47, i32 50, i32 53, i32 56, i32 59, i32 62, i32 65, i32 68, i32 71, i32 74, i32 77, i32 80, i32 83, i32 86, i32 89, i32 92, i32 95>

  store <32 x i8> %strided.vec0, <32 x i8>* %out.vec0, align 32
  store <32 x i8> %strided.vec1, <32 x i8>* %out.vec1, align 32
  store <32 x i8> %strided.vec2, <32 x i8>* %out.vec2, align 32

  ret void
}
