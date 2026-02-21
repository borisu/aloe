; ModuleID = 'a.alo'
source_filename = "a.alo"

define i32 @xmain() !dbg !5 {
entry:
  ret i32 0, !dbg !9
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.dbg.cu = !{!3}

!0 = !{i32 2, !"CodeView", i32 1}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = distinct !DICompileUnit(language: DW_LANG_C, file: !4, producer: "aloe-frontend", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!4 = !DIFile(filename: "a.alo", directory: "D:\\Projects\\aloe\\aloe_dbg")
!5 = distinct !DISubprogram(name: "xmain", linkageName: "xmain", scope: !4, file: !4, line: 1, type: !6, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !3)
!6 = !DISubroutineType(types: !7)
!7 = !{!8}
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DILocation(line: 1, column: 1, scope: !5)
