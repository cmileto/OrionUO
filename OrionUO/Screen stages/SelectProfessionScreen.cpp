﻿/***********************************************************************************
**
** SelectProfessionScreen.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "stdafx.h"
//----------------------------------------------------------------------------------
CSelectProfessionScreen g_SelectProfessionScreen;
//----------------------------------------------------------------------------------
CSelectProfessionScreen::CSelectProfessionScreen()
: CBaseScreen(m_SelectProfessionGump)
{
}
//----------------------------------------------------------------------------------
CSelectProfessionScreen::~CSelectProfessionScreen()
{
}
//----------------------------------------------------------------------------------
void CSelectProfessionScreen::OnChangeSkillSelection(const int &val)
{
	m_Gump.WantUpdateContent = true;
}
//----------------------------------------------------------------------------------
/*!
Инициализация
@return 
*/
void CSelectProfessionScreen::Init()
{
	WISPFUN_DEBUG("c166_f1");
	g_ProfessionManager.Selected = (CBaseProfession*)g_ProfessionManager.m_Items;
	m_SkillSelection = 0;

	g_ScreenEffectManager.UseSunrise();
	m_SmoothScreenAction = 0;

	m_Gump.PrepareTextures();
	m_Gump.WantUpdateContent = true;
}
//----------------------------------------------------------------------------------
/*!
Обработка события после плавного затемнения экрана
@param [__in_opt] action Идентификатор действия
@return 
*/
void CSelectProfessionScreen::ProcessSmoothAction(uchar action)
{
	WISPFUN_DEBUG("c166_f2");
	if (action == 0xFF)
		action = m_SmoothScreenAction;

	if (action == ID_SMOOTH_SPS_QUIT)
		g_OrionWindow.Destroy();
	else if (action == ID_SMOOTH_SPS_GO_SCREEN_CHARACTER)
		g_Orion.InitScreen(GS_CHARACTER);
	else if (action == ID_SMOOTH_SPS_GO_SCREEN_GAME_CONNECT)
	{
		g_Orion.InitScreen(GS_GAME_CONNECT);
		g_ConnectionScreen.Type = CST_SELECT_PROFESSOIN;
	}
	else if (action == ID_SMOOTH_SPS_GO_SCREEN_CREATE)
		g_Orion.InitScreen(GS_CREATE);
}
//----------------------------------------------------------------------------------
