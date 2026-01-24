./vixc test.vix -q test
./qbe -o test.s test.ssa
as test.s -o test.o
ld test.o io.o -o test 
