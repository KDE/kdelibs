#ifndef KERAMIK_IMAGE_H
#define KERAMIK_IMAGE_H

struct KeramikEmbedImage
{
	int width;
	int height;
	int id;
	unsigned char* data;
};

extern KeramikEmbedImage* KeramikGetDbImage(int id);
extern void KeramikDbCleanup();

enum KeramikTileType
{
	KeramikTileTL = 0,
	KeramikTileTC = 1,
	KeramikTileTR = 2,
	KeramikTileCL = 3,
	KeramikTileCC = 4,
	KeramikTileCR = 5,	
	KeramikTileRL = 6,
	KeramikTileRC = 7,
	KeramikTileRR = 8,
	KeramikTileSeparator = 16,
	KeramikSlider1 = 32,
	KeramikSlider2 = 48,
	KeramikSlider3 = 64,
	KeramikSlider4 = 80,
	KeramikGroove1 = 96,
	KeramikGroove2 = 112,
	KeramikSmallSlider1 = 128
};

#endif
