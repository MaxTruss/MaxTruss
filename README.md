# README #

This folder contains all codes of paper "Adaptive Truss Maximization on Large Graphs: A Minimum Cut Approach".

## This folder contains the following: ##
* **code**: code folder, all source codes of algorithms proposed in the paper.

## Dependencies : ##
* **software**: Oracle Linux 8.7 (64-bit), gcc version 8.5.0 20210514 (Red Hat 8.5.0-16.0.2) (GCC), python 3
* **hardware**: Xeon E5-2630 v4 @ 2.2GHz (2S/10C) / 256GB RAM / 128GB SWAP

## How to Use the Code? ##

* First, use "make" to get executable files "index.out" and "PCFR.out".
* Then, use "./index.out dataFile indexFile" to get the index.
* Next, use "./PCFR.out indexFile b k 1" to get the result, where b and k are integers.
