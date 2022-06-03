To generate pdf's of plots:

```
./plotdata.py ../results/micro/metal-r0.csv 
for x in *.gp; do echo $x; gnuplot $x; done
```
