/*****************************************************************//**
 *      @file  emat_init.h
 *      @brief Provide mat initilization functions related to emat
 *
 *  Detail Decsription starts here
 *
 *   @internal
 *     Project
 *     Created  1/7/2019
 *    Revision  1/2/2020
 *     Company
 *   Copyright
 *
 * *******************************************************************/

#ifndef EMAT_INIT_H_
#define EMAT_INIT_H_

#include "emat_core.hpp"
#include "emat_omp.hpp"

namespace emat {
	/**
	create mat with initial values
	**/
	template<typename T_res>
	inline void create(const Size& size, const T_res default_value, Mat& dst) {
		auto new_cv_type = sys_to_cv_type<T_res>();
		if (dst.type() != new_cv_type || dst.rows != size.height || dst.cols != size.width) {
			dst = Mat(size, new_cv_type, Scalar(default_value));
		}
		else {
			dst.setTo(default_value);
		}
	}

	/**
	create mat with initial values
	**/
	template<typename T_res>
	inline Mat create(const Size& size, const T_res default_value) {
		Mat dst;
		create<T_res>(size, default_value, dst);
		return dst;
	}

	/**
	create mat
	**/
	template<typename T_res>
	inline void create(const Size& size, Mat& dst) {
		auto new_cv_type = sys_to_cv_type<T_res>();
		if (dst.type() != new_cv_type || dst.rows != size.height || dst.cols != size.width) {
			dst = Mat(size, new_cv_type);
		}
	}

	/**
	create mat
	**/
	template<typename T_res>
	inline Mat create(const Size& size) {
		Mat dst;
		create<T_res>(size, dst);
		return dst;
	}

	/**
	generate meshgrid
	**/
	template<typename T_Src>
	inline void meshgrid(const Range& xs, const Range& ys, Mat& X, Mat& Y) {
		register i32 x_end = xs.end, x_start = xs.start,
			y_end = ys.end, y_start = ys.start,
			y_len = y_end - y_start,
			x_len = x_end - x_start;
		create<T_Src>(Size(x_len, y_len), X);
		create<T_Src>(Size(x_len, y_len), Y);

#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
		emat_omp{
			register auto y_offset = emat_omp_offset(y_len), y_offset_next = emat_omp_offset_next(y_len);
			register auto p_x = (T_Src*)X.data + y_offset * x_len;
			register auto p_y = (T_Src*)Y.data + y_offset * x_len;
			register i32 y_start_omp = y_start + y_offset, y_end_omp = y_start + y_offset_next;
			for (i32 y = y_start_omp, x = 0; y < y_end_omp; ++y) {
				for (x = x_start; x < x_end; ++x) {
					*p_x++ = (T_Src)x;
					*p_y++ = (T_Src)y;
				}
			}
		}
	}

	/**
	generate range mat
	**/
	template<typename T_res>
	inline void range(const T_res start, const T_res step, const Size& size, Mat& res)
	{
		create<T_res>(size, res);
		register auto res_len = (u32)res.total();
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
		emat_omp{
			register auto res_offset = emat_omp_offset(res_len), res_offset_next = emat_omp_offset_next(res_len);
			register auto p_res = (T_res*)res.data + res_offset;
			register auto p_res_end = (T_res*)res.data + res_offset_next;
			register auto v_start = (T_res)(start + step * res_offset);
			while (p_res < p_res_end) {
				*(p_res++) = v_start;
				v_start += step;
			}
		}
	}

	/**
	generate range mat
	**/
	template<typename T_res>
	inline Mat range(const T_res start, const T_res step, const Size& size)
	{
		Mat res;
		range<T_res>(start, step, size, res);
		return res;
	}

	/**
	generate range mat and select roi 
	**/
	template<typename T_res>
	inline void range(const T_res start, const T_res step, const Size& size, const Rect& ROI, Mat& res)
	{
		create<T_res>(Size(ROI.width, ROI.height), res);
		register u32 y_start = ROI.y;
		register T_res x_start_mul_step = ROI.x * step,
			x_end_mul_step = (ROI.x + ROI.width) * step,
			step_mul_matwidth = size.width * step;
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
		emat_omp{
			register auto y_offset = emat_omp_offset(ROI.height), y_offset_next = emat_omp_offset_next(ROI.height);
			register T_res y = 0, x = 0;
			register T_res y_start_omp = (T_res)(y_start + y_offset) * step_mul_matwidth + start,
				y_end_omp = (T_res)(y_start + y_offset_next) * step_mul_matwidth + start,
				x_end_omp;
			register auto p_res = (T_res*)res.data + y_offset * ROI.width;
			for (y = y_start_omp; y < y_end_omp; y += step_mul_matwidth) {
				for (x = y + x_start_mul_step, x_end_omp = y + x_end_mul_step; x < x_end_omp; ++x) {
					*(p_res++) = x;
				}
			}
		}
	}

	/**
	generate range mat and select roi
	**/
	template<typename T_res>
	inline Mat range(const T_res start, const T_res step, const Size& size, const Rect& ROI)
	{
		Mat res;
		range<T_res>(start, step, size, ROI, res);
		return res;
	}
}
#endif