# Programming language "overCooked"

## Build

Repo includes Makefile. Build with `make` in order to build all three programs at once.
Also each program directory (frontend, middleend, backend) has its own Makefile.

NOTE: `tree` library is used in the project! You can find it in its own repo. The only place it is included in is `common\src\common.h`.

## Usage

The project is the compiler of the programming language, separated into three parts: `fronted`, `middleend` and `backend`.

1. `Frontend` takes `text file with program` and creates `compilator tree`.
2. `Middleend` does some work with the `compilator tree`, e.g. folding constants.
3. `Backend` takes `compilator tree` and creates `assembler code`, which can be later executed using `SPU (Software Processing Unit)` from repo `maybe_calculator`.

### Command line flags (common for all parts of the compiler)

1. `-h` - prints help.
2. `-i` - specifies input file.
3. `-o` - specifies output file.
4. `-l` - specifies log file.

When there is no corresponding flag, default configuration is used (see below).

### Return values (common for all parts of the compiler)

Please, see corresponding `*_statuses.h`.

### Compiler Frontend

Run `frontend.exe` or `make run_front` from root folder.

Default configuration (all files are located in the root folder):

1. Input file: `prog.txt`.
2. Output file: `compiler_tree.txt`.
3. Log file: `stdout`.

### Compiler Middleend

Run `middleend.exe` or `make run_middle` from root folder.

Default configuration (all files are located in the root folder):

1. Input file: `compiler_tree.txt`.
2. Output file: `compiler_tree_new.txt`.
3. Log file: `stdout`.

### Compiler Backend

Run `backend.exe` or `make run_back` from root folder.

Default configuration (all files are located in the root folder):

1. Input file: `compiler_tree.txt`.
2. Output file: `asm.txt`.
3. Log file: `stdout`.

## Programming language description

### Main idea

Programs written in this programming language try to look like crazy recipes of some carzy dish. "Crazy" means that there is no list of ingredients in the beginning, or some other common sense things, which one can usually find in real-life recipes. So, the programmer plays the role of the author of a recipe, and the processor (SPU) - of the chef, who is going to cook it. That's why most of the verbs in the text of the program must be read in the imperative mood.

### Some notes and details

1. All space symbols are ignored, new line symbols also don't matter, but they separate keywords and other constructions from each other.
2. Everything surrounded by single `#` is considered a comment and is ignored, e.g. `Quickly Obtain #comment# 10 Units Of Milk!`. (NOT IMPLEMENTED YET)
3. Inside functions global variables can't be seen, only formal paramatres and local vars. (NOT IMPLEMENTED YET)

### Grammar

Note:

1. Terminal symbols and keywords are surrounded with qoutes `""`, non-terminal symbols are written as is.
2. `+` means `one or more`.
3. `*` means `zero or more`.
4. `?...?` means some special case.

```py
Prog      ::= "ProgStart" Operators "ProgEnd"
Operators ::= Op+
Op        ::= VarBirth | VarDeath | Assign | If | While
VarBirth  ::= "VarBirthOp" Num "UnitsOf" Id "Dot"
VarDeath  ::= "VarDeathOp" Id<Var> "Dot"
Assign    ::= "Asgn1" Expr "Asgn2" Id<Var> "Dot"
Expr      ::= Mulive ( ?Amp (group)? ("OpAdd" | "OpSub") Mulive )*
Mulive    ::= Unr (  ?Amp (group)? ( "OpMul" | "OpDiv" ) Unr )*
Unr       ::= (  ?Amp (group)? ?UnrOp (group)? Primal ) | Primal
Primal    ::= ("BracketOpn" Expr "BracketCls") | Id<Var>
Id        ::= ['a'-'z','A'-'Z']['a'-'z','A'-'Z','_','0'-'9']*
Num       ::= <float number>
If        ::= "If1" "Cond" ?CmpOp (group)? Expr "Than" Expr "If2" Operators "IfEnd"
While     ::= "While1" "Cond" ?CmpOp (group)? Expr "Than" Expr "While2" Operators "WhileEnd"
```

### Keywords

|Designation in grammar|Designation in program|Comment
|-|-|-|
|Amp_1|Angrily|
|Amp_2|Unfortunately|
|Amp_3|Thoroughly|
|Amp_4|Thickly|
|Asgn1|Place|
|Asgn2|Right Into|
|BracketOpn|The Following Prepared Beforehand:|
|CmpOp_equal|Just The Same Amount Of|
|CmpOp_lessOrEqual|The Same Amount Or Less Of|
|CmpOp_less|Less|
|CmpOp_moreOrEqual|The Same Amount Or More Of|
|CmpOp_more|More|
|CmpOp_notEqual|Not The Same Amount Of|
|Cond|There Happens To Be|
|If1|In Case|
|If2|Urgently Do The Following Steps:|
|IfEnd|Now, Breathe Out And Continue Whatever Your Were Doing!|
|OpAdd|Mixed With|
|OpDiv|Spread On|
|OpMul|Fried With|
|OpSub|Without|
|ProgEnd|That's All! Don't Forget To Check It On Your Friends Before Tasting Yourself!|
|ProgStart|The Recipe Of The Most Delicious Dish One Can Ever Imagine!|
|Than|Than|
|UnitsOf|Units Of|
|UnrOp_cos|Diced|
|UnrOp_exp|Grated|
|UnrOp_ln|Squeezed|
|UnrOp_minus|Rinsed|
|UnrOp_sin|Sliced|
|UnrOp_sqrt|Peeled|
|VarBirthOp|Quickly Obtain|
|VarDeathOp|Throw Away|
|While1|As long as|
|While2|Repeat the following:|
|WhileEnd|Go Further If You Are Tired Of Repetition!|

### Separating symbols

|Designation in grammar|Designation in program|Comment|
|-|-|-|
|Dot|!|
|BracketCls|,|

### Program examples

```c

The Recipe Of The Most Delicious Dish One Can Ever Imagine!
Quickly Obtain 10 Units Of Milk!
Quickly Obtain 1 Units Of Eggs!
Place Milk Right Into Eggs!
That's All! Don't Forget To Check It On Your Friends Before Tasting Yourself!

```
