#include <stdio.h>

void S(int a, int b, int c, int d, int data[4][8], int target[4][32]) {
  target[c][d] = data[a][b];
}

int main() {
  int data[4][8];
  int target[4][32] = {0}; // Initialize to 0

  // Fill data array with 1, 2, 3, ...
  int val = 1;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      data[i][j] = val++;
    }
  }

  // Run the given loop
  for (int c0 = 0; c0 <= 3; c0 += 1)
    for (int c1 = 0; c1 <= 31; c1 += 1)
      S(c1 % 4, -((-c0 - c1 + 39) % 8) + 7, c0, c1, data, target);

  // print the original array
  printf("Original data array:\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      printf("%3d ", data[i][j]);
    }
    printf("\n");
  }

  // Print resulting target array
  printf("Target array:\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 32; j++) {
      printf("%3d ", target[i][j]);
    }
    printf("\n");
  }

  return 0;
}
