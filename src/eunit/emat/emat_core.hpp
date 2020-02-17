/*****************************************************************//**
 *      @file  emat_core.h
 *      @brief Provide basic functions related to emat
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

#ifndef EMAT_CORE_H_
#define EMAT_CORE_H_

#include <opencv2/opencv.hpp>
#include <typeinfo>

using namespace cv;

#define emat_security_protection_en	true	//Whether check the validation of mat-type inputs

namespace emat {
	/***************** data structure *****************/
	typedef char				i8;
	typedef short				i16;
	typedef int					i32;
	typedef long long			i64;
	typedef unsigned char		u8;
	typedef unsigned short		u16;
	typedef unsigned int		u32;
	typedef unsigned long long	u64;
	/**/

	/**
	check whether two types are same
	**/
	template<typename T1, typename T2>
	struct is_same_type {
		operator bool() {
			return false;
		}
	};

	template<typename T1>
	struct is_same_type<T1, T1> {
		operator bool() {
			return true;
		}
	};

	/**
	return corresponding cv type
	**/
	template<typename T_Src>
	inline int sys_to_cv_type() {
		if (is_same_type<T_Src, u8>()) return CV_8U;
		else if (is_same_type<T_Src, u16>()) return CV_16U;
		else if (is_same_type<T_Src, i8>()) return CV_8S;
		else if (is_same_type<T_Src, i16>()) return CV_16S;
		else if (is_same_type<T_Src, i32>()) return CV_32S;
		else if (is_same_type<T_Src, float>()) return CV_32F;
		else if (is_same_type<T_Src, double>()) return CV_64F;

		else if (is_same_type<T_Src, Point2i>()) return CV_32SC2;
		else if (is_same_type<T_Src, Point2f>()) return CV_32FC2;
		else if (is_same_type<T_Src, Point2d>()) return CV_64FC2;
		else if (is_same_type<T_Src, Point3i>()) return CV_32SC3;
		else if (is_same_type<T_Src, Point3f>()) return CV_32FC3;
		else if (is_same_type<T_Src, Point3d>()) return CV_64FC3;

		else if (is_same_type<T_Src, Vec<u8, 2>>()) return CV_8UC2;
		else if (is_same_type<T_Src, Vec<u16, 2>>()) return CV_16UC2;
		else if (is_same_type<T_Src, Vec<i8, 2>>()) return CV_8SC2;
		else if (is_same_type<T_Src, Vec<i16, 2>>()) return CV_16SC2;
		else if (is_same_type<T_Src, Vec<i32, 2>>()) return CV_32SC2;
		else if (is_same_type<T_Src, Vec<float, 2>>()) return CV_32FC2;
		else if (is_same_type<T_Src, Vec<double, 2>>()) return CV_64FC2;

		else if (is_same_type<T_Src, Vec<u8, 3>>()) return CV_8UC3;
		else if (is_same_type<T_Src, Vec<u16, 3>>()) return CV_16UC3;
		else if (is_same_type<T_Src, Vec<i8, 3>>()) return CV_8SC3;
		else if (is_same_type<T_Src, Vec<i16, 3>>()) return CV_16SC3;
		else if (is_same_type<T_Src, Vec<i32, 3>>()) return CV_32SC3;
		else if (is_same_type<T_Src, Vec<float, 3>>()) return CV_32FC3;
		else if (is_same_type<T_Src, Vec<double, 3>>()) return CV_64FC3;

		else if (is_same_type<T_Src, Vec<u8, 4>>()) return CV_8UC4;
		else if (is_same_type<T_Src, Vec<u16, 4>>()) return CV_16UC4;
		else if (is_same_type<T_Src, Vec<i8, 4>>()) return CV_8SC4;
		else if (is_same_type<T_Src, Vec<i16, 4>>()) return CV_16SC4;
		else if (is_same_type<T_Src, Vec<i32, 4>>()) return CV_32SC4;
		else if (is_same_type<T_Src, Vec<float, 4>>()) return CV_32FC4;
		else if (is_same_type<T_Src, Vec<double, 4>>()) return CV_64FC4;
		return -1;
	}

	/**
	check cv type
	**/
	template<typename T_res>
	inline void type_check(const Mat& dst, const bool matched) {
		assert(dst.isContinuous() && ((dst.type() == sys_to_cv_type<T_res>()) == matched) && "CV type check failed.");
	}

	/**
	get length of mat
	**/
	inline u32 len(const Mat& src) {
		return (u32)(src.channels() * src.total());
	}

	/**
	get sizeof of mat
	**/
	inline u32 size_of(const int type) {
		if (type == CV_8UC1 || type == CV_8UC2 || type == CV_8UC3 || type == CV_8UC4) return sizeof(u8);
		else if (type == CV_16UC1 || type == CV_16UC2 || type == CV_16UC3 || type == CV_16UC4) return sizeof(u16);
		else if (type == CV_8SC1 || type == CV_8SC2 || type == CV_8SC3 || type == CV_8SC4) return sizeof(i8);
		else if (type == CV_16SC1 || type == CV_16SC2 || type == CV_16SC3 || type == CV_16SC4) return sizeof(i16);
		else if (type == CV_32SC1 || type == CV_32SC2 || type == CV_32SC3 || type == CV_32SC4) return sizeof(i32);
		else if (type == CV_32FC1 || type == CV_32FC2 || type == CV_32FC3 || type == CV_32FC4) return sizeof(float);
		else if (type == CV_64FC1 || type == CV_64FC2 || type == CV_64FC3 || type == CV_64FC4) return sizeof(double);
		else assert("CV Type Error");
		return 0;
	}


	/**
	get sizeof of mat
	**/
	inline u32 size_of(const Mat& src) {
		return size_of(src.type()) * len(src);
	}

	/**
	return data pointer of a mat
	**/
	template<typename T_Src>
	inline T_Src* pointer(const Mat& src_mat) {
#if emat_security_protection_en
		if ((u32)src_mat.total() > 0) {
			type_check<T_Src>(src_mat, true);
		}
#endif
		return (T_Src*)src_mat.data;
	}
}
#endif