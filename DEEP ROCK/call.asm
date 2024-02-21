
extern old_Randcall:far
extern old_Randcall2:far
.code

MyAsmRandcall proc
sub rsp,108h
mov rax ,old_Randcall
mov rax,[rax]
call rax
mov rax,4h
add rsp,108h
ret
MyAsmRandcall endp

MyAsmRandcall2 proc
sub rsp,108h
mov rax ,old_Randcall2
mov rax,[rax]
call rax
mov rax,4h
add rsp,108h
ret
MyAsmRandcall2 endp
end