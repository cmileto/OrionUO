﻿/***********************************************************************************
**
** SkillGroup.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef SKILLGROUP_H
#define SKILLGROUP_H
//----------------------------------------------------------------------------------
//Класс группы навыков
class CSkillGroupObject
{
	SETGET(int, Count, 0);
	SETGET(bool, Maximized, false);
	SETGET(string, Name, "No Name");

private:
	//Номера навыков
	uchar m_Items[60];

public:
	//Ссылки на следующую и предыдущую группы
	CSkillGroupObject *m_Next{ NULL };
	CSkillGroupObject *m_Prev{ NULL };

	CSkillGroupObject();
	~CSkillGroupObject();

	uchar GetItem(int index);

	//Добавить навык в группу	
	void Add(uchar index);

	//Добавить навык и отсортировать
	void AddSorted(uchar index);

	//Удалить навык
	void Remove(uchar index);

	//Проверка, содержит ли группа навык
	bool Contains(uchar index);

	//Сортировать навыки
	void Sort();

	//Передать навык другой группе
	void TransferTo(CSkillGroupObject *group);
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
