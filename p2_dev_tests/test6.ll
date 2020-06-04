; ModuleID = 'test6.cpp'
source_filename = "test6.cpp"
target datalayout = "e-p:32:32-i32:32"
target triple = "p2"

; Function Attrs: noinline nounwind optnone
define dso_local void @_Z5blinkv() #0 {
entry:
  call void asm sideeffect "outh #56", ""() #2, !srcloc !2
  call void asm sideeffect "augd #39062", ""() #2, !srcloc !3
  call void asm sideeffect "waitx #256", ""() #2, !srcloc !4
  call void asm sideeffect "outl #56", ""() #2, !srcloc !5
  call void asm sideeffect "augd #39062", ""() #2, !srcloc !6
  call void asm sideeffect "waitx #256", ""() #2, !srcloc !7
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone
define dso_local i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void asm sideeffect "dirh #56", ""() #2, !srcloc !8
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  call void @_Z5blinkv()
  br label %while.body
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git bec8e00fb7c08cc0cdaa1eefb16d144ebeb868a6)"}
!2 = !{i32 262}
!3 = !{i32 283}
!4 = !{i32 307}
!5 = !{i32 330}
!6 = !{i32 351}
!7 = !{i32 375}
!8 = !{i32 415}
