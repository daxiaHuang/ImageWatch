/*****************************************************************//**
 *      @file  emat_visual.h
 *      @brief Provide mat visualization functions related to emat
 *
 *  Detail Decsription starts here
 *  Example:

 *   @internal
 *     Project
 *     Created  1/7/2019
 *    Revision  2/9/2020
 *     Company
 *   Copyright
 *
 * *******************************************************************/
#ifndef EMAT_VISUAL_H_
#define EMAT_VISUAL_H_

#include <opencv2/opencv.hpp>
#include "../../eunit/emat/emat_core.hpp"
#include "../../eunit/emat/emat_init.hpp"
#include "../../eunit/emat/emat_omp.hpp"
#include <unordered_map>
#include <mutex>

using namespace emat;
using namespace std;
using namespace cv;

namespace emat {
	/**
	apply grid concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline void vis_gconcat(const vector<Mat>& imgs, const uint32_t& cols, Mat& res) {
		if (imgs.size() == 0) return;
#if  emat_security_protection_en
		for (int i = 0, len = (int)imgs.size() - 1; i < len; ++i) {
			assert(imgs[i].type() == imgs[i + 1].type());
		}
#endif
		Size size_mat;
		for (auto& img : imgs) {
			size_mat.width = max(size_mat.width, img.cols);
			size_mat.height = max(size_mat.height, img.rows);
		}
		res = Mat::zeros(Size(size_mat.width * min(cols, (uint32_t)imgs.size()), size_mat.height * (int)ceil((float)imgs.size() / cols)), imgs[0].type());
		for (int i = 0, len = (int)imgs.size(); i < len; ++i) {
			auto fxy = min((double)size_mat.width / (double)imgs[i].cols, (double)size_mat.height / (double)imgs[i].rows);
			Mat img_resized;
			if (fxy == 1) {
				img_resized = imgs[i];
			}
			else {
				resize(imgs[i], img_resized, Size(), fxy, fxy, INTER_NEAREST);
			}
			//rectangle(img_resized, Rect(0, 0, img_resized.cols, img_resized.rows), Scalar::all(255), 1);
			Rect roi = Rect((i % cols) * size_mat.width + (size_mat.width - img_resized.cols) / 2, (int)(i / cols) * size_mat.height + (size_mat.height - img_resized.rows) / 2, img_resized.cols, img_resized.rows);
			img_resized.copyTo(res(roi));
		}
	}

	/**
	apply grid concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline Mat vis_gconcat(const vector<Mat>& imgs, const uint32_t& cols) {
		Mat res;
		vis_gconcat(imgs, cols, res);
		return res;
	}

	/**
	apply vertical concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline void vis_vconcat(const vector<Mat>& imgs, Mat& res) {
		vis_gconcat(imgs, 1, res);
	}

	/**
	apply vertical concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline Mat vis_vconcat(const vector<Mat>& imgs) {
		return vis_gconcat(imgs, 1);
	}

	/**
	apply horizontal concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline void vis_hconcat(const vector<Mat>& imgs, Mat& res) {
		vis_gconcat(imgs, (uint32_t)imgs.size(), res);
	}

	/**
	apply grid concat
	@param imgs [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline Mat vis_hconcat(const vector<Mat>& imgs) {
		return vis_gconcat(imgs, (uint32_t)imgs.size());
	}

	/**
	apply COLORMAP_JET
	@param img [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline void vis_colormap_jet(const Mat& img, Mat& res)
	{
		assert(img.channels() == 1);
		if (img.channels() == 1) {
			if (img.type() != CV_8U) {
				normalize(img, res, 0, 255, NORM_MINMAX, CV_8U);
			}
			else {
				res = img;
			}
			applyColorMap(res, res, COLORMAP_JET);
		}
		else {
			res = img.clone();
		}
	}

	/**
	apply COLORMAP_JET
	@param img [in] array with Mat type
	@param res [out] variable with Mat type
	@return
	**/
	inline Mat vis_colormap_jet(const Mat& img)
	{
		Mat res;
		vis_colormap_jet(img, res);
		return res;
	}

	/**
	apply "U8"
	**/
	inline void vis_gray(const Mat& img, Mat& res)
	{
		assert((img.channels() == 1));
		if (img.type() != CV_8U) {
			normalize(img, res, 0, 255, NORM_MINMAX, CV_8U);
		}
		else {
			res = img;
		}
		cvtColor(res, res, COLOR_BGR2RGB);
	}

	/**
	apply "U8"
	**/
	inline Mat vis_gray(const Mat& img)
	{
		Mat res;
		vis_gray(img, res);
		return res;
	}

	/**
	apply scale
	**/
	inline void vis_scale(const Mat& img, const float& scale_factor, Mat& res) {
		resize(img, res, Size(), scale_factor, scale_factor, INTER_NEAREST);
	}

	/**
	apply scale
	**/
	inline Mat vis_scale(const Mat& img, const float& scale_factor) {
		Mat res;
		vis_scale(img, scale_factor, res);
		return res;
	}

	/**
	Apply color map
	@param img_u8 [in] array with Mat type
	@param colormap [in] color map
	@param res [out] variable with Mat type
	@return
	**/
	inline Mat vis_apply_color_map(const Mat& img_u8, const int& colormap) {
		Mat res;
		applyColorMap(img_u8, res, colormap);
		return res;
	}

	/**
	Convert Mat with uint16 type into Mat with gray type
	@param gray_u16 [in] array with Mat type
	@param balance [in] [-1024, 1024] -> [darkest, brightest]
	@param res [out] variable with Mat type
	@return
	**/
	template<typename T_Src>
	inline void vis_gray_u16(const Mat& gray_u16, const int16_t& balance, Mat& res)
	{
#if emat_security_protection_en
		type_check<T_Src>(gray_u16, true);
#endif
		res = Mat(gray_u16.size(), CV_8UC3);
		auto src = (T_Src*)gray_u16.data;
		u8* dst = res.data;
		auto src_len = (int32_t)gray_u16.total();

		i16 v_balance = balance - 640;
		register u16 v_mean = (u16)cv::mean(gray_u16).val[0];
		v_mean = ((i16)v_mean <= v_balance) ? 1 : (i16)v_mean - v_balance;
		register u16 v_mean_mul_2 = v_mean * 2;
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
		for (i32 i = 0; i < src_len; ++i) {
			memset(dst + i * 3, (src[i] < v_mean_mul_2) ? (128 * src[i]) / v_mean : 255, 3);
		}
	}

	template<typename T_Src>
	inline Mat vis_gray_u16(const Mat& gray_u16, const int16_t& balance) {
		Mat res;
		vis_gray_u16<T_Src>(gray_u16, balance, res);
		return res;
	}

	template<typename T_Src>
	inline void vis_gray_u16(const Mat& gray_u16, Mat& res) {
		vis_gray_u16<u16>(gray_u16, 400, res);
	}

	template<typename T_Src>
	inline Mat vis_gray_u16(const Mat& gray_u16) {
		return vis_gray_u16<T_Src>(gray_u16, 400);
	}

	/**
	Convert Mat with uint16 type into Mat with rgb type
	@param dist_u16 [in] array with Mat type
	@param pseudo_min [in] minimum range of source point
	@param pseudo_max [in] maximum range of source point
	@param res [out] variable with Mat type
	@return
	**/
	template<typename T_Src>
	inline void vis_dist_u16(const Mat& dist_u16, const uint16_t& pseudo_min, const uint16_t& pseudo_max, Mat& res)
	{
#if emat_security_protection_en
		type_check<T_Src>(dist_u16, true);
#endif

		static Mat color_map_u8_sl = vis_apply_color_map(range<u8>(255, -1, Size(16, 16)), COLORMAP_JET);
		static Mat color_map_u8_ls = vis_apply_color_map(range<u8>(0, 1, Size(16, 16)), COLORMAP_JET);

		// Search By Table
		res = Mat(dist_u16.size(), CV_8UC3);
		register auto src = (T_Src*)dist_u16.data;
		register uint8_t* dst = res.data;
		register i32 src_len = (i32)dist_u16.total();
		register u8* p_color_map = pseudo_max >= pseudo_min ? color_map_u8_sl.data : color_map_u8_ls.data;
		register u16 v_max = max(pseudo_max, pseudo_min), v_min = min(pseudo_max, pseudo_min), v_diff = v_max - v_min;
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
		for (int i = 0; i < src_len; ++i) {
			register auto v_src = src[i];
			if (v_min < v_src && v_src < v_max) {
				memcpy(dst + i * 3, p_color_map + (int)(255 * (v_src - v_min) / v_diff) * 3, 3);
			}
			else {
				memset(dst + i * 3, 0, 3);
			}
		}
	}

	template<typename T_Src>
	inline Mat vis_dist_u16(const Mat& dist_u16, const uint16_t& pseudo_min, const uint16_t& pseudo_max) {
		Mat res;
		vis_dist_u16<T_Src>(dist_u16, pseudo_min, pseudo_max, res);
		return res;
	}

	template<typename T_Src>
	inline void vis_dist_u16(const Mat& dist_u16, Mat& res)
	{
		vis_dist_u16<T_Src>(dist_u16, 0, 4000, res);
	}

	template<typename T_Src>
	inline Mat vis_dist_u16(const Mat& dist_u16) {
		return vis_dist_u16<T_Src>(dist_u16, 0, 4000);
	}
}

#endif