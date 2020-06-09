/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
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
int i,j,k,h,temp0,temp1,temp2,temp3,temp4,temp5,temp6,temp7;
	if(N==32&&M==32){
		for(j=0;j<32;j+=8){
			for(i=0;i<32;i++){
				temp0=A[i][j];
				temp1=A[i][j+1];
				temp2=A[i][j+2];
				temp3=A[i][j+3];
				temp4=A[i][j+4];
				temp5=A[i][j+5];
				temp6=A[i][j+6];
				temp7=A[i][j+7];
				B[j][i]=temp0;
				B[j+1][i]=temp1;
				B[j+2][i]=temp2;
				B[j+3][i]=temp3;
				B[j+4][i]=temp4;
				B[j+5][i]=temp5;
				B[j+6][i]=temp6;
				B[j+7][i]=temp7;
			}
		}
	}
	
	if(N==64&&M==64){
		for(j=0;j<64;j+=8){
			for(i=0;i<64;i+=8){
				for(k=i;k<i+4;k++){
					temp0=A[k][j];
					temp1=A[k][j+1];
					temp2=A[k][j+2];
					temp3=A[k][j+3];
					temp4=A[k][j+4];
					temp5=A[k][j+5];
					temp6=A[k][j+6];
					temp7=A[k][j+7];
					
					B[j][k]=temp0;
					B[j+1][k]=temp1;
					B[j+2][k]=temp2; 
					B[j+3][k]=temp3;
					B[j][k+4]=temp4;
					B[j+1][k+4]=temp5;
					B[j+2][k+4]=temp6;
					B[j+3][k+4]=temp7;
				}
				
				for(k=j;k<j+4;k++){
					temp0=B[k][i+4];
					temp1=B[k][i+5];
					temp2=B[k][i+6];
					temp3=B[k][i+7];

					temp4=A[i+4][k];
					temp5=A[i+5][k];
					temp6=A[i+6][k];
					temp7=A[i+7][k];
					
					B[k][i+4]=temp4;
					B[k][i+5]=temp5;
					B[k][i+6]=temp6;
					B[k][i+7]=temp7;
					
					B[k+4][i]=temp0;
                                        B[k+4][i+1]=temp1;
                                        B[k+4][i+2]=temp2;
                                        B[k+4][i+3]=temp3;

				}
				for(k=j+4;k<j+8;k++){
					temp0=A[i+4][k];
					temp1=A[i+5][k];
					temp2=A[i+6][k];
					temp3=A[i+7][k];

					B[k][i+4]=temp0;
					B[k][i+5]=temp1;
					B[k][i+6]=temp2;
					B[k][i+7]=temp3;
				}
			}
		}
	}	
	
	if(M==61&&N==67){
		for(j=0;j<M;j+=17){
			for(i=0;i<N;i+=17){
				for(h=j;h<j+17&&h<M;h++){
					 for(k=i;k<i+17&&k<N;k++){
						B[h][k]=A[k][h];
					}
				}
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
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
   /* registerTransFunction(trans, trans_desc); */

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

