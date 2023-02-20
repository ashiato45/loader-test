import gdb

# gdb -x trackpc.py ./simple
# gdb.execute("break main")
gdb.execute("break *0x00400a50")
gdb.execute("run")

#  gdb -x trackpc.py ./loader > test1.txt
# gdb.execute("break loader.c:153")
# gdb.execute("run ./simple")

for i in range(10000000):
    gdb.execute("stepi")
    gdb.execute("x/i $pc")
    gdb.execute('printf "pc=0x%x, rip=0x%x, rdi=0x%x, rax=0x%x, rbx=0x%x, rsp=0x%x, rsi=0x%x\n", $pc, $rip, $rdi, $rax, $rbx, $rsp, $rsi')
    # gdb.execute('x/x $sp')

gdb.execute("quit")