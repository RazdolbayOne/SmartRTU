#include "Engine.h"

#include <unistd.h>/*usleep() e.tc*/

// C++
#include <cstdio>/*std::fprintf*/

//libshape stuff probably can cut some not used
/*OpenVG stuff like vgLoadIdenty*/
extern "C"
{
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"
}


#include"timetable.h"//to delete timetable
#include "board.h"/*to also delete board*/
#include "Timer.h"/*StrNow()*/



const char *fmt(const char *f, const char *s1, const char *s2, const char *s3)
{
	static char buf[100];
	std::sprintf(buf, f, s1, s2, s3);
	return buf;
}

//********************************************************************************
//*** Engine
//********************************************************************************

#define DEBUG_OUTPUT_PRD (60 * 60 * 1000) // 1hour
#define FLUSH_LOG_PRD 1000				  // 1sec

Engine *engine = nullptr;

Engine::Engine()
{
	is_terminated_ = false;
	board_ = new Board;
	is_running_ = false;
	frame_capacity_ = 1000 / FRAMES_PER_SECOND;
}

Engine::~Engine()
{
	if (board_){
		delete board_;
	}
	
	if (timetable){
		delete timetable;
	}
}

void Engine::start()
{
	if (!is_running_){
		this->run_();
	}
}

void Engine::run_()
{
	is_running_ = true;

	LongTimeMs idleTime = 0;
	LongTimeMs lastRender = 0;
	LongTimeMs lastDebug = 0;
	LongTimeMs lastFlush = 0;
	int fps = 0;

	this->ForceUpdate();
	this->render(true);

	std::fprintf(stdout,"%s\tEngine main circle  started\n", StrNow());

	while (is_running_ && !is_terminated_)
	{
		LongTimeMs time = timer.GetTime();

		this->update();

		if ((time - lastRender) >= static_cast<long long unsigned int>(frame_capacity_))
		{
			this->render(false);
			lastRender = time;
			fps++;
		}

#ifdef DEBUG_OUTPUT_PRD

		if ((time - lastDebug) >= DEBUG_OUTPUT_PRD)
		{
			unsigned int tsec = static_cast<unsigned int>((time / 1000LLU));
			unsigned int tm_d = tsec / (24 * 3600);
			tsec %= (24 * 3600);
			unsigned int tm_h = tsec / 3600;
			tsec %= 3600;
			unsigned int tm_m = tsec / 60;
			tsec %= 60;
			unsigned int tm_s = tsec;
			std::fprintf(stdout,
				"%s\tStatus: on time: %u:%02u:%02u:%02u, idle: %llu%%, fps: %.0f\n",
				StrNow(), tm_d, tm_h, tm_m, tm_s,
				idleTime * 100 / (time - lastDebug),
				(float)(fps)*1000 / (time - lastDebug));

			fps = 0;
			idleTime = 0;
			lastDebug = time;
		}

#endif

#ifdef FLUSH_LOG_PRD

		if (time - lastFlush >= FLUSH_LOG_PRD)
		{
			std::fflush(stdout);
			lastFlush = time;
		}

#endif

		usleep(1000); // 1ms
		idleTime += 1;
	}

	std::fprintf(stdout,"%s\tEngine main circle  finished\n", StrNow());
}

void Engine::update()
{
	board_->update(false);
}

void Engine::ForceUpdate()
{
	board_->update(true);
}

void Engine::render(bool All)
{

	VGfloat color[4] = {1, 1, 1, 1};

	color[0] = 0, color[1] = 0, color[2] = 0;
	setfill(color);
	setstroke(color);
	StrokeWidth(0);
	vgLoadIdentity();

	VGfloat colour[4];
	RGB(0, 0, 0, colour);
	vgSetfv(VG_CLEAR_COLOR, 4, colour);

	board_->render(All);

	End();
}
