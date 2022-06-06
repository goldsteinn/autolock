To generate pdf's of plots:

```
./plotdata.py ../results/micro/metal-r0.csv 
for x in *.gp; do echo $x; gnuplot $x; done
```

To get plots comparing each lock to best in class per thread:


```
# put all plot data in directy xxx
./plotdata.py -a -o xxx ../results/micro/metal-r0.csv 
cd xxx
for x in *.gp; do gnuplot $x; done
```
