# 2021 Operating System First Assignment KU_MMU  
##Basic design  
![운영체제 디자인](https://user-images.githubusercontent.com/62276222/117556268-fb767600-b0a1-11eb-8b85-ca377e4dd1fa.jpg)
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
