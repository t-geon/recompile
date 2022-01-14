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
The moment when the optimized code is saved is when a duplicate operation occurs and then another operation occurs.
Add, sub, and imul are optimized using o2, and div is optimized as above, and then div dl operation is added as it is and optimized.
As described above, if the range is exceeded, the command is completed once and then restarted.
I continued reading the original code using num and generated the optimization code through save.
When a new instruction is issued through check, the code is optimized by determining whether a previously duplicate instruction has arrived.

4. Execute the modified function (add execute permission to the modified function (r-x))
To execute the code stored in the compiled_code area, you must have execution permission.
All copied and optimized codes are stored in compiled_code.
However, the area is set to have read and write permissions.
Use the mprotect function to change the compiled_code area to read and execute permissions.
해당 영역의 권한을 바꿔주면 이전에 func에 저장해 둔 코드들을 실행할 수 있다.
