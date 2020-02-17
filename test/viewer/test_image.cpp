#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "../../src/eunit/emat/emat_visual.hpp"
#include "../../src/eunit/emat/emat_viewer.hpp"

using namespace std;
using namespace cv;
using namespace emat;

int main(int argc, const char** argv)
{
	string win_name_1 = "Demo 1";
	string win_name_2 = "Demo 2";
	emat::viewer viewer;
	s_viewer_text viewer_text_1, viewer_text_2;
	viewer_text_1.text = win_name_1;
	viewer_text_2.text = win_name_2;
	Mat img = emat::range<i32>(0, 1, Size(160, 120));					//Generate a 
	while (!viewer.is_win_closed(win_name_1) && !viewer.is_win_closed(win_name_2)) {
		//Update Window 1
		viewer.imshow_cache(
			win_name_1,													//Window Name
			Size(320, 480),												//Window Size
			vis_gconcat({ vis_gray(img), vis_colormap_jet(img) }, 1),	//combine two images
			vis_gconcat({ img, img }, 1),								//combine two images
			{ viewer_text_1 });											//Texts(Subtitles)
		//Update Window 2						
		viewer.imshow_cache(
			win_name_2,													//Window Name
			Size(320, 240),												//Window Size
			vis_colormap_jet(img),										//Displayed Image
			img,														//Mat with related values
			{ viewer_text_1 });											//Texts(Subtitles)
		viewer.imshow(false);											//Display all images
		waitKey(50);	
	}
}