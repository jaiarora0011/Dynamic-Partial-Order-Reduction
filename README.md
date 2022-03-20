# COL750 Assignment 2: Dynamic Partial Order Reduction

## Input Format
The input file is a `.txt` file. The input follows the following grammar given in `src/parse.y`.
Sample input:
```
P1 {
  t01: acquire(z)
  t02: x := 1
  t03: release(z)
}

P2 {
  t11: acquire(z)
  t12: x := 2
  t13: release(z)
}


PROGRAM_ORDER: {(t01, t02), (t02, t03), (t01, t03), (t11, t12), (t12, t13), (t11, t13)}  
```

## Running the executable
- Calling `make` will compile all the files and generate an executable `dpor`
- To generate the output `.dot` file: `./dpor <input.txt> <output.dot>`. This will generate the `.dot` file in the `output` folder
- Run `make test` to run all the testcases in the `input` folder. This will also generate the final pdf in the `output` folder

## Status
Currently, the algorithm explores extra executions for some testcases