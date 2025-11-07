#pragma once
#include "헤더.h"

// Vec2 구조체 정의 (RGBA는 CommonHeaders.h에 있음)
struct Vec2 { float x, y; };

// winX, winY : 윈도우(또는 프레임버퍼) 기준 픽셀 좌표 (원점: 좌상단)
// pixelCenter : 픽셀 센터를 맞추려면 true (마우스 클릭 지점이 픽셀 중앙에 매핑됨)
Vec2 WindowToNDC(int winX, int winY, bool pixelCenter = true) {
	// 현재 glViewport 값 가져오기: {x, y, w, h}
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	const float vx = (float)vp[0];
	const float vy = (float)vp[1];
	const float vw = (float)vp[2];
	const float vh = (float)vp[3];

	// 픽셀 센터 보정(원하면 0.5 더함)
	const float px = pixelCenter ? (winX + 0.5f) : (float)winX;
	const float py = pixelCenter ? (winY + 0.5f) : (float)winY;

	// 윈도우 좌표(좌상단 원점, +y 아래) → 뷰포트 정규 [0,1]
	const float nx = (px - vx) / vw;          // 0(left) ~ 1(right)
	const float ny_down = (py - vy) / vh;     // 0(top)  ~ 1(bottom)

	// [0,1] → [-1,1], y축 뒤집기
	Vec2 ndc;
	ndc.x = nx * 2.0f - 1.0f;
	ndc.y = 1.0f - (ny_down * 2.0f);         // 위로 증가하도록 플립

	return ndc;
}