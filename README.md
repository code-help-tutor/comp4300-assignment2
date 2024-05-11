# COMP4300/8300 Parallel Systems Assignment 2, 2024
# Advection Solver For Shared Address Space Programming Models

## Deadline: 26/05/2024, 11:55PM

*(Please report any errors, omissions and ambiguities to the course lecturer)* \
*(We may make small amendments and fixes to this document after its release; please keep an eye on the commit history after release date just in case)*

This assignment is worth 25% of your total course mark.

## Assignment Setup and Submission

This assignment must be submitted electronically. To obtain a copy of the template source files, you must *fork* the gitlab project [`comp4300-assignment2`](https://gitlab.cecs.anu.edu.au/comp4300/2024/comp4300-assignment2) into your own gitlab area, and *push* the final version of your files into your fork of the `comp4300-assignment2` gitlab project before the deadline.

The files that will be marked are a report `ps-ass2Rep.pdf` and the C/CUDA files `parAdvect.c`, `parAdvect.cu`.  The report must be **at most**  2,000 words long. 
A good report will present information in an easy to digest format - for example figures and plots are often much easier to interpret than raw tables of data. You should avoid putting snippets of code in the report and instead keep the discussion at a high level, referring the reader to the appropriate locations in your code where appropriate. **The report has a large relative weight in the final mark, and should be well-written. Submissions without report will get a very low mark.**

Please see [here](https://comp.anu.edu.au/courses/comp4300/assignments_workflow/) for additional workflow notes. Most importantly, use the login node only for debugging purposes on a moderate number of OpenMP threads (e.g., 1-8). The actual performance analysis should be done on the compute nodes using the job scripts provided in this repo. 

## Learning Objectives

* Implement a shared memory parallel algorithm using the OpenMP and CUDA programming models.
* Gain an understanding of some of the performance and programmability implications of SMP and GPU systems.

## Setup

Your assignment project directory will contain a `ps-ass2Rep.pdf`, initially empty, which you must overwrite with your own report. It also contains two sub-directories, `openmp` and `cuda`. The former contains a test program `testAdvect.c`, a file `serAdvect.c` containing serial advection functions, some header files, and a template OpenMP advection solver `parAdvect.c`. The test program can be built using the command `make`.

The usage for the test program is:

```bash
OMP_NUM_THREADS=p ./testAdvect [-P P] [-x] M N [r]
```

The test program operates much like that for Assignment 1 except as follows. The `-P` option invokes an optimization where the parallel region is over all timesteps of the simulation, and *P* by *Q* block distribution is used to parallelize the threads, where *p=PQ*. The `-x` option is used to invoke an optional extra optimization.

The directory cuda is similar, except the test program is called `testAdvect.cu`, and the template CUDA parallel solver file is called `parAdvect.cu`. The usage for the test program is:

```bash
./testAdvect [-h] [-s] [-g Gx[,Gy]] [-b Bx[,By]] [-o] [-w w] [-d d] M N [r]
```

with default values of `Gx=Gy=Bx=By=r=1` and `v=w=d=0`. `Gx,Gy` specifies the grid dimensions of the GPU kernels; `Bx,By` specifies the block dimensions.

The option `-h` runs the solver on the host; this may be useful for comparing the 'error' of the GPU runs (and for comparing GPU and CPU speeds). The option `-s` forces a serial implementation (run on a single GPU thread); all other options are ignored. If neither of `-h,-s,-o` are given, `Gx,Gy` thread blocks of size `Bx,By` are used in a 2D GPU parallelization of the solver. If `-o` is specified, an optimized GPU implementation is used, which may use the tuning parameter w as well.

The option `-d` can be used to specify the id of the GPU to be used (`stugpu2` has 4 GPUs, so you can use `d` equal to either `0`, `1`, `2`, or `3`). This may be useful if a particular GPU (e.g. GPU 0) is currently loaded.

You should read the files and familiarize yourself with what they contain.

## Part 1: OpenMP

Experimentation for this section should be done on the Gadi supercomputer. Results should use a batch job run on a single supercomputer node; see the provided file `batchAdv1Node.sh`. Note that for *p ≤ 24*, it forces all threads on one socket (and allocates memory adjacent to the same socket) - this seems to give the best and most consistent results.

Tasks 1. and 2. below are mandatory. Tasks 3. and 4. are optional, and do not contribute to the marks of the assignment. They won't be marked, and as such, feedback won't be provided on these if they are submitted. 
You can use these to test your knowledge in preparation for the final exam.  


1. **Parallelization via 1D Decomposition and Simple Directives** [15/50 marks]

    Parallelize the functions `omp_update_advection_field_1D_decomposition()` and `omp_copy_field_1D_decomposition()` in `parAdvect.c`. Each loop nesting must have its own parallel region, and the parallelization must be over one of the two loop indices (hint: this can be done via simple OMP parallel for directives).

    For `omp_update_advection_field_1D_decomposition()`, there are various ways this can be done: (i) parallelize the outer or inner loop, (ii) interchange the loop order and (iii) schedule the iterations in a block or cyclic fashion. Choose combinations which:
   
      1. maximize performance.
      2. maximize the number of OpenMP parallel region entry/exits (without significantly increasing the number of cache misses during reads/writes).
      3. maximize cache misses involving read operations (without significantly increasing parallel region entry/exits or cache writes).
      4. maximize cache misses involving write operations (without significantly increasing parallel region entry/exits).
  
    This exercise tests your knowledge regarding cache line transfers. It requires that you understand the parallel memory access patterns resulting from each combination, and how these are laid out in the cache lines of each of the cores. 

    The loops in `omp_update_boundary_1D_decomposition()` can potentially be parallelized as well. Experiment with this for the purpose of improving the performance of case (1) above.

    Cut-and-paste the directive / loop-nesting combinations for each case in your report, and discuss why you chose them. Similarly discuss your strategy in parallelizing the boundary update loops, and whether and by how much it improved the performance of case (1). Choosing suitable values for `M`, `N`,  and `r`, record the performance at various `p` and put the results in your report, discussing the differences.

    Now, leave the best performing parallelization combination in your file.

2. **Parallelization via 2D Decomposition and an Extended Parallel Region** [10/50 marks]

    In the function `run_parallel_omp_advection_2D_decomposition()`, write an advection solver such there is only a single parallel region (over all timesteps), and the parallelization is over a `P` by `Q` block distribution. *Hint:* each thread could call `update_advection_field()` and `copy_field()` over its respective sub-array; alternately you could 'inline' these functions and restrict the `i,j` loops to operate on the thread's sub-array.

    For suitable measurements with the best-performing `p` from Q1 and varying `P`, compare performance. Discuss whether there is any gain from the 2D distribution. Comparing this with the 1D vs 2D case for the MPI version of the solver, and explain any differences with the relative advantages. Comparing with your results in Q1 for the `P=p` case, was there any advantage in having a single parallel region?


3. **Optional: Performance Modelling of Shared Memory Programs** [0/50 marks]

    Let $`t_s`$ denote the cost of a parallel region entry/exit, and $`t_{w,R}`$ and $`t_{w,W}`$ denote the cost per (double) word of a cache miss for read and writes, respectively. By counting the expected number of these events in cases 1 to 4 above, and using your measurements from the previous question, derive values for these parameters for the value of `p` in your experiments in Q1 where the performance of case (1) was best.

    Construct a performance model for the best-performing variant (*hint:* it should be similar to the 1D model you used in Assignment 1). You may neglect the update of the boundary in your model. Discuss whether the $`t_s`$ and $`t_w`$ values are comparable with what you obtained for single-node MPI (note that you are *not* being asked to compare your model's predictions with the actual performance in this assignment).


4. **Optional: Additional optimization** [0/50 marks]

    Find and describe some new optimization that could potentially improve the performance of the OpenMP advection solver. Implement and evaluate this on Gadi. Put your code in `run_parallel_omp_advection_with_extra_opts()`, which is activated by the `-x` option.

## Part 2: CUDA

Unless otherwise specified, experimental results for this section should be made on `stugpu2.anu.edu.au`, as described in [Lab 7](https://gitlab.cecs.anu.edu.au/comp4300/2024/comp4300-lab6). *Please note that this is a shared resource and choose your parameters so that the advection time is about 1 second or smaller: this should be plenty long enough to demonstrate performance!* Click [here](https://comp.anu.edu.au/courses/comp4300/assignments_workflow/#access-and-usage-of-stugpu2anueduau-gpu-programming-with-cuda) for `stugpu2.anu.edu.au` access instructions. 

Tasks 5. and 6. below are mandatory. Tasks 7. and 8. are optional (see description for Tasks 3. and 4. above for the implications of this).

5. **Baseline GPU Implementation** [15/50 marks]

   Using the code of `run_serial_advection_device()` and its kernels in `serAdvect.cu` as a starting point, implement a solver whose field update kernels operate on $`Gx \times Gy`$ thread blocks of size $`Bx \times By`$ (without restrictions, except you may assume $`Bx*By \leq`$ the maximum thread block size). You may choose what, if any, parallelization strategy you apply for the boundary updates (justify this in your report).

   In `parAdvect.cu`, you will need to create new kernel functions, and your outer-level solver calling these must be implemented in `run_parallel_cuda_advection_2D_decomposition()`. *Hint:* to help in debugging, replace the kernels from `serAdvect.cu` one by one with your own, and do the simpler parallelizations first.

    Perform experiments to determine the effects of varying `Gx,Gy,Bx,By` (given some reasonably sized problem to solve). Report and discuss the optimal combination, and any results of interest (including a comparison of $`1 \times B`$ vs $`B \times 1`$ blocks).

    Perform suitable experiments to determine the overhead of a kernel invocation, and report your findings. Include also some experiments to determine speedups against the single GPU and host (x86-64) cores (but do not use much smaller parameters, as a single GPU core is very, very, slow...).

6. **Optimized GPU Implementation** [10/50 marks]
    
    In `run_parallel_cuda_advection_optimized()`, create an optimized solver and its associated kernels. It should be (in principle) significantly faster than the previous version. You might have ideas of what optimizations you might consider from Assignment 1; if not, read the paper [<span style="color:blue">Optimized Three-Dimensional Stencil Computation on Fermi and Kepler GPUs</span>](Opt3Dstencils.pdf) by Vizitiu et al. Please note that the architecture used in this paper is different from the GPUs you will use on stugpu2 and Gadi. In your report, describe your optimization(s) and their rationale. Perform suitable experiments which should demonstrate the efficacy of your approach, and discuss them in your report.

7. **Optional: Comparison of the Programming Models** [0/50 marks]
   
    Based on your experiences in this course, comment on the programmability (relative ease of designing, implementing and debugging programs) in the three paradigms you have used so far: OpenMP (shared memory), CUDA (streaming device programming) and MPI (distributed memory). Compare this with the speedups obtained.

8. **Optional: GPU Performance Comparison** [0/50 marks]

    Port your codes to the Gadi GPUs, and perform some experiments to compare your results with those on stugpu2 (*warning:* the batch queues for the GPU nodes are known for their long wait times!). 
    Any code changes should *not* go in your submitted version of `parAdvect.cu`; instead, describe these with words.

## Requirements

Your code must be written using C/OpenMP or Cuda, as appropriate. Your code will be tested against the standard `serAdvect` and `testAdvect` files and must compile and run correctly with them. Solutions which are highly incompatible may be rejected entirely. The exact given field update calculation, as specified in `update_advection_field()` in `serAdvect.c`, must be used in all cases - others will be rejected. You code should be written with good programming style. It should be well commented, so as to enable the reader (this includes the tutor/marker!) to understand how it works.

In your report, include:

* A disclaimer with your name and student number stating what (if any) contributions from others (not including course staff) are in your submission. Note that significant contributions may require a revision of your final mark, as this is intended to be an individual assignment.
* Your answers to all the questions given above.
* Details of any notable deficiencies, bugs, or issues with your work.
* Any feedback on what you found helpful in doing this work, or what from the course could be improved in helping you to carry out this work.
# comp4300 assignment2

# 程序代做代写 CS编程辅导

# WeChat: cstutorcs

# Email: tutorcs@163.com

# CS Tutor

# Code Help

# Programming Help

# Computer Science Tutor

# QQ: 749389476
