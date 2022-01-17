hirawala@ste-rddev:~/CURLIT$ make clean; make
rm -f -rf  sktest.o median.o *.dSYM
gcc -fPIC -g -Wall    -c -o sktest.o sktest.c
gcc -fPIC -g -Wall    -c -o median.o median.c
gcc -shared sktest.o median.o -lcurl -o libsktest.so
rm -f -rf sktest.o median.o
hirawala@ste-rddev:~/CURLIT$ gcc -o sktest main.c -lsktest -L.
hirawala@ste-rddev:~/CURLIT$ export LD_LIBRARY_PATH=.
hirawala@ste-rddev:~/CURLIT$ ./sktest -n 3
SKTEST;142.250.180.4;200;0.000045;0.000045;0.053348;0.057154
hirawala@ste-rddev:~/CURLIT$ ./sktest -n 103
SKTEST;142.250.180.4;200;0.000040;0.000041;0.054838;0.059357
