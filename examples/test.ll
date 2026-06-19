=========================LLVM IR===================
; ModuleID = 'VixModule'
source_filename = "VixModule"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%BrainfuckState = type { ptr, i32 }

@str_lit.3 = private unnamed_addr constant [32 x i8] c"brainfuck: unmatched '[' at %d\0A\00", align 1
@str_lit.4 = private unnamed_addr constant [32 x i8] c"brainfuck: unmatched ']' at %d\0A\00", align 1
@str = private unnamed_addr constant [25 x i8] c"usage: brainfuck PROGRAM\00", align 1
@str.5 = private unnamed_addr constant [46 x i8] c"example: brainfuck \22++++++++[>++++++++<-]>+.\22\00", align 1
@str.6 = private unnamed_addr constant [44 x i8] c"brainfuck: tape pointer moved before cell 0\00", align 1

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr noundef readonly captures(none), ...) local_unnamed_addr #0

; Function Attrs: nofree nounwind
declare noundef i32 @getchar() local_unnamed_addr #0

; Function Attrs: nofree nounwind
declare noundef i32 @putchar(i32 noundef) local_unnamed_addr #0

; Function Attrs: nofree nounwind
define noundef i32 @usage() local_unnamed_addr #0 {
entry:
  %puts = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  %puts2 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.5)
  ret i32 2
}

; Function Attrs: nounwind memory(readwrite, target_mem0: none, target_mem1: none)
define void @init_state(ptr noalias writeonly sret(%BrainfuckState) captures(none) %__sret) local_unnamed_addr #1 {
entry:
  br label %forbody

forbody:                                          ; preds = %entry, %tape_old_len_merge
  %tape.012 = phi ptr [ null, %entry ], [ %push_new_data_i8, %tape_old_len_merge ]
  %i.011 = phi i32 [ 0, %entry ], [ %inc, %tape_old_len_merge ]
  %tape_old_len_is_null = icmp eq ptr %tape.012, null
  br i1 %tape_old_len_is_null, label %tape_old_len_merge, label %tape_old_len_nonnull

forcont:                                          ; preds = %tape_old_len_merge
  %ret_sret_val.fca.0.insert = insertvalue %BrainfuckState poison, ptr %push_new_data_i8, 0
  %ret_sret_val.fca.1.insert = insertvalue %BrainfuckState %ret_sret_val.fca.0.insert, i32 0, 1
  store %BrainfuckState %ret_sret_val.fca.1.insert, ptr %__sret, align 8
  ret void

tape_old_len_nonnull:                             ; preds = %forbody
  %0 = ptrtoint ptr %tape.012 to i64
  %1 = add i64 %0, -8
  %2 = inttoptr i64 %1 to ptr
  %tape_old_len_loaded = load i32, ptr %2, align 4
  br label %tape_old_len_merge

tape_old_len_merge:                               ; preds = %forbody, %tape_old_len_nonnull
  %push_base_ptr = phi ptr [ %2, %tape_old_len_nonnull ], [ null, %forbody ]
  %tape_old_len = phi i32 [ %tape_old_len_loaded, %tape_old_len_nonnull ], [ 0, %forbody ]
  %tape__len_new = add i32 %tape_old_len, 1
  %3 = sext i32 %tape__len_new to i64
  %push_data_bytes = shl nsw i64 %3, 2
  %push_total_bytes = add nsw i64 %push_data_bytes, 8
  %push_realloc = tail call ptr @realloc(ptr %push_base_ptr, i64 %push_total_bytes)
  store i32 %tape__len_new, ptr %push_realloc, align 4
  %push_new_data_i8 = getelementptr inbounds nuw i8, ptr %push_realloc, i64 8
  %4 = sext i32 %tape_old_len to i64
  %push_dst_ptr = getelementptr inbounds i32, ptr %push_new_data_i8, i64 %4
  store i32 0, ptr %push_dst_ptr, align 4
  %inc = add nuw nsw i32 %i.011, 1
  %exitcond.not = icmp eq i32 %inc, 30000
  br i1 %exitcond.not, label %forcont, label %forbody
}

; Function Attrs: mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite)
declare noalias noundef ptr @realloc(ptr allocptr captures(none), i64 noundef) local_unnamed_addr #2

; Function Attrs: nounwind memory(readwrite, target_mem0: none, target_mem1: none)
define noundef i32 @grow_tape(ptr captures(none) %state, i32 %min_size) local_unnamed_addr #1 {
entry:
  %min_size.fr = freeze i32 %min_size
  %tape4.pr = load ptr, ptr %state, align 8
  %letmp.not = icmp slt i32 %min_size.fr, 0
  br i1 %letmp.not, label %entry.split.us, label %whilecond

entry.split.us:                                   ; preds = %entry
  %member_arr_len_is_null.us15 = icmp eq ptr %tape4.pr, null
  br i1 %member_arr_len_is_null.us15, label %whilecont, label %member_arr_len_merge.thread.us.preheader

member_arr_len_merge.thread.us.preheader:         ; preds = %entry.split.us
  %0 = ptrtoint ptr %tape4.pr to i64
  %1 = add i64 %0, -8
  %2 = inttoptr i64 %1 to ptr
  %member_arr_len_loaded.us25 = load i32, ptr %2, align 4
  %letmp.not14.us26 = icmp sgt i32 %member_arr_len_loaded.us25, %min_size.fr
  br i1 %letmp.not14.us26, label %whilecont, label %__tmp_push_94181924030560_old_len_nonnull.us

__tmp_push_94181924030560_old_len_nonnull.us:     ; preds = %member_arr_len_merge.thread.us.preheader, %__tmp_push_94181924030560_old_len_nonnull.us
  %member_arr_len_loaded.us27 = phi i32 [ %member_arr_len_loaded.us, %__tmp_push_94181924030560_old_len_nonnull.us ], [ %member_arr_len_loaded.us25, %member_arr_len_merge.thread.us.preheader ]
  %3 = phi ptr [ %8, %__tmp_push_94181924030560_old_len_nonnull.us ], [ %2, %member_arr_len_merge.thread.us.preheader ]
  %__tmp_push_94181924030560__len_new.us = add nsw i32 %member_arr_len_loaded.us27, 1
  %4 = sext i32 %__tmp_push_94181924030560__len_new.us to i64
  %push_data_bytes.us = shl nsw i64 %4, 2
  %push_total_bytes.us = add nsw i64 %push_data_bytes.us, 8
  %push_realloc.us = tail call ptr @realloc(ptr nonnull %3, i64 %push_total_bytes.us)
  store i32 %__tmp_push_94181924030560__len_new.us, ptr %push_realloc.us, align 4
  %push_new_data_i8.us = getelementptr inbounds nuw i8, ptr %push_realloc.us, i64 8
  %5 = sext i32 %member_arr_len_loaded.us27 to i64
  %push_dst_ptr.us = getelementptr inbounds i32, ptr %push_new_data_i8.us, i64 %5
  store i32 0, ptr %push_dst_ptr.us, align 4
  store ptr %push_new_data_i8.us, ptr %state, align 8
  %6 = ptrtoint ptr %push_new_data_i8.us to i64
  %7 = add i64 %6, -8
  %8 = inttoptr i64 %7 to ptr
  %member_arr_len_loaded.us = load i32, ptr %8, align 4
  %letmp.not14.us = icmp sgt i32 %member_arr_len_loaded.us, %min_size.fr
  br i1 %letmp.not14.us, label %whilecont, label %__tmp_push_94181924030560_old_len_nonnull.us

whilecond:                                        ; preds = %entry, %__tmp_push_94181924030560_old_len_merge
  %tape4 = phi ptr [ %push_new_data_i8, %__tmp_push_94181924030560_old_len_merge ], [ %tape4.pr, %entry ]
  %member_arr_len_is_null = icmp eq ptr %tape4, null
  br i1 %member_arr_len_is_null, label %__tmp_push_94181924030560_old_len_merge, label %member_arr_len_merge.thread

member_arr_len_merge.thread:                      ; preds = %whilecond
  %9 = ptrtoint ptr %tape4 to i64
  %10 = add i64 %9, -8
  %11 = inttoptr i64 %10 to ptr
  %member_arr_len_loaded = load i32, ptr %11, align 4
  %letmp.not14 = icmp sgt i32 %member_arr_len_loaded, %min_size.fr
  br i1 %letmp.not14, label %whilecont, label %__tmp_push_94181924030560_old_len_merge

whilecont:                                        ; preds = %member_arr_len_merge.thread, %__tmp_push_94181924030560_old_len_nonnull.us, %member_arr_len_merge.thread.us.preheader, %entry.split.us
  ret i32 0

__tmp_push_94181924030560_old_len_merge:          ; preds = %member_arr_len_merge.thread, %whilecond
  %push_base_ptr = phi ptr [ null, %whilecond ], [ %11, %member_arr_len_merge.thread ]
  %__tmp_push_94181924030560_old_len = phi i32 [ 0, %whilecond ], [ %member_arr_len_loaded, %member_arr_len_merge.thread ]
  %__tmp_push_94181924030560__len_new = add i32 %__tmp_push_94181924030560_old_len, 1
  %12 = sext i32 %__tmp_push_94181924030560__len_new to i64
  %push_data_bytes = shl nsw i64 %12, 2
  %push_total_bytes = add nsw i64 %push_data_bytes, 8
  %push_realloc = tail call ptr @realloc(ptr %push_base_ptr, i64 %push_total_bytes)
  store i32 %__tmp_push_94181924030560__len_new, ptr %push_realloc, align 4
  %push_new_data_i8 = getelementptr inbounds nuw i8, ptr %push_realloc, i64 8
  %13 = sext i32 %__tmp_push_94181924030560_old_len to i64
  %push_dst_ptr = getelementptr inbounds i32, ptr %push_new_data_i8, i64 %13
  store i32 0, ptr %push_dst_ptr, align 4
  store ptr %push_new_data_i8, ptr %state, align 8
  br label %whilecond
}

; Function Attrs: nounwind memory(readwrite, target_mem0: none, target_mem1: none)
define noundef i32 @move_right(ptr captures(none) %state) local_unnamed_addr #1 {
entry:
  %data_ptr = getelementptr inbounds nuw i8, ptr %state, i64 8
  %data_ptr5 = load i32, ptr %data_ptr, align 4
  %data_ptr5.fr = freeze i32 %data_ptr5
  %addtmp = add i32 %data_ptr5.fr, 1
  store i32 %addtmp, ptr %data_ptr, align 4
  %tape10 = load ptr, ptr %state, align 8
  %member_arr_len_is_null = icmp eq ptr %tape10, null
  br i1 %member_arr_len_is_null, label %member_arr_len_merge, label %member_arr_len_nonnull

member_arr_len_nonnull:                           ; preds = %entry
  %0 = ptrtoint ptr %tape10 to i64
  %1 = add i64 %0, -8
  %2 = inttoptr i64 %1 to ptr
  %member_arr_len_loaded = load i32, ptr %2, align 4
  br label %member_arr_len_merge

member_arr_len_merge:                             ; preds = %entry, %member_arr_len_nonnull
  %member_arr_len = phi i32 [ %member_arr_len_loaded, %member_arr_len_nonnull ], [ 0, %entry ]
  %getmp.not = icmp slt i32 %addtmp, %member_arr_len
  br i1 %getmp.not, label %common.ret, label %then

common.ret:                                       ; preds = %member_arr_len_merge.thread.i, %__tmp_push_94181924030560_old_len_nonnull.us.i, %member_arr_len_merge.thread.us.i.preheader, %entry.split.us.i, %member_arr_len_merge
  ret i32 0

then:                                             ; preds = %member_arr_len_merge
  %letmp.not.i = icmp slt i32 %addtmp, 0
  br i1 %letmp.not.i, label %entry.split.us.i, label %whilecond.i

entry.split.us.i:                                 ; preds = %then
  br i1 %member_arr_len_is_null, label %common.ret, label %member_arr_len_merge.thread.us.i.preheader

member_arr_len_merge.thread.us.i.preheader:       ; preds = %entry.split.us.i
  %3 = ptrtoint ptr %tape10 to i64
  %4 = add i64 %3, -8
  %5 = inttoptr i64 %4 to ptr
  %member_arr_len_loaded.us.i16 = load i32, ptr %5, align 4
  %letmp.not14.us.i17 = icmp sgt i32 %member_arr_len_loaded.us.i16, %addtmp
  br i1 %letmp.not14.us.i17, label %common.ret, label %__tmp_push_94181924030560_old_len_nonnull.us.i

__tmp_push_94181924030560_old_len_nonnull.us.i:   ; preds = %member_arr_len_merge.thread.us.i.preheader, %__tmp_push_94181924030560_old_len_nonnull.us.i
  %member_arr_len_loaded.us.i18 = phi i32 [ %member_arr_len_loaded.us.i, %__tmp_push_94181924030560_old_len_nonnull.us.i ], [ %member_arr_len_loaded.us.i16, %member_arr_len_merge.thread.us.i.preheader ]
  %6 = phi ptr [ %11, %__tmp_push_94181924030560_old_len_nonnull.us.i ], [ %5, %member_arr_len_merge.thread.us.i.preheader ]
  %__tmp_push_94181924030560__len_new.us.i = add nsw i32 %member_arr_len_loaded.us.i18, 1
  %7 = sext i32 %__tmp_push_94181924030560__len_new.us.i to i64
  %push_data_bytes.us.i = shl nsw i64 %7, 2
  %push_total_bytes.us.i = add nsw i64 %push_data_bytes.us.i, 8
  %push_realloc.us.i = tail call ptr @realloc(ptr nonnull %6, i64 %push_total_bytes.us.i)
  store i32 %__tmp_push_94181924030560__len_new.us.i, ptr %push_realloc.us.i, align 4
  %push_new_data_i8.us.i = getelementptr inbounds nuw i8, ptr %push_realloc.us.i, i64 8
  %8 = sext i32 %member_arr_len_loaded.us.i18 to i64
  %push_dst_ptr.us.i = getelementptr inbounds i32, ptr %push_new_data_i8.us.i, i64 %8
  store i32 0, ptr %push_dst_ptr.us.i, align 4
  store ptr %push_new_data_i8.us.i, ptr %state, align 8
  %9 = ptrtoint ptr %push_new_data_i8.us.i to i64
  %10 = add i64 %9, -8
  %11 = inttoptr i64 %10 to ptr
  %member_arr_len_loaded.us.i = load i32, ptr %11, align 4
  %letmp.not14.us.i = icmp sgt i32 %member_arr_len_loaded.us.i, %addtmp
  br i1 %letmp.not14.us.i, label %common.ret, label %__tmp_push_94181924030560_old_len_nonnull.us.i

whilecond.i:                                      ; preds = %then, %__tmp_push_94181924030560_old_len_merge.i
  %tape4.i = phi ptr [ %push_new_data_i8.i, %__tmp_push_94181924030560_old_len_merge.i ], [ %tape10, %then ]
  %member_arr_len_is_null.i = icmp eq ptr %tape4.i, null
  br i1 %member_arr_len_is_null.i, label %__tmp_push_94181924030560_old_len_merge.i, label %member_arr_len_merge.thread.i

member_arr_len_merge.thread.i:                    ; preds = %whilecond.i
  %12 = ptrtoint ptr %tape4.i to i64
  %13 = add i64 %12, -8
  %14 = inttoptr i64 %13 to ptr
  %member_arr_len_loaded.i = load i32, ptr %14, align 4
  %letmp.not14.i = icmp sgt i32 %member_arr_len_loaded.i, %addtmp
  br i1 %letmp.not14.i, label %common.ret, label %__tmp_push_94181924030560_old_len_merge.i

__tmp_push_94181924030560_old_len_merge.i:        ; preds = %member_arr_len_merge.thread.i, %whilecond.i
  %push_base_ptr.i = phi ptr [ null, %whilecond.i ], [ %14, %member_arr_len_merge.thread.i ]
  %__tmp_push_94181924030560_old_len.i = phi i32 [ 0, %whilecond.i ], [ %member_arr_len_loaded.i, %member_arr_len_merge.thread.i ]
  %__tmp_push_94181924030560__len_new.i = add i32 %__tmp_push_94181924030560_old_len.i, 1
  %15 = sext i32 %__tmp_push_94181924030560__len_new.i to i64
  %push_data_bytes.i = shl nsw i64 %15, 2
  %push_total_bytes.i = add nsw i64 %push_data_bytes.i, 8
  %push_realloc.i = tail call ptr @realloc(ptr %push_base_ptr.i, i64 %push_total_bytes.i)
  store i32 %__tmp_push_94181924030560__len_new.i, ptr %push_realloc.i, align 4
  %push_new_data_i8.i = getelementptr inbounds nuw i8, ptr %push_realloc.i, i64 8
  %16 = sext i32 %__tmp_push_94181924030560_old_len.i to i64
  %push_dst_ptr.i = getelementptr inbounds i32, ptr %push_new_data_i8.i, i64 %16
  store i32 0, ptr %push_dst_ptr.i, align 4
  store ptr %push_new_data_i8.i, ptr %state, align 8
  br label %whilecond.i
}

; Function Attrs: nofree nounwind
define range(i32 0, 2) i32 @move_left(ptr captures(none) %state) local_unnamed_addr #0 {
entry:
  %data_ptr = getelementptr inbounds nuw i8, ptr %state, i64 8
  %data_ptr3 = load i32, ptr %data_ptr, align 4
  %eqtmp = icmp eq i32 %data_ptr3, 0
  br i1 %eqtmp, label %then, label %ifcont

common.ret:                                       ; preds = %ifcont, %then
  %common.ret.op = phi i32 [ 1, %then ], [ 0, %ifcont ]
  ret i32 %common.ret.op

then:                                             ; preds = %entry
  %puts = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.6)
  br label %common.ret

ifcont:                                           ; preds = %entry
  %subtmp = add i32 %data_ptr3, -1
  store i32 %subtmp, ptr %data_ptr, align 4
  br label %common.ret
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none)
define void @inc_cell(ptr readonly captures(none) %state) local_unnamed_addr #3 {
entry:
  %data_ptr = getelementptr inbounds nuw i8, ptr %state, i64 8
  %tape5 = load ptr, ptr %state, align 8
  %idx_ptr_is_null2 = icmp eq ptr %tape5, null
  %data_ptr15.pre = load i32, ptr %data_ptr, align 8
  %.pre = sext i32 %data_ptr15.pre to i64
  br i1 %idx_ptr_is_null2, label %else, label %idx_cont2

idx_cont2:                                        ; preds = %entry
  %arr_index_ptr3 = getelementptr inbounds i32, ptr %tape5, i64 %.pre
  %arr_index_load3 = load i32, ptr %arr_index_ptr3, align 4
  %eqtmp = icmp eq i32 %arr_index_load3, 255
  br i1 %eqtmp, label %then, label %else

then:                                             ; preds = %idx_cont2
  %arr_index_ptr = getelementptr inbounds i8, ptr %tape5, i64 %.pre
  store i8 0, ptr %arr_index_ptr, align 1
  br label %func_end

else:                                             ; preds = %entry, %idx_cont2
  %idx_safe_val224 = phi i32 [ %arr_index_load3, %idx_cont2 ], [ 0, %entry ]
  %arr_index_ptr19 = getelementptr inbounds i8, ptr %tape5, i64 %.pre
  %0 = trunc i32 %idx_safe_val224 to i8
  %icast = add i8 %0, 1
  store i8 %icast, ptr %arr_index_ptr19, align 1
  br label %func_end

func_end:                                         ; preds = %then, %else
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none)
define void @dec_cell(ptr readonly captures(none) %state) local_unnamed_addr #3 {
entry:
  %data_ptr = getelementptr inbounds nuw i8, ptr %state, i64 8
  %tape5 = load ptr, ptr %state, align 8
  %idx_ptr_is_null2 = icmp eq ptr %tape5, null
  %data_ptr9.pre = load i32, ptr %data_ptr, align 8
  %.pre = sext i32 %data_ptr9.pre to i64
  br i1 %idx_ptr_is_null2, label %then, label %idx_cont2

idx_cont2:                                        ; preds = %entry
  %arr_index_ptr3 = getelementptr inbounds i32, ptr %tape5, i64 %.pre
  %arr_index_load3 = load i32, ptr %arr_index_ptr3, align 4
  %eqtmp = icmp eq i32 %arr_index_load3, 0
  br i1 %eqtmp, label %then, label %else

then:                                             ; preds = %entry, %idx_cont2
  %arr_index_ptr = getelementptr inbounds i8, ptr %tape5, i64 %.pre
  store i8 -1, ptr %arr_index_ptr, align 1
  br label %func_end

else:                                             ; preds = %idx_cont2
  %arr_index_ptr19 = getelementptr inbounds i8, ptr %tape5, i64 %.pre
  %0 = trunc i32 %arr_index_load3 to i8
  %icast = add i8 %0, -1
  store i8 %icast, ptr %arr_index_ptr19, align 1
  br label %func_end

func_end:                                         ; preds = %then, %else
  ret void
}

; Function Attrs: nofree nounwind
define void @read_cell(ptr readonly captures(none) %state) local_unnamed_addr #0 {
entry:
  %calltmp = tail call i32 @getchar()
  %data_ptr = getelementptr inbounds nuw i8, ptr %state, i64 8
  %data_ptr4 = load i32, ptr %data_ptr, align 4
  %tape6 = load ptr, ptr %state, align 8
  %0 = sext i32 %data_ptr4 to i64
  %arr_index_ptr = getelementptr inbounds i8, ptr %tape6, i64 %0
  %.sink16 = tail call i32 @llvm.smax.i32(i32 %calltmp, i32 0)
  %.sink = trunc i32 %.sink16 to i8
  store i8 %.sink, ptr %arr_index_ptr, align 1
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: read)
define i32 @find_forward(ptr readonly captures(none) %program, i32 %pc) local_unnamed_addr #4 {
entry:
  br label %whilecond.outer

whilecond.outer:                                  ; preds = %whilecond.outer.backedge, %entry
  %depth.0.ph = phi i32 [ 1, %entry ], [ %depth.0.ph.be, %whilecond.outer.backedge ]
  %i.0.in.ph = phi i32 [ %pc, %entry ], [ %i.0, %whilecond.outer.backedge ]
  br label %whilecond

whilecond:                                        ; preds = %whilecond.outer, %whilecond
  %i.0.in = phi i32 [ %i.0, %whilecond ], [ %i.0.in.ph, %whilecond.outer ]
  %i.0 = add i32 %i.0.in, 1
  %0 = sext i32 %i.0 to i64
  %char_ptr = getelementptr inbounds i8, ptr %program, i64 %0
  %char = load i8, ptr %char_ptr, align 1
  switch i8 %char, label %whilecond [
    i8 0, label %common.ret
    i8 91, label %then
    i8 93, label %then19
  ]

common.ret:                                       ; preds = %then19, %whilecond
  %common.ret.op = phi i32 [ -1, %whilecond ], [ %i.0, %then19 ]
  ret i32 %common.ret.op

then:                                             ; preds = %whilecond
  %addtmp12 = add i32 %depth.0.ph, 1
  br label %whilecond.outer.backedge

whilecond.outer.backedge:                         ; preds = %then, %then19
  %depth.0.ph.be = phi i32 [ %subtmp, %then19 ], [ %addtmp12, %then ]
  br label %whilecond.outer

then19:                                           ; preds = %whilecond
  %subtmp = add i32 %depth.0.ph, -1
  %eqtmp22 = icmp eq i32 %subtmp, 0
  br i1 %eqtmp22, label %common.ret, label %whilecond.outer.backedge
}

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: read)
define range(i32 -1, -2147483648) i32 @find_backward(ptr readonly captures(none) %program, i32 %pc) local_unnamed_addr #4 {
entry:
  %i.033 = add i32 %pc, -1
  %getmp34 = icmp sgt i32 %i.033, -1
  br i1 %getmp34, label %whilebody.preheader, label %common.ret

whilebody.preheader:                              ; preds = %entry
  %0 = zext nneg i32 %i.033 to i64
  br label %whilebody

whilebody:                                        ; preds = %whilebody.preheader, %ifcont
  %indvars.iv = phi i64 [ %0, %whilebody.preheader ], [ %indvars.iv.next, %ifcont ]
  %depth.035 = phi i32 [ 1, %whilebody.preheader ], [ %depth.1, %ifcont ]
  %char_ptr = getelementptr inbounds nuw i8, ptr %program, i64 %indvars.iv
  %char = load i8, ptr %char_ptr, align 1
  switch i8 %char, label %ifcont [
    i8 93, label %then
    i8 91, label %then14
  ]

common.ret.loopexit.split.loop.exit:              ; preds = %then14
  %1 = trunc nuw nsw i64 %indvars.iv to i32
  br label %common.ret

common.ret:                                       ; preds = %ifcont, %common.ret.loopexit.split.loop.exit, %entry
  %common.ret.op = phi i32 [ -1, %entry ], [ %1, %common.ret.loopexit.split.loop.exit ], [ -1, %ifcont ]
  ret i32 %common.ret.op

then:                                             ; preds = %whilebody
  %addtmp = add i32 %depth.035, 1
  br label %ifcont

ifcont:                                           ; preds = %whilebody, %then14, %then
  %depth.1 = phi i32 [ %addtmp, %then ], [ %subtmp16, %then14 ], [ %depth.035, %whilebody ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %getmp = icmp sgt i64 %indvars.iv, 0
  br i1 %getmp, label %whilebody, label %common.ret

then14:                                           ; preds = %whilebody
  %subtmp16 = add i32 %depth.035, -1
  %eqtmp18 = icmp eq i32 %subtmp16, 0
  br i1 %eqtmp18, label %common.ret.loopexit.split.loop.exit, label %ifcont
}

; Function Attrs: nounwind
define range(i32 0, 2) i32 @run_brainfuck(ptr readonly captures(none) %program) local_unnamed_addr #5 {
entry:
  br label %forbody.i

forbody.i:                                        ; preds = %tape_old_len_merge.i, %entry
  %tape.012.i = phi ptr [ null, %entry ], [ %push_new_data_i8.i, %tape_old_len_merge.i ]
  %i.011.i = phi i32 [ 0, %entry ], [ %inc.i, %tape_old_len_merge.i ]
  %tape_old_len_is_null.i = icmp eq ptr %tape.012.i, null
  br i1 %tape_old_len_is_null.i, label %tape_old_len_merge.i, label %tape_old_len_nonnull.i

tape_old_len_nonnull.i:                           ; preds = %forbody.i
  %0 = ptrtoint ptr %tape.012.i to i64
  %1 = add i64 %0, -8
  %2 = inttoptr i64 %1 to ptr
  %tape_old_len_loaded.i = load i32, ptr %2, align 4, !noalias !0
  br label %tape_old_len_merge.i

tape_old_len_merge.i:                             ; preds = %tape_old_len_nonnull.i, %forbody.i
  %push_base_ptr.i = phi ptr [ %2, %tape_old_len_nonnull.i ], [ null, %forbody.i ]
  %tape_old_len.i = phi i32 [ %tape_old_len_loaded.i, %tape_old_len_nonnull.i ], [ 0, %forbody.i ]
  %tape__len_new.i = add i32 %tape_old_len.i, 1
  %3 = sext i32 %tape__len_new.i to i64
  %push_data_bytes.i = shl nsw i64 %3, 2
  %push_total_bytes.i = add nsw i64 %push_data_bytes.i, 8
  %push_realloc.i = tail call ptr @realloc(ptr %push_base_ptr.i, i64 %push_total_bytes.i), !noalias !0
  store i32 %tape__len_new.i, ptr %push_realloc.i, align 4, !noalias !0
  %push_new_data_i8.i = getelementptr inbounds nuw i8, ptr %push_realloc.i, i64 8
  %4 = sext i32 %tape_old_len.i to i64
  %push_dst_ptr.i = getelementptr inbounds i32, ptr %push_new_data_i8.i, i64 %4
  store i32 0, ptr %push_dst_ptr.i, align 4, !noalias !0
  %inc.i = add nuw nsw i32 %i.011.i, 1
  %exitcond.not.i = icmp eq i32 %inc.i, 30000
  br i1 %exitcond.not.i, label %whilecond, label %forbody.i

whilecond:                                        ; preds = %tape_old_len_merge.i, %ifcont12
  %state.sroa.0.0 = phi ptr [ %state.sroa.0.1, %ifcont12 ], [ %push_new_data_i8.i, %tape_old_len_merge.i ]
  %state.sroa.10.0 = phi i32 [ %state.sroa.10.1, %ifcont12 ], [ 0, %tape_old_len_merge.i ]
  %pc.0 = phi i32 [ %addtmp, %ifcont12 ], [ 0, %tape_old_len_merge.i ]
  %5 = sext i32 %pc.0 to i64
  %char_ptr = getelementptr inbounds i8, ptr %program, i64 %5
  %char = load i8, ptr %char_ptr, align 1
  switch i8 %char, label %ifcont12 [
    i8 0, label %common.ret
    i8 62, label %then
    i8 60, label %then15
    i8 43, label %then25
    i8 45, label %then30
    i8 46, label %then35
    i8 44, label %then43
    i8 91, label %then48
    i8 93, label %then78
  ]

common.ret:                                       ; preds = %whilecond, %move_left.exit, %then98, %then66
  %common.ret.op = phi i32 [ 1, %then98 ], [ 1, %then66 ], [ 1, %move_left.exit ], [ 0, %whilecond ]
  ret i32 %common.ret.op

then:                                             ; preds = %whilecond
  %addtmp.i = add i32 %state.sroa.10.0, 1
  %member_arr_len_is_null.i = icmp eq ptr %state.sroa.0.0, null
  br i1 %member_arr_len_is_null.i, label %member_arr_len_merge.i, label %member_arr_len_nonnull.i

member_arr_len_nonnull.i:                         ; preds = %then
  %6 = ptrtoint ptr %state.sroa.0.0 to i64
  %7 = add i64 %6, -8
  %8 = inttoptr i64 %7 to ptr
  %member_arr_len_loaded.i = load i32, ptr %8, align 4
  br label %member_arr_len_merge.i

member_arr_len_merge.i:                           ; preds = %member_arr_len_nonnull.i, %then
  %member_arr_len.i = phi i32 [ %member_arr_len_loaded.i, %member_arr_len_nonnull.i ], [ 0, %then ]
  %getmp.not.i = icmp slt i32 %addtmp.i, %member_arr_len.i
  br i1 %getmp.not.i, label %ifcont12, label %then.i

then.i:                                           ; preds = %member_arr_len_merge.i
  %letmp.not.i.i = icmp slt i32 %addtmp.i, 0
  br i1 %letmp.not.i.i, label %entry.split.us.i.i, label %whilecond.i.i

entry.split.us.i.i:                               ; preds = %then.i
  br i1 %member_arr_len_is_null.i, label %ifcont12, label %member_arr_len_merge.thread.us.i.preheader.i

member_arr_len_merge.thread.us.i.preheader.i:     ; preds = %entry.split.us.i.i
  %9 = ptrtoint ptr %state.sroa.0.0 to i64
  %10 = add i64 %9, -8
  %11 = inttoptr i64 %10 to ptr
  %member_arr_len_loaded.us.i16.i = load i32, ptr %11, align 4
  %letmp.not14.us.i17.i = icmp sgt i32 %member_arr_len_loaded.us.i16.i, %addtmp.i
  br i1 %letmp.not14.us.i17.i, label %ifcont12, label %__tmp_push_94181924030560_old_len_nonnull.us.i.i

__tmp_push_94181924030560_old_len_nonnull.us.i.i: ; preds = %member_arr_len_merge.thread.us.i.preheader.i, %__tmp_push_94181924030560_old_len_nonnull.us.i.i
  %member_arr_len_loaded.us.i18.i = phi i32 [ %member_arr_len_loaded.us.i.i, %__tmp_push_94181924030560_old_len_nonnull.us.i.i ], [ %member_arr_len_loaded.us.i16.i, %member_arr_len_merge.thread.us.i.preheader.i ]
  %12 = phi ptr [ %17, %__tmp_push_94181924030560_old_len_nonnull.us.i.i ], [ %11, %member_arr_len_merge.thread.us.i.preheader.i ]
  %__tmp_push_94181924030560__len_new.us.i.i = add nsw i32 %member_arr_len_loaded.us.i18.i, 1
  %13 = sext i32 %__tmp_push_94181924030560__len_new.us.i.i to i64
  %push_data_bytes.us.i.i = shl nsw i64 %13, 2
  %push_total_bytes.us.i.i = add nsw i64 %push_data_bytes.us.i.i, 8
  %push_realloc.us.i.i = tail call ptr @realloc(ptr nonnull %12, i64 %push_total_bytes.us.i.i)
  store i32 %__tmp_push_94181924030560__len_new.us.i.i, ptr %push_realloc.us.i.i, align 4
  %push_new_data_i8.us.i.i = getelementptr inbounds nuw i8, ptr %push_realloc.us.i.i, i64 8
  %14 = sext i32 %member_arr_len_loaded.us.i18.i to i64
  %push_dst_ptr.us.i.i = getelementptr inbounds i32, ptr %push_new_data_i8.us.i.i, i64 %14
  store i32 0, ptr %push_dst_ptr.us.i.i, align 4
  %15 = ptrtoint ptr %push_new_data_i8.us.i.i to i64
  %16 = add i64 %15, -8
  %17 = inttoptr i64 %16 to ptr
  %member_arr_len_loaded.us.i.i = load i32, ptr %17, align 4
  %letmp.not14.us.i.i = icmp sgt i32 %member_arr_len_loaded.us.i.i, %addtmp.i
  br i1 %letmp.not14.us.i.i, label %ifcont12, label %__tmp_push_94181924030560_old_len_nonnull.us.i.i

whilecond.i.i:                                    ; preds = %then.i, %__tmp_push_94181924030560_old_len_merge.i.i
  %state.sroa.0.2 = phi ptr [ %push_new_data_i8.i.i, %__tmp_push_94181924030560_old_len_merge.i.i ], [ %state.sroa.0.0, %then.i ]
  %member_arr_len_is_null.i.i = icmp eq ptr %state.sroa.0.2, null
  br i1 %member_arr_len_is_null.i.i, label %__tmp_push_94181924030560_old_len_merge.i.i, label %member_arr_len_merge.thread.i.i

member_arr_len_merge.thread.i.i:                  ; preds = %whilecond.i.i
  %18 = ptrtoint ptr %state.sroa.0.2 to i64
  %19 = add i64 %18, -8
  %20 = inttoptr i64 %19 to ptr
  %member_arr_len_loaded.i.i = load i32, ptr %20, align 4
  %letmp.not14.i.i = icmp sgt i32 %member_arr_len_loaded.i.i, %addtmp.i
  br i1 %letmp.not14.i.i, label %ifcont12, label %__tmp_push_94181924030560_old_len_merge.i.i

__tmp_push_94181924030560_old_len_merge.i.i:      ; preds = %member_arr_len_merge.thread.i.i, %whilecond.i.i
  %push_base_ptr.i.i = phi ptr [ null, %whilecond.i.i ], [ %20, %member_arr_len_merge.thread.i.i ]
  %__tmp_push_94181924030560_old_len.i.i = phi i32 [ 0, %whilecond.i.i ], [ %member_arr_len_loaded.i.i, %member_arr_len_merge.thread.i.i ]
  %__tmp_push_94181924030560__len_new.i.i = add i32 %__tmp_push_94181924030560_old_len.i.i, 1
  %21 = sext i32 %__tmp_push_94181924030560__len_new.i.i to i64
  %push_data_bytes.i.i = shl nsw i64 %21, 2
  %push_total_bytes.i.i = add nsw i64 %push_data_bytes.i.i, 8
  %push_realloc.i.i = tail call ptr @realloc(ptr %push_base_ptr.i.i, i64 %push_total_bytes.i.i)
  store i32 %__tmp_push_94181924030560__len_new.i.i, ptr %push_realloc.i.i, align 4
  %push_new_data_i8.i.i = getelementptr inbounds nuw i8, ptr %push_realloc.i.i, i64 8
  %22 = sext i32 %__tmp_push_94181924030560_old_len.i.i to i64
  %push_dst_ptr.i.i = getelementptr inbounds i32, ptr %push_new_data_i8.i.i, i64 %22
  store i32 0, ptr %push_dst_ptr.i.i, align 4
  br label %whilecond.i.i

ifcont12:                                         ; preds = %member_arr_len_merge.thread.i.i, %__tmp_push_94181924030560_old_len_nonnull.us.i.i, %find_backward.exit, %else.i130, %then.i133, %else.i, %then.i121, %move_left.exit.thread, %member_arr_len_merge.thread.us.i.preheader.i, %entry.split.us.i.i, %member_arr_len_merge.i, %then78, %whilecond, %find_forward.exit, %idx_load284, %idx_load254, %then43, %idx_cont2
  %state.sroa.0.1 = phi ptr [ %state.sroa.0.0, %whilecond ], [ %state.sroa.0.0, %find_backward.exit ], [ %state.sroa.0.0, %move_left.exit.thread ], [ %push_new_data_i8.us.i.i, %__tmp_push_94181924030560_old_len_nonnull.us.i.i ], [ %state.sroa.0.0, %else.i ], [ %state.sroa.0.0, %idx_cont2 ], [ %state.sroa.0.0, %then43 ], [ %state.sroa.0.0, %find_forward.exit ], [ %state.sroa.0.0, %idx_load254 ], [ null, %then78 ], [ %state.sroa.0.0, %idx_load284 ], [ %state.sroa.0.0, %member_arr_len_merge.i ], [ null, %entry.split.us.i.i ], [ %state.sroa.0.0, %member_arr_len_merge.thread.us.i.preheader.i ], [ %state.sroa.0.0, %else.i130 ], [ %state.sroa.0.0, %then.i121 ], [ %state.sroa.0.0, %then.i133 ], [ %state.sroa.0.2, %member_arr_len_merge.thread.i.i ]
  %state.sroa.10.1 = phi i32 [ %state.sroa.10.0, %whilecond ], [ %state.sroa.10.0, %find_backward.exit ], [ %subtmp.i, %move_left.exit.thread ], [ %addtmp.i, %__tmp_push_94181924030560_old_len_nonnull.us.i.i ], [ %state.sroa.10.0, %else.i ], [ %state.sroa.10.0, %idx_cont2 ], [ %state.sroa.10.0, %then43 ], [ %state.sroa.10.0, %find_forward.exit ], [ %state.sroa.10.0, %idx_load254 ], [ %state.sroa.10.0, %then78 ], [ %state.sroa.10.0, %idx_load284 ], [ %addtmp.i, %member_arr_len_merge.i ], [ %addtmp.i, %entry.split.us.i.i ], [ %addtmp.i, %member_arr_len_merge.thread.us.i.preheader.i ], [ %state.sroa.10.0, %else.i130 ], [ %state.sroa.10.0, %then.i121 ], [ %state.sroa.10.0, %then.i133 ], [ %addtmp.i, %member_arr_len_merge.thread.i.i ]
  %pc.1 = phi i32 [ %pc.0, %whilecond ], [ %33, %find_backward.exit ], [ %pc.0, %move_left.exit.thread ], [ %pc.0, %__tmp_push_94181924030560_old_len_nonnull.us.i.i ], [ %pc.0, %else.i ], [ %pc.0, %idx_cont2 ], [ %pc.0, %then43 ], [ %i.0.i, %find_forward.exit ], [ %pc.0, %idx_load254 ], [ %pc.0, %then78 ], [ %pc.0, %idx_load284 ], [ %pc.0, %member_arr_len_merge.i ], [ %pc.0, %entry.split.us.i.i ], [ %pc.0, %member_arr_len_merge.thread.us.i.preheader.i ], [ %pc.0, %else.i130 ], [ %pc.0, %then.i121 ], [ %pc.0, %then.i133 ], [ %pc.0, %member_arr_len_merge.thread.i.i ]
  %addtmp = add i32 %pc.1, 1
  br label %whilecond

then15:                                           ; preds = %whilecond
  %eqtmp.i = icmp eq i32 %state.sroa.10.0, 0
  br i1 %eqtmp.i, label %move_left.exit, label %move_left.exit.thread

move_left.exit.thread:                            ; preds = %then15
  %subtmp.i = add i32 %state.sroa.10.0, -1
  br label %ifcont12

move_left.exit:                                   ; preds = %then15
  %puts.i = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.6)
  br label %common.ret

then25:                                           ; preds = %whilecond
  %idx_ptr_is_null2.i = icmp eq ptr %state.sroa.0.0, null
  %.pre.i = sext i32 %state.sroa.10.0 to i64
  br i1 %idx_ptr_is_null2.i, label %else.i, label %idx_cont2.i

idx_cont2.i:                                      ; preds = %then25
  %arr_index_ptr3.i = getelementptr inbounds i32, ptr %state.sroa.0.0, i64 %.pre.i
  %arr_index_load3.i = load i32, ptr %arr_index_ptr3.i, align 4
  %eqtmp.i120 = icmp eq i32 %arr_index_load3.i, 255
  br i1 %eqtmp.i120, label %then.i121, label %else.i

then.i121:                                        ; preds = %idx_cont2.i
  %arr_index_ptr.i = getelementptr inbounds i8, ptr %state.sroa.0.0, i64 %.pre.i
  store i8 0, ptr %arr_index_ptr.i, align 1
  br label %ifcont12

else.i:                                           ; preds = %idx_cont2.i, %then25
  %idx_safe_val224.i = phi i32 [ %arr_index_load3.i, %idx_cont2.i ], [ 0, %then25 ]
  %arr_index_ptr19.i = getelementptr inbounds i8, ptr %state.sroa.0.0, i64 %.pre.i
  %23 = trunc i32 %idx_safe_val224.i to i8
  %icast.i = add i8 %23, 1
  store i8 %icast.i, ptr %arr_index_ptr19.i, align 1
  br label %ifcont12

then30:                                           ; preds = %whilecond
  %idx_ptr_is_null2.i124 = icmp eq ptr %state.sroa.0.0, null
  %.pre.i125 = sext i32 %state.sroa.10.0 to i64
  br i1 %idx_ptr_is_null2.i124, label %then.i133, label %idx_cont2.i126

idx_cont2.i126:                                   ; preds = %then30
  %arr_index_ptr3.i127 = getelementptr inbounds i32, ptr %state.sroa.0.0, i64 %.pre.i125
  %arr_index_load3.i128 = load i32, ptr %arr_index_ptr3.i127, align 4
  %eqtmp.i129 = icmp eq i32 %arr_index_load3.i128, 0
  br i1 %eqtmp.i129, label %then.i133, label %else.i130

then.i133:                                        ; preds = %idx_cont2.i126, %then30
  %arr_index_ptr.i134 = getelementptr inbounds i8, ptr %state.sroa.0.0, i64 %.pre.i125
  store i8 -1, ptr %arr_index_ptr.i134, align 1
  br label %ifcont12

else.i130:                                        ; preds = %idx_cont2.i126
  %arr_index_ptr19.i131 = getelementptr inbounds i8, ptr %state.sroa.0.0, i64 %.pre.i125
  %24 = trunc i32 %arr_index_load3.i128 to i8
  %icast.i132 = add i8 %24, -1
  store i8 %icast.i132, ptr %arr_index_ptr19.i131, align 1
  br label %ifcont12

then35:                                           ; preds = %whilecond
  %idx_ptr_is_null2 = icmp eq ptr %state.sroa.0.0, null
  br i1 %idx_ptr_is_null2, label %idx_cont2, label %idx_load2

idx_load2:                                        ; preds = %then35
  %25 = sext i32 %state.sroa.10.0 to i64
  %arr_index_ptr3 = getelementptr inbounds i32, ptr %state.sroa.0.0, i64 %25
  %arr_index_load3 = load i32, ptr %arr_index_ptr3, align 4
  br label %idx_cont2

idx_cont2:                                        ; preds = %then35, %idx_load2
  %idx_safe_val2 = phi i32 [ %arr_index_load3, %idx_load2 ], [ 0, %then35 ]
  %calltmp38 = tail call i32 @putchar(i32 %idx_safe_val2)
  br label %ifcont12

then43:                                           ; preds = %whilecond
  %calltmp.i = tail call i32 @getchar()
  %26 = sext i32 %state.sroa.10.0 to i64
  %arr_index_ptr.i136 = getelementptr inbounds i8, ptr %state.sroa.0.0, i64 %26
  %.sink16.i = tail call i32 @llvm.smax.i32(i32 %calltmp.i, i32 0)
  %.sink.i = trunc i32 %.sink16.i to i8
  store i8 %.sink.i, ptr %arr_index_ptr.i136, align 1
  br label %ifcont12

then48:                                           ; preds = %whilecond
  %idx_ptr_is_null256 = icmp eq ptr %state.sroa.0.0, null
  br i1 %idx_ptr_is_null256, label %whilecond.i.preheader, label %idx_load254

idx_load254:                                      ; preds = %then48
  %27 = sext i32 %state.sroa.10.0 to i64
  %arr_index_ptr357 = getelementptr inbounds i32, ptr %state.sroa.0.0, i64 %27
  %arr_index_load358 = load i32, ptr %arr_index_ptr357, align 4
  %28 = icmp eq i32 %arr_index_load358, 0
  br i1 %28, label %whilecond.i.preheader, label %ifcont12

whilecond.i.preheader:                            ; preds = %then48, %idx_load254
  br label %whilecond.i.outer

whilecond.i.outer:                                ; preds = %whilecond.i.outer.backedge, %whilecond.i.preheader
  %depth.0.i.ph = phi i32 [ 1, %whilecond.i.preheader ], [ %depth.0.i.ph.be, %whilecond.i.outer.backedge ]
  %i.0.in.i.ph = phi i32 [ %pc.0, %whilecond.i.preheader ], [ %i.0.i, %whilecond.i.outer.backedge ]
  br label %whilecond.i

whilecond.i:                                      ; preds = %whilecond.i.outer, %whilecond.i
  %i.0.in.i = phi i32 [ %i.0.i, %whilecond.i ], [ %i.0.in.i.ph, %whilecond.i.outer ]
  %i.0.i = add i32 %i.0.in.i, 1
  %29 = sext i32 %i.0.i to i64
  %char_ptr.i = getelementptr inbounds i8, ptr %program, i64 %29
  %char.i = load i8, ptr %char_ptr.i, align 1
  switch i8 %char.i, label %whilecond.i [
    i8 0, label %then66
    i8 91, label %then.i140
    i8 93, label %then19.i
  ]

then.i140:                                        ; preds = %whilecond.i
  %addtmp12.i = add i32 %depth.0.i.ph, 1
  br label %whilecond.i.outer.backedge

then19.i:                                         ; preds = %whilecond.i
  %subtmp.i137 = add i32 %depth.0.i.ph, -1
  %eqtmp22.i = icmp eq i32 %subtmp.i137, 0
  br i1 %eqtmp22.i, label %find_forward.exit, label %whilecond.i.outer.backedge

whilecond.i.outer.backedge:                       ; preds = %then19.i, %then.i140
  %depth.0.i.ph.be = phi i32 [ %addtmp12.i, %then.i140 ], [ %subtmp.i137, %then19.i ]
  br label %whilecond.i.outer

find_forward.exit:                                ; preds = %then19.i
  %lttmp = icmp slt i32 %i.0.i, 0
  br i1 %lttmp, label %then66, label %ifcont12

then66:                                           ; preds = %find_forward.exit, %whilecond.i
  %calltmp68 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @str_lit.3, i32 %pc.0)
  br label %common.ret

then78:                                           ; preds = %whilecond
  %idx_ptr_is_null286 = icmp eq ptr %state.sroa.0.0, null
  br i1 %idx_ptr_is_null286, label %ifcont12, label %idx_load284

idx_load284:                                      ; preds = %then78
  %30 = sext i32 %state.sroa.10.0 to i64
  %arr_index_ptr387 = getelementptr inbounds i32, ptr %state.sroa.0.0, i64 %30
  %arr_index_load388 = load i32, ptr %arr_index_ptr387, align 4
  %31 = icmp eq i32 %arr_index_load388, 0
  br i1 %31, label %ifcont12, label %then91

then91:                                           ; preds = %idx_load284
  %i.033.i = add i32 %pc.0, -1
  %getmp34.i = icmp sgt i32 %i.033.i, -1
  br i1 %getmp34.i, label %whilebody.preheader.i, label %then98

whilebody.preheader.i:                            ; preds = %then91
  %32 = zext nneg i32 %i.033.i to i64
  br label %whilebody.i

whilebody.i:                                      ; preds = %ifcont.i144, %whilebody.preheader.i
  %indvars.iv.i = phi i64 [ %32, %whilebody.preheader.i ], [ %indvars.iv.next.i, %ifcont.i144 ]
  %depth.035.i = phi i32 [ 1, %whilebody.preheader.i ], [ %depth.1.i145, %ifcont.i144 ]
  %char_ptr.i142 = getelementptr inbounds nuw i8, ptr %program, i64 %indvars.iv.i
  %char.i143 = load i8, ptr %char_ptr.i142, align 1
  switch i8 %char.i143, label %ifcont.i144 [
    i8 93, label %then.i146
    i8 91, label %then14.i
  ]

then.i146:                                        ; preds = %whilebody.i
  %addtmp.i147 = add i32 %depth.035.i, 1
  br label %ifcont.i144

ifcont.i144:                                      ; preds = %then14.i, %then.i146, %whilebody.i
  %depth.1.i145 = phi i32 [ %addtmp.i147, %then.i146 ], [ %subtmp16.i, %then14.i ], [ %depth.035.i, %whilebody.i ]
  %indvars.iv.next.i = add nsw i64 %indvars.iv.i, -1
  %getmp.i = icmp sgt i64 %indvars.iv.i, 0
  br i1 %getmp.i, label %whilebody.i, label %then98

then14.i:                                         ; preds = %whilebody.i
  %subtmp16.i = add i32 %depth.035.i, -1
  %eqtmp18.i = icmp eq i32 %subtmp16.i, 0
  br i1 %eqtmp18.i, label %find_backward.exit, label %ifcont.i144

find_backward.exit:                               ; preds = %then14.i
  %33 = trunc nuw nsw i64 %indvars.iv.i to i32
  br label %ifcont12

then98:                                           ; preds = %then91, %ifcont.i144
  %calltmp100 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @str_lit.4, i32 %pc.0)
  br label %common.ret
}

; Function Attrs: nounwind
define range(i32 0, 3) i32 @main(i32 %argc, ptr readonly captures(address_is_null) %argv) local_unnamed_addr #5 {
entry:
  %lttmp = icmp slt i32 %argc, 2
  br i1 %lttmp, label %then, label %ifcont

common.ret:                                       ; preds = %idx_cont, %then
  %common.ret.op = phi i32 [ 2, %then ], [ %calltmp4, %idx_cont ]
  ret i32 %common.ret.op

then:                                             ; preds = %entry
  %puts.i = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  %puts2.i = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.5)
  br label %common.ret

ifcont:                                           ; preds = %entry
  %idx_ptr_is_null = icmp eq ptr %argv, null
  br i1 %idx_ptr_is_null, label %idx_cont, label %idx_load

idx_load:                                         ; preds = %ifcont
  %ptr_index_ptr = getelementptr inbounds nuw i8, ptr %argv, i64 8
  %ptr_index_load = load ptr, ptr %ptr_index_ptr, align 8
  br label %idx_cont

idx_cont:                                         ; preds = %ifcont, %idx_load
  %idx_safe_val = phi ptr [ %ptr_index_load, %idx_load ], [ null, %ifcont ]
  %calltmp4 = tail call i32 @run_brainfuck(ptr %idx_safe_val)
  br label %common.ret
}

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr noundef readonly captures(none)) local_unnamed_addr #0

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #6

attributes #0 = { nofree nounwind }
attributes #1 = { nounwind memory(readwrite, target_mem0: none, target_mem1: none) }
attributes #2 = { mustprogress nounwind willreturn allockind("realloc") allocsize(1) memory(argmem: readwrite, inaccessiblemem: readwrite) "alloc-family"="malloc" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none, target_mem0: none, target_mem1: none) }
attributes #4 = { nofree norecurse nosync nounwind memory(argmem: read) }
attributes #5 = { nounwind }
attributes #6 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }

!0 = !{!1}
!1 = distinct !{!1, !2, !"init_state: %__sret"}
!2 = distinct !{!2, !"init_state"}
===================================================
