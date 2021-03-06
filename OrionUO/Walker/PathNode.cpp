﻿/***********************************************************************************
**
** PathNode.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "stdafx.h"
//----------------------------------------------------------------------------------
//------------------------------------CPathNode-------------------------------------
//----------------------------------------------------------------------------------
CPathNode::CPathNode()
{
}
//----------------------------------------------------------------------------------
CPathNode::~CPathNode()
{
	m_Parent = NULL;
}
//----------------------------------------------------------------------------------
void CPathNode::Reset()
{
	m_Parent = NULL;
	m_Used = false;
	m_X = m_Y = m_Z = m_Direction = m_Cost = m_DistFromStartCost = m_DistFromGoalCost = 0;
}
//----------------------------------------------------------------------------------
