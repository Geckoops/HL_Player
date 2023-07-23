#ifndef HL_PLAYER_INCLUDE_VIDEOCALL_H_
#define HL_PLAYER_INCLUDE_VIDEOCALL_H_
#include "Reference.h"
class VideoCall
{
public:
	virtual void init(int width, int height) = 0;
	virtual void rePaint(AVFrame *frame) = 0;
};
#endif // HL_PLAYER_INCLUDE_VIDEOCALL_H_
