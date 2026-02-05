vixc test.vix -q test
qbe -o test.s test.ssa
as test.s -o test.o
gcc test.o -o test