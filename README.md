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

3. Optimize and modify the function (divide by zero, no negative due to -)
The part that optimizes the function is the drecompile() function.
In this function, the add, sub, imul, and div commands are optimized.
Since compiled_code is unit8_t, the value is recognized as -128 from 128.
That is, if it is recognized as a signed value, 8 bits or more are discarded and calculated.
Therefore, when optimizing, programming was carried out in the direction of optimizing again after completing the instruction before the 0x80 value.
The instruction is stored in op, the 1st operand in o1, and the 2nd operand in o2. If op and o1 are the same, it is the same instruction. For add and sub, it is added to o2, and in the case of imul, it is multiplied by o2.
div uses dl. However, since the value received as an argument is stored in dl, the value cannot be known before the function is executed.
To optimize div, store edx, the original value of dl, in ebx, and then multiply ebx in edx every time a div appears to optimize the duplicate div.
At this time, it can be seen as an optimization because imul and mul operations take shorter time than div.
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
