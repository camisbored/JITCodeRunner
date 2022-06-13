#include <stddef.h>    // size_t
#include <stdint.h>    // uint8_t, uint32_t
#include <stdio.h>     // printf
#include <memory.h>    // memcpy_s
#include <tchar.h>     // Must be included before strsafe.h
#include <strsafe.h>
#include <windows.h>

int runCode(uint8_t *instructions, int size);
void convertAndRunCode(char *array);

//Main function- starts loop that will take byte strings as input then run dynamically
int main(int argc, char **argv) {
    char str[6144];
    printf("\nEnter a series of bytes to perform instructions, or type exit to exit!\n");
    printf("IE- B878563412C3 would return 12345678, C3 would just return.\n");
    while (strcmp(str, "exit") != 0){
        scanf(" %s", str);
		if (strlen(str)==510)
		{
		  char extraChars[512];
		  do {
			memset(extraChars,0,strlen(extraChars));
			printf("Extended String detected- enter next section (510 characters per section): \n");
			scanf(" %s", extraChars);
			strcat(str, extraChars);
			 } while (strlen(extraChars)==510);
		}
        if (strcmp(str, "exit")!=0)
           convertAndRunCode(str);
    }
    return 0;
}

//function to convert given string to bytes, then pass to function to be run.
//this should be called directly to run byte sequences
void convertAndRunCode(char *array){
        int sizeCounter=0;
        for (int i=0; i<strlen(array); i+=2)
                sizeCounter++;

        uint8_t codeSeq[sizeCounter];
        for (int i=0, counter = 0; i<strlen(array);){
                char tempArr[2];
                tempArr[0]=array[i++];
                tempArr[1]=array[i++];
                codeSeq[counter++]= strtol(tempArr, NULL, 16);
        }
        runCode(codeSeq, sizeCounter);
}

//function to handle errors and run code on new memory page
//this will print out the value of eax at time of return, which 
//holds the return value
int runCode(uint8_t *instructions, int size){
    LPVOID mem = VirtualAlloc(NULL, size,
        MEM_COMMIT, PAGE_READWRITE);
    if (mem == NULL) {
        printf("VirtualAlloc");
        return 1;
    }

    if (memcpy_s(mem, size, instructions, size)) {
        printf("memcpy_s");
        return 2;
    }

    DWORD ignore;
    if (!VirtualProtect(mem, size, PAGE_EXECUTE_READ, &ignore)) {
        printf("VirtualAlloc");
        return 3;
    }

    if (!FlushInstructionCache(GetCurrentProcess(), mem, size)) {
        printf("FlushInstructionCache");
        return 4;
    }

    uint32_t(*fn)() = (uint32_t(*)()) mem;
    uint32_t result = fn();
    _tprintf(TEXT("result = 0x%x\n"), result);

    if (!VirtualFree(mem, 0, MEM_RELEASE)) {
        printf("VirtualFree");
        return 5;
    }

    return 0;
}
