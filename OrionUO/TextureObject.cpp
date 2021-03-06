﻿/***********************************************************************************
**
** TextureObject.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "stdafx.h"
//----------------------------------------------------------------------------------
CTextureAnimationFrame::CTextureAnimationFrame()
: m_CenterX(0), m_CenterY(0), m_Texture(), m_PixelData(NULL)
{
}
//----------------------------------------------------------------------------------
CTextureAnimationFrame::~CTextureAnimationFrame()
{
	m_PixelData.clear();
}
//----------------------------------------------------------------------------------
CTextureAnimationDirection::CTextureAnimationDirection()
: m_FrameCount(0), m_BaseAddress(0), m_BaseSize(0), m_PatchedAddress(0),
m_PatchedSize(0), m_Address(0), m_Size(0), m_LastAccessTime(0), m_Frames(NULL), m_IsUOP(false)
{
}
//----------------------------------------------------------------------------------
CTextureAnimationDirection::~CTextureAnimationDirection()
{
}
//----------------------------------------------------------------------------------
CTextureAnimationGroup::CTextureAnimationGroup()
: m_IsUOP(false)
{
}
//----------------------------------------------------------------------------------
CTextureAnimationGroup::~CTextureAnimationGroup()
{
}
//----------------------------------------------------------------------------------
