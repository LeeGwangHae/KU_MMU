# KU_MMU
Basic design
![운영체제 디자인](C:\Users\yds05\osAssig\운영체제 디자인.jpg)

## For compile
+ gcc -o [실행 파일 이름] ku_cpu.c ku_trav.o
## To execute
+ [./실행 파일 이름] [input.txt] [physical memory 크기] [swap space 크기]
## input.txt 형식
pid va
1 10
1 20
1 100
.
.
2 20
2 10 -> 예시 값들은 형식이 맞다면 임의 변경 가능
## 1. KU_MMU_init
+ init freeList
+ init swap space

## 2. KU_run_proc


## 3. KU_page_fault

