; ModuleID = 'test7.cpp'
source_filename = "test7.cpp"
target datalayout = "e-p:32:32-i32:32"
target triple = "p2"

; Function Attrs: noinline norecurse nounwind optnone
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %x, align 4
  %0 = call i32 asm "add $0, #1", "=r"() #1, !srcloc !2
  store i32 %0, i32* %x, align 4
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git bec8e00fb7c08cc0cdaa1eefb16d144ebeb868a6)"}
!2 = !{i32 640}
