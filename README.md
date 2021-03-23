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
* When __\_\_XG_ENHANCED\_\___ is defined, the XORGAUSS-extended module is used for Gaussian Elimination and the input instance has to be in ANF form. (see input forms section)
* When __\_\_FIND_ALL_SOLUTIONS\_\___ is defined, the solver outputs all solutions instead of stopping after the first solution is found, and outputs UNSAT at the end. 
* __\_\_MAX_ANF_ID\_\___ defines the maximum number of unary variables, i.e. variables in the ANF form. Always add +1 to the actual value.
* __\_\_MAX_DEGREE\_\___ defines the maximum degree of the Boolean polynomial systems from which SAT instances are derived. Always add +1 to the actual value.
* __\_\_MAX_ID\_\___ defines the maximum number of variables in the CNF-XOR form, but has to be set even when the given input is in ANF. For good performance, it is important to set this constant to the exact value. If the estimation is badly made, the solver will alert you at the beginning and will give you the correct value for the current instance. 
* __\_\_MAX_BUFFER_SIZE\_\___ is complicated to estimate. Start with 5000 and increase if not enough (execution will stop immediately, with an appropriate error message).
* __\_\_MAX_EQ\_\___ defines the maximum number of OR-clauses. 
* __\_\_MAX_EQ_SIZE\_\___ defines the maximum size of OR-clauses. Always add +1 to the actual value. This value always corresponds to (__\_\_MAX_DEGREE\_\___ + 1).
* __\_\_MAX_XEQ\_\___ defines the maximum number of XOR-clauses. 
* __\_\_MAX_XEQ_SIZE\_\___ defines the maximum size of XOR-clauses. Be careful, the size of XOR-clauses can increase when Gaussian Elimination is performed. Try to estimate, or at worst, set to __\_\_MAX_ID\_\___. When this value is underestimated, execution fails, but not necessarily straightaway. 

### Input forms
#### CNF
This input corresponds to the classical dimacs CNF form. 
#### Example 
&#172; x<sub>3</sub>  &#8744;  x<sub>2</sub>

&#172; x<sub>3</sub>  &#8744;  x<sub>1</sub>

&#172; x<sub>1</sub> &#8744; &#172; x<sub>2</sub> &#8744; x<sub>3</sub>

is written as

```
p cnf 3 3
-3 2 0
-3 1 0
-1 -2 3 0
```


#### CNF-XOR
This input corresponds to the classical CNF-XOR input, where XOR-clauses are distinguished from OR-clauses by adding an 'x' at the start of the line.

#### Example
&#172; x<sub>3</sub>  &#8744;  x<sub>2</sub>

&#172; x<sub>3</sub>  &#8744;  x<sub>1</sub>

&#172; x<sub>1</sub> &#8744; &#172; x<sub>2</sub> &#8744; x<sub>3</sub>

x<sub>1</sub> &#8853; x<sub>2</sub> &#8853; &#172; x<sub>4</sub> &#8853; x<sub>5</sub>

x<sub>1</sub> &#8853; x<sub>3</sub> &#8853; x<sub>6</sub>

is written as

```
p cnf 6 5
-3 2 0
-3 1 0
-1 -2 3 0
x 1 2 -4 5 0
x 1 3 6 0
```

#### ANF
This input describes XOR-clauses that can contain both unary literals and conjunctions of two or more literals. When a term is unary, only the corresponding literal is written. When a term is a conjunction, first we write ```.d```, where ```d``` is the number of literals that comprise the conjunction and then, we list the literals. There can not be negative literals in the formula, but we can add a &#8868; constant instead. 

#### Example
x<sub>1</sub> &#8853; x<sub>2</sub> &#8853; x<sub>4</sub> &#8853; x<sub>5</sub> &#8853; &#8868;

x<sub>1</sub> &#8853; (x<sub>1</sub> &#8743; x<sub>2</sub>) &#8853; x<sub>6</sub>

(x<sub>2</sub> &#8743; x<sub>3</sub> &#8743; x<sub>6</sub>) &#8853; x<sub>6</sub> &#8853; &#8868;

is written as

```
p cnf 6 3
x 1 2 4 5 T 0
x 1 .2 1 2 6 0
x .3 2 3 6 6 T 0
```

