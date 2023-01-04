import gdb

f = open("log_simple.txt", "w")

gdb.execute("break *0x400960")
gdb.execute("run")
for i in range(10):
    ret = gdb.execute("stepi", True, to_string=True)
    f.write(str(ret) + "\n")
    

f.close()