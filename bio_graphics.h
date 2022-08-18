//////////////////////////////////ANSI
#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS
//#define fwrite(buffer,size,leng,fp) {fwrite(buffer,size,leng,fp);fw++;}


#include <stdio.h>
#include<stdint.h>
//#include<Windows.h>
#ifndef MAX_PATH
#define MAX_PATH 260
#endif // !MAX_PATH
#include <stdlib.h>
//#include"./bionukg_optimize.h"
#ifndef bionukg_graphics
#define bionukg_graphics



typedef struct bmp_header {
	uint32_t width;//像素宽度
	uint32_t memwidth;//内存中一行的字节数
	uint32_t height;//像素高度
	uint16_t bitdepth;//位深度(24或32)
	uint32_t size;//总大小=数据大小+bmp_headerlength
	uint32_t datasize;//数据块大小
	void init(uint32_t wid, uint32_t hei, uint32_t bitdep)
	{
		width = wid;
		height = hei;
		bitdepth = bitdep;
		while (wid%4!=0)
		{
			wid++;
		}
		memwidth = wid * (bitdepth / 8);
		datasize = memwidth * height;
		size = datasize + 54;
	}
}bmphd;




/// <summary>
/// 右下角为原点，坐标系在第二象限，坑的一批
/// </summary>
class bmp
{

#define bmp_headerlength 54

public:
	uint32_t width;//像素宽度
	uint32_t memwidth;//内存中一行的字节数
	uint32_t height;//像素高度
	uint16_t bitdepth;//位深度(24或32)
	uint32_t size;//总大小
	uint32_t datasize;//数据块大小

	uint8_t* file = 0;//文件数据地址
	uint8_t* data = 0;//像素数据地址= file + bmp_headerlength;

	char path[MAX_PATH] = { 0 };//文件路径
	//以上为静态值
	//以下为动态值
	uint32_t x=0;//操作点横坐标（右下角原点）
	uint32_t y=0;//操作点纵坐标（右下角原点）
	uint32_t memoffset=0;//操作点内存坐标
	uint8_t rgb[4]={0};//兼容普通位图的单个像素的颜色RGB值存放


	//创建一个格式大小一模一样的新的空bmp
	bmp(bmp* formatted)
	{
		this->width = formatted->width;
		this->memwidth = formatted->memwidth;
		this->height = formatted->height;
		this->bitdepth = formatted->bitdepth;
		this->size = formatted->size;
		this->datasize = formatted->datasize;
		this->file= new uint8_t[size];
		this->data = this->file + bmp_headerlength;

	}
	//复制一个bmp
	bmp(bmp& copyed)
	{
		this->width = copyed.width;
		this->memwidth = copyed.memwidth;
		this->height = copyed.height;
		this->bitdepth = copyed.bitdepth;
		this->size = copyed.size;
		this->datasize = copyed.datasize;
		this->file = new uint8_t[this->size];
		this->data = this->file + bmp_headerlength;
		memcpy(this->file, copyed.file, this->size);
	}
	//利用bmp文件头结构创建空bmp
	bmp(bmphd hd)
	{
		width = hd.width;//
		memwidth = hd.memwidth;//内存中一行的字节数
		height = hd.height;//像素高度
		bitdepth = hd.bitdepth;//位深度(24或32)
		size = hd.size;//总大小
		datasize = hd.datasize;//数据块大小
		this->file = new uint8_t[size];
		this->data = this->file + bmp_headerlength;
	}
	//啥都没有初始化
	bmp(void) {};

//打开路径下的.bmp文件
	bmp(const char* newpath)
	{
		this->open(newpath);
	}
	~bmp()
	{
		close();
	}

	uint32_t close()
	{
		if (file != NULL)
		{
			delete[] file;
			file = NULL;
			data = NULL;
		}
	}

	//把文件newpath写入内存file(原内存未删除)
	int32_t// -1:路径不存在 -3:内存不足
		open(const char* newpath)
	{
		if (newpath == 0)
			return -1;
		if (path == 0)
			return -1;
		FILE* fp;
		strcpy_s(path, newpath);
		fopen_s(&fp, path, "rb");
		if (fp == 0)
		{
			return -2;
		}
		uint8_t headerdata[bmp_headerlength] = { 0 };
		fread(headerdata, 1, bmp_headerlength, fp);

		size = *(uint32_t*)&headerdata[2];
		width = *(uint32_t*)&headerdata[18];
		height = *(uint32_t*)&headerdata[22];
		bitdepth = *(uint16_t*)&headerdata[28];
		datasize = *(uint32_t*)&headerdata[34];
		file = new uint8_t[size];
		if (file == 0)
		{
			fclose(fp);
			return -3;
		}
		data = file + bmp_headerlength;
		fseek(fp, 0, SEEK_SET);
		fread(file, 1, size, fp);
		
		fclose(fp);
		memwidth = width * bitdepth/8;
		while (memwidth%4!=0)
		{
			memwidth++;
		}
		return 0;

	}
	//把文件写入硬盘
	uint32_t //-1:路径不存在；-2:
		save_as(
		const char* newpath//输入0时存入原路径，否则修改路径为输入路径
	)
	{
		if (newpath == 0)
			return -1;
		strcpy_s(path, newpath);					
		if (path == 0)
			return -1;
		FILE* fp;
		fopen_s(&fp, path, "wb+");
		if (fp == 0)
		{
			return -2;
		}
		uint8_t def[bmp_headerlength] = {
66,		77,//固定BM 0~1
0,		0,		0,		0,//文件大小size=datasize+bmp_headerlength 2~5
0,		0,		0,		0,//固定 6~9
54,		0,		0,		0,//固定文件开头长度 10~13
40,		0,		0,		0,//固定 14~17
0,		0,		0,		0,//宽width 18~21
0,		0,		0,		0,//高height 22~25
1,		0,//固定 26~27
24,		0,//24位图bitdepth24，固定 28~29
0,		0,		0,		0,//固定 30~33
0,		0,		0,		0,//像素数据大小datasize 34~37
0,		0,		0,		0,//固定 38~41
0,		0,		0,		0,//固定 42~45
0,		0,		0,		0,//固定 46~49
0,		0,		0,		0,//固定 50~53
	//默认文件开头
		};
		*(uint32_t*)&def[2] = size;
		*(uint32_t*)&def[18] = width;
		*(uint32_t*)&def[22] = height;
		*(uint16_t*)&def[28] = bitdepth;
		*(uint32_t*)&def[34] = datasize;
		fwrite(def, 1, bmp_headerlength, fp);
		fwrite(data, 1, datasize, fp);
		fclose(fp);
		return 0;

		//fopen_s(&fp, path, "wb+");
		//if (fp == 0)
		//{
		//	return -2;
		//}

		//*(uint32_t*)&def[2] = size;
		//*(uint32_t*)&def[18] = width;
		//*(uint32_t*)&def[22] = height;
		//*(uint16_t*)&def[28] = bitdepth;
		//*(uint32_t*)&def[34] = datasize;
		//fwrite(def, 1, bmp_headerlength, fp);
		//fwrite(data, 1, datasize, fp);
		//fclose(fp);
		//return 0;
	}

	///另存为png
	//参考文献：
	// [1]PNG文件格式白皮书 W3C https://blog.csdn.net/yongzhewuwei_2008/article/details/1007204 [EB/OL]2022/7/31
	// [2]svpng miloyip http://github.com/miloyip/svpng [EB/OL]2022/7/31
	int32_t//-1传入路径无效,-2文件打开失败，-3文件位深度不为24或32
		save_as_png(//参考：
			_In_ const char* newpath)
	{
#ifndef SVPNG_INC_
#ifdef bionukg_optimize
#define fwriteFp(ptr,len) fwrite_biopt(ptr,size,fp,buffer,idx)
#else // bionukg_optimize
#define fwriteFp(ptr,len) fwrite(ptr,1,len,fp)
#endif
#define fwrite32swap(u32p) {*(uint32_t*)u32p=_byteswap_ulong(*(uint32_t*)u32p);fwriteFp(u32p,4);*(uint32_t*)u32p=_byteswap_ulong(*(uint32_t*)u32p);}
#define fwriteFpCRC(ptr,len) {fwriteFp(ptr,len);for (i = 0; i < len; i++){crc^=((uint8_t*)ptr)[i];crc = (crc >> 4) ^ CRCtable[crc & 15];crc = (crc >> 4) ^ CRCtable[crc & 15];}}
#define fwrite32CRCswap(u32p) {*(uint32_t*)u32p=_byteswap_ulong(*(uint32_t*)u32p);fwriteFpCRC(u32p,4);*(uint32_t*)u32p=_byteswap_ulong(*(uint32_t*)u32p);}
#define fwrite8CRCadler(u8p) {fwriteFpCRC(u8p,1);ADLER_a = (ADLER_a + (*u8p)) % 0xfff1; ADLER_b = (ADLER_b + ADLER_a) % 0xfff1;}
#define fwriteChunkBegin(name32p,leng32p) {fwrite32swap(leng32p); crc = ~0U; fwriteFpCRC(name32p, 4);}
#define fwriteChunkEnd() {crc=~crc;fwrite32swap(&crc)};
		if (newpath == 0)
			return -1;
		bool alpha = false;

		if (this->bitdepth == 24)
			alpha = false;
		else if (this->bitdepth == 32)
			alpha = true;
		else
			return -3;
		FILE* fp;
		fopen_s(&fp,newpath, "wb+");
		if (fp == 0)
			return -2;
		uint8_t pngStart[] =
		{
			0x89,'P','N','G',
			'\r','\n',0x1A,'\n'
		};
		static const unsigned CRCtable[] = { // CRC校验表
			0, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
			0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
			0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
			0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
		unsigned ADLER_a = 1, ADLER_b = 0, crc;
		uint16_t MemWidthP = this->width * (alpha ? 4 : 3) + 1, x, y, i;
		uint32_t uint32;
		uint16_t uint16, idx = 0;
		uint8_t  uint8, buffer[256] = { 0 };

		fwriteFp(pngStart, 8);

		uint32 = 13;

		fwriteChunkBegin("IHDR", &uint32);
		fwrite32CRCswap(&(this->width));
		fwrite32CRCswap(&(this->height));
		uint8 = 8;
		fwriteFpCRC(&uint8, 1);
		uint8 = (alpha ? 6 : 2);
		fwriteFpCRC(&uint8, 1);
		uint32 = 0;
		fwriteFpCRC(&uint32, 3);
		fwriteChunkEnd();

		uint32 = 2 + this->height * (5 + MemWidthP) + 4;

		fwriteChunkBegin("IDAT", &uint32);
		uint16 = 0x0178;
		fwriteFpCRC(&uint16, 2);
		uint8_t* img = this->data;

		uint32 = ((~MemWidthP) << 16) | MemWidthP;
		MemWidthP -= 1;
		for (y = 0; y < this->height; y++) {
			uint8 = (y == height - 1) ? 1 : 0;
			fwriteFpCRC(&uint8, 1);
			fwriteFpCRC(&uint32, 4);
			uint8 = 0;
			fwriteFpCRC(&uint8, 1);
			fwriteFpCRC(img, MemWidthP);

			ADLER_a %= 0xfff1;
			ADLER_b = (ADLER_b + ADLER_a) % 0xfff1;
			for (x = 0; x < MemWidthP; x++, img++)
			{
				ADLER_a = (ADLER_a + (*img)) % 0xfff1;
				ADLER_b = (ADLER_b + ADLER_a) % 0xfff1;
			}

		}
		uint32 = (ADLER_b << 16) | ADLER_a;
		fwrite32CRCswap(&uint32);
		fwriteChunkEnd();

		uint32 = 0;

		fwriteChunkBegin("IEND", &uint32);
		fwriteChunkEnd();

		fclose(fp);
		//printf("2:%lld\n", fw);

		return 0;
		//#ifdef fwriteFp
		//#undef fwriteFp
		//#endif
#ifdef fwrite32swap
#undef fwrite32swap
#endif
#else
		if (newpath == 0)
			return -1;
		bool alpha = false;

		if (this->bitdepth == 24)
			alpha = false;
		else if (this->bitdepth == 32)
			alpha = true;
		else
			return -3;
		FILE* fp = fopen(newpath, "wb+");
		if (fp == 0)
			return -2;
		svpng(fp, this->width, this->height, this->data, alpha);
		fclose(fp);
#endif
	}
	//从坐标找内存
	//uint(& open)(const char*newpath) = memloadfile;
	//uint(bmp::* save)(const char*) = diskloadfile;

	void pointfind(void)
	{
		if (x > width)
			x = width;
		if (x < 0)
			x = 0;
		if (y > height)
			y = height;
		if (y < 0)
			y = 0;


		memoffset = y * memwidth + x * (bitdepth / 8) + bmp_headerlength;
		return;

	}
	uint32_t pointfind(uint32_t x, uint32_t y)
	{
		if (x > width)
			x = width;
		if (x < 0)
			x = 0;
		if (y > height)
			y = height;
		if (y < 0)
			y = 0;
		return y * memwidth + x * (bitdepth / 8) + bmp_headerlength;
		
	}
	void pointfind_r(void)
	{
		if (x > width)
			x = width;
		if (x < 0)
			x = 0;
		if (y > height)
			y = height;
		if (y < 0)
			y = 0;


		memoffset = (height-y-1) * memwidth + x * (bitdepth / 8) + bmp_headerlength;
		return;
	}
	uint32_t pointfind_r(uint32_t x, uint32_t y)
	{
		if (x > width)
			x = width;
		if (x < 0)
			x = 0;
		if (y > height)
			y = height;
		if (y < 0)
			y = 0;
		return (height - y-1) * memwidth + x * (bitdepth / 8) + bmp_headerlength;

	}

	void pointread(uint32_t x, uint32_t y,uint8_t*rgb)
	{
		memcpy(rgb, file + pointfind(x, y), bitdepth / 8);
	}
	void pointread(uint32_t x, uint32_t y)
	{
		memcpy(rgb, file + pointfind(x, y), bitdepth / 8);
	}
	void pointread(void)
	{
		pointfind();
		memcpy(rgb, file + memoffset, bitdepth / 8);
	}
	void pointread_r(uint32_t x, uint32_t y, uint8_t* rgb)
	{
		memcpy(rgb, file + pointfind_r(x, y), bitdepth / 8);
	}
	void pointread_r(uint32_t x, uint32_t y)
	{
		memcpy(rgb, file + pointfind_r(x, y), bitdepth / 8);
	}
	void pointread_r(void)
	{
		pointfind_r();
		memcpy(rgb, file + memoffset, bitdepth / 8);
	}
	void pointwrite(uint32_t x, uint32_t y, uint8_t* rgb)
	{
		memcpy(file + pointfind(x, y), rgb, bitdepth / 8);
	}
	void pointwrite(uint32_t x, uint32_t y)
	{
		memcpy(file + pointfind(x, y), rgb, bitdepth / 8);
	}
	void pointwrite(void)
	{
		pointfind();
		memcpy(file + memoffset, rgb, bitdepth / 8);
	}
	void pointwrite_r(uint32_t x, uint32_t y, uint8_t* rgb)
	{
		memcpy(file + pointfind_r(x, y), rgb, bitdepth / 8);
	}
	void pointwrite_r(uint32_t x, uint32_t y)
	{
		memcpy(file + pointfind_r(x, y), rgb, bitdepth / 8);
	}
	void pointwrite_r(void)
	{
		pointfind_r();
		memcpy(file + memoffset, rgb, bitdepth / 8);
	}

	//uint pixelfunc(uint32_t x,uint32_t y,void(*fun)(uint*))
	void traversalpoints(void (*func)(bmp*))
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				pointfind();
				memcpy(rgb, file + memoffset, bitdepth / 8);;
				func(this);
				memcpy(file + memoffset, rgb, bitdepth / 8);
			}
		}
	}
	void printtextfile(void)
	{
		//ix = 0;
		for (uint32_t i = 0; i < bmp_headerlength; i++)
		{

			printf("%3d,", file[i]);
			i++;
			printf("%3d,", file[i]);
			i++;
			printf("%3d\t", file[i]);
			//ix++;
			/*if (ix == 4)
			{
				ix = 0;
				printf("\n");
			}*/
		}
		//ix = 0;
		printf("\n");
		for (uint32_t i = bmp_headerlength; i < size; i++)
		{

			printf("%3d,", file[i]);
			i++;
			printf("%3d,", file[i]);
			i++;
			printf("%3d\t", file[i]);			
			/*if (ix == 4)
			{
				ix = 0;
				printf("\n");
			}*/
		}

	}
	void clear()
	{
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				memcpy(file + y * memwidth + x * (bitdepth / 8) + bmp_headerlength, "\0\0\0\0\0\0", bitdepth / 8);
			}
		}

	}
	/*void blockread(uint8_t*** block, uint wid, uint hei)
	{
		uint32_t x0 = x;
		uint32_t y0 = y;
		for (uint i = 0; i < hei; i++)
		{
			x++;
			for (uint j = 0; j < wid; j++)
			{
				y++;
				pointread();
				memcpy(block[i][j], rgb, bitdepth / 8);
			}
		}
		x = x0;
		y = y0;
		return;
	}*/
};
inline void bmp_mixup_pixel(bmp& base, bmp& add, uint32_t x, uint32_t y, float base_percentage)
{
	uint8_t rgbbase[4] = { 0 }, rgbadd[4] = { 0 };
	base.pointread(x, y, rgbbase);
	add.pointread(x, y, rgbadd);
	for (int i = 0; i < 4; i++)
	{
		rgbbase[i] = rgbbase[i] * base_percentage + rgbadd[i] * (1 - base_percentage);
	}
	base.pointwrite(x, y, rgbbase);
	return;
}
void bmp_mixup_block(bmp& base, bmp& add, uint32_t x, uint32_t y,uint32_t wid, uint32_t hei, float base_percentage)
{
	uint32_t xw = x + wid, yh = y + hei;
	if (xw > base.width)
		xw = base.width;
	if (yh > base.height)
		yh = base.height;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	for (int i = x; i < xw; i++)
	{
		for (int j = y; j < yh; j++)
		{
			bmp_mixup_pixel(base, add, i, j, base_percentage);
		}
	}
	return;
}

//color in rgb[3]
bmp* bmp_mixup_color(bmp& base)
{
	bmphd bhd;
	bhd.init(base.width, base.height, 24);
	bmp* ret = new bmp(bhd);
	uint8_t color[4] = { 0 };
	for (int i = 0; i < 3; i++)
	{
		color[i] = base.rgb[i];
	}
	for (int i = 0; i < base.width; i++)
	{
		for (int j = 0; j < base.height; j++)
		{
			base.pointread(i, j);
			for (int p = 0; p < 3; p++) {
				ret->rgb[p] = (float)base.rgb[p] * (255 - base.rgb[3]) / 255 + color[p] * base.rgb[3] / 255;
			}
			ret->pointwrite(i, j);
		}
	}
	return ret;

}


#endif // !bionukg_graphics