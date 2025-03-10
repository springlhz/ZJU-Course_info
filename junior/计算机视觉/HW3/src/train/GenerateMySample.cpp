#include<iostream>
#include<opencv2/opencv.hpp>
#include<string>
#include<opencv2/core/core.hpp>
#include <io.h>
#include<direct.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d/calib3d.hpp"
using namespace std;
using namespace cv;

vector<string> getFiles(string cate_dir)
{
	vector<string> files;//存放文件名
	_finddata_t file;
	intptr_t lf;
	//输入文件夹路径
	if ((lf = _findfirst(cate_dir.c_str(), &file)) == -1) {
		cout << cate_dir << " not found!!!" << endl;
	}
	else {
		while (_findnext(lf, &file) == 0) {
			//输出文件名
			//cout<<file.name<<endl;
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)//去除上一层文件和当前文件夹
				continue;
			files.push_back(file.name);
		}
	}
	_findclose(lf);

	//排序，按从小到大排序
	sort(files.begin(), files.end());
	return files;
}

void DetectEye(Mat src, Point (&eyes)[2]) {
	Mat gray, equalize;
	cvtColor(src, gray, COLOR_RGB2GRAY);
	equalizeHist(gray, equalize); //直方图均衡化，用于提高图像的质量

	CascadeClassifier objDetector("../model/haarcascade_eye.xml");

	vector<Rect> objs;
	objDetector.detectMultiScale(equalize, objs, 1.2, 2, CASCADE_SCALE_IMAGE, Size(30, 30));

	eyes[0] = Point(objs[0].x, objs[0].y);
	eyes[1] = Point(objs[1].x, objs[1].y);
}


Mat alignImages(Mat& src, Mat& dst)
{
	//Mat src_grey, dst_grey;
	//cvtColor(src, src_grey, COLOR_RGB2GRAY);
	//cvtColor(dst, dst_grey, COLOR_RGB2GRAY);
	Point eyes_src[2];
	DetectEye(src, eyes_src);
	Point2f eyesCenter = Point2f( (eyes_src[0].x + eyes_src[1].x)*0.5f, (eyes_src[0].y + eyes_src[1].y)*0.5f);
	double dy = eyes_src[1].y - eyes_src[0].y;
	double dx = eyes_src[1].x - eyes_src[0].x;
	double angle = atan2(dy, dx) * 180.0 / CV_PI; // Convert from radians to degrees.
	Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, 1.0);
	Mat tmp;
	warpAffine(src, tmp, rot_mat, Size(256,256));
	imshow("tmp", tmp);
	DetectEye(tmp, eyes_src);
	eyesCenter = Point2f((eyes_src[0].x + eyes_src[1].x)*0.5f, (eyes_src[0].y + eyes_src[1].y)*0.5f);
	//cout << eyesCenter << endl;
	Point eyes_dst[2];
	DetectEye(dst, eyes_dst);
	Point2f dst_eyesCenter = Point2f((eyes_dst[0].x + eyes_dst[1].x)*0.5f, (eyes_dst[0].y + eyes_dst[1].y)*0.5f);
	//cout << dst_eyesCenter << endl;
	Mat rot;
	Mat t_mat = cv::Mat::zeros(2, 3, CV_32FC1);
	t_mat.at<float>(0, 0) = 1;
	t_mat.at<float>(0, 2) = dst_eyesCenter.x - eyesCenter.x; //水平平移量
	t_mat.at<float>(1, 1) = 1;
	t_mat.at<float>(1, 2) = dst_eyesCenter.y - eyesCenter.y; //竖直平移量
	warpAffine(tmp, rot, t_mat, Size(256, 256));
	return rot;
}
void generatemysample() {
	string dir = "..\\dataset\\me\\";
	vector<string> filenames = getFiles(dir+'*');
	Mat dst = imread("../dataset/coursedata/KA.FE3.47.tiff");
	char count= 0;
	for (vector<string>::iterator it = filenames.begin(); it != filenames.end(); it++)
	{
		Mat src = imread(dir + *it);
		if (src.empty()) {
			throw("Faild open file:" + dir + *it );
			exit(0);
		}
		Mat res, res_gray;
		res = alignImages(src, dst);
		cvtColor(res, res_gray, COLOR_RGB2GRAY);
		imshow("result", res);
		imwrite("../dataset/coursedata/"+to_string(count)+".png", res_gray);
		count++;
		waitKey(10);
	}
	destroyAllWindows();
}