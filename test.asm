; sound chip is 8192
; voice0 is 8202

; rise
imm.b r0, b 20
sto.b [8204], r0

; vol
imm.b r0, b 100
sto.b [8205], r0

; len
imm.b r0, b 1
sto.b [8206], r0

; fall 
imm.b r0, b 40
sto.b [8207], r0

; low 
imm.b r0, b 0
sto.b [8208], r0

; high 
imm.b r0, b 255
sto.b [8209], r0

; echo (ignored)
imm.b r0, b 0
sto.b [8210], r0

; wavelen
imm.b r0, b 80
sto.b [8211], r0


imm.b r1, b 120
   ;  loop

; freq
sto.b [8203], r1

; trigger
sto.b [8202], r0

; 2 seconds
imm.b r0, b 200
; tc is 12288
; ch4
sto.b [12296], r0
; wait 0 is 12304
sto.b [12308]
sto.b [12296], r0
; wait 0 is 12304
sto.b [12308]

addi.b r1, b 1

jmp [pc - 38]

