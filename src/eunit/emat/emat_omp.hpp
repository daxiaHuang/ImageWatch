/*****************************************************************//**
 *      @file  emat_omp.h
 *      @brief Easy for openmp
 *
 *  Detail Decsription starts here
 *  [Example]:
 *  Mat mt;
 *  create<T_res>(size(src1_mat), mt);
 *  register auto len = (u32)mt.total();
 *  #pragma omp parallel for num_threads(emat_omp_cnt)
 *  emat_omp{
 *  	register auto res_offset = emat_omp_offset(len), res_offset_next = emat_omp_offset_next(len);
 *  	register auto p_src1 = (T_Src1*)src1_mat.data + res_offset;
 *  	register auto p_src2 = (T_Src2*)src2_mat.data + res_offset;
 *  	register auto p_res = (T_res*)mt.data + res_offset;
 *  	register auto p_res_end = (T_res*)mt.data + res_offset_next;
 *  	while (p_res < p_res_end) {
 *  		(*p_res++) = (T_res)((*p_src1++) + (*p_src2++));
 *  	}
 *  }

 *  [Example]:
 *  Mat mt;
 *  create<T_res>(size(src1_mat), mt);
 *  #pragma omp parallel for num_threads(emat_omp_cnt)
 *  emat_omp{
 *  	register auto p_src1 = emat_omp_offset_mat(T_Src1, src1_mat);
 *  	register auto p_src2 = emat_omp_offset_mat(T_Src2, src2_mat);
 *  	register auto p_res = emat_omp_offset_mat(T_res, mt);
 *  	register auto p_res_end = emat_omp_offset_next_mat(T_res, mt);
 *  	while (p_res < p_res_end) {
 *  		(*p_res++) = (T_res)((*p_src1++) + (*p_src2++));
 *  	}
 *  }
 *   @internal
 *     Project
 *     Created  1/7/2019
 *    Revision  1/2/2020
 *     Company
 *   Copyright
 *
 * *******************************************************************/

#ifndef EMAT_OMP_H_
#define EMAT_OMP_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#define emat_omp_cnt 4
#define emat_omp_idx emat_omp_idx
#define emat_omp for(i32 emat_omp_idx = 0; emat_omp_idx < emat_omp_cnt; ++emat_omp_idx)
#define emat_omp_step(len) ((emat_omp_cnt > 1) ? (u32)ceil((float)(len) / emat_omp_cnt) : len)
#define emat_omp_offset(len) (u32)(emat_omp_idx * emat_omp_step(len))
#define emat_omp_offset_next(len) std::min((u32)((emat_omp_idx + 1) * emat_omp_step(len)), (u32)len)
#define emat_omp_offset_range(start, end) ((start) + emat_omp_offset(((end) - (start))))
#define emat_omp_offset_next_range(start, end) ((start) + emat_omp_offset_next(((end) - (start))))
#define emat_omp_offset_mat(T, M) ((T*)(M).data + emat_omp_offset((u32)(M).total()))
#define emat_omp_offset_next_mat(T, M) ((T*)(M).data + emat_omp_offset_next((u32)(M).total()))


#endif