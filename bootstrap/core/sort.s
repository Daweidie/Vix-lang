default rel
section .text
extern printf
extern strcmp
extern strlen
extern vix_array_len
extern vix_array_push_i32
extern vix_array_push_ptr
extern vix_array_push_bytes
extern vix_string_concat
global partition
partition:
    push rbp
    mov rbp, rsp
    sub rsp, 528
    mov qword [rbp-8], rdi
    mov dword [rbp-16], esi
    mov dword [rbp-24], edx
    mov rax, qword [rbp-8]
    mov qword [rbp-32], rax
    mov eax, dword [rbp-16]
    mov dword [rbp-40], eax
    mov eax, dword [rbp-24]
    mov dword [rbp-48], eax
    mov rax, qword [rbp-32]
    mov [rbp-64], rax
    mov eax, dword [rbp-48]
    mov [rbp-72], eax
    mov rax, qword [rbp-64]
    mov ecx, dword [rbp-72]
    imul rcx, 4
    add rax, rcx
    mov [rbp-80], rax
    mov rax, qword [rbp-80]
    mov eax, dword [rax]
    mov [rbp-88], eax
    mov eax, dword [rbp-88]
    mov dword [rbp-56], eax
    mov eax, dword [rbp-40]
    mov [rbp-104], eax
    mov eax, dword [rbp-104]
    sub eax, 1
    mov [rbp-112], eax
    mov eax, dword [rbp-112]
    mov dword [rbp-96], eax
    mov eax, dword [rbp-40]
    mov [rbp-128], eax
    mov eax, dword [rbp-128]
    mov dword [rbp-120], eax
    mov eax, dword [rbp-48]
    mov [rbp-144], eax
    mov eax, dword [rbp-144]
    mov dword [rbp-136], eax
    jmp partition__for_cond0
partition__for_cond0:
    mov eax, dword [rbp-120]
    mov [rbp-152], eax
    mov eax, dword [rbp-136]
    mov [rbp-160], eax
    mov eax, dword [rbp-152]
    cmp eax, dword [rbp-160]
    setl al
    movzx eax, al
    mov [rbp-168], eax
    cmp dword [rbp-168], 0
    je partition__for_end3
    jmp partition__for_body1
partition__for_body1:
    mov rax, qword [rbp-32]
    mov [rbp-176], rax
    mov eax, dword [rbp-120]
    mov [rbp-184], eax
    mov rax, qword [rbp-176]
    mov ecx, dword [rbp-184]
    imul rcx, 4
    add rax, rcx
    mov [rbp-192], rax
    mov rax, qword [rbp-192]
    mov eax, dword [rax]
    mov [rbp-200], eax
    mov eax, dword [rbp-56]
    mov [rbp-208], eax
    mov eax, dword [rbp-200]
    cmp eax, dword [rbp-208]
    setle al
    movzx eax, al
    mov [rbp-216], eax
    cmp dword [rbp-216], 0
    je partition__if_else5
    jmp partition__if_then4
partition__if_then4:
    mov eax, dword [rbp-96]
    mov [rbp-224], eax
    mov eax, dword [rbp-224]
    add eax, 1
    mov [rbp-232], eax
    mov eax, dword [rbp-232]
    mov dword [rbp-96], eax
    mov rax, qword [rbp-32]
    mov [rbp-248], rax
    mov eax, dword [rbp-96]
    mov [rbp-256], eax
    mov rax, qword [rbp-248]
    mov ecx, dword [rbp-256]
    imul rcx, 4
    add rax, rcx
    mov [rbp-264], rax
    mov rax, qword [rbp-264]
    mov eax, dword [rax]
    mov [rbp-272], eax
    mov eax, dword [rbp-272]
    mov dword [rbp-240], eax
    mov rax, qword [rbp-32]
    mov [rbp-280], rax
    mov eax, dword [rbp-96]
    mov [rbp-288], eax
    mov rax, qword [rbp-280]
    mov ecx, dword [rbp-288]
    imul rcx, 4
    add rax, rcx
    mov [rbp-296], rax
    mov rax, qword [rbp-32]
    mov [rbp-304], rax
    mov eax, dword [rbp-120]
    mov [rbp-312], eax
    mov rax, qword [rbp-304]
    mov ecx, dword [rbp-312]
    imul rcx, 4
    add rax, rcx
    mov [rbp-320], rax
    mov rax, qword [rbp-320]
    mov eax, dword [rax]
    mov [rbp-328], eax
    mov eax, dword [rbp-328]
    mov r11, qword [rbp-296]
    mov dword [r11], eax
    mov rax, qword [rbp-32]
    mov [rbp-336], rax
    mov eax, dword [rbp-120]
    mov [rbp-344], eax
    mov rax, qword [rbp-336]
    mov ecx, dword [rbp-344]
    imul rcx, 4
    add rax, rcx
    mov [rbp-352], rax
    mov eax, dword [rbp-240]
    mov [rbp-360], eax
    mov eax, dword [rbp-360]
    mov r11, qword [rbp-352]
    mov dword [r11], eax
    jmp partition__if_end6
partition__if_else5:
    jmp partition__if_end6
partition__if_end6:
    jmp partition__for_step2
partition__for_step2:
    mov eax, dword [rbp-120]
    mov [rbp-368], eax
    mov eax, dword [rbp-368]
    add eax, 1
    mov [rbp-376], eax
    mov eax, dword [rbp-376]
    mov dword [rbp-120], eax
    jmp partition__for_cond0
partition__for_end3:
    mov eax, dword [rbp-96]
    mov [rbp-384], eax
    mov eax, dword [rbp-384]
    add eax, 1
    mov [rbp-392], eax
    mov eax, dword [rbp-392]
    mov dword [rbp-96], eax
    mov rax, qword [rbp-32]
    mov [rbp-400], rax
    mov eax, dword [rbp-96]
    mov [rbp-408], eax
    mov rax, qword [rbp-400]
    mov ecx, dword [rbp-408]
    imul rcx, 4
    add rax, rcx
    mov [rbp-416], rax
    mov rax, qword [rbp-416]
    mov eax, dword [rax]
    mov [rbp-424], eax
    mov eax, dword [rbp-424]
    mov dword [rbp-240], eax
    mov rax, qword [rbp-32]
    mov [rbp-432], rax
    mov eax, dword [rbp-96]
    mov [rbp-440], eax
    mov rax, qword [rbp-432]
    mov ecx, dword [rbp-440]
    imul rcx, 4
    add rax, rcx
    mov [rbp-448], rax
    mov rax, qword [rbp-32]
    mov [rbp-456], rax
    mov eax, dword [rbp-48]
    mov [rbp-464], eax
    mov rax, qword [rbp-456]
    mov ecx, dword [rbp-464]
    imul rcx, 4
    add rax, rcx
    mov [rbp-472], rax
    mov rax, qword [rbp-472]
    mov eax, dword [rax]
    mov [rbp-480], eax
    mov eax, dword [rbp-480]
    mov r11, qword [rbp-448]
    mov dword [r11], eax
    mov rax, qword [rbp-32]
    mov [rbp-488], rax
    mov eax, dword [rbp-48]
    mov [rbp-496], eax
    mov rax, qword [rbp-488]
    mov ecx, dword [rbp-496]
    imul rcx, 4
    add rax, rcx
    mov [rbp-504], rax
    mov eax, dword [rbp-240]
    mov [rbp-512], eax
    mov eax, dword [rbp-512]
    mov r11, qword [rbp-504]
    mov dword [r11], eax
    mov eax, dword [rbp-96]
    mov [rbp-520], eax
    mov eax, dword [rbp-520]
    jmp partition__return
partition__return:
    mov rsp, rbp
    pop rbp
    ret
global quicksort
quicksort:
    push rbp
    mov rbp, rsp
    sub rsp, 176
    mov qword [rbp-8], rdi
    mov dword [rbp-16], esi
    mov dword [rbp-24], edx
    mov rax, qword [rbp-8]
    mov qword [rbp-32], rax
    mov eax, dword [rbp-16]
    mov dword [rbp-40], eax
    mov eax, dword [rbp-24]
    mov dword [rbp-48], eax
    mov eax, dword [rbp-40]
    mov [rbp-56], eax
    mov eax, dword [rbp-48]
    mov [rbp-64], eax
    mov eax, dword [rbp-56]
    cmp eax, dword [rbp-64]
    setl al
    movzx eax, al
    mov [rbp-72], eax
    cmp dword [rbp-72], 0
    je quicksort__if_else1
    jmp quicksort__if_then0
quicksort__if_then0:
    mov rax, qword [rbp-32]
    mov [rbp-88], rax
    mov eax, dword [rbp-40]
    mov [rbp-96], eax
    mov eax, dword [rbp-48]
    mov [rbp-104], eax
    mov rdi, qword [rbp-88]
    mov esi, dword [rbp-96]
    mov edx, dword [rbp-104]
    call partition
    mov [rbp-112], eax
    mov eax, dword [rbp-112]
    mov dword [rbp-80], eax
    mov rax, qword [rbp-32]
    mov [rbp-120], rax
    mov eax, dword [rbp-40]
    mov [rbp-128], eax
    mov eax, dword [rbp-80]
    mov [rbp-136], eax
    mov eax, dword [rbp-136]
    sub eax, 1
    mov [rbp-144], eax
    mov rdi, qword [rbp-120]
    mov esi, dword [rbp-128]
    mov edx, dword [rbp-144]
    call quicksort
    mov rax, qword [rbp-32]
    mov [rbp-152], rax
    mov eax, dword [rbp-80]
    mov [rbp-160], eax
    mov eax, dword [rbp-160]
    add eax, 1
    mov [rbp-168], eax
    mov eax, dword [rbp-48]
    mov [rbp-176], eax
    mov rdi, qword [rbp-152]
    mov esi, dword [rbp-168]
    mov edx, dword [rbp-176]
    call quicksort
    jmp quicksort__if_end2
quicksort__if_else1:
    jmp quicksort__if_end2
quicksort__if_end2:
    jmp quicksort__return
quicksort__return:
    mov rsp, rbp
    pop rbp
    ret
global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 192
    mov rdi, 0
    mov esi, 10
    call vix_array_push_i32
    mov [rbp-16], rax
    mov rdi, qword [rbp-16]
    mov esi, 7
    call vix_array_push_i32
    mov [rbp-24], rax
    mov rdi, qword [rbp-24]
    mov esi, 8
    call vix_array_push_i32
    mov [rbp-32], rax
    mov rdi, qword [rbp-32]
    mov esi, 9
    call vix_array_push_i32
    mov [rbp-40], rax
    mov rdi, qword [rbp-40]
    mov esi, 1
    call vix_array_push_i32
    mov [rbp-48], rax
    mov rdi, qword [rbp-48]
    mov esi, 5
    call vix_array_push_i32
    mov [rbp-56], rax
    mov rax, qword [rbp-56]
    mov qword [rbp-8], rax
    mov rax, qword [rbp-8]
    mov [rbp-64], rax
    mov rdi, qword [rbp-64]
    mov esi, 0
    mov edx, 5
    call quicksort
    mov eax, 0
    mov dword [rbp-72], eax
    mov eax, 6
    mov dword [rbp-80], eax
    jmp main__for_cond0
main__for_cond0:
    mov eax, dword [rbp-72]
    mov [rbp-88], eax
    mov eax, dword [rbp-80]
    mov [rbp-96], eax
    mov eax, dword [rbp-88]
    cmp eax, dword [rbp-96]
    setl al
    movzx eax, al
    mov [rbp-104], eax
    cmp dword [rbp-104], 0
    je main__for_end3
    jmp main__for_body1
main__for_body1:
    lea rax, [Lstr0]
    mov [rbp-112], rax
    mov rax, qword [rbp-8]
    mov [rbp-120], rax
    mov eax, dword [rbp-72]
    mov [rbp-128], eax
    mov rax, qword [rbp-120]
    mov ecx, dword [rbp-128]
    imul rcx, 4
    add rax, rcx
    mov [rbp-136], rax
    mov rax, qword [rbp-136]
    mov eax, dword [rax]
    mov [rbp-144], eax
    mov rdi, qword [rbp-112]
    mov esi, dword [rbp-144]
    call printf
    mov [rbp-152], eax
    jmp main__for_step2
main__for_step2:
    mov eax, dword [rbp-72]
    mov [rbp-160], eax
    mov eax, dword [rbp-160]
    add eax, 1
    mov [rbp-168], eax
    mov eax, dword [rbp-168]
    mov dword [rbp-72], eax
    jmp main__for_cond0
main__for_end3:
    lea rax, [Lstr1]
    mov [rbp-176], rax
    mov rdi, qword [rbp-176]
    call printf
    mov [rbp-184], eax
    mov eax, 0
    jmp main__return
main__return:
    mov rsp, rbp
    pop rbp
    ret
section .data
Lstr0: db 37, 100, 32, 0
Lstr1: db 10, 0
