/*****************************************************************//**
 *      @file  emat_viewer.h
 *      @brief Provide mat view window
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

#ifndef EMAT_VIEWER_H_
#define EMAT_VIEWER_H_

#include <unordered_map>
#include "emat_core.hpp"
#include "emat_omp.hpp"
#include <string.h>
#include <set>
#include <mutex>
#include <unordered_map>
#include "emat_omp.hpp"

using namespace std;

#define arr_len(arr)	(sizeof(arr) / sizeof(arr[0]))	

namespace emat {
	/*
	text can be rendered on window
	*/
	class s_viewer_text {
	public:
		int font_face = (int)FONT_HERSHEY_PLAIN;
		float font_scale = 1.f;
		int font_thickness = 2;
		Scalar font_color = Scalar::all(255);
		string text;
		Point2i loc;
		Point2f font_offset = Point2f(0.f, 1.f);
		Point2f win_offset = Point2f(0.f, 0.f);

	};

	/*
	provide a watch window for visualizing mat
	*/
	class viewer {
	private:
		class s_cache_display
		{
		private:
			const int m_mul_txts_pad = 4;
			unique_ptr<vector<string>[]> raw_vals_txts;
			/**
			Convert value of a point in Mat to string
			@param src [in] array with Mat type
			@param loc_x [in] location x
			@param loc_y [in] localtion y
			@param res [out] strings
			@return
			**/
			inline void raw_val_to_txt(const int& offset, vector<string>*& res) {
				res = &raw_vals_txts[offset];
				if (res->size() == 0) {
#define format_mat(t) { auto c = m_raw.channels(); auto p = (t*)m_raw.data + offset * c; for(int i = 0; i < c; ++i) {res->emplace_back(to_string(p[i]));}}
					if (m_raw.type() == CV_8UC1 || m_raw.type() == CV_8UC2 || m_raw.type() == CV_8UC3 || m_raw.type() == CV_8UC4) format_mat(u8)
					else if (m_raw.type() == CV_8SC1 || m_raw.type() == CV_8SC2 || m_raw.type() == CV_8SC3 || m_raw.type() == CV_8SC4) format_mat(i8)
					else if (m_raw.type() == CV_16UC1 || m_raw.type() == CV_16UC2 || m_raw.type() == CV_16UC3 || m_raw.type() == CV_16UC4) format_mat(u16)
					else if (m_raw.type() == CV_16SC1 || m_raw.type() == CV_16SC2 || m_raw.type() == CV_16SC3 || m_raw.type() == CV_16SC4) format_mat(i16)
					else if (m_raw.type() == CV_32SC1 || m_raw.type() == CV_32SC2 || m_raw.type() == CV_32SC3 || m_raw.type() == CV_32SC4) format_mat(i32)
					else if (m_raw.type() == CV_32FC1 || m_raw.type() == CV_32FC2 || m_raw.type() == CV_32FC3 || m_raw.type() == CV_32FC4) format_mat(float)
					else if (m_raw.type() == CV_64FC1 || m_raw.type() == CV_64FC2 || m_raw.type() == CV_64FC3 || m_raw.type() == CV_64FC4) format_mat(double)
#undef format_mat
				}
			}

			inline void raw_val_to_txt(const int& loc_x, const int& loc_y, vector<string>*& res) {
				raw_val_to_txt(loc_y * m_raw.cols + loc_x, res);
			}

			/**
			Convert value of a point in Mat to string
			@param src [in] array with Mat type
			@param loc [in] location
			@param res [out] strings
			@return
			**/
			inline void raw_val_to_txt(const Point2i& loc, vector<string>*& res) {
				raw_val_to_txt(loc.y * m_raw.cols + loc.x, res);
			}

			/**
			get total size of texts
			**/
			inline void get_txt_size(const string& txt, const int& font_face, const double& font_scale, const int& font_thickness, Size& res) {
				int baseline;
				res = getTextSize(txt, font_face, font_scale, font_thickness, &baseline);
				res.height += 3;

			}

			inline void get_txts_size(const vector<string>& txts, const int& font_face, const double& font_scale, const int& font_thickness, Size& res) {
				int cnt = (int)txts.size();
				if (cnt == 1) {
					get_txt_size(txts[0], font_face, font_scale, font_thickness, res);
				}
				else if(cnt > 1){
					int idx = 0;
					for (int i = 1; i < cnt; ++i) {
						if (txts[i].size() > txts[idx].size()) {
							idx = i;
						}
					}
					get_txt_size(txts[idx], font_face, font_scale, font_thickness, res);
					res.height = res.height * cnt + m_mul_txts_pad * (cnt - 1);
				}
				else {
					res.width = res.height = 0;
				}
			}

			/**
			draw single text on Mat
			**/
			inline void put_txt(const string& txt, const Point& loc, const int& font_face, const double& font_scale, const Scalar& font_color, const int& font_thickness, Mat& res) {
				cv::putText(res, txt, Point(loc.x, loc.y - 2), font_face, font_scale, font_color, font_thickness);
			}

			/**
			draw multipy texts on Mat
			**/
			inline void put_txts(const vector<string>& txts, const Point& loc, const int& font_face, const double& font_scale, const Scalar& font_color, const int& font_thickness, Mat& res) {
				int cnt = (int)txts.size();
				if (cnt == 1) {
					put_txt(txts[0], loc, font_face, font_scale, font_color, font_thickness, res);
				}
				else if (cnt > 1) {
					Size txts_size[4];
					Size txts_max_size;
					int txt_idx = 0;
					for (auto& txt : txts) {
						get_txt_size(txt, font_face, font_scale, font_thickness, txts_size[txt_idx]);
						txts_max_size.width = max(txts_max_size.width, txts_size[txt_idx].width);
						txts_max_size.height += txts_size[txt_idx].height;
						++txt_idx;
					}
					txts_max_size.height += (cnt - 1) * m_mul_txts_pad;
					for (int i = (int)txts.size() - 1, y_offset = 0; i >= 0; --i) {
						put_txt(txts[i], Point(loc.x + (txts_max_size.width - txts_size[i].width) / 2, loc.y + y_offset), font_face, font_scale, font_color, font_thickness, res);
						y_offset -= txts_size[i].height + m_mul_txts_pad;
					}
				}
			}


			/*
			calculate roi
			*/
			inline void cal_roi(const Point2f& center, const float& scale_factor,
				float& roi_x, float& roi_y, float& roi_w, float& roi_h,
				float& roi_w_div_win_w, float& roi_h_div_win_h, float& win_w_div_roi_w, float& win_h_div_roi_h) {
				float org_w = (float)m_org_size.width, org_h = (float)m_org_size.height;
				float win_w = (float)m_win_size.width, win_h = (float)m_win_size.height;
				float win_w_div_org_w = win_w / org_w, win_h_div_org_h = win_h / org_h;

				roi_w = (float)(scale_factor * org_w);
				roi_h = (float)(scale_factor * org_h);
				if (win_w_div_org_w > win_h_div_org_h) {
					roi_w *= win_w_div_org_w / win_h_div_org_h;
				}
				else {
					roi_h *= win_h_div_org_h / win_w_div_org_w;
				}
				roi_x = (float)(center.x - roi_w / 2);
				roi_y = (float)(center.y - roi_h / 2);

				roi_w_div_win_w = roi_w / win_w;
				roi_h_div_win_h = roi_h / win_h;
				win_w_div_roi_w = win_w / roi_w;
				win_h_div_roi_h = win_h / roi_h;
			}

			/*
			select roi (avoid out of bounds exception)
			*/
			inline void select_roi(const Mat& src, const Rect& roi, Rect& res_roi, Vec4i& res_pad) {
				// res_border   top / bottom / left /right
				res_roi = roi;
				res_pad = Vec4i::all(0);
				//left
				if (res_roi.x < 0) {
					res_roi.width += res_roi.x;
					res_pad[2] = -res_roi.x;
					res_roi.x = 0;
				}
				//top
				if (res_roi.y < 0) {
					res_roi.height += res_roi.y;
					res_pad[0] = -res_roi.x;
					res_roi.y = 0;
				}
				//right
				if (res_roi.x + res_roi.width > src.cols) {
					res_pad[3] = res_roi.x + res_roi.width - src.cols;
					res_roi.width = src.cols - res_roi.x;
				}
				//bottom
				if (res_roi.y + res_roi.height > src.rows) {
					res_pad[1] = res_roi.y + res_roi.height - src.rows;
					res_roi.height = src.rows - res_roi.y;
				}
			}
			Mat m_colored_vis;
		public:
			string m_win_name;
			Mat m_colored;
			Mat m_colored_vis_tiptool;
			vector<s_viewer_text> m_colored_txts;
			Mat m_raw;
			Point2f m_center;
			float m_scale_factor;
			u64 m_idx;
			Size m_win_size;
			Size m_org_size;
			float m_val_font_scale = 0.34f;
			int m_val_font_thickness = 1;
			int m_val_font_face = (int)FONT_HERSHEY_SIMPLEX;
			Scalar m_tiptool_font_color = Scalar::all(255);
			int m_tiptool_font_face = (int)FONT_HERSHEY_SIMPLEX;
			float m_tiptool_font_scale = 0.5f;
			int m_tiptool_font_thickness = 2;
			Scalar m_grid_color = Scalar::all(200);
			int m_grid_thickness = 1;
			Point2f m_tiptool_loc = Point2f(-1.f, -1.f);
			bool m_grid_view_mode = false;
			Size m_val_font_max_size;
			int m_box_margin = 10;
			int m_box_size = 200;
			Scalar m_box_color = Scalar::all(255);
			int m_box_thickness = 2;
			bool m_box_en = false;
			void* m_tag;

			s_cache_display(const string& win_name, const Size& win_size, const Mat& colored, const Mat& raw, const Point2f& center, const float& scale_factor, const  u64& idx, const vector<s_viewer_text>& txts, const Point2f& tiptool_loc, const bool& box_en)
			{
				assert(colored.size() == raw.size());
				m_org_size = colored.size();
				m_win_size = win_size;

				colored.copyTo(m_colored);
				raw.copyTo(m_raw);

				m_win_name = win_name;
				m_idx = idx;
				m_colored_txts = txts;
				m_box_en = box_en;
				raw_vals_txts = unique_ptr<vector<string>[]>(new vector<string>[m_raw.total()]);
				{
					//Calculate the max text size of values in Mat
					vector<string> val_txts;
					Size min_val_font_size, max_val_font_size;
					Mat m_raw_reshape = m_raw.reshape(1, 1);
					double min_val, max_val;
					minMaxLoc(m_raw_reshape, &min_val, &max_val);
					vector<string> min_val_txts, max_val_txts;
					bool is_f = (m_raw.type() == CV_32FC1 || m_raw.type() == CV_32FC2 || m_raw.type() == CV_32FC3 || m_raw.type() == CV_32FC4 ||
						m_raw.type() == CV_64FC1 || m_raw.type() == CV_64FC2 || m_raw.type() == CV_64FC3 || m_raw.type() == CV_64FC4);
					min_val_txts.emplace_back(is_f ? to_string(min_val) : to_string((int)min_val));
					max_val_txts.emplace_back(is_f ? to_string(max_val) : to_string((int)max_val));
					for (int i = 1, len = m_raw.channels(); i < len; ++i)
					{
						min_val_txts.emplace_back(min_val_txts.back());
						max_val_txts.emplace_back(max_val_txts.back());
					}
					get_txts_size(min_val_txts, m_val_font_face, m_val_font_scale, m_val_font_thickness, min_val_font_size);
					get_txts_size(max_val_txts, m_val_font_face, m_val_font_scale, m_val_font_thickness, max_val_font_size);
					m_val_font_max_size.width = max(min_val_font_size.width, max_val_font_size.width) + 4;
					m_val_font_max_size.height = max(min_val_font_size.height, max_val_font_size.height) + 4;
				}
				m_tiptool_loc = tiptool_loc;
				set_roi(center, scale_factor);
			}

			~s_cache_display() {
			}

			/**
			set color of font
			**/
			inline void set_tiptool_color(const Scalar& font_color) {
				m_tiptool_font_color = font_color;
			}

			/**
			get location of Mat corrospond to the locaition of mouse
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_from_mouse(const Point2f& center, const float& scale_factor, const T_Src& src_x, const T_Src& src_y, T_Res& res_x, T_Res& res_y) {
				float roi_w, roi_h, roi_x, roi_y, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h;
				cal_roi(center, scale_factor, roi_x, roi_y, roi_w, roi_h, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h);
				res_x = (T_Res)(roi_x + src_x * roi_w_div_win_w);
				res_y = (T_Res)(roi_y + src_y * roi_h_div_win_h);
			}

			/**
			get location of Mat corrospond to the locaition of mouse
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_from_mouse(const T_Src& src_x, const T_Src& src_y, T_Res& res_x, T_Res& res_y) {
				loc_from_mouse<T_Src, T_Res>(m_center, m_scale_factor, src_x, src_y, res_x, res_y);
			}

			/**
			get location of Mat corrospond to the locaition of mouse
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_from_mouse(const Point2f& center, const float& scale_factor, const Point_<T_Src>& src, Point_<T_Res>& res) {
				loc_from_mouse<T_Src, T_Res>(center, scale_factor, src.x, src.y, res.x, res.y);
			}

			/**
			get location of Mat corrospond to the locaition of mouse
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_from_mouse(const Point_<T_Src>& src, Point_<T_Res>& res) {
				loc_from_mouse<T_Src, T_Res>(m_center, m_scale_factor, src.x, src.y, res.x, res.y);
			}

			/**
			get location of mouse corrospond to the locaition of Mat
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_to_mouse(const Point2f& center, const float& scale_factor, const T_Src& src_x, const T_Src& src_y, T_Res& res_x, T_Res& res_y) {
				float roi_w, roi_h, roi_x, roi_y, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h;
				cal_roi(center, scale_factor, roi_x, roi_y, roi_w, roi_h, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h);
				res_x = (T_Res)((src_x - roi_x) * win_w_div_roi_w);
				res_y = (T_Res)((src_y - roi_y) * win_h_div_roi_h);
			}

			/**
			get location of mouse corrospond to the locaition of Mat
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_to_mouse(const T_Src& src_x, const T_Src& src_y, T_Res& res_x, T_Res& res_y) {
				loc_to_mouse<T_Src, T_Res>(m_center, m_scale_factor, src_x, src_y, res_x, res_y);
			}

			/**
			get location of mouse corrospond to the locaition of Mat
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_to_mouse(const Point2f& center, const float& scale_factor, const Point_<T_Src>& src, Point_<T_Res>& res) {
				loc_to_mouse<T_Src, T_Res>(center, scale_factor, src.x, src.y, res.x, res.y);
			}


			/**
			get location of mouse corrospond to the locaition of Mat
			**/
			template<typename T_Src, typename T_Res>
			inline void loc_to_mouse(const Point_<T_Src>& src, Point_<T_Res>& res) {
				loc_to_mouse<T_Src, T_Res>(m_center, m_scale_factor, src.x, src.y, res.x, res.y);
			}


			/**
			redraw when center or scale_factor changed
			**/
			inline void set_roi(const Point2f& center, const float& scale_factor) {
				m_center = center;
				m_scale_factor = scale_factor;

				//register float win_w = (float)m_win_size.width, win_h = (float)m_win_size.height;
				register float roi_w, roi_h, roi_x, roi_y, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h;
				cal_roi(m_center, m_scale_factor, roi_x, roi_y, roi_w, roi_h, roi_w_div_win_w, roi_h_div_win_h, win_w_div_roi_w, win_h_div_roi_h);
				const int v_channels = 3;
				assert(m_colored.channels() == v_channels);
				register int w = m_win_size.width, h = m_win_size.height, wo = m_org_size.width, ho = m_org_size.height;
				if (m_colored_vis.size() != m_win_size) {
					m_colored_vis = Mat::zeros(m_win_size, CV_8UC3);
				}
				else {
					m_colored_vis.setTo(0);
				}
				//
				{
					u8* p_colored_vis_head = (u8*)m_colored_vis.data;
					u8* p_colored_head = (u8*)m_colored.data;

					int* p_yos = new int[h], * p_xos = new int[w];
					register int yos_min_threshold = 0 * (wo * v_channels), yos_max_threshold = ho * (wo * v_channels), xos_min_threshold = 0 * v_channels, xos_max_threshold = wo * (v_channels);
					register int x_start = 0, x_end = 0, y_start = 0, y_end = 0;
					for (x_end = 0; x_end < w; ++x_end) {
						p_xos[x_end] = (int)floor(roi_x + roi_w_div_win_w * x_end) * v_channels;
						if (p_xos[x_end] < xos_min_threshold) ++x_start;
						else if (p_xos[x_end] >= xos_max_threshold) break;
					}
					for (y_end = 0; y_end < h; ++y_end) {
						p_yos[y_end] = (int)floor(roi_y + roi_h_div_win_h * y_end) * (wo * v_channels);
						if (p_yos[y_end] < yos_min_threshold) ++y_start;
						else if (p_yos[y_end] >= yos_max_threshold) break;
					}
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
					emat_omp{
						u8 * p_colored_vis = 0, *p_colored = 0;
						for (int y = emat_omp_offset_range(y_start, y_end), y_len = emat_omp_offset_next_range(y_start, y_end), x = 0; y < y_len; ++y) {
							p_colored_vis = p_colored_vis_head + ((y * w + x_start) * v_channels);
							p_colored = p_colored_head + p_yos[y];
							for (x = x_start; x < x_end; ++x, p_colored_vis += v_channels) {
								memcpy(p_colored_vis, p_colored + p_xos[x], v_channels);
							}
						}
					}
					delete[] p_yos;
					delete[] p_xos;
				}
				//
				{
#define x_to_win(x) ((x - roi_x) * win_w_div_roi_w)
#define y_to_win(y) ((y - roi_y) * win_h_div_roi_h)

					int top_to_win = (int)round(y_to_win(0)), bottom_to_win = (int)round(y_to_win(m_org_size.height)),
						left_to_win = (int)round(x_to_win(0)), right_to_win = (int)round(x_to_win(m_org_size.width));
					m_grid_view_mode = m_val_font_max_size.width <= ((float)m_win_size.width / roi_w) && m_val_font_max_size.height <= ((float)m_win_size.height / roi_h);
					if (m_grid_view_mode) 
					{
						int y_start = max(0, (int)(roi_y - 1)), y_end = min(ho, (int)(roi_y + roi_h + 2));
						int x_start = max(0, (int)(roi_x - 1)), x_end = min(wo, (int)(roi_x + roi_w + 2));
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
						for (int i = x_start; i <= x_end; ++i) {
							auto x_pos = (int)round(x_to_win(i));
							line(m_colored_vis, Point(x_pos, top_to_win), Point(x_pos, bottom_to_win), m_grid_color, m_grid_thickness);
						}
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
						for (int i = y_start; i <= y_end; ++i) {
							auto y_pos = (int)round(y_to_win(i));
							line(m_colored_vis, Point(left_to_win, y_pos), Point(right_to_win, y_pos), m_grid_color, m_grid_thickness);
						}
#ifdef _OPENMP
#pragma omp parallel for num_threads(emat_omp_cnt)
#endif
						emat_omp{
							Point2i txt_loc;
							vector<string>* txts;
							Size txts_size;
							u8* bg_color;
							for (int y = emat_omp_offset_range(y_start, y_end), y_len = emat_omp_offset_next_range(y_start, y_end), x = 0; y < y_len; ++y) {
								for (x = x_start; x < x_end; ++x) {
									raw_val_to_txt(x, y, txts);
									bg_color = m_colored.at<Vec3b>(y, x).val;
									get_txts_size(*txts, m_val_font_face, m_val_font_scale, m_val_font_thickness, txts_size);
									txt_loc.x = (int)round(x_to_win(x + 0.5f)) - txts_size.width / 2;
									txt_loc.y = (int)round(y_to_win(y + 0.5f)) + txts_size.height / 2;
									put_txts(*txts, txt_loc, m_val_font_face, m_val_font_scale, Scalar::all((bg_color[0] + bg_color[1] + bg_color[2] > 127 * 3) ? 0 : 255), m_val_font_thickness, m_colored_vis);
								}
							}
						}
					}
					else {
						rectangle(m_colored_vis, Rect(Point2i(left_to_win - 1, top_to_win - 1), Point2i(right_to_win + 1, bottom_to_win + 1)), m_grid_color, m_grid_thickness);
					}
#undef x_to_win
#undef y_to_win
				}
				if (m_colored_txts.size() > 0) {
					Size txt_size;
					for (auto& txt : m_colored_txts) {
						get_txt_size(txt.text, txt.font_face, txt.font_scale, txt.font_thickness, txt_size);
						put_txt(txt.text,
							Point2i(txt.loc.x + (int)round(txt.font_offset.x * txt_size.width + txt.win_offset.x * m_win_size.width),
								txt.loc.y + (int)round(txt.font_offset.y * txt_size.height + txt.win_offset.y * m_win_size.height)),
							txt.font_face, txt.font_scale, txt.font_color, txt.font_thickness, m_colored_vis);
					}
				}
				//draw box
				if (m_box_en) {
					int  box_size_w = 0, box_size_h = 0;
					if (m_org_size.width > m_org_size.height) {
						box_size_w = m_box_size;
						box_size_h = box_size_w * m_org_size.height / m_org_size.width;
					}
					else {
						box_size_h = m_box_size;
						box_size_w = box_size_h * m_org_size.width / m_org_size.height;
					}
					auto box_rect = Rect(m_colored_vis.cols - box_size_w - m_box_margin, m_box_margin, box_size_w, box_size_h);
					if (box_rect.x > 0 && box_rect.y > 0 && box_rect.x + box_rect.width <= m_win_size.width && box_rect.y + box_rect.height <= m_win_size.height) {
						Mat img_box, img_box_roi;
						resize(m_colored, img_box, box_rect.size());
						img_box_roi = Mat::zeros(img_box.size(), img_box.type());
						float img_box_w_scale_factor = (float)img_box.cols / m_colored.cols, img_box_h_scale_factor = (float)img_box.rows / m_colored.rows;
						rectangle(img_box_roi,
							Rect((int)round(roi_x * img_box_w_scale_factor), (int)round(roi_y * img_box_h_scale_factor), (int)ceil(roi_w * img_box_w_scale_factor), (int)ceil(roi_h * img_box_h_scale_factor)),
							m_box_color, -1);
						addWeighted(img_box, 0.5, img_box_roi, 0.5, 0.0, img_box);
						img_box.copyTo(m_colored_vis(box_rect));
						cv::rectangle(m_colored_vis, box_rect, m_box_color, m_box_thickness);
					}
				}
				update_tiptool(m_tiptool_loc, true);
			}

			/**
			reset roi
			**/
			inline void set_box_enable(const bool& enable) {
				m_box_en = enable;
				set_roi(m_center, m_scale_factor);
			}

			/**
			reset roi
			**/
			inline void reset_roi() {
				float org_w = (float)m_org_size.width, org_h = (float)m_org_size.height;
				set_roi(Point2f(org_w / 2, org_h / 2), 1.f);
			}

			/**
			update tiptool
			**/
			inline void update_tiptool(const Point2f& mouse, const bool& force) {
				if (force == false && mouse == m_tiptool_loc)
					return;
				m_tiptool_loc = mouse;
				m_colored_vis.copyTo(m_colored_vis_tiptool);
				if (m_tiptool_loc.x >= 0.f && m_tiptool_loc.y >= 0.f && m_tiptool_loc.x < (float)m_win_size.width && m_tiptool_loc.y < (float)m_win_size.height) {
					Point2f anchor_after;
					loc_from_mouse(mouse, anchor_after);
					if (anchor_after.x >= 0 && anchor_after.y >= 0 && anchor_after.x < m_raw.cols && anchor_after.y < m_raw.rows && m_grid_view_mode == false) {
						vector<string>* txts;
						raw_val_to_txt((int)(anchor_after.x), (int)(anchor_after.y), txts);
						Size txts_size;
						get_txts_size(*txts, m_tiptool_font_face, m_tiptool_font_scale, m_tiptool_font_thickness, txts_size);
						if (txts_size.width > 0 && txts_size.height > 0) {
							auto font_loc = Point(min((int)mouse.x + 2, m_win_size.width - txts_size.width - 2), max((int)mouse.y - 2, txts_size.height + 2));
							Rect rect_roi = Rect(font_loc.x, font_loc.y - txts_size.height, txts_size.width, txts_size.height);
							Vec4i vec_roi_pad;
							select_roi(m_colored_vis_tiptool, rect_roi, rect_roi, vec_roi_pad);
							Mat img_roi = m_colored_vis_tiptool(rect_roi) / 2;
							put_txts(*txts, Point(0 - vec_roi_pad[2], txts_size.height - vec_roi_pad[0]), m_tiptool_font_face, m_tiptool_font_scale, m_tiptool_font_color, m_tiptool_font_thickness, img_roi);
							img_roi.copyTo(m_colored_vis_tiptool(rect_roi));
						}
					}
				}
			}

			/**
			set size of window
			**/
			inline void set_win_size(const Size& win_size) {
				if ((win_size.width > 0 && win_size.height > 0) && (win_size != m_win_size)) {
					m_win_size = win_size;
					set_roi(m_center, m_scale_factor);
				}
			}
		};
		mutex m_lock;
		unordered_map<string, unique_ptr<s_cache_display>> m_cache_display;
		u64 m_idx = 0;
		std::set<string> m_imshow_histroy;
	public:
		~viewer() {
			destroyAllWindows();
			m_cache_display.clear();
		}

		/**
		cache imshow: window will be updated/shown after call "imshow"
		@param win_name [in] name of window.
		@param win_size [in] size of window.
		@param img_colored [in] image to display (CV_8U3C).
		@param img_raw [in] image with related values (Should be same size with img_colored, but can be different types)
		@param texts [in] texts will be rendered on screen.
		@return
		**/
		void imshow_cache(const string& win_name, const Size& win_size, const Mat& img_colored, const Mat& img_raw, const vector<s_viewer_text>& texts)
		{
			lock_guard<mutex> lock_(m_lock);
			assert(img_colored.type() == CV_8UC3);
			bool use_prev_setting = (m_cache_display.find(win_name) != m_cache_display.end()) &&
				(m_cache_display[win_name]->m_org_size == img_colored.size());
			if (use_prev_setting) {
				m_cache_display[win_name]->set_win_size(getWindowImageRect(win_name).size());
			}
			m_cache_display[win_name] = unique_ptr<s_cache_display>(new s_cache_display(
				win_name,
				use_prev_setting ? m_cache_display[win_name]->m_win_size : win_size,
				img_colored,
				img_raw.total() ? img_raw: Mat::zeros(img_colored.size(), CV_8U),
				use_prev_setting ? m_cache_display[win_name]->m_center : Point2f((float)img_colored.cols / 2, (float)img_colored.rows / 2),
				use_prev_setting ? m_cache_display[win_name]->m_scale_factor : 1.f,
				m_idx,
				texts,
				use_prev_setting ? m_cache_display[win_name]->m_tiptool_loc : Point2f(-1.f, -1.f),
				use_prev_setting ? m_cache_display[win_name]->m_box_en : false));
			m_cache_display[win_name]->m_tag = (void*)&m_cache_display;
		}

		void imshow_cache(const string& win_name, const Mat& img_colored, const Mat& img_raw, const vector<s_viewer_text>& texts)
		{
			imshow_cache(win_name, img_colored.size(), img_colored, img_raw, texts);
		}

		void imshow_cache(const string& win_name, const float& scaled, const Mat& img_colored, const Mat& img_raw, const vector<s_viewer_text>& texts)
		{
			imshow_cache(win_name, Size((int)(img_colored.cols * scaled), (int)(img_colored.rows * scaled)), img_colored, img_raw, texts);
		}

		/**
		get visiable wins
		**/
		void visiable_wins(vector<string>& win_names) {
			win_names.clear();
			for (auto& key : m_cache_display) {
				if (getWindowProperty(key.first, WindowPropertyFlags::WND_PROP_VISIBLE) > 0) {
					win_names.emplace_back(key.first);
				}
			}
		}

		/**
		is windown closed
		**/
		bool is_win_closed(const string& win_names) {
			if (find(m_imshow_histroy.begin(), m_imshow_histroy.end(), win_names) == m_imshow_histroy.end() || getWindowProperty(win_names, WindowPropertyFlags::WND_PROP_VISIBLE) > 0) {
				return false;
			}
			return true;
		}

		/**
		imshow all images cached
		@param reopen_win [in] whether reopen window, when a window is closed by user. 
		@return
		**/
		void imshow(bool reopen_win) {
			lock_guard<mutex> lock_(m_lock);
			for (auto& key : m_cache_display) {
				auto item = key.second.get();
				if (item->m_idx == m_idx) {
					if (reopen_win || !is_win_closed(item->m_win_name)) {
						namedWindow(item->m_win_name, WINDOW_NORMAL);
						resizeWindow(item->m_win_name, item->m_win_size.width, item->m_win_size.height);
						cv::imshow(item->m_win_name, item->m_colored_vis_tiptool);
						m_imshow_histroy.emplace(item->m_win_name);
						auto mouse_func = [](int event, int x, int y, int flags, void* param) {
							static bool mouse_down = false;
							static Point2f mouse_down_img_loc, mouse_down_img_center;
							auto item = (s_cache_display*)param;
							bool show_tiptool = false;
							if (event == EVENT_LBUTTONDOWN) {
								mouse_down = true;
								mouse_down_img_center = item->m_center;
								item->loc_from_mouse(Point(x, y), mouse_down_img_loc);
								show_tiptool = true;
							}
							if (event == EVENT_LBUTTONUP) {
								mouse_down = false;
								show_tiptool = true;
							}
							if (event == EVENT_LBUTTONDBLCLK) {
								item->reset_roi();
								show_tiptool = true;
							}
							if (event == EVENT_MOUSEWHEEL && mouse_down == false) {
								int curr_vis_blocks = (int)(item->m_org_size.width * item->m_scale_factor);
								int thresholds[] = { 100, 34, 12, 2 };
								for (auto threshold : thresholds) {
									if (curr_vis_blocks > threshold || threshold == thresholds[arr_len(thresholds) - 1]) {
										auto new_scale_factor = (float)max(threshold, curr_vis_blocks + (getMouseWheelDelta(flags) > 0 ? -threshold : threshold)) / item->m_org_size.width;
										Point mouse(x, y);
										Point2f anchor_before, anchor_after;
										item->loc_from_mouse(mouse, anchor_before);
										item->loc_from_mouse(item->m_center, new_scale_factor, mouse, anchor_after);
										item->set_roi(item->m_center - (anchor_after - anchor_before), new_scale_factor);
										show_tiptool = true;
										break;
									}
								}
							}
							if (event == EVENT_MOUSEMOVE && mouse_down == true) {
								Point2f mouse((float)x, (float)y), mouse_move_img_loc;
								item->loc_from_mouse(mouse_down_img_center, item->m_scale_factor, mouse, mouse_move_img_loc);
								item->set_roi(mouse_down_img_center - (mouse_move_img_loc - mouse_down_img_loc), item->m_scale_factor);
								show_tiptool = true;
							}
							if (event == EVENT_MOUSEMOVE) {
								show_tiptool = true;
							}
							if (event == EVENT_RBUTTONDOWN) {
								item->set_box_enable(!item->m_box_en);
							}
							if (show_tiptool) {
								item->set_win_size(getWindowImageRect(item->m_win_name).size());
								for (auto& it : *((unordered_map<string, unique_ptr<s_cache_display>>*)item->m_tag)) {
									auto& item_iter = it.second;
									if (getWindowProperty(item_iter->m_win_name, WindowPropertyFlags::WND_PROP_VISIBLE) > 0) {
										Point2f new_tiptool_loc = (/*item_iter->m_grid_view_mode == false && */item_iter->m_win_name == item->m_win_name) ? Point2f((float)x, (float)y) : Point2f(-1.f, -1.f);
										item_iter->update_tiptool(new_tiptool_loc, false);
										cv::imshow(item_iter->m_win_name, item_iter->m_colored_vis_tiptool);
									}
								}
							}
						};
						setMouseCallback(item->m_win_name, mouse_func, (void*)item);
					}
				}
				else {
					if (reopen_win || !is_win_closed(item->m_win_name)) {
						m_imshow_histroy.erase(key.first);
					}
					destroyWindow(key.first);
					m_cache_display.erase(key.first);
				}
			}
			++m_idx;
		}

		/**
		destroy all winodws
		**/
		void destroy_all() {
			lock_guard<mutex> lock_(m_lock);
			destroyAllWindows();
			m_cache_display.clear();
			m_imshow_histroy.clear();
		}

		/**
		destroy specific window
		**/
		void destroy(const string& win_name) {
			lock_guard<mutex> lock_(m_lock);
			destroyWindow(win_name);
			m_imshow_histroy.erase(win_name);
			m_cache_display.erase(win_name);
		}
	};
}

#endif