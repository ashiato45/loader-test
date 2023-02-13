import gdb

# gdb.execute("break main")
gdb.execute("break loader.c:153")
# gdb.execute("run")
gdb.execute("run ./simple")

for i in range(1000):
    gdb.execute("stepi")
    gdb.execute("x/i $pc")

gdb.execute("quit")