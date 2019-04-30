/*
 * Kepei Lei
 * Mingxi Liu
 */

/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is etmpuated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int i, j, k, l;

    if (N == 32) {
        // Divide into 8 x 8 blocks and utilize the allowed amount of variables to transfer them.
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j ++) {
                for (k = i * 8; k < (i + 1) * 8; k ++) {
                    l = j * 8;
                    tmp1 = A[k][h];
                    tmp2 = A[k][h + 1];
                    tmp3 = A[k][h + 2];
                    tmp4 = A[k][h + 3];
                    tmp5 = A[k][h + 4];
                    tmp6 = A[k][h + 5];
                    tmp7 = A[k][h + 6];
                    tmp8 = A[k][h + 7];
                    B[h][k] = tmp1;
                    B[h + 1][k] = tmp2;
                    B[h + 2][k] = tmp3;
                    B[h + 3][k] = tmp4;
                    B[h + 4][k] = tmp5;
                    B[h + 5][k] = tmp6;
                    B[h + 6][k] = tmp7;
                    B[h + 7][k] = tmp8;
                }
            }
        }

    } else if (N == 64) {
        // For each 8x8 block
        // We load in the top half of A into B, but the top right quarter of A will also be in top right corner of B
        // Next step, we move the top right of B to where it should be (bottom left) and move the bottom left in A into
        // the top right of B (This is the key operation)
        // We finish the bottom right corner moving
        for (i = 0; i < 64; i += 8) {
            for (j = 0; j < 64; j += 8) {
                for(k = j;k < j + 4; ++k){
                    tmp1 = A[k][i];
                    tmp2 = A[k][i + 1];
                    tmp3 = A[k][i + 2];
                    tmp4 = A[k][i + 3];
                    tmp5 = A[k][i + 4];
                    tmp6 = A[k][i + 5];
                    tmp7 = A[k][i + 6];
                    tmp8 = A[k][i + 7];

                    B[i][k] = tmp1;
                    B[i + 1][k] = tmp2;
                    B[i + 2][k] = tmp3;
                    B[i + 3][k] = tmp4;
                    B[i][k+ 4] = tmp5;
                    B[i + 1][k + 4] = tmp6;
                    B[i + 2][k + 4] = tmp7;
                    B[i + 3][k + 4] = tmp8;
                }
                for(k= i;k< i + 4; ++k){
                    tmp1 = B[k][j + 4];
                    tmp2 = B[k][j + 5];
                    tmp3 = B[k][j + 6];
                    tmp4 = B[k][j + 7];
                    tmp5 = A[j + 4][k];
                    tmp6 = A[j + 5][k];
                    tmp7 = A[j + 6][k];
                    tmp8 = A[j + 7][k];

                    B[k + 4][j] = tmp1;
                    B[k + 4][j + 1] = tmp2;
                    B[k + 4][j + 2] = tmp3;
                    B[k + 4][j + 3] = tmp4;
                    B[k][j + 4] = tmp5;
                    B[k][j + 5] = tmp6;
                    B[k][j + 6] = tmp7;
                    B[k][j + 7] = tmp8;
                }
                for(k= i + 4;k < i + 8; ++k){
                    tmp1 = A[j + 4][k];
                    tmp2 = A[j + 5][k];
                    tmp3 = A[j + 6][k];
                    tmp4 = A[j + 7][k];

                    B[k][j + 4] = tmp1;
                    B[k][j + 5] = tmp2;
                    B[k][j + 6] = tmp3;
                    B[k][j + 7] = tmp4;
                }
            }
        }
    } else {
        // Divide up into 16x16 blocks, and deal with left overs
        // Somehow this is easier than 64 x 64
        for (i = 0; i+16 < N; i+=16) {
            for (j = 0; j+16 < M; j+=16) {
                for (k = i; k < i+16; ++k) {
                    tmp1 = A[k][j];
                    tmp2 = A[k][j + 1];
                    tmp3 = A[k][j + 2];
                    tmp4 = A[k][j + 3];
                    tmp5 = A[k][j + 4];
                    tmp6 = A[k][j + 5];
                    tmp7 = A[k][j + 6];
                    tmp8 = A[k][j + 7];
                    B[j][k] = tmp1;
                    B[j + 1][k] = tmp2;
                    B[j + 2][k] = tmp3;
                    B[j + 3][k] = tmp4;
                    B[j + 4][k] = tmp5;
                    B[j + 5][k] = tmp6;
                    B[j + 6][k] = tmp7;
                    B[j + 7][k] = tmp8;

                    tmp1 = A[k][j + 8];
                    tmp2 = A[k][j + 9];
                    tmp3 = A[k][j + 10];
                    tmp4 = A[k][j + 11];
                    tmp5 = A[k][j + 12];
                    tmp6 = A[k][j + 13];
                    tmp7 = A[k][j + 14];
                    tmp8 = A[k][j + 15];
                    B[j + 8][k] = tmp1;
                    B[j + 9][k] = tmp2;
                    B[j + 10][k] = tmp3;
                    B[j + 11][k] = tmp4;
                    B[j + 12][k] = tmp5;
                    B[j + 13][k] = tmp6;
                    B[j + 14][k] = tmp7;
                    B[j + 15][k] = tmp8;

                }
            }
        }

        for (k = i; k < N; ++k) {
            for (l = 0; l < M; ++l){
                B[l][k] = A[k][l];
            }
        }
        for (k = 0; k < i; ++k) {
            for (l = j; l < M; ++l) {
                B[l][k] = A[k][l];
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     etmpuate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

