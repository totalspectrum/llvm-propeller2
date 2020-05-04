; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=x86_64-unknown-linux-gnu -mattr=-bmi < %s | FileCheck %s --check-prefix=CHECK-NOBMI
; RUN: llc -mtriple=x86_64-unknown-linux-gnu -mattr=+bmi < %s | FileCheck %s --check-prefix=CHECK-BMI

; Optimize (x > -1) to (x >= 0) etc.
; Optimize (cmp (add / sub), 0): eliminate the subs used to update flag
;   for comparison only
; rdar://10233472

define i32 @t1(i64 %a) {
; CHECK-NOBMI-LABEL: t1:
; CHECK-NOBMI:       # %bb.0:
; CHECK-NOBMI-NEXT:    xorl %eax, %eax
; CHECK-NOBMI-NEXT:    testq %rdi, %rdi
; CHECK-NOBMI-NEXT:    setns %al
; CHECK-NOBMI-NEXT:    retq
;
; CHECK-BMI-LABEL: t1:
; CHECK-BMI:       # %bb.0:
; CHECK-BMI-NEXT:    xorl %eax, %eax
; CHECK-BMI-NEXT:    testq %rdi, %rdi
; CHECK-BMI-NEXT:    setns %al
; CHECK-BMI-NEXT:    retq
  %cmp = icmp sgt i64 %a, -1
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}
