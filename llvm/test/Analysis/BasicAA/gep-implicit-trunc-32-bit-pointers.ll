; RUN: opt -basic-aa -aa-eval -print-all-alias-modref-info -disable-output %s 2>&1 | FileCheck %s

target datalayout = "p:32:32:32"

; Test cases with i64 bit GEP indices that will get truncated implicitly to 32
; bit due to the datalayout.

declare void @llvm.assume(i1)

define void @mustalias_overflow_in_32_bit_constants(i8* %ptr) {
; CHECK-LABEL: Function: mustalias_overflow_in_32_bit_constants: 3 pointers, 0 call sites
; CHECK-NEXT:    MustAlias: i8* %gep.1, i8* %ptr
; CHECK-NEXT:    MustAlias:    i8* %gep.2, i8* %ptr
; CHECK-NEXT:    MustAlias:    i8* %gep.1, i8* %gep.2
;
  %gep.1 = getelementptr i8, i8* %ptr, i64 4294967296
  store i8 0, i8* %gep.1
  %gep.2 = getelementptr i8, i8* %ptr, i64 0
  store i8 1, i8* %gep.2
  ret void
}

define void @noalias_overflow_in_32_bit_constants(i8* %ptr) {
; CHECK-LABEL: Function: noalias_overflow_in_32_bit_constants: 3 pointers, 0 call sites
; CHECK-NEXT:    MustAlias: i8* %gep.1, i8* %ptr
; CHECK-NEXT:    NoAlias:  i8* %gep.2, i8* %ptr
; CHECK-NEXT:    NoAlias:  i8* %gep.1, i8* %gep.2
;
  %gep.1 = getelementptr i8, i8* %ptr, i64 4294967296
  store i8 0, i8* %gep.1
  %gep.2 = getelementptr i8, i8* %ptr, i64 1
  store i8 1, i8* %gep.2
  ret void
}

; FIXME: Currently we incorrectly determine NoAlias for %gep.1 and %gep.2. The
; GEP indices get implicitly truncated to 32 bit, so multiples of 2^32
; (=4294967296) will be 0.
; See https://alive2.llvm.org/ce/z/HHjQgb.
define void @mustalias_overflow_in_32_bit_add_mul_gep(i8* %ptr, i64 %i) {
; CHECK-LABEL: Function: mustalias_overflow_in_32_bit_add_mul_gep: 3 pointers, 1 call sites
; CHECK-NEXT:    NoAlias:  i8* %gep.1, i8* %ptr
; CHECK-NEXT:    NoAlias:  i8* %gep.2, i8* %ptr
; CHECK-NEXT:    NoAlias:  i8* %gep.1, i8* %gep.2
;
  %s.1 = icmp sgt i64 %i, 0
  call void @llvm.assume(i1 %s.1)

  %mul = mul nuw nsw i64 %i, 4294967296
  %add = add nuw nsw i64 %mul, %i
  %gep.1 = getelementptr i8, i8* %ptr, i64 %add
  store i8 0, i8* %gep.1
  %gep.2 = getelementptr i8, i8* %ptr, i64 %i
  store i8 1, i8* %gep.2
  ret void
}
