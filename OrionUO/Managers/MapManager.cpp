﻿/***********************************************************************************
**
** MapManager.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "stdafx.h"
//----------------------------------------------------------------------------------
CMapManager *g_MapManager = NULL;
//----------------------------------------------------------------------------------
CIndexMap::CIndexMap()
{
}
//----------------------------------------------------------------------------------
CIndexMap::~CIndexMap()
{
}
//----------------------------------------------------------------------------------
CMapManager::CMapManager()
: CBaseQueue()
{
}
//----------------------------------------------------------------------------------
CMapManager::~CMapManager()
{
	WISPFUN_DEBUG("c146_f1");
	if (m_Blocks != NULL)
	{
		ClearUsedBlocks();

		delete[] m_Blocks;
		m_Blocks = NULL;
	}

	m_MaxBlockIndex = 0;
}
//----------------------------------------------------------------------------------
void CMapManager::CreateBlocksTable()
{
	WISPFUN_DEBUG("c146_f2");
	IFOR(map, 0, MAX_MAPS_COUNT)
		CreateBlockTable(map);
}
//----------------------------------------------------------------------------------
void CMapManager::CreateBlockTable(int map)
{
	WISPFUN_DEBUG("c146_f3");
	MAP_INDEX_LIST &list = m_BlockData[map];
	WISP_GEOMETRY::CSize &size = g_MapBlockSize[map];

	int maxBlockCount = size.Width * size.Height;

	//Return and error notification?
	if (maxBlockCount < 1)
		return;

	list.resize(maxBlockCount);

	uint mapAddress = (uint)g_FileManager.m_MapMul[map].Start;
	uint endMapAddress = mapAddress + g_FileManager.m_MapMul[map].Size;

	uint staticIdxAddress = (uint)g_FileManager.m_StaticIdx[map].Start;
	uint endStaticIdxAddress = staticIdxAddress + g_FileManager.m_StaticIdx[map].Size;

	uint staticAddress = (uint)g_FileManager.m_StaticMul[map].Start;
	uint endStaticAddress = staticAddress + g_FileManager.m_StaticMul[map].Size;

	if (!mapAddress || !staticIdxAddress || !staticAddress)
		return;

	IFOR(block, 0, maxBlockCount)
	{
		CIndexMap &index = list[block];

		uint realMapAddress = 0;
		uint realStaticAddress = 0;
		int realStaticCount = 0;

		if (mapAddress != 0)
		{
			uint address = mapAddress + (block * sizeof(MAP_BLOCK));

			if (address < endMapAddress)
				realMapAddress = address;
		}
		uint staticIdxBlockAddress = staticIdxAddress + block * sizeof(STAIDX_BLOCK);
		if (staticIdxAddress != 0 && staticAddress != 0 && staticIdxBlockAddress < endStaticIdxAddress)
		{
			PSTAIDX_BLOCK sidx = reinterpret_cast<PSTAIDX_BLOCK>(staticIdxBlockAddress);

			if (sidx->Size > 0 && sidx->Position != 0xFFFFFFFF && (uint)sidx < endStaticIdxAddress)
			{
				uint address = staticAddress + sidx->Position;

				if (address < endStaticAddress)
				{
					realStaticAddress = address;
					realStaticCount = sidx->Size / sizeof(STATICS_BLOCK);

					if (realStaticCount > 1024)
						realStaticCount = 1024;
				}
			}
		}

		index.OriginalMapAddress = realMapAddress;
		index.OriginalStaticAddress = realStaticAddress;
		index.OriginalStaticCount = realStaticCount;

		index.MapAddress = realMapAddress;
		index.StaticAddress = realStaticAddress;
		index.StaticCount = realStaticCount;
	}
}
//----------------------------------------------------------------------------------
void CMapManager::SetPatchedMapBlock(const uint &block, const uint &address)
{
	WISPFUN_DEBUG("c146_f4");
	MAP_INDEX_LIST &list = m_BlockData[0];
	WISP_GEOMETRY::CSize &size = g_MapBlockSize[0];

	int maxBlockCount = size.Width * size.Height;

	if (maxBlockCount < 1)
		return;

	list[block].OriginalMapAddress = address;
	list[block].MapAddress = address;
}
//----------------------------------------------------------------------------------
void CMapManager::ResetPatchesInBlockTable()
{
	WISPFUN_DEBUG("c146_f5");
	IFOR(map, 0, MAX_MAPS_COUNT)
	{
		MAP_INDEX_LIST &list = m_BlockData[map];
		WISP_GEOMETRY::CSize &size = g_MapBlockSize[map];

		int maxBlockCount = size.Width * size.Height;

		//Return and error notification?
		if (maxBlockCount < 1)
			return;

		if (g_FileManager.m_MapMul[map].Start == NULL || g_FileManager.m_StaticIdx[map].Start == NULL || g_FileManager.m_StaticMul[map].Start == NULL)
			return;

		IFOR(block, 0, maxBlockCount)
		{
			CIndexMap &index = list[block];

			index.MapAddress = index.OriginalMapAddress;
			index.StaticAddress = index.OriginalStaticAddress;
			index.StaticCount = index.OriginalStaticCount;
		}
	}
}
//----------------------------------------------------------------------------------
void CMapManager::ApplyPatches(WISP_DATASTREAM::CDataReader &stream)
{
	WISPFUN_DEBUG("c146_f6");
	ResetPatchesInBlockTable();

	int count = stream.ReadUInt32BE();

	if (count < 0)
		count = 0;

	if (count > MAX_MAPS_COUNT)
		count = MAX_MAPS_COUNT;

	IFOR(i, 0, count)
	{
		int mapPatchesCount = stream.ReadUInt32BE();
		int staticsPatchesCount = stream.ReadUInt32BE();

		MAP_INDEX_LIST &list = m_BlockData[i];
		WISP_GEOMETRY::CSize &size = g_MapBlockSize[i];

		uint maxBlockCount = size.Height * size.Width;

		if (mapPatchesCount)
		{
			WISP_FILE::CMappedFile &difl = g_FileManager.m_MapDifl[i];
			WISP_FILE::CMappedFile &dif = g_FileManager.m_MapDif[i];

			mapPatchesCount = min(mapPatchesCount, difl.Size / 4);

			difl.ResetPtr();
			dif.ResetPtr();

			IFOR(j, 0, mapPatchesCount)
			{
				uint blockIndex = difl.ReadUInt32LE();

				if (blockIndex < maxBlockCount)
					list[blockIndex].MapAddress = (uint)dif.Ptr;

				dif.Move(sizeof(MAP_BLOCK));
			}
		}

		if (staticsPatchesCount)
		{
			WISP_FILE::CMappedFile &difl = g_FileManager.m_StaDifl[i];
			WISP_FILE::CMappedFile &difi = g_FileManager.m_StaDifi[i];
			uint startAddress = (uint)g_FileManager.m_StaDif[i].Start;

			staticsPatchesCount = min(staticsPatchesCount, difl.Size / 4);

			difl.ResetPtr();
			difi.ResetPtr();

			IFOR(j, 0, staticsPatchesCount)
			{
				uint blockIndex = difl.ReadUInt32LE();

				PSTAIDX_BLOCK sidx = (PSTAIDX_BLOCK)difi.Ptr;

				difi.Move(sizeof(STAIDX_BLOCK));

				if (blockIndex < maxBlockCount)
				{
					uint realStaticAddress = 0;
					int realStaticCount = 0;

					if (sidx->Size > 0 && sidx->Position != 0xFFFFFFFF)
					{
						realStaticAddress = startAddress + sidx->Position;
						realStaticCount = sidx->Size / sizeof(STATICS_BLOCK);

						if (realStaticCount > 0)
						{
							if (realStaticCount > 1024)
								realStaticCount = 1024;
						}
					}

					list[blockIndex].StaticAddress = realStaticAddress;
					list[blockIndex].StaticCount = realStaticCount;
				}
			}
		}
	}

	UpdatePatched();
}
//----------------------------------------------------------------------------------
void CMapManager::UpdatePatched()
{
	WISPFUN_DEBUG("c146_f7");
	if (g_Player == NULL)
		return;

	deque<CRenderWorldObject*> objectsList;

	if (m_Blocks != NULL)
	{
		QFOR(block, m_Items, CMapBlock*)
		{
			IFOR(x, 0, 8)
			{
				IFOR(y, 0, 8)
				{
					for (CRenderWorldObject *item = block->GetRender(x, y); item != NULL; item = item->m_NextXY)
{
						if (!item->IsLandObject() && !item->IsStaticObject())
							objectsList.push_back(item);
					}
				}
			}
		}
	}

	Init(false);

	for (CRenderWorldObject *item : objectsList)
		AddRender(item);

	CGumpMinimap *gump = (CGumpMinimap*)g_GumpManager.UpdateGump(g_PlayerSerial, 0, GT_MINIMAP);

	if (gump != NULL)
		gump->LastX = 0;
}
//----------------------------------------------------------------------------------
CIndexMap *CMapManager::GetIndex(const uint &map, const int &blockX, const int &blockY)
{
	WISPFUN_DEBUG("c146_f8");
	if (map >= MAX_MAPS_COUNT)
		return NULL;

	uint block = (blockX * g_MapBlockSize[map].Height) + blockY;
	MAP_INDEX_LIST &list = m_BlockData[map];

	if (block >= list.size())
		return NULL;

	return &list[block];
}
//----------------------------------------------------------------------------------
void CMapManager::ClearBlockAccess()
{
	memset(&m_BlockAccessList[0], 0, sizeof(m_BlockAccessList));
}
//----------------------------------------------------------------------------------
char CMapManager::CalculateNearZ(char defaultZ, const int &x, const int &y, const int &z)
{
	int blockX = x / 8;
	int blockY = y / 8;
	uint index = (blockX * g_MapBlockSize[g_CurrentMap].Height) + blockY;

	bool &accessBlock = m_BlockAccessList[(x & 0x3F) + ((y & 0x3F) << 6)];

	if (accessBlock)
		return defaultZ;

	accessBlock = true;
	CMapBlock *block = GetBlock(index);

	if (block != NULL)
	{
		CMapObject *item = block->Block[x % 8][y % 8];

		for (; item != NULL; item = (CMapObject*)item->m_Next)
		{
			if (!item->IsGameObject())
			{
				if (!item->IsStaticObject() && !item->IsMultiObject())
					continue;
			}
			else if (((CGameObject*)item)->NPC)
				continue;

			if (!item->IsRoof() || abs(z - item->Z) > 6)
				continue;

			break;
		}

		if (item == NULL)
			return defaultZ;

		char tileZ = item->Z;

		if (tileZ < defaultZ)
			defaultZ = tileZ;

		defaultZ = CalculateNearZ(defaultZ, x - 1, y, tileZ);
		defaultZ = CalculateNearZ(defaultZ, x + 1, y, tileZ);
		defaultZ = CalculateNearZ(defaultZ, x, y - 1, tileZ);
		defaultZ = CalculateNearZ(defaultZ, x, y + 1, tileZ);
	}

	return defaultZ;
}
//----------------------------------------------------------------------------------
/*!
Получить блок карты напрямую из мулов
@param [__in] map Индекс карты
@param [__in] blockX Координата X блока
@param [__in] blockY Координата Y блока
@param [__out] mb Ссылка на блок
@return Код ошибки (0 - успешно)
*/
void CMapManager::GetWorldMapBlock(const int &map, const int &blockX, const int &blockY, MAP_BLOCK &mb)
{
	WISPFUN_DEBUG("c146_f9");
	CIndexMap *indexMap = GetIndex(map, blockX, blockY);

	if (indexMap == NULL || indexMap->MapAddress == 0)
		return;

	PMAP_BLOCK pmb = (PMAP_BLOCK)indexMap->MapAddress;

	IFOR(x, 0, 8)
	{
		IFOR(y, 0, 8)
		{
			int pos = (y * 8) + x;
			mb.Cells[pos].TileID = pmb->Cells[pos].TileID;
			mb.Cells[pos].Z = pmb->Cells[pos].Z;
		}
	}

	PSTATICS_BLOCK sb = (PSTATICS_BLOCK)indexMap->StaticAddress;

	if (sb != NULL)
	{
		int count = indexMap->StaticCount;

		IFOR(c, 0, count)
		{
			if (sb->Color && sb->Color != 0xFFFF)
			{
				int pos = (sb->Y * 8) + sb->X;
				//if (pos > 64) continue;

				if (mb.Cells[pos].Z <= sb->Z)
				{
					mb.Cells[pos].TileID = sb->Color;
					mb.Cells[pos].Z = sb->Z;
				}
			}

			sb++;
		}
	}
}
//----------------------------------------------------------------------------------
/*!
Получить блок для радара из муллов
@param [__in] blockX Координата X блока
@param [__in] blockY Координата Y блока
@param [__out] mb Ссылка на блок
@return 
*/
void CMapManager::GetRadarMapBlock(const int &blockX, const int &blockY, MAP_BLOCK &mb)
{
	WISPFUN_DEBUG("c146_f10");
	CIndexMap *indexMap = GetIndex(GetActualMap(), blockX, blockY);

	if (indexMap == NULL || indexMap->MapAddress == 0)
		return;

	PMAP_BLOCK pmb = (PMAP_BLOCK)indexMap->MapAddress;
	
	IFOR(x, 0, 8)
	{
		IFOR(y, 0, 8)
		{
			int pos = (y * 8) + x;
			mb.Cells[pos].TileID = pmb->Cells[pos].TileID;
			mb.Cells[pos].Z = pmb->Cells[pos].Z;
		}
	}

	PSTATICS_BLOCK sb = (PSTATICS_BLOCK)indexMap->StaticAddress;

	if (sb != NULL)
	{
		int count = indexMap->StaticCount;

		IFOR(c, 0, count)
		{
			if (sb->Color && sb->Color != 0xFFFF)
			{
				int pos = (sb->Y * 8) + sb->X;
				//if (pos > 64) continue;

				if (mb.Cells[pos].Z <= sb->Z)
				{
					mb.Cells[pos].TileID = sb->Color + 0x4000;
					mb.Cells[pos].Z = sb->Z;
				}
			}

			sb++;
		}
	}
}
//----------------------------------------------------------------------------------
/*!
Получить значение Z координаты для указанной точки в мире
@param [__in] x Координата X
@param [__in] y Координата Y
@param [__out] groundZ Значение Z коррдинаты земли
@param [__out] staticZ Значение Z коррдинаты статики
@return 
*/
void CMapManager::GetMapZ(const int &x, const int &y, int &groundZ, int &staticZ)
{
	WISPFUN_DEBUG("c146_f11");
	int blockX = x / 8;
	int blockY = y / 8;
	uint index = (blockX * g_MapBlockSize[g_CurrentMap].Height) + blockY;

	if (index < m_MaxBlockIndex)
	{
		CMapBlock *block = GetBlock(index);

		if (block == NULL)
		{
			block = AddBlock(index);
			block->X = blockX;
			block->Y = blockY;
			LoadBlock(block);
		}

		CMapObject *item = block->Block[x % 8][y % 8];

		while (item != NULL)
		{
			if (item->IsLandObject())
				groundZ = item->Z;
			else if (staticZ < item->Z)
				staticZ = item->Z;

			item = (CMapObject*)item->m_Next;
		}
	}
}
//----------------------------------------------------------------------------------
/*!
Удалить неиспользуемые блоки
@return 
*/
void CMapManager::ClearUnusedBlocks()
{
	WISPFUN_DEBUG("c146_f12");
	CMapBlock *block = (CMapBlock*)m_Items;
	uint ticks = g_Ticks - CLEAR_TEXTURES_DELAY;
	int count = 0;

	while (block != NULL)
	{
		CMapBlock *next = (CMapBlock*)block->m_Next;

		if (block->LastAccessTime < ticks && block->HasNoExternalData())
		{
			uint index = block->Index;
			Delete(block);

			m_Blocks[index] = NULL;

			if (++count >= MAX_MAP_OBJECT_REMOVED_BY_GARBAGE_COLLECTOR)
				break;
		}

		block = next;
	}
}
//----------------------------------------------------------------------------------
void CMapManager::ClearUsedBlocks()
{
	WISPFUN_DEBUG("c146_f13");
	CMapBlock *block = (CMapBlock*)m_Items;

	while (block != NULL)
	{
		CMapBlock *next = (CMapBlock*)block->m_Next;

		uint index = block->Index;
		Delete(block);

		m_Blocks[index] = NULL;

		block = next;
	}
}
//----------------------------------------------------------------------------------
/*!
Инициализация
@param [__in_opt] delayed По истечении времени на загрузку выходить из цикла
@return 
*/
void CMapManager::Init(const bool &delayed)
{
	WISPFUN_DEBUG("c146_f14");
	if (g_Player == NULL)
		return;

	int map = GetActualMap();

	if (!delayed)
	{
		if (m_Blocks != NULL)
		{
			ClearUsedBlocks();

			delete[] m_Blocks;
			m_Blocks = NULL;
		}

		m_MaxBlockIndex = g_MapBlockSize[map].Width * g_MapBlockSize[map].Height;
		m_Blocks = new CMapBlock*[m_MaxBlockIndex];
		memset(&m_Blocks[0], 0, sizeof(CMapBlock*) * m_MaxBlockIndex);
		memset(&m_BlockAccessList[0], 0, sizeof(m_BlockAccessList));
	}
	
	const int XY_Offset = 30; //70;

	int minBlockX = (g_Player->X - XY_Offset) / 8 - 1;
	int minBlockY = (g_Player->Y - XY_Offset) / 8 - 1;
	int maxBlockX = ((g_Player->X + XY_Offset) / 8) + 1;
	int maxBlockY = ((g_Player->Y + XY_Offset) / 8) + 1;

	if (minBlockX < 0)
		minBlockX = 0;

	if (minBlockY < 0)
		minBlockY = 0;

	if (maxBlockX >= g_MapBlockSize[map].Width)
		maxBlockX = g_MapBlockSize[map].Width - 1;

	if (maxBlockY >= g_MapBlockSize[map].Height)
		maxBlockY = g_MapBlockSize[map].Height - 1;

	uint ticks = g_Ticks;
	uint maxDelay = g_FrameDelay[1] / 2;

	for (int i = minBlockX; i <= maxBlockX; i++)
	{
		uint index = i * g_MapBlockSize[map].Height;

		for (int j = minBlockY; j <= maxBlockY; j++)
		{
			uint realIndex = index + j;

			if (realIndex < m_MaxBlockIndex)
			{
				CMapBlock *block = GetBlock(realIndex);

				if (block == NULL)
				{
					if (delayed && g_Ticks - ticks >= maxDelay)
						return;

					block = AddBlock(realIndex);
					block->X = i;
					block->Y = j;
					LoadBlock(block);
				}
			}
		}
	}
}
//----------------------------------------------------------------------------------
/*!
Загрузить блок
@param [__inout] block Ссылка на блок для загрузки
@return 
*/
void CMapManager::LoadBlock(CMapBlock *block)
{
	WISPFUN_DEBUG("c146_f15");
	int map = GetActualMap();

	CIndexMap *indexMap = GetIndex(GetActualMap(), block->X, block->Y);

	if (indexMap == NULL || indexMap->MapAddress == 0)
		return;

	PMAP_BLOCK pmb = (PMAP_BLOCK)indexMap->MapAddress;

	int bx = block->X * 8;
	int by = block->Y * 8;

	IFOR(x, 0, 8)
	{
		IFOR(y, 0, 8)
		{
			int pos = y * 8 + x;
			CMapObject *obj = new CLandObject(pos, pmb->Cells[pos].TileID & 0x3FFF, 0, bx + x, by + y, pmb->Cells[pos].Z);
			block->AddObject(obj, x, y);
		}
	}

	PSTATICS_BLOCK sb = (PSTATICS_BLOCK)indexMap->StaticAddress;

	if (sb != NULL)
	{
		int count = indexMap->StaticCount;

		for (int c = 0; c < count; c++, sb++)
		{
			if (sb->Color && sb->Color != 0xFFFF)
			{
				int x = sb->X;
				int y = sb->Y;

				int pos = (y * 8) + x;

				if (pos >= 64)
					continue;

				CRenderStaticObject *obj = new CStaticObject(pos, sb->Color, sb->Hue, bx + x, by + y, sb->Z);

				string lowerName = ToLowerA(obj->GetStaticData()->Name);
				obj->NoDrawTile = (lowerName == "nodraw" || lowerName == "no draw");

				block->AddObject(obj, x, y);
			}
		}
	}

	block->CreateLandTextureRect();
}
//----------------------------------------------------------------------------------
/*!
Получить индекс текущей карты
@return
*/
int CMapManager::GetActualMap()
{
	WISPFUN_DEBUG("c146_f16");
	if (g_CurrentMap == 1 && (!g_FileManager.m_MapMul[1].Start || !g_FileManager.m_StaticIdx[1].Start || !g_FileManager.m_StaticMul[1].Start))
		return 0;

	return g_CurrentMap;
}
//----------------------------------------------------------------------------------
/*!
Добавить объект рендера
@param [__in] item Ссылка на объект
@return 
*/
void CMapManager::AddRender(CRenderWorldObject *item)
{
	WISPFUN_DEBUG("c146_f17");
	int itemX = item->X;
	int itemY = item->Y;

	int x = itemX / 8;
	int y = itemY / 8;
	
	uint index = (x * g_MapBlockSize[g_CurrentMap].Height) + y;
	
	if (index < m_MaxBlockIndex)
	{
		CMapBlock *block = GetBlock(index);

		if (block == NULL)
		{
			block = AddBlock(index);
			block->X = x;
			block->Y = y;
			LoadBlock(block);
		}

		x = itemX % 8;
		y = itemY % 8;

		block->AddRender(item, x, y);
	}
}
//----------------------------------------------------------------------------------
/*!
Получить ссылку на блок
@param [__in] index Индекс блока
@return Ссылка на блок или NULL
*/
CMapBlock *CMapManager::GetBlock(const uint &index)
{
	WISPFUN_DEBUG("c146_f18");
	CMapBlock *block = NULL;

	if (index < m_MaxBlockIndex)
	{
		block = m_Blocks[index];

		if (block != NULL)
			block->LastAccessTime = g_Ticks;
	}

	return block;
}
//----------------------------------------------------------------------------------
/*!
Добавить блок
@param [__in] index Индекс блока
@return Ссылка на блок или NULL
*/
CMapBlock *CMapManager::AddBlock(const uint &index)
{
	WISPFUN_DEBUG("c146_f19");
	CMapBlock *block = (CMapBlock*)Add(new CMapBlock(index));

	m_Blocks[index] = block;

	return block;
}
//----------------------------------------------------------------------------------
/*!
Удалить блок
@param [__in] index Индекс блока
@return 
*/
void CMapManager::DeleteBlock(const uint &index)
{
	WISPFUN_DEBUG("c146_f20");
	CMapBlock *block = (CMapBlock*)m_Items;

	while (block != NULL)
	{
		if (block->Index == index)
		{
			Delete(block);
			m_Blocks[index] = NULL;

			break;
		}

		block = (CMapBlock*)block->m_Next;
	}
}
//----------------------------------------------------------------------------------
CUopMapManager::CUopMapManager()
: CMapManager()
{
}
//----------------------------------------------------------------------------------
CUopMapManager::~CUopMapManager()
{
}
//----------------------------------------------------------------------------------
/*!
Получить индекс текущей карты
@return
*/
int CUopMapManager::GetActualMap()
{
	if (g_CurrentMap == 1 && (!g_FileManager.m_MapUOP[1].Start || !g_FileManager.m_StaticIdx[1].Start || !g_FileManager.m_StaticMul[1].Start))
		return 0;

	return g_CurrentMap;
}
//----------------------------------------------------------------------------------
void CUopMapManager::CreateBlockTable(int map)
{
	MAP_INDEX_LIST &list = m_BlockData[map];
	WISP_GEOMETRY::CSize &size = g_MapBlockSize[map];

	int maxBlockCount = size.Width * size.Height;

	//Return and error notification?
	if (maxBlockCount < 1)
		return;

	list.resize(maxBlockCount);

	auto &uopFile = g_FileManager.m_MapUOP[map];
	uint mapAddress = reinterpret_cast<uint>(g_FileManager.m_MapUOP[map].Start);
	uint endMapAddress = mapAddress + g_FileManager.m_MapUOP[map].Size;

	uint staticIdxAddress = reinterpret_cast<uint>(g_FileManager.m_StaticIdx[map].Start);
	uint endStaticIdxAddress = staticIdxAddress + g_FileManager.m_StaticIdx[map].Size;

	uint staticAddress = reinterpret_cast<uint>(g_FileManager.m_StaticMul[map].Start);
	uint endStaticAddress = staticAddress + g_FileManager.m_StaticMul[map].Size;

	if (!mapAddress || !staticIdxAddress || !staticAddress)
		return;

	//Начинаем читать УОП
	if (uopFile.ReadInt32LE() != 0x50594D)
	{
		LOG("Bad Uop file %s", uopFile);
		return;
	}

	uopFile.ReadInt64LE(); // version + signature
	long long nextBlock = uopFile.ReadInt64LE();

	std::unordered_map<unsigned long long, UOPMapaData> hashes;

	uopFile.ResetPtr();
	uopFile.Move(static_cast<int>(nextBlock));

	do
	{
		int fileCount = uopFile.ReadInt32LE();
		nextBlock = uopFile.ReadInt64LE();
		IFOR(i, 0, fileCount)
		{
			auto offset = uopFile.ReadInt64LE();
			auto headerLength = uopFile.ReadInt32LE();
			auto compressedLength = uopFile.ReadInt32LE();
			auto decompressedLength = uopFile.ReadInt32LE();
			auto hash = uopFile.ReadInt64LE();
			uopFile.ReadInt32LE();
			auto flag = uopFile.ReadInt16LE();

			if (offset == 0)
			{
				continue;
			}
			UOPMapaData dataStruct;
			dataStruct.offset = static_cast<int>(offset + headerLength);
			dataStruct.length = compressedLength;
			hashes[hash] = dataStruct;
		}

		uopFile.ResetPtr();
		uopFile.Move(static_cast<int>(nextBlock));
	} while (nextBlock != 0);



	unsigned long long hash;
	int fileNumber = -1;
	UOPMapaData uopDataStruct;
	IFOR(block, 0, maxBlockCount)
	{
		CIndexMap &index = list[block];
		int shifted = block >> 12;
		if (fileNumber != shifted)
		{
			fileNumber = shifted;
			char mapFilePath[200] = { 0 };
			sprintf_s(mapFilePath, "build/map%ilegacymul/%08i.dat", map, shifted);
			hash = COrion::CreateHash(mapFilePath);
			if (hashes.find(hash) != hashes.end())
			{
				uopDataStruct = hashes.at(hash);
			}
			else
			{
				LOG("False hash in uop map %i file.", map);
			}
		}

		uint realMapAddress = 0;
		uint realStaticAddress = 0;
		int realStaticCount = 0;
		int blockNumber = block & 4095;
		if (mapAddress != 0)
		{
			uint address = mapAddress + uopDataStruct.offset + (blockNumber * 196);

			if (address < endMapAddress)
				realMapAddress = address;
		}

		if (staticIdxAddress != 0 && staticAddress != 0)
		{
			PSTAIDX_BLOCK sidx = (PSTAIDX_BLOCK)(staticIdxAddress + (block * sizeof(STAIDX_BLOCK)));

			if (sidx->Size > 0 && sidx->Position != 0xFFFFFFFF && (uint)sidx < endStaticIdxAddress)
			{
				uint address = staticAddress + sidx->Position;

				if (address < endStaticAddress)
				{
					realStaticAddress = address;
					realStaticCount = sidx->Size / sizeof(STATICS_BLOCK);

					if (realStaticCount > 0)
					{
						if (realStaticCount > 1024)
							realStaticCount = 1024;
					}
				}
			}
		}

		if (!realStaticCount)
			realStaticCount = 0;

		index.OriginalMapAddress = realMapAddress;
		index.OriginalStaticAddress = realStaticAddress;
		index.OriginalStaticCount = realStaticCount;

		index.MapAddress = realMapAddress;
		index.StaticAddress = realStaticAddress;
		index.StaticCount = realStaticCount;
	}

}
