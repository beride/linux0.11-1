





.586p
.model flat

































































































extern _do_tty_interrupt:proc, _show_stat:proc
extern _table_list:dword
public _keyboard_interrupt 








bsize = 1024 

			
			
head = 4 
tail = 8 
proc_list = 12 
buf = 16 

.code

mode db 0 











leds db 2 






e0 db 0













_keyboard_interrupt:
	push eax
	push ebx
	push ecx
	push edx
	push ds
	push es
	mov eax,10h 
	mov ds,ax
	mov es,ax
	xor al,al  
	in al,60h 
	cmp al,0e0h 
	je set_e0
	cmp al,0e1h 
	je set_e1
	call key_table[eax*4] 
	mov e0,0 



e0_e1: 
	in al,61h 
	jmp l1 
l1: jmp l2
l2: or al,80h 
	jmp l3 
l3: jmp l4
l4: out 61h,al 
	jmp l5 
l5: jmp l6
l6: and al,7Fh 
	out 61h,al 
	mov al,20h 
	out 20h,al
	push 0 
	call _do_tty_interrupt 
	add esp,4 
	pop es
	pop ds
	pop edx
	pop ecx
	pop ebx
	pop eax
	iretd
set_e0: 
	mov e0,1 
	jmp e0_e1
set_e1: 
	mov e0,2 
	jmp e0_e1





put_queue:
	push ecx 
	push edx 
	mov edx,_table_list 
	mov ecx,head[edx] 
l7: mov buf[edx+ecx],al 
	inc ecx 
	and ecx,bsize-1 
	cmp ecx,tail[edx] 

	je l9 
	shrd eax,ebx,8 
	je l8 
	shr ebx,8 
	jmp l7
l8: mov head[edx],ecx 
	mov ecx,proc_list[edx] 
	test ecx,ecx 
	je l9 
	mov dword ptr [ecx],0 
l9: pop edx 
	pop ecx
	ret




ctrl:
	mov al,04h 
	jmp l10
alt:
	mov al,10h 
l10:
	cmp e0,0 
	je l11 
	add al,al 
l11:
	or mode,al 
	ret


unctrl:
	mov al,04h 
	jmp l12
unalt:
	mov al,10h 
l12:
	cmp e0,0 
	je l13 
	add al,al 
l13:
	not al 
	and mode,al
	ret

lshift:
	or mode,01h 
	ret
unlshift:
	and mode,0feh 
	ret
rshift:
	or mode,02h 
	ret
unrshift:
	and mode,0fdh 
	ret

caps:
	test mode,80h 
	jne l14 
	xor leds,4 
	xor mode,40h 
	or mode,80h 

set_leds:
	call kb_wait 
	mov al,0edh  
	out 60h,al 
	call kb_wait 
	mov al,leds 
	out 60h,al 
	ret
uncaps:
	and mode,7fh 
	ret
scroll:
	xor leds,1 
	jmp set_leds 
num:
	xor leds,2 
	jmp set_leds 








cursor:
	sub al,47h 
	jb l14 
	cmp al,12 
	ja l14 
	jne cur2  


	test mode,0ch 
	je cur2 
	test mode,30h 
	jne reboot 
cur2:
	cmp e0,01h  

	je cur 
	test leds,02h  

	je cur 
	test mode,03h  

	jne cur 
	xor ebx,ebx 
	mov al,num_table[eax] 
	jmp put_queue 
l14:
	ret


cur:
	mov al,cur_table[eax] 
	cmp al,'9' 
	ja ok_cur 
	mov ah,'~'
ok_cur:
	shl eax,16 
	mov ax,5b1bh 
	xor ebx,ebx
	jmp put_queue 




num_table db "789 456 1230,"

cur_table db  "HA5 DGC YB623" 





func:
	push eax
	push ecx
	push edx
	call _show_stat 
	pop edx
	pop ecx
	pop eax
	sub al,3Bh 
	jb end_func 
	cmp al,9 
	jbe ok_func 
	sub al,18 
	cmp al,10 
	jb end_func 
	cmp al,11 
	ja end_func 
ok_func:
	cmp ecx,4  
	jl end_func 
	mov eax,func_table[eax*4] 
	xor ebx,ebx
	jmp put_queue 
end_func:
	ret




func_table:
 DD 415b5b1bh,425b5b1bh,435b5b1bh,445b5b1bh
 DD 455b5b1bh,465b5b1bh,475b5b1bh,485b5b1bh
 DD 495b5b1bh,4a5b5b1bh,4b5b5b1bh,4c5b5b1bh




























































key_map:
 db 0,27
 db "1234567890-="
 db 127,9
 db "qwertyuiop[]"
 db 13,0
 db "asdfghjkl;'"
 db '`',0
 db "\zxcvbnm,./"
 db 0,'*',0,32 
 db 16 dup(0)  
 db '-',0,0,0,'+' 
 db 0,0,0,0,0,0,0 
 db '<'
 db 10 dup(0)


shift_map:
 db 0,27
 db "!@#$%^&*()_+"
 db 127,9
 db "QWERTYUIOP{}"
 db 13,0
 db "ASDFGHJKL:",'"'
 db '~',0
 db "|ZXCVBNM<>?"
 db 0,'*',0,32 
 db 16 dup(0)  
 db '-',0,0,0,'+' 
 db 0,0,0,0,0,0,0 
 db '>'
 db 10 dup(0)

alt_map:
 db 0,0
 db 0,'@',0,'$',0,0,"{[]}","\",0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db '~',13,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0
 db 0,0,0,0,0,0,0,0,0,0,0
 db 0,0,0,0 
 db 16 dup(0)  
 db 0,0,0,0,0 
 db 0,0,0,0,0,0,0 
 db '|'
 db 10 dup(0)



















































































































do_self:

	lea ebx,alt_map 
	test mode,20  
	jne l15 
	lea ebx,shift_map 
	test mode,03h 
	jne l15 
	lea ebx,key_map 

l15:
	mov al,[ebx+eax] 
	or al,al 
	je none 


	test mode,4ch  
	je l16 
	cmp al,'a' 
	jb l16 
	cmp al,'}' 
	ja l16 
	sub al,32 


l16: 
	test mode,0ch  
	je l17 
	cmp al,64 
	jb l17 
	cmp al,64+32 
	jae l17 
	sub al,64 


l17: 
	test mode,10h  
	je l18 
	or al,80h 

l18: 
	and eax,0ffh 
	xor ebx,ebx 
	call put_queue 
none: 
	ret










minus: 
	cmp e0,1 
	jne do_self 
	mov eax,'/' 
	xor ebx,ebx
	jmp put_queue 









key_table:
 DD none,   do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,ctrl,   do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,lshift, do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,do_self,do_self,do_self 
 DD do_self,minus,  rshift, do_self 
 DD alt,    do_self,caps,   func 
 DD func,   func,   func,   func 
 DD func,   func,   func,   func 
 DD func,   num,    scroll, cursor 
 DD cursor, cursor, do_self,cursor 
 DD cursor, cursor, do_self,cursor 
 DD cursor, cursor, cursor, cursor 
 DD none,   none,   do_self,func 
 DD func,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   unctrl, none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   unlshift,none 
 DD none,   none,   none,   none 
 DD none,   none,   none,   none 
 DD none,   none,   unrshift,none 
 DD unalt,  none,   uncaps, none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 
 DD none,none,none,none 









kb_wait:
	push eax
l19: 
	in al,64h 
	test al,02h 
	jne l19 
	pop eax
	ret







reboot:
	call kb_wait 
	mov word ptr ds:[472h],1234h 
	mov al,0fch 
	out 64h,al 
die: 
	jmp die 

end
