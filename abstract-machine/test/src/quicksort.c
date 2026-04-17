#include <stdio.h>
#define N 8

int a[N] = {6, 4, 7, 1, 7, 9, 0, 5};

int partition(int start, int end) {
    int pivot = a[start];
    int n = start + 1;
    int m = end;

    while (n <= m) {
        while (n <= end && a[n] <= pivot) {
            n++;
        }
        while (a[m] > pivot) {
            m--;
        }

        if (n < m) {
            int tmp = a[n];
            a[n] = a[m];
            a[m] = tmp;
        }
    }
    int tmp = a[start];
    a[start] = a[m];
    a[m] = tmp;
    // printf("%d\n", m);
    return m;
}

void quicksort(int start, int end)
{
	int mid;
	if (end > start) {
		mid = partition(start, end);
		quicksort(start, mid-1);
		quicksort(mid+1, end);
	}
}

int main (void) {
    quicksort(0, N - 1);
    // for (int i = 0; i < N; i++) {
    //     printf("%d ", a[i]);
    // }
    // printf("\n");
    return 0;
}