# recompile

dynamic recompilation을 하는 것으로 shared memory에 D_recompile_test.c의 연산을 하는 컴파일 된 어셈블리 코드를 저장한다.
D_recompile.c를 이용해 해당 shared memory에 접근해 code를 복사하고 복사한 code를 최적화하는 코드로 수정한다.
이 때 중복된 연산은 합치는 방식으로 최적화하며, 최적화 후에는 해당 코드를 실행해 최적화 전과 최적화 후의 시간을 각각 출력할 수 있도록 코드를 구현한다. 
