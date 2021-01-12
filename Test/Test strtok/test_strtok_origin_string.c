#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char test_str[] = "1-2-3-4-5-6";

    char* token;
    token = strtok(test_str, "-");
    printf("Origin string: %s\n", test_str);

    while(token != NULL)
    {
        printf("token: %s\n", token);
        token = strtok (NULL, "-");
        printf("Origin string: %s\n", test_str);
    }
}

/*
    NOTICE: strtok sửa đổi chuỗi đầu vào của nó:
    Nó thay thế ky tự ngắtbằng ký tự null ('\ 0') trong đó để nó trả về các bit của chuỗi gốc dưới dạng mã thông báo.
    Trong thực tế strtok không phân bổ bộ nhớ.
*/
