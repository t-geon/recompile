# recompile

dynamic recompilation을 하는 것으로 shared memory에 D_recompile_test.c의 연산을 하는 컴파일 된 어셈블리 코드를 저장한다.
D_recompile.c를 이용해 해당 shared memory에 접근해 code를 복사하고 복사한 code를 최적화하는 코드로 수정한다.
이 때 중복된 연산은 합치는 방식으로 최적화하며, 최적화 후에는 해당 코드를 실행해 최적화 전과 최적화 후의 시간을 각각 출력할 수 있도록 코드를 구현한다. 

1)	shared memory에서 컴파일 된 코드 접근
shared memory를 접근하는 함수는 sharedmem_init()함수로 shmget과 shmat을 통해 shared memory의 저장된 code를 접근할 수 있다. 이전에 shrared memory key값을 1234로한 뒤 저장했기 때문에 shared memory segment를 요청할 때 key값을 1234로 준다. 이후 Operation에 shared memory segment의 pointer를 저장한다.

2)	code 영역의 함수는 수정 불가능 하기 때문에 write권한 있는 영역으로 복사
1)에서 접근한 코드는 직접 수정할 수 없기 때문에 write권한이 있는 영역을 할당해 줘야한다. 이때 영역을 할당하는 함수는 drecompile_init()함수이다. mmap이라는 함수를 사용해서 영역을 할당하라는 제한이 있었기 때문에 mmap을 사용하기 위해 t라는 파일을 생성하고 write로 해당 파일에 문자를 쓴 뒤 mmap을 이용해 영역을 할당했다. 이때 write를 한 이유는 아무것도 쓰지 않은 상태에서 이후에 compiled_code에 값을 쓰면 에러가 발생해 write를 했다. 마지막으로 strcpy를 통해 Operation에 있는 코드를 모두 compiled_code에 복사했다.

3)	함수를 최적화 및 수정 (0으로 나누기, -로 인한 음수처리 없음)
함수를 최적화하는 부분은 drecompile()함수이다. 해당 함수에서는 add, sub, imul, div명령어를 최적화한다.
compiled_code가 unit8_t이기 때문에 값이 128인경우부터 -128으로 인식한다. 즉 부호가 있는 값으로 인식하면 8비트 이상의 값은 overflow가 발생한 bit는 버리고 계산하기 때문에 해당 최적화를 할 때 0x80값이 넘기 전에 1번 명령어를 완성한 뒤 다시 최적화하는 방향으로 프로그래밍을 진행했다.
op에 명령어, o1에 1번째 operand, o2에 2번째 operand를 저장해서 op과 o1이 같으면 같은 명령어이기 때문에 add, sub의 경우 o2에 더하고 imul의 경우 o2에 곱한다. div는 dl을 사용하는데 dl은 인자로 받은 값이 저장되어 있기 때문에 함수가 실행되기 전에 값을 알 수 없다. 즉 div를 최적화하기 위해서는 dl의 원래 값이 edx를 ebx에 저장해둔 후 ebx를 edx에 div가 나올 때 마다 곱해주면 중복된 div를 최적화해줄 수 있다. 이 때 div보다 imul, mul연산이 더 짧은 시간 걸리기 때문에 최적화로 볼 수 있다. 최적화된 코드가 저장되는 순간은 중복된 연산이 나오다가 다른 연산이 나온 경우이다. add, sub, imul은 o2를 이용해 최적화하면 div는 위 방법처럼 최적화후 마지막은 div dl연산을 그대로 넣어 최적화한다.
위에서 설명한 것과 같이 해당 범위를 넘으면 한번 쓴 뒤 다시 시작했다. num을 이용해 원본 코드를 계속 읽고 save를 통해 최적화 코드를 생성했다. check를 통해 새로운 명령어가 왔을 때 이전에 중복된 명령어가 왔었는지 판단해 코드를 최적화한다.

4)	수정 함수 실행 (수정함수에 execute 권한 추가(r-x))
compiled_code영역에 저장된 코드를 실행하기 위해서는 실행권한이 있어야 한다. 복사한코드나 최적화한 코드는 모두 compiled_code에 저장되어 있다. 하지만 해당 영역은 읽기와 쓰기의 권한을 갖도록 설정해 뒀기 때문에 mprotect함수를 사용해 compiled_code영역을 읽기와 실행 권한으로 변경한다. 해당 영역의 권한을 바꿔주면 이전에 func에 저장해 둔 코드들을 실행할 수 있다.

5)	컴파일한 코드와 기존 코드 실행시간 비교 (50번 실행해 표로 나타내고 평균 취하기)
질문을 한 결과 D_recompile.c의 전체 실행시간이 아닌 리컴파일한 코드와 기존 코드의 실행시간을 비교하는 것임을 알 수 있었다. 해당 실행시간을 비교하기 위해 func(1)이 실행되는 시간을 측정했다.
