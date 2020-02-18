#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "../../src/eunit/emat/emat_viewer.hpp"

using namespace std;
using namespace cv;
using namespace emat;

int main(int argc, const char** argv)
{
	string win_name = "Demo";
	emat::viewer viewer;
	cv::VideoCapture capture(0);
	s_viewer_text viewer_text;
	viewer_text.text = win_name;
	Mat frame;
	while (!viewer.is_win_closed(win_name)) {
		//Get Captured Frame
		capture >> frame;
		//Update Window
		viewer.img_show_cache(win_name, frame, frame, { viewer_text });
		viewer.imgs_show(false);
		waitKey(10);
	}
}