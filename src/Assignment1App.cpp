#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class Assignment1App : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseMove(MouseEvent event);
	void update();
	void draw();
	void clearScreen();

private: 
	static const int kAppWidth = 800;
	static const int kAppHeight = 800;
	static const int kTextureSize = 1024;
	
	double PI_; // won't let me initialize even if it's a static const


	int mouse_X_;
	int mouse_Y_;
	float bg_color_r_;
	float bg_color_g_;
	float bg_color_b_;

	Surface* bg_Surface_; //background
	Surface* work_Surface_; //drawing

	struct mist_info{
		int x;
		int y;
		int width;
		int height;
		float r;
		float g;
		float b;
		float transparency;
	};
	
	void draw_mist(uint8_t* pixels, mist_info m);
	void create_mist();

	deque<mist_info> mist_list_;

	boost::posix_time::ptime app_start_time_;
	boost::posix_time::ptime current_time_;
	uint32_t milliseconds_;

};

void Assignment1App::setup()
{
	PI_ = 3.1415927;

	app_start_time_ = boost::posix_time::microsec_clock::local_time();
	milliseconds_ = 0;

	bg_Surface_ = new Surface(kTextureSize,kTextureSize,false);
	clearScreen();
	work_Surface_ = new Surface(kTextureSize,kTextureSize,false);
	
}

void Assignment1App::clearScreen(){
	
	uint8_t* pixels = (*bg_Surface_).getData();

	for(int y = 0; y < kAppHeight; y++){
		for(int x = 0; x < kAppWidth; x++){
			int offset = 3* (x + y*kTextureSize);
			pixels[offset] = 255;
			pixels[offset+1] = 255;
			pixels[offset+2] = 255;
		}
	}
}

void Assignment1App::mouseMove(MouseEvent event){

	mouse_X_ = event.getX();
	mouse_Y_ = event.getY();

	bg_color_r_ = mouse_X_ / ((float)kAppWidth);
	bg_color_g_ = sqrt(pow(mouse_X_,2.0) * pow(mouse_Y_,2.0)) /
		(float) sqrt(pow(kAppHeight,2.0) * pow(kAppWidth,2.0));
	bg_color_b_ = mouse_Y_ / ((float)kAppWidth);



}

void Assignment1App::mouseDown( MouseEvent event )
{
}

void Assignment1App::create_mist(){
	
	//create a mist packet
	mist_info m;
	m.x = mouse_X_;
	m.y = mouse_Y_;
	m.width = 5;
	m.height = 5;
	m.transparency = 0;
	m.r = 0;
	m.g = 0;
	m.b = 0;

	mist_list_.push_front(m);
	if(mist_list_.size() > 10)
		mist_list_.pop_back();

}

void Assignment1App::draw_mist(uint8_t* pixels, mist_info m){

	for(int y = m.y; y < m.y + m.height; y++){
		for(int x = m.x; x < m.x + m.width; x++){
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight)
				continue;

			int offset = 3* (x + y*kTextureSize);
			pixels[offset] = m.r;
			pixels[offset+1] = m.g;
			pixels[offset+2] = m.b;

		}
	}
}

void Assignment1App::update()
{
	//don't want to overflow the time variable
	if(milliseconds_ >= UINT32_MAX - 5000)
		app_start_time_ = boost::posix_time::microsec_clock::local_time();
	
	//count time since application start
	current_time_ = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration msdiff = current_time_ - app_start_time_;
	uint32_t milliseconds_ = msdiff.total_milliseconds();

	if(milliseconds_ % 500 < 100) // give a little margin for error for the update rate
		create_mist();

	*work_Surface_ = (*bg_Surface_).clone();

	uint8_t* pixels = (*work_Surface_).getData();

	for(int i = 0; i < mist_list_.size(); i++){
		draw_mist(pixels,mist_list_[i]);
		mist_list_[i].height += 5;
		mist_list_[i].width += 5;
		mist_list_[i].x += 10*cos(2*PI_*milliseconds_/6000.0);
		mist_list_[i].y += 10*sin(2*PI_*milliseconds_/6000.0);
		mist_list_[i].transparency += 5;
	}

}

void Assignment1App::draw()
{
	gl::draw(*work_Surface_);
}

CINDER_APP_BASIC( Assignment1App, RendererGl )
