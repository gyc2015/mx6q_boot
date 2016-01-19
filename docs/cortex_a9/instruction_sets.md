# 指令集 Instruction Sets

ARMv7有两个主要的指令集: ARM和Thumb指令集. 两个指令集中大部分功能都是一样的, 其不同之处在于指令编码不同:

* Thumb指令集是16位或者32位编码的, 而且是2字节对齐的. 16位和32位指令之间可以自由地混合使用. 很多常用的操作都用
16位的代码做过优化, 但是大部分16位指令只能够访问前8个[内核寄存器(R0-R7)](/docs/cortex_a9/cortex_a9.md), 所有的
32位的代码都可以访问所有的内核寄存器.

* ARM指令集全是32位编码的, 而且是4字节对齐的.

不同的程序(procedures)可以编译为ARM或者Thumb的指令, 相互之间可以方便地调用.

ThumbEE指令集是针对动态代码(dynamically generated code)而设计的Thumb指令集的一个变种. 但是它不能够与ARM和Thumb指令
自由地相互调用.

处理器在ARM状态下执行ARM指令, 在Thumb状态下执行Thumb指令. Thumb状态下的处理器可以通过执行BX, BLX或者LDR, LDM直接装载PC寄存器,
进入ARM状态. 通过执行相同的指令也可以从ARM状态切换到Thumb状态.

在ARMv7中, ARM状态下的处理器也可以通过执行ADC, ADD, AND, ASR, BIC, EOR, LSL, LSR, MOV, MVN, ORR, ROR, RRX, RSB,
RSC, SBC或者SUB指令进入Thumb状态, 这需要以PC作为指令的目标寄存器, 并且不能设定条件.

进入异常处理程序或者返回时, 处理器都可以在ARM和Thumb状态之间切换.

## 条件执行 Conditional Execution

在ARM和Thumb指令集中, 大部分指令都可以被有条件地执行.

在ARM指令集中, 条件执行指的是一条指令只有在[程序状态寄存器APSR](/docs/cortex_a9/cortex_a9.md)的N, Z, C, V位满足特定条件时
才会被执行. 若条件不满足, 处理器将执行一条NOP指令.

每条ARM指令都包含4位的条件码(cond), 位于指令的最高4位[31:28]. 条件码共有16种, 每种条件嘛可用两个字符表示, 这两个字符可以添加
在指令助记符的后面和指令同时使用. 如BEQ是跳转指令B加上后缀BEQ表示"相等则跳转", 即, Z=1时跳转.

| cond | 助记符 | 意义(整形)                    | 意义(浮点数)                      | flag                       |
|:----:|:------:|:----------------------------:|:---------------------------------:|:--------------------------:|
| 0000 | EQ     | EQual                        | EQual                             | Z == 1                     |
| 0001 | NE     | Not Equal                    | Not Equal, or unordered           | Z == 0                     |
| 0010 | CS     | Carry Set                    | Greater than, equal, or unordered | C == 1                     |
| 0011 | CC     | Carry Clear                  | Less than                         | C == 0                     |
| 0100 | MI     | MInux, negative              | Less than                         | N == 1                     |
| 0101 | PL     | PLus, positive or zero       | Greater than, equal, or unordered | N == 0                     |
| 0110 | VS     | Overflow                     | Unordered                         | V == 1                     |
| 0111 | VC     | No overflow                  | Not unordered                     | V == 0                     |
| 1000 | HI     | Unsigned higher              | Greater than, or unordered        | C == 1 && Z == 0           |
| 1001 | LS     | unsigned Lower or same       | Less than or equal                | C == 0 &#124;&#124; Z == 1 |
| 1010 | GE     | Signed greater than or equal | Greater than or equal             | N == V                     |
| 1011 | LT     | Signed Less than             | Less than, or unordered           | N != V                     |
| 1100 | GT     | Signed greater than          | Greater than                      | Z == 0 && N == V           |
| 1101 | LE     | Signed less than or equal    | Less than, equal, or unordered    | Z == 1 &#124;&#124; N != V |
| 1110 | AL     | Always                       | Always                            | Any                        |

对于Thumb指令集, 若条件不是AL, 通常编码为IT指令, 也有一些不需要IT指令的.




---

* ARM推荐使用BX LR从一个程序中返回, 避免使用MOV PC, LR.
