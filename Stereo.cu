/* Copyright (c) 2007, University of North Carolina at Chapel Hill
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY University of North Carolina at Chapel Hill ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL University of North Carolina at Chapel Hill BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <cutil.h>

#define COMPUTE_BOTH_DEPTHMAPS
#define DOWNLOAD_BOTH_DEPTHMAPS
#define PERFORM_CONSISTENCY_CHECK
#define CONSISTENCY_THRESHOLD  1
/*ORIGINAL
#define IMAGE_ALIGN                 64

#define STEREO_MIND                 0
#define STEREO_MAXD                 55
#define STEREO_WINSIZE_W            7
#define STEREO_WINSIZE_H            7

#define STEREO_SHARED_W             96
#define STEREO_SHARED_H             38
#define STEREO_RADIUS_W             (STEREO_WINSIZE_W/2)
#define STEREO_RADIUS_H             (STEREO_WINSIZE_H/2)
#define STEREO_DRANGE               (STEREO_MAXD-STEREO_MIND+1)
#define STEREO_APRON_W              64
#define STEREO_APRON_H              (2*STEREO_RADIUS_H)
#define STEREO_BLOCK_W              (STEREO_SHARED_W-STEREO_APRON_W)
#define STEREO_BLOCK_H              (STEREO_SHARED_H-STEREO_APRON_H)
#define STEREO_THREADS_W            8
#define STEREO_THREADS_H            STEREO_SHARED_H
#define STEREO_SHARED_MEM           (STEREO_SHARED_W*STEREO_SHARED_H*2+STEREO_THREADS_W*STEREO_THREADS_H*4)
*/
/*TESTE1
#define IMAGE_ALIGN                 64

#define STEREO_MIND                 0
#define STEREO_MAXD                 127
#define STEREO_WINSIZE_W            7
#define STEREO_WINSIZE_H            7

#define STEREO_SHARED_W             96
#define STEREO_SHARED_H             38
#define STEREO_RADIUS_W             (STEREO_WINSIZE_W/2)
#define STEREO_RADIUS_H             (STEREO_WINSIZE_H/2)
#define STEREO_DRANGE               (STEREO_MAXD-STEREO_MIND+1)
#define STEREO_APRON_W              64
#define STEREO_APRON_H              (2*STEREO_RADIUS_H)
#define STEREO_BLOCK_W              (STEREO_SHARED_W-STEREO_APRON_W)
#define STEREO_BLOCK_H              (STEREO_SHARED_H-STEREO_APRON_H)
#define STEREO_THREADS_W            8
#define STEREO_THREADS_H            STEREO_SHARED_H
#define STEREO_SHARED_MEM           (STEREO_SHARED_W*STEREO_SHARED_H*2+STEREO_THREADS_W*STEREO_THREADS_H*4)
*/
/*TESTE2
#define IMAGE_ALIGN                 64

#define STEREO_MIND                 0
#define STEREO_MAXD                 127
#define STEREO_WINSIZE_W            11
#define STEREO_WINSIZE_H            11

#define STEREO_SHARED_W             96
#define STEREO_SHARED_H             38
#define STEREO_RADIUS_W             (STEREO_WINSIZE_W/2)
#define STEREO_RADIUS_H             (STEREO_WINSIZE_H/2)
#define STEREO_DRANGE               (STEREO_MAXD-STEREO_MIND+1)
#define STEREO_APRON_W              64
#define STEREO_APRON_H              (2*STEREO_RADIUS_H)
#define STEREO_BLOCK_W              (STEREO_SHARED_W-STEREO_APRON_W)
#define STEREO_BLOCK_H              (STEREO_SHARED_H-STEREO_APRON_H)
#define STEREO_THREADS_W            8
#define STEREO_THREADS_H            STEREO_SHARED_H
#define STEREO_SHARED_MEM           (STEREO_SHARED_W*STEREO_SHARED_H*2+STEREO_THREADS_W*STEREO_THREADS_H*4)
*/
#define IMAGE_ALIGN                 10

#define STEREO_MIND                 0
#define STEREO_MAXD                 55
#define STEREO_WINSIZE_W            7
#define STEREO_WINSIZE_H            7

#define STEREO_SHARED_W             96
#define STEREO_SHARED_H             38
#define STEREO_RADIUS_W             (STEREO_WINSIZE_W/2)
#define STEREO_RADIUS_H             (STEREO_WINSIZE_H/2)
#define STEREO_DRANGE               (STEREO_MAXD-STEREO_MIND+1)
#define STEREO_APRON_W              64
#define STEREO_APRON_H              (2*STEREO_RADIUS_H)
#define STEREO_BLOCK_W              (STEREO_SHARED_W-STEREO_APRON_W)
#define STEREO_BLOCK_H              (STEREO_SHARED_H-STEREO_APRON_H)
#define STEREO_THREADS_W            8
#define STEREO_THREADS_H            STEREO_SHARED_H
#define STEREO_SHARED_MEM           (STEREO_SHARED_W*STEREO_SHARED_H*2+STEREO_THREADS_W*STEREO_THREADS_H*4)

static int g_w;
static int g_h;
static int g_alignW;
static unsigned char *g_imageLeft;
static unsigned char *g_imageRight;
static float *g_disparityLeft;
static float *g_disparityRight;


__device__ int diff( int l, int r )
{
    return abs(l-r);
    //return (l-r)*(l-r);
}

__device__ float subpixel( int c0, int c1, int c2 )
{
    float denom,doff;
    denom = 2*(c0-2*c1+c2);
    if(denom<1e-2 || c1>c0 || c1>c2) {
        return 0;
    } else {
        doff = (c0-c2)/denom;
        return doff;
    }
}

__global__ void stereo( float *disparityLeft,
                        float *disparityRight,
                        const unsigned char *left,
                        const unsigned char *right,
                        size_t width )
{
    extern __shared__ unsigned char sdata[];
    unsigned char *sleft = sdata;
    unsigned char *sright = sdata + STEREO_SHARED_W*STEREO_SHARED_H;
    unsigned int *stemp = (unsigned int*)(sdata + STEREO_SHARED_W*STEREO_SHARED_H*2);
    float bestd[4];
    unsigned int sum;
    int ii,it,is,i;
    unsigned int lastcs[3];
    unsigned int bestcs[3];
    int d;

    // Read image blocks into shared memory.
    const int si = __mul24(threadIdx.y,STEREO_SHARED_W) + 4*threadIdx.x;
    const int gi = __mul24(__mul24(blockIdx.y,STEREO_BLOCK_H) + threadIdx.y,width) + __mul24(blockIdx.x,STEREO_BLOCK_W) + 4*threadIdx.x;
    *(unsigned int*)(sleft+si)                     = *(unsigned int*)(left+gi);
    *(unsigned int*)(sleft+si+4*STEREO_THREADS_W)  = *(unsigned int*)(left+gi+4*STEREO_THREADS_W);
    *(unsigned int*)(sleft+si+8*STEREO_THREADS_W)  = *(unsigned int*)(left+gi+8*STEREO_THREADS_W);
    *(unsigned int*)(sright+si)                     = *(unsigned int*)(right+gi);
    *(unsigned int*)(sright+si+4*STEREO_THREADS_W)  = *(unsigned int*)(right+gi+4*STEREO_THREADS_W);
    *(unsigned int*)(sright+si+8*STEREO_THREADS_W)  = *(unsigned int*)(right+gi+8*STEREO_THREADS_W);
    __syncthreads();

    // Do left/right matching with separable box filter.
    for(int pix=0; pix<4; pix++) {
        ii = __mul24(threadIdx.y,STEREO_SHARED_W)+STEREO_APRON_W-STEREO_RADIUS_W+4*threadIdx.x+pix;
        it = __mul24(threadIdx.y,STEREO_THREADS_W)+threadIdx.x;
        bestcs[0] = bestcs[1] = bestcs[2] = INT_MAX;
        for(d=STEREO_MIND; d<=STEREO_MAXD; d++) {
            sum = 0;
            // Horizontal sum.
            for(is=ii-STEREO_RADIUS_W; is<=ii+STEREO_RADIUS_W; is++) {
                sum += diff(sleft[is],sright[is-d]);
            }
            stemp[it] = sum;
            __syncthreads();
            if(threadIdx.y>=STEREO_RADIUS_H && threadIdx.y<STEREO_SHARED_H-STEREO_RADIUS_H) {
                // Vertical sum.
                sum = 0;
                is = it-STEREO_RADIUS_H*STEREO_THREADS_W;
                for(i=-STEREO_RADIUS_H; i<=STEREO_RADIUS_H; i++,is+=STEREO_THREADS_W) {
                    sum += stemp[is];
                }
                // Best.
                if(d==STEREO_MIND) {
                    lastcs[1] = lastcs[2] = sum;
                } else {
                    lastcs[0] = lastcs[1];
                    lastcs[1] = lastcs[2];
                    lastcs[2] = sum;
                    if(lastcs[1] < bestcs[1]) {
                        bestcs[0] = lastcs[0];
                        bestcs[1] = lastcs[1];
                        bestcs[2] = lastcs[2];
                        bestd[pix] = d-1;
                    }
                }
            }
        }
        bestd[pix] += subpixel(bestcs[0],bestcs[1],bestcs[2]);
    }
    // Write results.
    if(threadIdx.y>=STEREO_RADIUS_H && threadIdx.y<STEREO_SHARED_H-STEREO_RADIUS_H) {
		ii = blockIdx.x*STEREO_BLOCK_W + STEREO_APRON_W - STEREO_RADIUS_W + 4*threadIdx.x;
		it = blockIdx.y*STEREO_BLOCK_H + threadIdx.y;
		is = __mul24(it,width) + ii;
		*(float*)(disparityLeft+is+0) = bestd[0];
		*(float*)(disparityLeft+is+1) = bestd[1];
		*(float*)(disparityLeft+is+2) = bestd[2];
		*(float*)(disparityLeft+is+3) = bestd[3];
    }

#ifdef COMPUTE_BOTH_DEPTHMAPS
    // Do right/left matching with separable box filter.
    for(int pix=0; pix<4; pix++) {
        ii = __mul24(threadIdx.y,STEREO_SHARED_W)+STEREO_RADIUS_W+4*threadIdx.x+pix;
        it = __mul24(threadIdx.y,STEREO_THREADS_W)+threadIdx.x;
        bestcs[0] = bestcs[1] = bestcs[2] = INT_MAX;
        for(d=STEREO_MIND; d<=STEREO_MAXD; d++) {
            sum = 0;
            // Horizontal sum.
            for(is=ii-STEREO_RADIUS_W; is<=ii+STEREO_RADIUS_W; is++) {
                sum += diff(sleft[is+d],sright[is]);
            }
            stemp[it] = sum;
            __syncthreads();
            if(threadIdx.y>=STEREO_RADIUS_H && threadIdx.y<STEREO_SHARED_H-STEREO_RADIUS_H) {
                // Vertical sum.
                sum = 0;
                is = it-STEREO_RADIUS_H*STEREO_THREADS_W;
                for(i=-STEREO_RADIUS_H; i<=STEREO_RADIUS_H; i++,is+=STEREO_THREADS_W) {
                    sum += stemp[is];
                }
                // Best.
                if(d==STEREO_MIND) {
                    lastcs[1] = lastcs[2] = sum;
                } else {
                    lastcs[0] = lastcs[1];
                    lastcs[1] = lastcs[2];
                    lastcs[2] = sum;
                    if(lastcs[1] < bestcs[1]) {
                        bestcs[0] = lastcs[0];
                        bestcs[1] = lastcs[1];
                        bestcs[2] = lastcs[2];
                        bestd[pix] = d-1;
                    }
                }
            }
        }
        bestd[pix] += subpixel(bestcs[0],bestcs[1],bestcs[2]);
    }
    // Write results.
    if(threadIdx.y>=STEREO_RADIUS_H && threadIdx.y<STEREO_SHARED_H-STEREO_RADIUS_H) {
		ii = blockIdx.x*STEREO_BLOCK_W + STEREO_RADIUS_W + 4*threadIdx.x;
		it = blockIdx.y*STEREO_BLOCK_H + threadIdx.y;
		is = __mul24(it,width) + ii;
		*(float*)(disparityRight+is+0) = bestd[0];
		*(float*)(disparityRight+is+1) = bestd[1];
		*(float*)(disparityRight+is+2) = bestd[2];
		*(float*)(disparityRight+is+3) = bestd[3];
    }
#endif //COMPUTE_BOTH_DEPTHMAPS
}

__global__ void consistency( float *left,
							 float *right,
							 size_t width,
							 int consist )
{
    const int x = __mul24(blockIdx.x,blockDim.x) + threadIdx.x;
    const int y = __mul24(blockIdx.y,blockDim.y) + threadIdx.y;
    const int i = __mul24(y,width)+x;
	int dl = (int)left[i];
	int dlr = (int)right[i-dl];
#ifdef DOWNLOAD_BOTH_DEPTHMAPS
	int dr = (int)right[i];
	int drl = (int)left[i+dr];
#endif
	__syncthreads();
	if(abs(dl-dlr)>consist)
		left[i] = 255;
#ifdef DOWNLOAD_BOTH_DEPTHMAPS
	if(abs(dr-drl)>consist)
		right[i] = 255;
#endif
}

int align( int n, int a )
{
    int r = n % a;
    if(r==0)
        return n;
    else
        return n-r+a;
}

extern "C" void stereoInit( int w, int h )
{
    size_t pitch;
    cudaChannelFormatDesc fmt = cudaCreateChannelDesc<unsigned int>();

    g_w = w;
    g_h = h;
    g_alignW = align(w,IMAGE_ALIGN);

    CUDA_SAFE_CALL(cudaMallocPitch((void**)&g_imageLeft,&pitch,g_alignW,h));
    CUDA_SAFE_CALL(cudaMallocPitch((void**)&g_imageRight,&pitch,g_alignW,h));
    CUDA_SAFE_CALL(cudaMallocPitch((void**)&g_disparityLeft,&pitch,g_alignW*sizeof(float),h));
    CUDA_SAFE_CALL(cudaMallocPitch((void**)&g_disparityRight,&pitch,g_alignW*sizeof(float),h));
	CUDA_SAFE_CALL(cudaMemset(g_disparityLeft,0,g_alignW*sizeof(float)*h));
	CUDA_SAFE_CALL(cudaMemset(g_disparityRight,0,g_alignW*sizeof(float)*h));
}

extern "C" void stereoUpload( const unsigned char *left, const unsigned char *right )
{
    CUDA_SAFE_CALL(cudaMemcpy2D(g_imageLeft,g_alignW,left,g_w,g_w,g_h,
        cudaMemcpyHostToDevice));
    CUDA_SAFE_CALL(cudaMemcpy2D(g_imageRight,g_alignW,right,g_w,g_w,g_h,
        cudaMemcpyHostToDevice));
}

int divUp( int n, int d )
{
    if(n%d==0)
        return n/d;
    else
        return n/d + 1;
}

extern "C" void stereoProcess()
{
    // Disparity map computation.
    dim3 threads(STEREO_THREADS_W,STEREO_THREADS_H);
    dim3 grid((g_alignW-STEREO_APRON_W)/STEREO_BLOCK_W,g_h/STEREO_BLOCK_H);
    stereo<<<grid,threads,STEREO_SHARED_MEM>>>(g_disparityLeft,g_disparityRight,
        g_imageLeft,g_imageRight,g_alignW);

#if defined(PERFORM_CONSISTENCY_CHECK) && defined(COMPUTE_BOTH_DEPTHMAPS)
    // Consistency check.
    dim3 cthreads(16,16);
    dim3 cgrid(g_alignW/cthreads.x,g_h/cthreads.y);
    consistency<<<cgrid,cthreads>>>(g_disparityLeft,g_disparityRight,g_alignW,
        CONSISTENCY_THRESHOLD);
#endif
}

extern "C" void stereoDownload( float *disparityLeft, float *disparityRight )
{
    CUDA_SAFE_CALL(cudaMemcpy2D(disparityLeft,g_w*sizeof(float),g_disparityLeft,g_alignW*sizeof(float),g_w*sizeof(float),g_h,
        cudaMemcpyDeviceToHost));
#if defined(DOWNLOAD_BOTH_DEPTHMAPS) && defined(COMPUTE_BOTH_DEPTHMAPS)
    CUDA_SAFE_CALL(cudaMemcpy2D(disparityRight,g_w*sizeof(float),g_disparityRight,g_alignW*sizeof(float),g_w*sizeof(float),g_h,
        cudaMemcpyDeviceToHost));
#endif
}
