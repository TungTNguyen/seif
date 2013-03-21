#include "../config.h"
#include "seif/world.h"
#include "plot.h"


Plot& Plot::instance() {
	static Plot inst("Simulation", MAP_WIDTH, MAP_HEIGHT);
	return inst;
}

// init opencv plot
Plot::Plot(std::string wndTitle, int width, int height)
	: canvas(height, width, CV_8UC3, cv::Scalar(0.)),
	  wndTitle(wndTitle),
	  poseTruthSub(n.subscribe("ground_truth", 1, Plot::poseTruthCB)),
	  poseOdomSub(n.subscribe("odom", 1, Plot::poseOdoCB)),
	  worldSub(n.subscribe("world", 1, Plot::worldCB))
	  {

	cv::namedWindow(wndTitle);
}

// close opencv plot
Plot::~Plot() {
	cv::destroyWindow(wndTitle);
}

cv::Point* Plot::calcTriangle(nav_msgs::Odometry& pose) {
	KDL::Frame frame;
	tf::poseMsgToKDL(pose.pose.pose, frame);
	static cv::Point robi[3];
	KDL::Vector rob[] = {
			frame * KDL::Vector(-8.0, 4.0, 0.0),
			frame * KDL::Vector(-8.0, -4.0, 0.0),
			frame * KDL::Vector(8.0, 0.0, 0.0)
	};
	robi[0] = cv::Point(rob[0].x(), rob[0].y());
	robi[1] = cv::Point(rob[1].x(), rob[1].y());
	robi[2] = cv::Point(rob[2].x(), rob[2].y());
	return robi;
}

// plot robot and world
void Plot::plot() {
	canvas.setTo(cv::Scalar(0., 0., 0.));
	cv::fillConvexPoly(canvas, calcTriangle(poseOdo), 3, cv::Scalar(128., 128., 255.));
	cv::fillConvexPoly(canvas, calcTriangle(poseTruth), 3, cv::Scalar(255., 255., 255.));
	//int npts[] = {3};
	//cv::polylines(canvas, (const cv::Point**) &robi, npts, 1, true, cv::Scalar(0., 0., 0.));
	std::vector<seif::landmark>::iterator it = world.landmarks.begin();
	for(;it != world.landmarks.end(); it++) {
		cv::circle(canvas, cv::Point((int)(*it).x,(int)(*it).y), 0, cv::Scalar(0., 0., 255.));
	}
	cv::imshow(wndTitle, canvas);
}


void Plot::poseTruthCB(const nav_msgs::Odometry& poseTruth) {
	Plot::instance().poseTruth = poseTruth;
}

void Plot::poseOdoCB(const nav_msgs::Odometry& poseOdo) {
	Plot::instance().poseOdo = poseOdo;
}

void Plot::worldCB(const seif::world& world) {
	Plot::instance().world = world;
}

int main(int argc, char** argv) {
	ros::init(argc, argv, "plot");
	Plot& plot = Plot::instance();
	bool run = true;
	while(run) {
		plot.plot();
		ros::spinOnce();
		int key = cv::waitKey(10) & 255;

		if(key == 27) run = false;
	}

	return 0;
}