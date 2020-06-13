#include <stdio.h>

int main()
{
    FILE* fd[2];
    fd[0] = fopen("alphabet.txt","w");
    fd[1] = fopen("alphabet.txt","w");
    
    char* abc = "abcdefghigklmnopqrstuvwxyz";

    for (i = 0; i < 26; ++i)
    {
        if (i % 2 == 0)
            fprintf(fd[0], "%c", abc[i]);
        else
            fprintf(fd[1], "%c", abc[i]);
    }
    
    fclose(fd[0]);
    fclose(fd[1]);
    return 0;
}
