import gdb

# gdb -x trackpc.py ./simple
gdb.execute("break main")
gdb.execute("run")

#  gdb -x trackpc.py ./loader > test1.txt
# gdb.execute("break loader.c:153")
# gdb.execute("run ./simple")

for i in range(1000):
    gdb.execute("stepi")
    gdb.execute("x/i $pc")

gdb.execute("quit")