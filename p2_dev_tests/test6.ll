; ModuleID = 'test6.cpp'
source_filename = "test6.cpp"
target datalayout = "e-p:32:32-i32:32"
target triple = "p2"

@x = dso_local global i32 0, align 4

; Function Attrs: noinline norecurse nounwind optnone
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  call void asm sideeffect "dirh #56", ""() #1, !srcloc !2
  call void asm sideeffect "augd #156250", ""() #1, !srcloc !3
  call void asm sideeffect "waitx #0", ""() #1, !srcloc !4
  call void asm sideeffect "dirl #56", ""() #1, !srcloc !5
  call void asm sideeffect "augd #156250", ""() #1, !srcloc !6
  call void asm sideeffect "waitx #0", ""() #1, !srcloc !7
  br label %while.body
}

attributes #0 = { noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git e9f9d355c78c353223bb80946791140aee671499)"}
!2 = !{i32 292}
!3 = !{i32 317}
!4 = !{i32 346}
!5 = !{i32 371}
!6 = !{i32 396}
!7 = !{i32 425}
