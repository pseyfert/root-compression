```
ALG=4
LVL=9
valgrind --massif-out-file=massif.$ALG.$LVL.out --tool=massif --peak-inaccuracy=0.5 ./generate $ALG $LVL
```
