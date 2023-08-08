reset
set terminal png
set title 'rbtree vs stree'
set xlabel 'number of int'
set ylabel 'mean time (s)'
set output 'results/plot.png'

plot \
"results/rbtest.dat" using 1:2:3:4 with yerrorbars title "", \
    '' using 1:2 with lines title "rbtree", \
"results/stree.dat" using 1:2:3:4 with yerrorbars title "", \
    '' using 1:2 with lines title "stree"
