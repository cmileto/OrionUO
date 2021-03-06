﻿/***********************************************************************************
**
** GUIScissor.h
**
** Компонента для указания области вывода
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef GUISCISSOR_H
#define GUISCISSOR_H
//----------------------------------------------------------------------------------
class CGUIScissor : public CGUIPolygonal
{
	//!Координата компоненты по оси X контейнера, в котором находится элемент, относительно начала гампа
	SETGET(int, BaseX, 0);

	//!Координата компоненты по оси Y контейнера, в котором находится элемент, относительно начала гампа
	SETGET(int, BaseY, 0);

public:
	CGUIScissor(const bool &enabled, const int &baseX = 0, const int &baseY = 0, const int &x = 0, const int &y = 0, const int &width = 0, const int &height = 0);
	virtual ~CGUIScissor();

	virtual void Draw(const bool &checktrans = false);
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
