#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
 
/* qsort int comparison function */ 
int int_cmp(const void *a, const void *b) 
{ 
    const int *ia = (const int *)a; // casting pointer types 
    const int *ib = (const int *)b;
    return *ia  - *ib; 
	/* integer comparison: returns negative if b > a 
	and positive if a > b */ 
} 
 
/* integer array printing function */ 
void print_int_array(const int *array, size_t len) 
{ 
    size_t i;
 
    for(i=0; i<len; i++) 
        printf("%d | ", array[i]);
 
    putchar('\n');
} 
 
/* sorting integers using qsort() example */ 
void sort_integers_example() 
{ 
    int numbers[] = { 7, 3, 4, 1, -1, 23, 12, 43, 2, -4, 5 }; 
    size_t numbers_len = sizeof(numbers)/sizeof(int);
 
    puts("*** Integer sorting...");
 
    /* print original integer array */ 
    print_int_array(numbers, numbers_len);
 
    /* sort array using qsort functions */ 
    qsort(numbers, numbers_len, sizeof(int), int_cmp);
 
    /* print sorted integer array */ 
    print_int_array(numbers, numbers_len);
} 
 
/* qsort C-string comparison function */ 
int cstring_cmp(const void *a, const void *b) 
{ 
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
	/* strcmp functions works exactly as expected from
	comparison function */ 
} 
 
/* C-string array printing function */ 
void print_cstring_array(char **array, size_t len) 
{ 
    size_t i;
 
    for(i=0; i<len; i++) 
        printf("%s | ", array[i]);
 
    putchar('\n');
} 
 
/* sorting C-strings array using qsort() example */ 
void sort_cstrings_example() 
{ 
    char *strings[] = { "Zorro", "Alex", "Celine", "Bill", "Forest", "Dexter" };
    size_t strings_len = sizeof(strings) / sizeof(char *);
 
    /** STRING */ 
    puts("*** String sorting...");
 
    /* print original string array */ 
    print_cstring_array(strings, strings_len);
 
    /* sort array using qsort functions */ 
    qsort(strings, strings_len, sizeof(char *), cstring_cmp);
 
    /* print sorted string array */ 
    print_cstring_array(strings, strings_len);
} 
 
 
 
/* an example of struct */ 
struct st_ex { 
    char product[16];
    float price;
};
 
/* qsort struct comparision function (price float field) */ 
int struct_cmp_by_price(const void *a, const void *b) 
{ 
    struct st_ex *ia = (struct st_ex *)a;
    struct st_ex *ib = (struct st_ex *)b;
    return (int)(100.f*ia->price - 100.f*ib->price);
	/* float comparison: returns negative if b > a 
	and positive if a > b. We multiplied result by 100.0
	to preserve decimal fraction */ 
 
} 
 
/* qsort struct comparision function (product C-string field) */ 
int struct_cmp_by_product(const void *a, const void *b) 
{ 
    struct st_ex *ia = (struct st_ex *)a;
    struct st_ex *ib = (struct st_ex *)b;
    return strcmp(ia->product, ib->product);
	/* strcmp functions works exactly as expected from
	comparison function */ 
} 
 
/* Example struct array printing function */ 
void print_struct_array(struct st_ex *array, size_t len) 
{ 
    size_t i;
 
    for(i=0; i<len; i++) 
        printf("[ product: %s \t price: $%.2f ]\n", array[i].product, array[i].price);
 
    puts("--");
} 
 
/* sorting structs using qsort() example */ 
void sort_structs_example(void) 
{ 
    struct st_ex structs[] = {{"mp3 player", 299.0f}, {"plasma tv", 2200.0f}, 
                              {"notebook", 1300.0f}, {"smartphone", 499.99f}, 
                              {"dvd player", 150.0f}, {"matches", 0.2f }};
 
    size_t structs_len = sizeof(structs) / sizeof(struct st_ex);
 
    puts("*** Struct sorting (price)...");
 
    /* print original struct array */ 
    print_struct_array(structs, structs_len);
 
    /* sort array using qsort functions */ 
    qsort(structs, structs_len, sizeof(struct st_ex), struct_cmp_by_price);
 
    /* print sorted struct array */ 
    print_struct_array(structs, structs_len);
 
    puts("*** Struct sorting (product)...");
 
    /* resort using other comparision function */ 
    qsort(structs, structs_len, sizeof(struct st_ex), struct_cmp_by_product);    
 
    /* print sorted struct array */ 
    print_struct_array(structs, structs_len);
} 
 
 
/* MAIN program (calls all other examples) */ 
int main() 
{ 
    /* run all example functions */ 
    sort_integers_example();
    sort_cstrings_example();
    sort_structs_example();
    return 0;
}







// #include <stdio.h>
// #include <unistd.h>   //sleep
// #include <stdint.h>   //uint_8, uint_16, uint_32, etc.
// #include <ctype.h>    //isalnum, tolower
// #include <stdlib.h>
// #include <fcntl.h>
// #include <string.h>

// #include "shared.h"
// #include "typedefs.h"

// // /* character sort utility */
// // void CharSort( const char * arr)
// // {
// //     int i, j, min_idx;
// //     char minStr[_GROUP_NAME_SIZE];
// //     // One by one move boundary of unsorted subarray
// //     for (i = 0; i < _NUMBER_OF_SENSORS-1; i++)
// //     {
// //         // Find the minimum element in unsorted array
// //         min_idx = i;
// //         strcpy(minStr, arr[i]);
// //         for (j = i+1; j < _NUMBER_OF_SENSORS; j++)
// //         {
// //         	printf("i %d, j %d\n", i, j);
// //             // If min is greater than arr[j]
// //             if (strcmp(minStr, arr[j]) > 0)
// //             {
// //                 // Make arr[j] as minStr and update min_idx
// //                 strcpy(minStr, arr[j]);
// //                 min_idx = j;
// //             }
// //         }

// //         // Swap the found minimum element with the first element
// //         if (min_idx != i)
// //         {
// //             char temp[_GROUP_NAME_SIZE];
// //             strcpy(temp, arr[i]); //swap item[pos] and item[i]
// //             strcpy(arr[i], arr[min_idx]);
// //             strcpy(arr[min_idx], temp);
// //         }
// //     }
// //     return;
// // }

// /* intiger sort utility */
// void IntSort(int *array)
// {
//     int i, j, temp;
 
//     for (i = 0; i < _NUMBER_OF_SENSORS; i++)
//     {
//         for (j = 0; j < (_NUMBER_OF_SENSORS - i - 1); j++)
//         {
//             if (array[j] > array[j + 1])
//             {
//                 temp = array[j];
//                 array[j] = array[j + 1];
//                 array[j + 1] = temp;
//             }
//         }
//     }
//     return;
// }

// int cstring_cmp (const void * a, const void * b)
// {
// 	printf("cstring_cmp called\n");
//     const char **ia = (const char **)a;
//     const char **ib = (const char **)b;
//     return strcmp(*ia, *ib);
// }

// int main(int argc, char *argv[])
// {
// 	int    			a[_NUMBER_OF_SENSORS] = {4,7,1,6,2,9,0,67,31,0,23};
// 	// const char * 	b[_NUMBER_OF_SENSORS][] = {"zong     ", "aplf     ", "frt       ", "last     ", "first    "};
// 	int 	i;

// 	char *b[_NUMBER_OF_SENSORS] = { "Z", "A", "C", "B", "F", "D" };
//     size_t strings_len = sizeof(b) / sizeof(char *);


// 	 for(i=0;i<_NUMBER_OF_SENSORS;i++)
// 	 	printf("index %i,  sid %i \n",i, a[i]);
// 	 printf("\n");

// 	 IntSort(a);

// 	 for(i=0;i<_NUMBER_OF_SENSORS;i++)
// 	 	printf("index %i,  sid %i \n",i, a[i]);
// 	 printf("\n");

// 	 for(i=0;i<_NUMBER_OF_SENSORS;i++)
// 	 	printf("index %i,  group %s \n",i, b[i]);
// 	 printf("\n");

// 	 // CharSort(b);
// 	 qsort(b, _GROUP_NAME_SIZE, _NUMBER_OF_SENSORS, cstring_cmp);
// 	 // qsort(b, strings_len, sizeof(char *), cstring_cmp);

// 	 for(i=0;i<_NUMBER_OF_SENSORS;i++)
// 	 	printf("index %i,  group %s \n",i, b[i]);
// 	 printf("\n");


// 	return 0;
// }

