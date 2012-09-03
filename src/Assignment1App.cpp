#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/CinderMath.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Assignment1App : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseMove(MouseEvent event);
	void mouseWheel(MouseEvent event);
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
	
	int mist_speed_;
	void draw_mist(uint8_t* pixels, mist_info m);
	void create_mist();

	deque<mist_info> mist_list_;

	Rand random_;

	int update_count_;

};

void Assignment1App::setup()
{
	PI_ = 3.1415927;
	update_count_ = 0;

	random_.seed(419);

	mist_speed_ = 1;

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

	bg_color_r_ = 255 *  mouse_X_ / ((float)kAppWidth);
	bg_color_g_ = 255 * sqrt(pow(mouse_X_,2.0) * pow(mouse_Y_,2.0)) /
		(float) sqrt(pow(kAppHeight,2.0) * pow(kAppWidth,2.0));
	bg_color_b_ = 255 * mouse_Y_ / ((float)kAppWidth);



}

void Assignment1App::mouseDown( MouseEvent event )
{

}

void Assignment1App::mouseWheel(MouseEvent event){
	if((mist_speed_ > -20 && mist_speed_ < 20))
		mist_speed_ += event.getWheelIncrement(); 
}

void Assignment1App::create_mist(){
	
	//create a mist packet
	mist_info m;
	m.x = mouse_X_;
	m.y = mouse_Y_;
	m.width = 5;
	m.height = 5;
	m.transparency = 0;
	m.r = bg_color_r_;
	m.g = bg_color_g_;
	m.b = bg_color_b_;

	mist_list_.push_front(m);
	if(mist_list_.size() > 15)
		mist_list_.pop_back();

}

void Assignment1App::draw_mist(uint8_t* pixels, mist_info m){

	for(int y = m.y; y < m.y + m.height; y++){
		for(int x = m.x; x < m.x + m.width; x++){
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight)
				continue;

			int offset = 3* (x + y*kTextureSize);
			pixels[offset] = pixels[offset]*m.transparency + m.r*(1-m.transparency);
			pixels[offset+1] = pixels[offset+1]*m.transparency + m.g*(1-m.transparency);
			pixels[offset+2] = pixels[offset+2]*m.transparency + m.b*(1-m.transparency);

		}
	}
}

void Assignment1App::update()
{
	update_count_++;

	if(update_count_ % 5 == 0){ //assuming 60 hz update, create new mist every second
		create_mist();
		update_count_ = 0; //reset counter
	}

	*work_Surface_ = (*bg_Surface_).clone();

	uint8_t* pixels = (*work_Surface_).getData();

	for(int i = 0; i < mist_list_.size(); i++){
		draw_mist(pixels,mist_list_[i]);
		if(mist_list_[i].height < 100 && mist_list_[i].width < 100){
			mist_list_[i].height += random_.nextInt(2);//get int from 0-1
			mist_list_[i].width += random_.nextInt(2);
		}
		mist_list_[i].x += mist_speed_*cos(2*PI_*update_count_/20.0 + random_.nextFloat());
		mist_list_[i].y += mist_speed_*sin(2*PI_*update_count_/20.0 + random_.nextFloat());
		if(mist_list_[i].transparency < 1)
			mist_list_[i].transparency += 0.005;
	}


}

void Assignment1App::draw()
{
	gl::draw(*work_Surface_);
}

CINDER_APP_BASIC( Assignment1App, RendererGl )
