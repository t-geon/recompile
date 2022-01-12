# recompile

Save the compiled assembly code of D_recompile_test.c in shared memory.
After accessing the shared memory using D_recompile.c, copy the code and optimize the copied code.
Duplicate operations are optimized by combining, and after optimization, the code is executed and the code is implemented so that the times before and after optimization are output respectively.

- How to implement
1. Accessing compiled code in shared memory
The sharedmem_init() function is a function that accesses shared memory.
You can access the code stored in the shared memory through shmget and shmat.
Since the key value of the shrared memory is stored as 1234, when requesting the shared memory segment, the key value is set to 1234.
Stores a pointer to a shared memory segment in Operation.

2. Since the function in the code area cannot be modified, it is copied to the area with write permission.
Since the code accessed in 1) cannot be directly modified, an area with write permission must be allocated.
Allocate the area using the drecompile_init() function.
I created a file called t, wrote a character to the file with write, and allocated an area using mmap.
If you write a value to compiled_code without writing anything, an error occurs and you write to the file.
Finally, I copied the compiled code into Operation via strcpy.

3. 함수를 최적화 및 수정 (0으로 나누기, -로 인한 음수처리 없음)
함수를 최적화하는 부분은 drecompile()함수이다.
해당 함수에서는 add, sub, imul, div명령어를 최적화한다.
compiled_code가 unit8_t이기 때문에 값이 128인경우부터 -128으로 인식한다.
즉 부호가 있는 값으로 인식하면 8비트 이상의 값은 overflow가 발생한 bit는 버리고 계산하기 때문에 해당 최적화를 할 때 0x80값이 넘기 전에 1번 명령어를 완성한 뒤 다시 최적화하는 방향으로 프로그래밍을 진행했다.
op에 명령어, o1에 1번째 operand, o2에 2번째 operand를 저장해서 op과 o1이 같으면 같은 명령어이기 때문에 add, sub의 경우 o2에 더하고 imul의 경우 o2에 곱한다.
div는 dl을 사용하는데 dl은 인자로 받은 값이 저장되어 있기 때문에 함수가 실행되기 전에 값을 알 수 없다
즉 div를 최적화하기 위해서는 dl의 원래 값이 edx를 ebx에 저장해둔 후 ebx를 edx에 div가 나올 때 마다 곱해주면 중복된 div를 최적화해줄 수 있다.
이 때 div보다 imul, mul연산이 더 짧은 시간 걸리기 때문에 최적화로 볼 수 있다.
최적화된 코드가 저장되는 순간은 중복된 연산이 나오다가 다른 연산이 나온 경우이다.
add, sub, imul은 o2를 이용해 최적화하면 div는 위 방법처럼 최적화후 마지막은 div dl연산을 그대로 넣어 최적화한다.
위에서 설명한 것과 같이 해당 범위를 넘으면 한번 쓴 뒤 다시 시작했다.
num을 이용해 원본 코드를 계속 읽고 save를 통해 최적화 코드를 생성했다.
check를 통해 새로운 명령어가 왔을 때 이전에 중복된 명령어가 왔었는지 판단해 코드를 최적화한다.

4. 수정 함수 실행 (수정함수에 execute 권한 추가(r-x))
compiled_code영역에 저장된 코드를 실행하기 위해서는 실행권한이 있어야 한다.
복사한코드나 최적화한 코드는 모두 compiled_code에 저장되어 있다.
하지만 해당 영역은 읽기와 쓰기의 권한을 갖도록 설정해 뒀기 때문에 mprotect함수를 사용해 compiled_code영역을 읽기와 실행 권한으로 변경한다.
해당 영역의 권한을 바꿔주면 이전에 func에 저장해 둔 코드들을 실행할 수 있다.
