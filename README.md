# WDSat
A SAT solver dedicated to solving instances derived from a Weil descent.

WDSat was introduced at CP20 [(see paper)](https://arxiv.org/abs/2001.11229) as a joint work with [Gilles Dequen](https://home.mis.u-picardie.fr/~dequen/doku.php) and [Sorina Ionica](https://home.mis.u-picardie.fr/~ionica/). 

### Dependencies
WDSat is built from scratch and has no particular dependencies, apart from the essential tools. 

### Compiling in Linux and Mac OS
```bash
make
```
in the ```src``` folder wil create the ```wdsat_solver``` executable in the root folder.

### Command-line arguments
```
-i file    : where file is the input file
-x : to enable Gaussian Elimination
-g mvc    : where mvc is a string of comma-separated variables that defines statically the branching order
```

### Configuration
The core structures in the different WDSat modules are statically allocated and, currently, several constants need to be set manually in the ```config.h``` file.

