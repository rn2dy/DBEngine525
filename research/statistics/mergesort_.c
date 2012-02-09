#include <limits.h>
#include <stdio.h>

// Yet another IN-MEMORY-SORTING algorithm! 
// The problem with this merge sort is that it consumes memory
// The maximum memeroty it consumes is about: 
//    length of input array/2 * sizeof(int)
// In another word, the memory grows in linear fashion, but since it uses
// stack here, it will cause problem if the input is just too much.

// the merge step
void do_merge(int a[], int low, int mid, int high){
  int n1, n2, i, j, k;
  n1 = mid - low + 1;
  n2 = high - mid;

  //Plus 1 because a sentinel value will be put at the end!
  int LEFT[n1+1]; 
  int RIGHT[n2+1];
  for(i = 0; i < n1; i++){
    LEFT[i] = a[low+i];
  }
  for(j = 0; j < n2; j++){
    RIGHT[j] = a[mid+j+1]; // + 1 !
  }
  // Set the sentinels
  LEFT[n1] = INT_MAX; 
  RIGHT[n2] = INT_MAX; 

  // The real magic 
  i = 0; j = 0;
  for(k = low; k <= high; k++){
    if(LEFT[i] <= RIGHT[j]){
      a[k] = LEFT[i];
      i++;
    }else{
      a[k] = RIGHT[j];
      j++;
    }
  }
}

void mergesort(int a[], int low, int high){
  if(low < high){
    int mid = (low + high)/2;// Take floor  
    mergesort(a, low, mid);
    mergesort(a, mid+1, high);
    do_merge(a, low, mid, high);
  }
}

